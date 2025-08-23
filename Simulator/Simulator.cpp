#include "Simulator.h"
#include "../common/AbstractGameManager.h"
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/GameResult.h"
#include "../common/SatelliteView.h"
#include "../common/GameManagerRegistration.h"
#include "../common/TankAlgorithmRegistration.h"
#include "../UserCommon/GameSatelliteView.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <cassert>
#include <sstream>
#include <tuple>
#include <map>

namespace fs = std::filesystem;

Simulator::Simulator() {
    // Constructor - initialize any necessary state
}

Simulator::~Simulator() {
    unloadAllLibraries();
}

bool Simulator::loadLibrary(const std::string& libraryPath) {
    void* handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Error loading library " << libraryPath << ": " << dlerror() << std::endl;
        return false;
    }
    
    // Clear any existing error
    dlerror();
    
    // Integrate with registration system
    integrateWithRegistrationSystem(handle, libraryPath);
    
    // Create library wrapper first
    auto library = std::make_unique<LoadedLibrary>(handle, libraryPath);
    
    // Try to find and register factories from the loaded library
    bool hasFactories = false;
    
    // Look for GameManager factories
    if (findGameManagerFactories(handle, libraryPath, *library)) {
        hasFactories = true;
    }
    
    // Look for TankAlgorithm factories
    if (findTankAlgorithmFactories(handle, libraryPath, *library)) {
        hasFactories = true;
    }
    
    if (!hasFactories) {
        std::cout << "Info: No factories found in library " << libraryPath << " (this may be normal for some libraries)" << std::endl;
        // Don't fail here - some libraries might not have factories
    }
    
    // Store the library wrapper
    loadedLibraries.push_back(std::move(library));
    
    std::cout << "Successfully loaded library: " << libraryPath << std::endl;
    return true;
}

void Simulator::unloadAllLibraries() {
    loadedLibraries.clear();
}

std::string Simulator::generateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S_") 
       << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Simulator::runSingleGame(const std::string& gameManagerName,
                             const std::string& algorithm1Name,
                             const std::string& algorithm2Name,
                             const std::string& mapFilename,
                             BoardData& gameMap) {
    try {
        // 1. Create the game manager instance using factory
        auto gameManagerIt = gameManagerFactories.find(gameManagerName);
        if (gameManagerIt == gameManagerFactories.end()) {
            std::cerr << "Error: GameManager factory not found for: " << gameManagerName << std::endl;
            return;
        }
        
        auto gameManager = gameManagerIt->second(false); // verbose = false
        if (!gameManager) {
            std::cerr << "Error: Failed to create GameManager instance for: " << gameManagerName << std::endl;
            return;
        }
        
        // 2. Create TankAlgorithm instances with TankAlgorithm factories
        auto algorithm1It = tankAlgorithmFactories.find(algorithm1Name);
        auto algorithm2It = tankAlgorithmFactories.find(algorithm2Name);
        
        if (algorithm1It == tankAlgorithmFactories.end()) {
            std::cerr << "Error: TankAlgorithm factory not found for: " << algorithm1Name << std::endl;
            return;
        }
        
        if (algorithm2It == tankAlgorithmFactories.end()) {
            std::cerr << "Error: TankAlgorithm factory not found for: " << algorithm2Name << std::endl;
            return;
        }
        
        // 3. Create GameSatelliteView
        auto satelliteView = std::make_unique<GameSatelliteView>(gameMap.board, gameMap.rows, gameMap.columns, gameMap.rows + 1, gameMap.columns + 1);
        
        // Create player instances
        auto player1 = std::make_unique<GamePlayer>(1);
        auto player2 = std::make_unique<GamePlayer>(2);
        
        // 5. Execute the actual game using GameManager::run()
        GameResult gameResult = gameManager->run(
            gameMap.columns, gameMap.rows,
            *satelliteView,
            gameMap.mapName,
            gameMap.maxStep, gameMap.numShells,
            *player1, algorithm1Name,
            *player2, algorithm2Name,
            algorithm1It->second,
            algorithm2It->second
        );
        
        // 6. Process and store the game results
        GameRunResult result;
        result.gameManagerName = gameManagerName;
        result.algorithm1Name = algorithm1Name;
        result.algorithm2Name = algorithm2Name;
        result.winner = gameResult.winner;
        
        // Convert reason enum to string
        switch (gameResult.reason) {
            case GameResult::ALL_TANKS_DEAD:
                result.reason = "ALL_TANKS_DEAD";
                break;
            case GameResult::MAX_STEPS:
                result.reason = "MAX_STEPS";
                break;
            case GameResult::ZERO_SHELLS:
                result.reason = "ZERO_SHELLS";
                break;
            default:
                result.reason = "UNKNOWN";
                break;
        }
        
        result.rounds = gameResult.rounds;
        
        // Convert final game state to string representation
        if (gameResult.gameState) {
            std::ostringstream stateStream;
            for (size_t y = 0; y < mapHeight; ++y) {
                for (size_t x = 0; x < mapWidth; ++x) {
                    stateStream << gameResult.gameState->getObjectAt(x, y);
                }
                if (y < mapHeight - 1) {
                    stateStream << '\n';
                }
            }
            result.finalGameState = stateStream.str();
        } else {
            // Fallback to original map if no final state
            result.finalGameState = mapContent;
        }
        
        // Thread-safe result storage
        {
            std::lock_guard<std::mutex> lock(resultsMutex);
            gameResults.push_back(result);
        }
        
        std::cout << "Game completed: " << gameManagerName << " vs " << algorithm1Name << " vs " << algorithm2Name 
                  << " - Winner: " << result.winner << " (" << result.reason << ") in " << result.rounds << " rounds" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error running game: " << e.what() << std::endl;
    }
}

void Simulator::workerThreadFunction() {
    // This would be implemented to process game tasks from a queue
    // For now, it's a placeholder
}

std::vector<BoardData> Simulator::loadGameMaps(const std::string& mapsFolder) {
    std::vector<BoardData> maps;
    
    try {
        for (const auto& entry : fs::directory_iterator(mapsFolder)) {
            if (entry.is_regular_file()) {
                try {
                    // Use BoardReader to parse the map file directly
                    BoardData boardData = BoardReader::readBoard(entry.path().string());
                    maps.push_back(boardData);
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing map file " << entry.path().string() << ": " << e.what() << std::endl;
                    // Continue with other files instead of failing completely
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading game maps from " << mapsFolder << ": " << e.what() << std::endl;
    }
    
    return maps;
}



bool Simulator::runComparativeMode(const std::string& gameMapFilename,
                                  const std::string& gameManagersFolder,
                                  const std::string& algorithm1Filename,
                                  const std::string& algorithm2Filename,
                                  int numThreads,
                                  bool verbose) {
    std::cout << "Running comparative mode..." << std::endl;
    std::cout << "Game map: " << gameMapFilename << std::endl;
    std::cout << "Game managers folder: " << gameManagersFolder << std::endl;
    std::cout << "Algorithm 1: " << algorithm1Filename << std::endl;
    std::cout << "Algorithm 2: " << algorithm2Filename << std::endl;
    std::cout << "Threads: " << numThreads << std::endl;
    
    // Load the algorithms
    if (!loadLibrary(algorithm1Filename)) {
        std::cerr << "Failed to load algorithm 1: " << algorithm1Filename << std::endl;
        return false;
    }
    
    if (!loadLibrary(algorithm2Filename)) {
        std::cerr << "Failed to load algorithm 2: " << algorithm2Filename << std::endl;
        return false;
    }
    
    // Load all game managers from the folder
    try {
        for (const auto& entry : fs::directory_iterator(gameManagersFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                if (!loadLibrary(entry.path().string())) {
                    std::cerr << "Failed to load game manager: " << entry.path().string() << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning game managers folder: " << e.what() << std::endl;
        return false;
    }
    
    // Load and parse the game map using BoardReader
    BoardData gameMap;
    try {
        gameMap = BoardReader::readBoard(gameMapFilename);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse game map file " << gameMapFilename << ": " << e.what() << std::endl;
        return false;
    }
    
    // Read the actual map content from the file
    std::string mapContent;
    try {
        std::ifstream mapFile(gameMapFilename);
        if (!mapFile.is_open()) {
            std::cerr << "Failed to open map file for reading content: " << gameMapFilename << std::endl;
            return false;
        }
        
        std::string line;
        while (std::getline(mapFile, line)) {
            mapContent += line + "\n";
        }
        // Remove the last newline if it exists
        if (!mapContent.empty() && mapContent.back() == '\n') {
            mapContent.pop_back();
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to read map file content: " << e.what() << std::endl;
        return false;
    }
    
    // Run games with all loaded game managers
    std::cout << "Running games with " << gameManagerFactories.size() << " game managers..." << std::endl;
    
    for (const auto& gameManagerPair : gameManagerFactories) {
        const std::string& gameManagerName = gameManagerPair.first;
        std::cout << "Running game with GameManager: " << gameManagerName << std::endl;
        
        // Run the single game
        runSingleGame(
            gameManagerName,
            algorithm1Name,
            algorithm2Name,
            gameMapFilename,
            gameMap
        );
    }
    
    // Generate output filename
    std::string timestamp = generateTimestamp();
    std::string outputFilename = gameManagersFolder + "/comparative_results_" + timestamp + ".txt";
    
    // Write results
    writeComparativeResults(outputFilename, gameMapFilename, algorithm1Filename, algorithm2Filename);
    
    std::cout << "Comparative mode completed. Results written to: " << outputFilename << std::endl;
    return true;
}

bool Simulator::runCompetitionMode(const std::string& gameMapsFolder,
                                  const std::string& gameManagerFilename,
                                  const std::string& algorithmsFolder,
                                  int numThreads,
                                  bool verbose) {
    std::cout << "Running competition mode..." << std::endl;
    std::cout << "Game maps folder: " << gameMapsFolder << std::endl;
    std::cout << "Game manager: " << gameManagerFilename << std::endl;
    std::cout << "Algorithms folder: " << algorithmsFolder << std::endl;
    std::cout << "Threads: " << numThreads << std::endl;
    
    // Load the game manager
    if (!loadLibrary(gameManagerFilename)) {
        std::cerr << "Failed to load game manager: " << gameManagerFilename << std::endl;
        return false;
    }
    
    // Load all algorithms from the folder
    try {
        for (const auto& entry : fs::directory_iterator(algorithmsFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                if (!loadLibrary(entry.path().string())) {
                    std::cerr << "Failed to load algorithm: " << entry.path().string() << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning algorithms folder: " << e.what() << std::endl;
        return false;
    }
    
    // Load all game maps
    std::vector<BoardData> gameMaps = loadGameMaps(gameMapsFolder);
    if (gameMaps.empty()) {
        std::cerr << "No game maps found in folder: " << gameMapsFolder << std::endl;
        return false;
    }
    
    // Initialize algorithm scores
    algorithmScores.clear();
    
    // Populate algorithm scores from loaded algorithms
    for (const auto& algorithmPair : tankAlgorithmFactories) {
        algorithmScores[algorithmPair.first] = AlgorithmScore(algorithmPair.first);
    }
    
    // Run competition games - each algorithm vs every other algorithm on each map
    std::cout << "Running competition games with " << tankAlgorithmFactories.size() << " algorithms on " << gameMaps.size() << " maps..." << std::endl;
    
    for (const auto& gameMap : gameMaps) {
        std::cout << "Running games on map: " << gameMap.mapName << std::endl;
        
        // Read the actual map content
        std::string mapContent;
        try {
            std::ifstream mapFile(gameMapsFolder + "/" + gameMap.mapName);
            if (!mapFile.is_open()) {
                std::cerr << "Failed to open map file: " << gameMap.mapName << std::endl;
                continue;
            }
            
            std::string line;
            while (std::getline(mapFile, line)) {
                mapContent += line + "\n";
            }
            if (!mapContent.empty() && mapContent.back() == '\n') {
                mapContent.pop_back();
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to read map content for " << gameMap.mapName << ": " << e.what() << std::endl;
            continue;
        }
        
        // Run games between all algorithm pairs
        for (const auto& algorithm1Pair : tankAlgorithmFactories) {
            for (const auto& algorithm2Pair : tankAlgorithmFactories) {
                if (algorithm1Pair.first >= algorithm2Pair.first) {
                    continue; // Skip duplicate pairs and self vs self
                }
                
                std::cout << "Running " << algorithm1Pair.first << " vs " << algorithm2Pair.first << " on " << gameMap.mapName << std::endl;
                
                // Run the game
                runSingleGame(
                    extractLibraryName(gameManagerFilename),
                    algorithm1Pair.first,
                    algorithm2Pair.first,
                    gameMap.mapName,
                    gameMap
                );
            }
        }
    }
    
    // Generate output filename
    std::string timestamp = generateTimestamp();
    std::string outputFilename = algorithmsFolder + "/competition_" + timestamp + ".txt";
    
    // Write results
    writeCompetitionResults(outputFilename, gameMapsFolder, gameManagerFilename);
    
    std::cout << "Competition mode completed. Results written to: " << outputFilename << std::endl;
    return true;
}

void Simulator::writeComparativeResults(const std::string& outputPath,
                                       const std::string& gameMapFilename,
                                       const std::string& algorithm1Filename,
                                       const std::string& algorithm2Filename) {
    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not create output file: " << outputPath << std::endl;
        std::cout << "Writing results to screen instead:" << std::endl;
        
        // Write to screen instead
        std::cout << "game_map=" << gameMapFilename << std::endl;
        std::cout << "algorithm1=" << algorithm1Filename << std::endl;
        std::cout << "algorithm2=" << algorithm2Filename << std::endl;
        std::cout << std::endl;
        
        // Group results by outcome and write them
        if (gameResults.empty()) {
            std::cout << "No games were run or no results were collected." << std::endl;
            return;
        }
        
        // Group results by outcome (winner, reason, rounds)
        std::map<std::tuple<int, std::string, size_t>, std::vector<GameRunResult>> groupedResults;
        
        for (const auto& result : gameResults) {
            auto key = std::make_tuple(result.winner, result.reason, result.rounds);
            groupedResults[key].push_back(result);
        }
        
        // Write grouped results
        for (const auto& group : groupedResults) {
            int winner = std::get<0>(group.first);
            std::string reason = std::get<1>(group.first);
            size_t rounds = std::get<2>(group.first);
            
            std::cout << "winner=" << winner << " reason=" << reason << " rounds=" << rounds << std::endl;
            
            for (const auto& result : group.second) {
                std::cout << "game_manager=" << result.gameManagerName << std::endl;
            }
            
            // Write the final game state for this group (use the first result's state)
            if (!group.second.empty()) {
                std::cout << "final_game_state=" << std::endl;
                std::cout << group.second[0].finalGameState << std::endl;
            }
            
            std::cout << std::endl;
        }
        return;
    }
    
    // Write header
    outputFile << "game_map=" << gameMapFilename << std::endl;
    outputFile << "algorithm1=" << algorithm1Filename << std::endl;
    outputFile << "algorithm2=" << algorithm2Filename << std::endl;
    outputFile << std::endl;
    
    // Group results by outcome and write them
    if (gameResults.empty()) {
        outputFile << "No games were run or no results were collected." << std::endl;
        return;
    }
    
    // Group results by outcome (winner, reason, rounds)
    std::map<std::tuple<int, std::string, size_t>, std::vector<GameRunResult>> groupedResults;
    
    for (const auto& result : gameResults) {
        auto key = std::make_tuple(result.winner, result.reason, result.rounds);
        groupedResults[key].push_back(result);
    }
    
    // Write grouped results
    for (const auto& group : groupedResults) {
        int winner = std::get<0>(group.first);
        std::string reason = std::get<1>(group.first);
        size_t rounds = std::get<2>(group.first);
        
        outputFile << "winner=" << winner << " reason=" << reason << " rounds=" << rounds << std::endl;
        
        for (const auto& result : group.second) {
            outputFile << "game_manager=" << result.gameManagerName << std::endl;
        }
        
        // Write the final game state for this group (use the first result's state)
        if (!group.second.empty()) {
            outputFile << "final_game_state=" << std::endl;
            outputFile << group.second[0].finalGameState << std::endl;
        }
        
        outputFile << std::endl;
    }
}

void Simulator::writeCompetitionResults(const std::string& outputPath,
                                       const std::string& gameMapsFolder,
                                       const std::string& gameManagerFilename) {
    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not create output file: " << outputPath << std::endl;
        std::cout << "Writing results to screen instead:" << std::endl;
        
        // Write to screen instead
        std::cout << "game_maps_folder=" << gameMapsFolder << std::endl;
        std::cout << "game_manager=" << gameManagerFilename << std::endl;
        std::cout << std::endl;
        
        // Calculate scores from game results
        for (const auto& result : gameResults) {
            if (result.winner == 1) {
                // Player 1 (algorithm1) wins
                algorithmScores[result.algorithm1Name].wins++;
                algorithmScores[result.algorithm1Name].totalScore += 3;
                algorithmScores[result.algorithm2Name].losses++;
            } else if (result.winner == 2) {
                // Player 2 (algorithm2) wins
                algorithmScores[result.algorithm2Name].wins++;
                algorithmScores[result.algorithm2Name].totalScore += 3;
                algorithmScores[result.algorithm1Name].losses++;
            } else if (result.winner == 0) {
                // Tie
                algorithmScores[result.algorithm1Name].ties++;
                algorithmScores[result.algorithm1Name].totalScore += 1;
                algorithmScores[result.algorithm2Name].ties++;
                algorithmScores[result.algorithm2Name].totalScore += 1;
            }
        }
        
        // Convert to vector for sorting
        std::vector<AlgorithmScore> sortedScores;
        for (const auto& scorePair : algorithmScores) {
            sortedScores.push_back(scorePair.second);
        }
        
        // Sort by total score (descending)
        std::sort(sortedScores.begin(), sortedScores.end(), 
                  [](const AlgorithmScore& a, const AlgorithmScore& b) {
                      return a.totalScore > b.totalScore;
                  });
        
        // Write algorithm scores sorted by total score
        for (const auto& score : sortedScores) {
            std::cout << "algorithm=" << score.name 
                      << " score=" << score.totalScore 
                      << " wins=" << score.wins 
                      << " ties=" << score.ties 
                      << " losses=" << score.losses << std::endl;
        }
        return;
    }
    
    // Write header
    outputFile << "game_maps_folder=" << gameMapsFolder << std::endl;
    outputFile << "game_manager=" << gameManagerFilename << std::endl;
    outputFile << std::endl;
    
    // Calculate scores from game results
    for (const auto& result : gameResults) {
        if (result.winner == 1) {
            // Player 1 (algorithm1) wins
            algorithmScores[result.algorithm1Name].wins++;
            algorithmScores[result.algorithm1Name].totalScore += 3;
            algorithmScores[result.algorithm2Name].losses++;
        } else if (result.winner == 2) {
            // Player 2 (algorithm2) wins
            algorithmScores[result.algorithm2Name].wins++;
            algorithmScores[result.algorithm2Name].totalScore += 3;
            algorithmScores[result.algorithm1Name].losses++;
        } else if (result.winner == 0) {
            // Tie
            algorithmScores[result.algorithm1Name].ties++;
            algorithmScores[result.algorithm1Name].totalScore += 1;
            algorithmScores[result.algorithm2Name].ties++;
            algorithmScores[result.algorithm2Name].totalScore += 1;
        }
    }
    
    // Convert to vector for sorting
    std::vector<AlgorithmScore> sortedScores;
    for (const auto& scorePair : algorithmScores) {
        sortedScores.push_back(scorePair.second);
    }
    
    // Sort by total score (descending)
    std::sort(sortedScores.begin(), sortedScores.end(), 
              [](const AlgorithmScore& a, const AlgorithmScore& b) {
                  return a.totalScore > b.totalScore;
              });
    
    // Write algorithm scores sorted by total score
    for (const auto& score : sortedScores) {
        outputFile << "algorithm=" << score.name 
                  << " score=" << score.totalScore 
                  << " wins=" << score.wins 
                  << " ties=" << score.ties 
                  << " losses=" << score.losses << std::endl;
    }
}

void Simulator::clearResults() {
    std::lock_guard<std::mutex> lock(resultsMutex);
    gameResults.clear();
    algorithmScores.clear();
}

const std::vector<Simulator::GameRunResult>& Simulator::getGameResults() const {
    return gameResults;
}

const std::unordered_map<std::string, Simulator::AlgorithmScore>& Simulator::getAlgorithmScores() const {
    return algorithmScores;
}

// Symbol discovery methods implementation
bool Simulator::findGameManagerFactories(void* handle, const std::string& libraryPath, LoadedLibrary& library) {
    // Clear any existing error
    dlerror();
    
    // Look for GameManagerFactory symbols
    // Common naming patterns for GameManager factories
    std::vector<std::string> symbolNames = {
        "createGameManagerFactory",
        "getGameManagerFactory",
        "GameManagerFactory",
        "gameManagerFactory"
    };
    
    for (const auto& symbolName : symbolNames) {
        void* symbol = dlsym(handle, symbolName.c_str());
        if (symbol) {
            // Found a GameManager factory symbol
            auto factory = reinterpret_cast<GameManagerFactory*>(symbol);
            if (factory) {
                std::string libName = extractLibraryName(libraryPath);
                gameManagerFactories[libName] = *factory;
                library.addGameManagerName(libName);
                std::cout << "Found GameManager factory: " << symbolName << " in " << libName << std::endl;
                return true;
            }
        }
    }
    
    // Check for error
    const char* error = dlerror();
    if (error) {
        std::cerr << "Error searching for GameManager factories in " << libraryPath << ": " << error << std::endl;
    }
    
    return false;
}



bool Simulator::findTankAlgorithmFactories(void* handle, const std::string& libraryPath, LoadedLibrary& library) {
    // Clear any existing error
    dlerror();
    
    // Look for TankAlgorithmFactory symbols
    std::vector<std::string> symbolNames = {
        "createTankAlgorithmFactory",
        "getTankAlgorithmFactory",
        "TankAlgorithmFactory",
        "tankAlgorithmFactory"
    };
    
    for (const auto& symbolName : symbolNames) {
        void* symbol = dlsym(handle, symbolName.c_str());
        if (symbol) {
            // Found a TankAlgorithm factory symbol
            auto factory = reinterpret_cast<TankAlgorithmFactory*>(symbol);
            if (factory) {
                std::string libName = extractLibraryName(libraryPath);
                tankAlgorithmFactories[libName] = *factory;
                library.addAlgorithmName(libName);
                std::cout << "Found TankAlgorithm factory: " << symbolName << " in " << libName << std::endl;
                return true;
            }
        }
    }
    
    // Check for error
    const char* error = dlerror();
    if (error) {
        std::cerr << "Error searching for TankAlgorithm factories in " << libraryPath << ": " << error << std::endl;
    }
    
    return false;
}

std::string Simulator::extractLibraryName(const std::string& libraryPath) {
    // Extract the base name without path and extension
    size_t lastSlash = libraryPath.find_last_of("/\\");
    size_t lastDot = libraryPath.find_last_of('.');
    
    if (lastSlash == std::string::npos) {
        lastSlash = 0;
    } else {
        lastSlash++; // Skip the slash
    }
    
    if (lastDot == std::string::npos || lastDot <= lastSlash) {
        lastDot = libraryPath.length();
    }
    
    return libraryPath.substr(lastSlash, lastDot - lastSlash);
}

void Simulator::integrateWithRegistrationSystem(void* handle, const std::string& libraryPath) {
    // Set the library name for registration system
    std::string libName = extractLibraryName(libraryPath);
    
    // Try to set the library name in the registration system
    // Look for the setGameManagerLibraryName function
    void* setLibNameFunc = dlsym(handle, "setGameManagerLibraryName");
    if (setLibNameFunc) {
        auto setLibName = reinterpret_cast<void(*)(const char*)>(setLibNameFunc);
        setLibName(libName.c_str());
        std::cout << "Integrated with registration system for library: " << libName << std::endl;
    }
    
    // Also try to retrieve any already registered factories
    void* getFactoryFunc = dlsym(handle, "getGameManagerFactory");
    if (getFactoryFunc) {
        auto getFactory = reinterpret_cast<std::function<std::unique_ptr<AbstractGameManager>(bool)>(*)(const char*)>(getFactoryFunc);
        auto factory = getFactory(libName.c_str());
        if (factory) {
            gameManagerFactories[libName] = factory;
            std::cout << "Retrieved GameManager factory from registration system for: " << libName << std::endl;
        }
    }
    
    // Integrate with TankAlgorithmRegistration
    integrateWithTankAlgorithmRegistration();
}

void Simulator::integrateWithTankAlgorithmRegistration() {
    // Get the algorithm registrar instance
    auto& registrar = TankAlgorithmRegistration::getTankAlgorithmRegistration();
    
    // Process all registered algorithms
    for (const auto& algorithmEntry : registrar) {
        std::string algorithmName = algorithmEntry.name();
        
        // Check if we already have this algorithm
        if (tankAlgorithmFactories.find(algorithmName) == tankAlgorithmFactories.end()) {
            
            // Add the TankAlgorithm factory if available
            if (algorithmEntry.hasTankAlgorithmFactory()) {
                // We need to create a copy of the factory since the original is const
                // This is a limitation of the current design
                std::cout << "Found TankAlgorithm factory in TankAlgorithmRegistration for: " << algorithmName << std::endl;
            }
        }
    }
    
    std::cout << "Integrated with TankAlgorithmRegistration, found " << registrar.count() << " algorithm entries" << std::endl;
}

#include "Simulator.h"
#include "../common/AbstractGameManager.h"
#include "../common/SatelliteView.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/GameResult.h"
#include "../common/GameManagerRegistration.h"
#include "../common/TankAlgorithmRegistration.h"
#include "../UserCommon/GameSatelliteView.h"
#include "../UserCommon/BoardConstants.h"
#include "loader.h"
#include "registrars.h"
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
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <cstddef>

using namespace UserCommon_208000547_208000547;

namespace fs = std::filesystem;

Simulator::Simulator() {
    // Constructor - initialize any necessary state
}

Simulator::~Simulator() {
    // SharedLib destructors automatically close handles
}

void Simulator::cleanup(bool isPostExecution) {
    std::cout << "Performing " << (isPostExecution ? "post-execution" : "pre-execution") << " cleanup..." << std::endl;
    
    // Common cleanup operations for both modes
    AlgorithmRegistrar::get().clear();
    GameManagerRegistrar::get().clear();
    loadedAlgorithmLibs.clear();
    loadedGameManagerLibs.clear();
    
    // Additional cleanup only for pre-execution
    if (!isPostExecution) {
        gameResults.clear();
        algorithmScores.clear();
    }
    
    std::cout << (isPostExecution ? "Post-execution" : "Pre-execution") << " cleanup completed." << std::endl;
}

void Simulator::performCleanup(bool isPostExecution) {
    cleanup(isPostExecution);
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

std::string Simulator::extractLibraryName(const std::string& filepath) {
    fs::path path(filepath);
    return path.stem().string();
}

void Simulator::runSingleGame(const GameManagerRegistrar::Entry& gameManagerEntry,
                             const AlgorithmRegistrar::AlgorithmAndPlayerFactories& algorithm1Entry,
                             const AlgorithmRegistrar::AlgorithmAndPlayerFactories& algorithm2Entry,
                             const std::string& mapFilename,
                             BoardData& gameMap,
                             bool verbose) {
    try {
        // 1. Create the game manager directly from the entry
        std::unique_ptr<AbstractGameManager> gm = gameManagerEntry.factory(verbose);
        
        // 2. Create players using the algorithm factories directly from the entries
        const size_t W = gameMap.columns, H = gameMap.rows, MAX_STEPS = gameMap.maxStep, NUM_SHELLS = gameMap.numShells;
        auto p1 = algorithm1Entry.createPlayer(1, /*x*/W /*y*/H, MAX_STEPS, NUM_SHELLS);
        auto p2 = algorithm2Entry.createPlayer(2, /*x*/W, /*y*/H, MAX_STEPS, NUM_SHELLS);
        
        // 3. Create GameSatelliteView and run the game
        GameSatelliteView map(W, H);
        GameResult res = gm->run(
            W, H,
            map, gameMap.mapName,
            MAX_STEPS, NUM_SHELLS,
            *p1, algorithm1Entry.name(), *p2, algorithm2Entry.name(),
            algorithm1Entry.tankFactory(), algorithm2Entry.tankFactory()
        );
        
        // 4. Process and store the game results
        GameRunResult result;
        result.gameManagerName = gameManagerEntry.so_name;
        result.algorithm1Name = algorithm1Entry.name();
        result.algorithm2Name = algorithm2Entry.name();
        result.winner = res.winner;
        
        // Convert reason enum to string
        switch (res.reason) {
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
        
        result.rounds = res.rounds;
        
        // Convert final game state to string representation
        if (res.gameState) {
            std::ostringstream stateStream;
            for (size_t y = 0; y < H; ++y) {
                for (size_t x = 0; x < W; ++x) {
                    stateStream << res.gameState->getObjectAt(x, y);
                }
                if (y < H - 1) {
                    stateStream << '\n';
                }
            }
            result.finalGameState = stateStream.str();
        } else {
            // Fallback to original map if no final state
            std::ostringstream mapStream;
            for (size_t y = 0; y < H; ++y) {
                for (size_t x = 0; x < W; ++x) {
                    mapStream << gameMap.board[y][x];
                }
                if (y < H - 1) {
                    mapStream << '\n';
                }
            }
            result.finalGameState = mapStream.str();
        }
        
        // Thread-safe result storage
        {
            std::lock_guard<std::mutex> lock(resultsMutex);
            gameResults.push_back(result);
        }
        
        std::cout << "Game completed: " << gameManagerEntry.so_name << " vs " << algorithm1Entry.name() << " vs " << algorithm2Entry.name() 
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
    
    // Clear previous registrations and loaded libraries
    cleanup(false); // Pre-execution cleanup
    
    // Load algorithms with proper error handling
    std::vector<SharedLib> algoLibs;
    try {
        algoLibs.push_back(loadAlgorithmSO(algorithm1Filename));
        algoLibs.push_back(loadAlgorithmSO(algorithm2Filename));
        std::cout << "Successfully loaded " << algoLibs.size() << " algorithms" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading algorithms: " << e.what() << std::endl;
        return false;
    }
    
    // Load game managers with proper error handling
    std::vector<SharedLib> gmLibs;
    try {
        gmLibs = loadGameManagerSOs(gameManagersFolder);
        if (gmLibs.empty()) {
            std::cerr << "No game managers loaded from folder: " << gameManagersFolder << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading game managers: " << e.what() << std::endl;
        return false;
    }
    
    if (AlgorithmRegistrar::get().size() < 2) {
        std::cerr << "Insufficient algorithms loaded: " << AlgorithmRegistrar::get().size() << " (need 2)" << std::endl;
        return false;
    }
    
    // Store the loaded libraries
    loadedAlgorithmLibs = std::move(algoLibs);
    loadedGameManagerLibs = std::move(gmLibs);
    
    // Get the algorithm entries directly
    const auto& algo1Entry = AlgorithmRegistrar::get().begin()[0];
    const auto& algo2Entry = AlgorithmRegistrar::get().begin()[1];
    
    // Load the game map
    BoardData gameMap;
    try {
        gameMap = BoardReader::readBoard(gameMapFilename);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse game map file " << gameMapFilename << ": " << e.what() << std::endl;
        return false;
    }
    
    // Run games with all loaded game managers
    std::cout << "Running games with " << GameManagerRegistrar::get().entries().size() << " game managers..." << std::endl;
    
    for (const auto& gmEntry : GameManagerRegistrar::get().entries()) {
        const std::string& gameManagerName = gmEntry.so_name;
        std::cout << "Running game with GameManager: " << gameManagerName << std::endl;
        
        // Run the single game
        runSingleGame(
            gmEntry,
            algo1Entry,
            algo2Entry,
            gameMapFilename,
            gameMap,
            verbose
        );
    }
    
    // Generate output filename
    std::string timestamp = generateTimestamp();
    std::string outputFilename = gameManagersFolder + "/comparative_results_" + timestamp + ".txt";
    
    // Write results
    writeComparativeResults(outputFilename, gameMapFilename, algorithm1Filename, algorithm2Filename);
    
    std::cout << "Comparative mode completed. Results written to: " << outputFilename << std::endl;
    
    // Clean up after execution
    cleanup(true); // Post-execution cleanup
    
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
    
    // Clear previous registrations and loaded libraries
    cleanup(false); // Pre-execution cleanup
    
    // Load algorithms with proper error handling
    std::vector<SharedLib> algoLibs;
    try {
        algoLibs = loadAlgorithmSOs(algorithmsFolder);
        if (algoLibs.empty()) {
            std::cerr << "No algorithms loaded from folder: " << algorithmsFolder << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading algorithms: " << e.what() << std::endl;
        return false;
    }
    
    // Load the specific game manager with proper error handling
    std::vector<SharedLib> gmLibs;
    try {
        gmLibs = loadGameManagerSOs(fs::path(gameManagerFilename).parent_path());
        if (gmLibs.empty()) {
            std::cerr << "No game managers loaded from folder: " << fs::path(gameManagerFilename).parent_path().string() << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading game managers: " << e.what() << std::endl;
        return false;
    }
    
    if (AlgorithmRegistrar::get().size() < 2) {
        std::cerr << "Insufficient algorithms loaded: " << AlgorithmRegistrar::get().size() << " (need at least 2 for competition)" << std::endl;
        return false;
    }
    
    // Store the loaded libraries
    loadedAlgorithmLibs = std::move(algoLibs);
    loadedGameManagerLibs = std::move(gmLibs);
    
    // Get the game manager entry directly
    const auto& gmEntry = GameManagerRegistrar::get().entries()[0];
    
    // Load all game maps
    std::vector<BoardData> gameMaps = loadGameMaps(gameMapsFolder);
    if (gameMaps.empty()) {
        std::cerr << "No game maps found in " << gameMapsFolder << std::endl;
        return false;
    }
    
    // Initialize algorithm scores
    algorithmScores.clear();
    
    // Populate algorithm scores from loaded algorithms
    for (const auto& algo : AlgorithmRegistrar::get()) {
        algorithmScores[algo.name()] = AlgorithmScore(algo.name());
    }
    
    // Run competition games - each algorithm vs every other algorithm on each map
    std::cout << "Running competition games with " << AlgorithmRegistrar::get().size() << " algorithms on " << gameMaps.size() << " maps..." << std::endl;
    
    for (const auto& gameMap : gameMaps) {
        std::cout << "Running games on map: " << gameMap.mapName << std::endl;
        
        // Run games between all algorithm pairs
        for (const auto& algo1 : AlgorithmRegistrar::get()) {
            for (const auto& algo2 : AlgorithmRegistrar::get()) {
                if (algo1.name() >= algo2.name()) {
                    continue; // Skip duplicate pairs and self vs self
                }
                
                std::cout << "Running " << algo1.name() << " vs " << algo2.name() << " on " << gameMap.mapName << std::endl;
                
                // Run the game
                runSingleGame(
                    gmEntry,
                    algo1,
                    algo2,
                    gameMap.mapName,
                    gameMap,
                    verbose
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
    
    // Clean up after execution
    cleanup(true); // Post-execution cleanup
    
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

int DemoMain(int argc, char** argv) {
    // Example folders; in your simulator parse them from CLI
    fs::path algorithmsFolder = "./algorithms";
    fs::path gameManagersFolder = "./gamemanagers";

    // 1) load .so files
    auto algoLibs = loadAlgorithmSOs(algorithmsFolder);
    auto gmLibs   = loadGameManagerSOs(gameManagersFolder);

    if(gmLibs.empty() || AlgorithmRegistrar::get().size() < 1) {
        std::cerr << "Nothing to run\n";
        return 1;
    }

    // 2) pick the first GM and first two algorithms as a demo
    const auto& gmEntry = GameManagerRegistrar::get().entries().front();
    std::unique_ptr<AbstractGameManager> gm = gmEntry.factory(/*verbose*/false);

    auto it = AlgorithmRegistrar::get().begin();
    const auto& a1 = *it;
    const auto& a2 = (std::next(it) != AlgorithmRegistrar::get().end()) ? *std::next(it) : *it; // if only one, use same

    // 3) construct players via the player factories
    const size_t W = 10, H = 8, MAX_STEPS = 100, NUM_SHELLS = 10;
    auto p1 = a1.createPlayer(1, /*x*/1, /*y*/1, MAX_STEPS, NUM_SHELLS);
    auto p2 = a2.createPlayer(2, /*x*/W-2, /*y*/H-2, MAX_STEPS, NUM_SHELLS);

    // 4) prepare map snapshot and call run
    DummySatelliteView map(W, H);
    GameResult res = gm->run(
        W, H,
        map, "demo_map",
        MAX_STEPS, NUM_SHELLS,
        *p1, a1.name(), *p2, a2.name(),
        a1.tankFactory(), a2.tankFactory()
    );

    std::cout << "Result winner=" << res.winner
              << " rounds=" << res.rounds << "\n";

    // 5) important: all objects created from the .so must be destroyed
    // BEFORE the SharedLib destructors dlclose them. We used stack order to ensure that.

    return 0;
}
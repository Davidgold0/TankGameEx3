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
#include "threadpool.h"
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
#include <thread>

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

std::string Simulator::formatGameResultMessage(int winner, const std::string& reason, size_t rounds) {
    if (winner == 0) {
        // Tie
        if (reason == "MAX_STEPS") {
            return "Tie, reached max steps = " + std::to_string(rounds);
        } else if (reason == "ZERO_SHELLS") {
            return "Tie, both players have zero shells for <40> steps";
        } else {
            return "Tie, both players have zero tanks";
        }
    } else {
        // Player won
        if (reason == "ALL_TANKS_DEAD") {
            return "Player " + std::to_string(winner) + " won with 0 tanks still alive";
        } else if (reason == "MAX_STEPS") {
            return "Player " + std::to_string(winner) + " won with more tanks alive";
        } else if (reason == "ZERO_SHELLS") {
            return "Player " + std::to_string(winner) + " won with more tanks alive";
        } else {
            return "Player " + std::to_string(winner) + " won";
        }
    }
}

void Simulator::runSingleGame(const GameManagerRegistrar::Entry& gameManagerEntry,
                             const AlgorithmRegistrar::AlgorithmAndPlayerFactories& algorithm1Entry,
                             const AlgorithmRegistrar::AlgorithmAndPlayerFactories& algorithm2Entry,
                             const std::string& mapFilename,
                             const BoardData& gameMap,
                             bool verbose) {
    try {
        std::cout << "[LOG] Step 1: Creating game manager..." << std::endl;
        // 1. Create the game manager directly from the entry
        std::unique_ptr<AbstractGameManager> gm = gameManagerEntry.factory(verbose);
        std::cout << "[LOG] Game manager created successfully" << std::endl;
        
        std::cout << "[LOG] Step 2: Creating players..." << std::endl;
        // 2. Create players using the algorithm factories directly from the entries
        const size_t W = gameMap.columns, H = gameMap.rows, MAX_STEPS = gameMap.maxStep, NUM_SHELLS = gameMap.numShells;
        auto p1 = algorithm1Entry.createPlayer(1, W, H, MAX_STEPS, NUM_SHELLS);
        std::cout << "[LOG] Player 1 created successfully" << std::endl;
        auto p2 = algorithm2Entry.createPlayer(2, W, H, MAX_STEPS, NUM_SHELLS);
        std::cout << "[LOG] Player 2 created successfully" << std::endl;
        
        std::cout << "[LOG] Step 3: Creating game view and starting game execution..." << std::endl;
        // 3. Create GameSatelliteView and run the game
        GameSatelliteView map(gameMap.board, W, H, W + 1, H + 1);
        std::cout << "[LOG] Game satellite view created with dimensions " << (W + 1) << "x" << (H + 1) << std::endl;
        
        std::cout << "[LOG] Starting game execution..." << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        GameResult res = gm->run(
            W, H,
            map, mapFilename,
            MAX_STEPS, NUM_SHELLS,
            *p1, algorithm1Entry.name(), *p2, algorithm2Entry.name(),
            algorithm1Entry.tankFactory(), algorithm2Entry.tankFactory()
        );
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "[LOG] Game execution completed in " << duration.count() << "ms" << std::endl;
        
        std::cout << "[LOG] Step 4: Processing game results..." << std::endl;
        // 4. Process and store the game results
        GameRunResult result;
        result.gameManagerName = gameManagerEntry.so_name;
        result.algorithm1Name = algorithm1Entry.name();
        result.algorithm2Name = algorithm2Entry.name();
        result.winner = res.winner;
        
        std::cout << "[LOG] Game winner: " << res.winner << std::endl;
        std::cout << "[LOG] Game rounds: " << res.rounds << std::endl;
        
        // Convert reason enum to string
        std::string reasonStr;
        switch (res.reason) {
            case GameResult::ALL_TANKS_DEAD:
                reasonStr = "ALL_TANKS_DEAD";
                break;
            case GameResult::MAX_STEPS:
                reasonStr = "MAX_STEPS";
                break;
            case GameResult::ZERO_SHELLS:
                reasonStr = "ZERO_SHELLS";
                break;
            default:
                reasonStr = "UNKNOWN";
                break;
        }
        result.reason = reasonStr;
        std::cout << "[LOG] Game end reason: " << reasonStr << std::endl;
        
        result.rounds = res.rounds;
        
        std::cout << "[LOG] Processing final game state..." << std::endl;
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
            std::cout << "[LOG] Final game state captured successfully" << std::endl;
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
            std::cout << "[LOG] Using original map as final state (no game state available)" << std::endl;
        }
        
        std::cout << "[LOG] Storing results in thread-safe manner..." << std::endl;
        // Thread-safe result storage
        {
            std::lock_guard<std::mutex> lock(resultsMutex);
            gameResults.push_back(result);
            std::cout << "[LOG] Results stored successfully. Total results count: " << gameResults.size() << std::endl;
        }
        
        std::cout << "[LOG] Game completed successfully: " << gameManagerEntry.so_name << " vs " << algorithm1Entry.name() << " vs " << algorithm2Entry.name() 
                  << " - Winner: " << result.winner << " (" << result.reason << ") in " << result.rounds << " rounds" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception occurred while running game: " << e.what() << std::endl;
        std::cerr << "[ERROR] Game Manager: " << gameManagerEntry.so_name << std::endl;
        std::cerr << "[ERROR] Algorithm 1: " << algorithm1Entry.name() << std::endl;
        std::cerr << "[ERROR] Algorithm 2: " << algorithm2Entry.name() << std::endl;
        std::cerr << "[ERROR] Map file: " << mapFilename << std::endl;
    }
    
    std::cout << "[LOG] Single game execution finished" << std::endl;
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
        // Check if both algorithms are the same file to avoid loading twice
        if (algorithm1Filename == algorithm2Filename) {
            algoLibs.resize(1);
            std::cout << "Both players using the same algorithm file: " << algorithm1Filename << std::endl;
            algoLibs[0] = (loadAlgorithmSO(algorithm1Filename));
            std::cout << "Successfully loaded 1 algorithm (shared between both players)" << std::endl;
        } else {
            algoLibs.resize(2);
            algoLibs[0] = (loadAlgorithmSO(algorithm1Filename));
            algoLibs[1] = (loadAlgorithmSO(algorithm2Filename));
            std::cout << "Successfully loaded " << algoLibs.size() << " algorithms" << std::endl;
        }
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
    
    // Store the loaded libraries
    loadedAlgorithmLibs = std::move(algoLibs);
    loadedGameManagerLibs = std::move(gmLibs);
    
    // Get the algorithm entries directly
    const auto& registrar = AlgorithmRegistrar::get();
    auto it1 = registrar.begin();
    
    // If both players use the same algorithm, both entries point to the same algorithm
    const auto& algo1Entry = *it1;
    const auto& algo2Entry = (algorithm1Filename == algorithm2Filename) ? 
        algo1Entry : *std::next(it1); // Both players use same algorithm or different ones
    
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
    
    size_t numGameManagers = GameManagerRegistrar::get().entries().size();
    
    // Helper function to run a single game (eliminates code duplication)
    auto runGame = [this, &algo1Entry, &algo2Entry, &gameMapFilename, &gameMap, verbose](
        const GameManagerRegistrar::Entry& gmEntry, const std::string& gameManagerName) {
        std::cout << "Running game with GameManager: " << gameManagerName << std::endl;
        
        runSingleGame(
            gmEntry,
            algo1Entry,
            algo2Entry,
            gameMapFilename,
            gameMap,
            verbose
        );
    };
    
    if (numThreads > 2 && numGameManagers > 1) {
        // Use multi-threading for better performance
        size_t actualThreads = std::min(static_cast<size_t>(numThreads), numGameManagers);
        std::cout << "Using " << actualThreads << " threads for parallel execution" << std::endl;
        
        // Create thread pool
        ThreadPool pool(actualThreads);
        
        // Submit all games to the thread pool
        for (const auto& gmEntry : GameManagerRegistrar::get().entries()) {
            const std::string& gameManagerName = gmEntry.so_name;
            std::cout << "Submitting game with GameManager: " << gameManagerName << " to thread pool" << std::endl;
            
            // Submit the game to run in a separate thread
            pool.submit([runGame, gmEntry, gameManagerName]() {
                std::cout << "Running game with GameManager: " << gameManagerName << " in thread " 
                          << std::this_thread::get_id() << std::endl;
                
                runGame(gmEntry, gameManagerName);
            });
        }
        
        // Wait for all games to complete
        std::cout << "Waiting for all games to complete..." << std::endl;
        pool.wait_idle();
        std::cout << "All games completed!" << std::endl;
        
    } else {
        // Single-threaded execution (original behavior)
        std::cout << "Using single-threaded execution" << std::endl;
        
        for (const auto& gmEntry : GameManagerRegistrar::get().entries()) {
            const std::string& gameManagerName = gmEntry.so_name;
            runGame(gmEntry, gameManagerName);
        }
    }
    
    // Write results
    writeComparativeOutput(gameManagersFolder, gameMapFilename, algorithm1Filename, algorithm2Filename);
    
    std::cout << "Comparative mode completed. Results written to game managers folder: " << gameManagersFolder << std::endl;
    
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
    
    // Run competition games with specific pairing logic
    std::cout << "Running competition games with " << AlgorithmRegistrar::get().size() << " algorithms on " << gameMaps.size() << " maps..." << std::endl;
    
    runCompetitionGames(gameMaps, gmEntry, numThreads, verbose);
    
    // Write results
    writeCompetitionResults(algorithmsFolder, gameMapsFolder, gameManagerFilename);
    
    std::cout << "Competition mode completed. Results written to algorithms folder: " << algorithmsFolder << std::endl;
    
    // Clean up after execution
    cleanup(true); // Post-execution cleanup
    
    return true;
}

void Simulator::runCompetitionGames(const std::vector<BoardData>& gameMaps,
                                    const GameManagerRegistrar::Entry& gameManagerEntry,
                                    int numThreads,
                                    bool verbose) {
    int N = AlgorithmRegistrar::get().size();
    
    // First, collect all games that need to be played
    std::vector<GameTask> allGames;
    int mapIndex = 0;
    
    for (const auto& gameMap : gameMaps) {
        // Calculate the offset for this map: k % (N-1)
        int offset = mapIndex % (N - 1);
        
        // For each algorithm, pair it with algorithm at position (i + 1 + offset) % N
        for (int i = 0; i < N; i++) {
            int opponentIndex = (i + 1 + offset) % N;
            
            // Skip if this would create a duplicate pair or self vs self
            if (i >= opponentIndex) {
                continue;
            }
            
            // Special case: if k = N/2 - 1 (and N is even), we might have duplicate pairings
            // Check if this pair was already played in a previous map
            if (N % 2 == 0 && offset == (N/2 - 1)) {
                // Check if this pair was already played in map 0
                if (mapIndex > 0) {
                    int map0Offset = 0;
                    int map0OpponentIndex = (i + 1 + map0Offset) % N;
                    if (map0OpponentIndex == opponentIndex) {
                        std::cout << "Skipping duplicate pair " << i << " vs " << opponentIndex << " on map " << mapIndex << std::endl;
                        continue;
                    }
                }
            }
            
            const auto& registrar = AlgorithmRegistrar::get();
            auto it1 = registrar.begin();
            std::advance(it1, i);
            auto it2 = registrar.begin();
            std::advance(it2, opponentIndex);
            const auto& algo1 = *it1;
            const auto& algo2 = *it2;
            
            allGames.push_back({algo1, algo2, gameMap, mapIndex, gameMap.mapName});
        }
        
        mapIndex++;
    }
    
    std::cout << "Collected " << allGames.size() << " games to play" << std::endl;
    
    // Helper function to run a single game
    auto runGame = [this, &gameManagerEntry, verbose](const GameTask& task) {
        std::cout << "Running " << task.algo1.name() << " vs " << task.algo2.name() 
                  << " on map " << task.mapIndex << " (" << task.mapName << ")" << std::endl;
        
        runSingleGame(
            gameManagerEntry,
            task.algo1,
            task.algo2,
            task.mapName,
            task.gameMap,
            verbose
        );
    };
    
    if (numThreads > 2 && allGames.size() > 1) {
        // Use multi-threading for better performance
        size_t actualThreads = std::min(static_cast<size_t>(numThreads), allGames.size());
        std::cout << "Using " << actualThreads << " threads for parallel execution" << std::endl;
        
        // Create thread pool
        ThreadPool pool(actualThreads);
        
        // Submit all games to the thread pool
        for (const auto& game : allGames) {
            pool.submit([runGame, game]() {
                runGame(game);
            });
        }
        
        // Wait for all games to complete
        std::cout << "Waiting for all " << allGames.size() << " games to complete..." << std::endl;
        pool.wait_idle();
        std::cout << "All games completed!" << std::endl;
        
    } else {
        // Single-threaded execution with progress tracking
        std::cout << "Using single-threaded execution" << std::endl;
        
        for (size_t i = 0; i < allGames.size(); ++i) {
            const auto& game = allGames[i];
            std::cout << "Progress: " << (i + 1) << "/" << allGames.size() << " - ";
            runGame(game);
        }
    }
}

void Simulator::writeComparativeOutput(const std::string& gameManagersFolder,
                                       const std::string& gameMapFilename,
                                       const std::string& algorithm1Filename,
                                       const std::string& algorithm2Filename) {
    // Generate output filename
    std::string timestamp = generateTimestamp();
    std::string outputPath = gameManagersFolder + "/comparative_results_" + timestamp + ".txt";
    
    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not create output file: " << outputPath << std::endl;
        return;
    }
    
    // Write header
    outputFile << "game_map=" << gameMapFilename << std::endl;
    outputFile << "algorithm1=" << extractLibraryName(algorithm1Filename) << std::endl;
    outputFile << "algorithm2=" << extractLibraryName(algorithm2Filename) << std::endl;
    outputFile << std::endl;
    
    // Group results by outcome and write them
    if (gameResults.empty()) {
        outputFile << "No games were run or no results were collected." << std::endl;
        return;
    }
    
    // Group results by exact same final result (winner, reason, rounds, final state)
    std::map<std::tuple<int, std::string, size_t, std::string>, std::vector<GameRunResult>> groupedResults;
    
    for (const auto& result : gameResults) {
        auto key = std::make_tuple(result.winner, result.reason, result.rounds, result.finalGameState);
        groupedResults[key].push_back(result);
    }
    
    // Write grouped results
    for (const auto& group : groupedResults) {
        int winner = std::get<0>(group.first);
        std::string reason = std::get<1>(group.first);
        size_t rounds = std::get<2>(group.first);
        
        // Write comma-separated list of game managers with identical results
        bool first = true;
        for (const auto& result : group.second) {
            if (!first) {
                outputFile << ",";
            }
            first = false;
            // Extract name without .so extension
            std::string name = extractLibraryName(result.gameManagerName);
            outputFile << name;
        }
        outputFile << std::endl;
        
        // Write game result message
        outputFile << formatGameResultMessage(winner, reason, rounds) << std::endl;
        
        // Write round number
        outputFile << rounds << std::endl;
        
        // Write final game state
        if (!group.second.empty()) {
            outputFile << group.second[0].finalGameState << std::endl;
        }
        
        outputFile << std::endl;
    }
}

void Simulator::writeCompetitionResults(const std::string& algorithmsFolder,
                                       const std::string& gameMapsFolder,
                                       const std::string& gameManagerFilename) {
    // Generate output filename
    std::string timestamp = generateTimestamp();
    std::string outputPath = algorithmsFolder + "/competition_" + timestamp + ".txt";
    
    std::ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not create output file: " << outputPath << std::endl;
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
    
    // Write algorithm scores sorted by total score (format: <algorithm name> <total score>)
    for (const auto& score : sortedScores) {
        outputFile << score.name << " " << score.totalScore << std::endl;
    }
}

void Simulator::clearResults() {
    std::lock_guard<std::mutex> lock(resultsMutex);
    gameResults.clear();
    algorithmScores.clear();
}

const std::vector<GameRunResult>& Simulator::getGameResults() const {
    return gameResults;
}

const std::unordered_map<std::string, AlgorithmScore>& Simulator::getAlgorithmScores() const {
    return algorithmScores;
}

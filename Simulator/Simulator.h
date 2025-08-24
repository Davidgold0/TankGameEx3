#pragma once

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <dlfcn.h>
#include "../UserCommon/BoardReader.h"
#include "loader.h"
#include "registrars.h"

using namespace UserCommon_208000547_208000547;

// Forward declarations
class AbstractGameManager;
class Player;
class TankAlgorithm;

// Type aliases for dynamic loading
using GameManagerFactory = std::function<std::unique_ptr<AbstractGameManager>(bool verbose)>;
using TankAlgorithmFactory = std::function<std::unique_ptr<TankAlgorithm>(int player_index, int tank_index)>;

// Structure for game results
struct GameRunResult {
    std::string gameManagerName;
    std::string algorithm1Name;
    std::string algorithm2Name;
    int winner; // 0 = tie, 1 = player1, 2 = player2
    std::string reason;
    size_t rounds;
    std::string finalGameState;
};

// Structure for competition results
struct AlgorithmScore {
    std::string name;
    int totalScore;
    int wins;
    int ties;
    int losses;
    
    AlgorithmScore(const std::string& n) : name(n), totalScore(0), wins(0), ties(0), losses(0) {}
};

class Simulator {
private:
    std::vector<SharedLib> loadedAlgorithmLibs;
    std::vector<SharedLib> loadedGameManagerLibs;
    
    std::vector<std::thread> workerThreads;
    std::mutex resultsMutex;
    std::condition_variable cv;
    
    // Results storage
    std::vector<GameRunResult> gameResults;
    std::unordered_map<std::string, AlgorithmScore> algorithmScores;
    
    std::string generateTimestamp();
    std::string extractLibraryName(const std::string& filepath);

    void runSingleGame(const GameManagerRegistrar::Entry& gameManagerEntry, 
                      const AlgorithmRegistrar::AlgorithmAndPlayerFactories& algorithm1Entry, 
                      const AlgorithmRegistrar::AlgorithmAndPlayerFactories& algorithm2Entry,
                      const std::string& mapFilename,
                      BoardData& gameMap,
                      bool verbose);
    
    // Competition game running logic
    void runCompetitionGames(const std::vector<BoardData>& gameMaps,
                            const GameManagerRegistrar::Entry& gameManagerEntry,
                            int numThreads,
                            bool verbose);
    
    // Thread worker methods
    void workerThreadFunction();
    
    // Output methods
    void writeComparativeResults(const std::string& outputPath, 
                               const std::string& gameMapFilename,
                               const std::string& algorithm1Filename,
                               const std::string& algorithm2Filename);
    void writeCompetitionResults(const std::string& outputPath,
                                const std::string& gameMapsFolder,
                                const std::string& gameManagerFilename);
    
    // Game map parsing - now using BoardData directly
    std::vector<BoardData> loadGameMaps(const std::string& mapsFolder);

    // Cleanup method for managing resources
    void cleanup(bool isPostExecution);

public:
    Simulator();
    ~Simulator();
    
    // Main execution methods
    bool runComparativeMode(const std::string& gameMapFilename,
                           const std::string& gameManagersFolder,
                           const std::string& algorithm1Filename,
                           const std::string& algorithm2Filename,
                           int numThreads = 1,
                           bool verbose = false);
    
    bool runCompetitionMode(const std::string& gameMapsFolder,
                           const std::string& gameManagerFilename,
                           const std::string& algorithmsFolder,
                           int numThreads = 1,
                           bool verbose = false);
    
    // Public cleanup method for external use
    void performCleanup(bool isPostExecution = false);
    
    // Utility methods
    void clearResults();
    const std::vector<GameRunResult>& getGameResults() const;
    const std::unordered_map<std::string, AlgorithmScore>& getAlgorithmScores() const;
};

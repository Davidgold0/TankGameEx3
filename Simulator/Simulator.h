#ifndef SIMULATOR_H
#define SIMULATOR_H

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
// Forward declarations
class AbstractGameManager;
class Player;
class TankAlgorithm;

// Type aliases for dynamic loading
using GameManagerFactory = std::function<std::unique_ptr<AbstractGameManager>(bool verbose)>;
using TankAlgorithmFactory = std::function<std::unique_ptr<TankAlgorithm>(int player_index, int tank_index)>;

// Structure to hold loaded libraries
struct LoadedLibrary {
    void* handle;
    std::string filename;
    std::vector<std::string> gameManagerNames;
    std::vector<std::string> algorithmNames;
    
    LoadedLibrary(void* h, const std::string& fname) : handle(h), filename(fname) {}
    ~LoadedLibrary() {
        if (handle) {
            dlclose(handle);
        }
    }
    
    // Helper methods to populate names
    void addGameManagerName(const std::string& name) {
        gameManagerNames.push_back(name);
    }
    
    void addAlgorithmName(const std::string& name) {
        algorithmNames.push_back(name);
    }
    
    bool hasGameManagers() const {
        return !gameManagerNames.empty();
    }
    
    bool hasAlgorithms() const {
        return !algorithmNames.empty();
    }
};

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
    std::vector<std::unique_ptr<LoadedLibrary>> loadedLibraries;
    std::unordered_map<std::string, GameManagerFactory> gameManagerFactories;
    std::unordered_map<std::string, TankAlgorithmFactory> tankAlgorithmFactories;
    
    // Threading support
    std::vector<std::thread> workerThreads;
    std::mutex resultsMutex;
    std::condition_variable cv;
    
    // Results storage
    std::vector<GameRunResult> gameResults;
    std::unordered_map<std::string, AlgorithmScore> algorithmScores;
    
    // Helper methods
    bool loadLibrary(const std::string& libraryPath);
    void unloadAllLibraries();
    std::string generateTimestamp();
    
    // Symbol discovery methods
    bool findGameManagerFactories(void* handle, const std::string& libraryPath, LoadedLibrary& library);
    bool findTankAlgorithmFactories(void* handle, const std::string& libraryPath, LoadedLibrary& library);
    std::string extractLibraryName(const std::string& libraryPath);
    
    // Registration system integration
    void integrateWithRegistrationSystem(void* handle, const std::string& libraryPath);
    void integrateWithTankAlgorithmRegistration();
    void runSingleGame(const std::string& gameManagerName, 
                      const std::string& algorithm1Name, 
                      const std::string& algorithm2Name,
                      const std::string& mapFilename,
                      BoardData& gameMap);
    
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
    
    // Utility methods
    void clearResults();
    const std::vector<GameRunResult>& getGameResults() const;
    const std::unordered_map<std::string, AlgorithmScore>& getAlgorithmScores() const;
};

#endif // SIMULATOR_H

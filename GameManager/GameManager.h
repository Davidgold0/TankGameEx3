#pragma once
#include <memory>
#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../UserCommon/BoardConstants.h"
#include "BoardReader.h"
#include "OutputWriter.h"
#include "TankInfo.h"
#include "Shell.h"
#include <map>
#include <utility>

using namespace std;

struct TankPosition {
    size_t x, y;
    int playerId;
    int tankIndex;
};

struct TankRoundInfo {
    ActionRequest action;
    bool wasActionIgnored;
    bool wasKilled;
};

class GameManager
{
private:
    BoardData gameData;
    unique_ptr<Player> playerOne;
    unique_ptr<Player> playerTwo;
    PlayerFactory& playerFactory;
    TankAlgorithmFactory& algorithmFactory;
    unique_ptr<OutputWriter> outputWriter;
    int creationOrderCounter;  // Added to track tank creation order across both players
    string inputFileName;  // Store the input filename
    
    // Store tank information for each player
    vector<TankInfo> player1Tanks;
    vector<TankInfo> player2Tanks;
    
    // Store active shells in the game
    vector<Shell> activeShells;

    // Store the board state at the start of each round
    vector<vector<char>> roundStartBoard;
    
    // Helper functions for game management
    bool checkImmediateGameEnd();
    void initializePlayersAndTanks();
    void runGameLoop();
    void logRound();  // Added to log round information for all tanks
    
    // Tank initialization helper functions
    vector<TankPosition> collectTankPositions();
    void sortTankPositions(vector<TankPosition>& positions);
    void createTanksFromPositions(const vector<TankPosition>& positions);

    // Game loop helper functions
    void moveShells();  // Move all active shells once
    void checkCollisions();  // Check for collisions between all game objects
    void updateTanks();   // Get and process tank actions
    void updateTankVector(vector<TankInfo>& tanks);  // Helper to update a vector of tanks
    void checkTankSwapping();  // Check for tanks that swapped places
    
    // Tank swapping helper functions
    map<pair<size_t, size_t>, TankInfo*> createTankPositionMap();  // Create map of current positions to tank pointers
    void handleTankSwap(TankInfo* tank1, TankInfo* tank2);  // Handle the case where two tanks swapped places
    bool tanksSwappedPlaces(TankInfo* tank1, TankInfo* tank2);  // Check if two tanks swapped places
    
    void processTankAction(TankInfo& tank, ActionRequest action);  // Process a single tank's action
    bool isValidTankAction(const TankInfo& tank, ActionRequest action) const;  // Validate if a tank action is legal
    void addShell(const TankInfo& tank);  // Create and add a new shell from a tank's position and direction
    char getNextCellState(char currentCell, const TankInfo& tank);  // Get the next cell state based on current cell and tank
    char getCurrentCellState(size_t x, size_t y);  // Get the current cell state after tank moves

    // Shell management
    void detectShellCrossings(vector<bool>& shellsToRemove, map<pair<size_t, size_t>, vector<size_t>>& nextPositions);
    void removeMarkedShells(const vector<bool>& shellsToRemove);
    std::vector<std::pair<size_t, size_t>> handleShellPositions(const map<pair<size_t, size_t>, vector<size_t>>& nextPositions);

    // Collision handling helpers
    void handleTankCollision(const pair<size_t, size_t>& pos);
    void handleWallCollision(const pair<size_t, size_t>& pos);
    void handleDamagedWallCollision(const pair<size_t, size_t>& pos);
    void handleMineCollision(const pair<size_t, size_t>& pos);
    void handleMultipleShellCollision(const pair<size_t, size_t>& pos);
    void findAndKillTank(size_t x, size_t y);
    void printBoard();  // Added to print the current board state

    bool allTanksOutOfShells;  // Track if all tanks have run out of shells
    int roundsSinceNoShells;   // Count rounds since all tanks ran out of shells

    bool checkAllTanksOutOfShells();  // Helper function to check if all tanks are out of shells

public:
    GameManager(PlayerFactory &player_factory, TankAlgorithmFactory &algorithmFactory);
    ~GameManager() {}
    void readBoard(string fileName);
    void setOutputFile();
    void run();
    
    // Added method to access game data
    const BoardData& getGameData() const { return gameData; }
};

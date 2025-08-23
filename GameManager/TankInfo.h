#pragma once
#include <memory>
#include "MovableObject.h"
#include "../common/TankAlgorithm.h"
#include "../common/ActionRequest.h"
#include "RoundInfo.h"
#include <optional>
#include <iostream>

class TankInfo : public MovableObject {
private:
    bool isAlive;
    int shootCooldown;  // Count of turns until next shoot is available
    std::unique_ptr<TankAlgorithm> algorithm;
    RoundInfo roundInfo;  // Added to store round information
    int playerId;  // Added to track which player owns this tank
    int creationOrder;  // Added to track the order of creation in the board
    int backwardMoveCounter;  // Counter for backward movement steps
    bool isMovingBackward;  // Flag to track if tank is in backward movement sequence
    int numShells;  // Number of shells the tank has

public:
    TankInfo(size_t x, size_t y, int dx, int dy, std::unique_ptr<TankAlgorithm> algo, size_t width, size_t height, int playerId, int creationOrder, int numShells);

    // Tank-specific getters
    bool getIsAlive() const;
    int getShootCooldown() const;
    TankAlgorithm* getAlgorithm();
    int getPlayerId() const { return playerId; }
    int getCreationOrder() const { return creationOrder; }
    bool getIsMovingBackward() const { return isMovingBackward; }
    int getBackwardMoveCounter() const { return backwardMoveCounter; }
    int getNumShells() const { return numShells; }
    void setNumShells(int shells) { numShells = shells; std::cout << "Tank " << creationOrder << " has " << numShells << " shells" << std::endl; }

    // RoundInfo getters and setters
    bool getRoundIsAlive() const { return roundInfo.isAlive; }
    ActionRequest getRoundAction() const { return roundInfo.action; }
    bool getRoundWasActionIgnored() const { return roundInfo.wasActionIgnored; }
    bool getRoundWasKilled() const { return roundInfo.wasKilled; }
    
    void setRoundIsAlive(bool value) { roundInfo.isAlive = value; }
    void setRoundAction(ActionRequest action) { roundInfo.action = action; }
    void setRoundWasActionIgnored(bool value) { roundInfo.wasActionIgnored = value; }
    void setRoundWasKilled(bool value) { roundInfo.wasKilled = value; }
    
    void clearRoundInfo() {
        roundInfo.action = ActionRequest::DoNothing;
        roundInfo.wasActionIgnored = false;
        roundInfo.wasKilled = false;
    }

    // Tank-specific actions
    void killTank();
    void rotate(ActionRequest action);
    void startShootCooldown();
    void updateShootCooldown();
    void startBackwardMove();
    void cancelBackwardMove();
    void updateBackwardMove();
    void resetRoundFlags() { 
        updateShootCooldown();       // Update cooldown
    }
    void beginRound() {
        clearRoundInfo();           // Clear round info
        updateShootCooldown();      // Update cooldown
        updateBackwardMove();       // Update backward movement state
        clearPreviousPosition();    // Clear previous position
    }
}; 
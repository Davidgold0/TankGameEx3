#include "TankInfo.h"
#include <iostream>

// All possible directions in the game:
// Cardinal directions (90 degrees):
// [1,0]   -> Right
// [0,1]   -> Down
// [-1,0]  -> Left
// [0,-1]  -> Up
//
// Diagonal directions (45 degrees):
// [1,1]   -> Down-Right
// [1,-1]  -> Up-Right
// [-1,1]  -> Down-Left
// [-1,-1] -> Up-Left
//
// Example of rotations:
// Starting at [1,0] (Right):
// - RotateRight45 -> [1,1] (Down-Right)
// - RotateRight45 -> [0,1] (Down)
// - RotateRight45 -> [-1,1] (Down-Left)
// - RotateRight45 -> [-1,0] (Left)
// - RotateRight45 -> [-1,-1] (Up-Left)
// - RotateRight45 -> [0,-1] (Up)
// - RotateRight45 -> [1,-1] (Up-Right)
// - RotateRight45 -> [1,0] (Right) - back to start
//
// Starting at [1,0] (Right):
// - RotateRight90 -> [0,1] (Down)
// - RotateRight90 -> [-1,0] (Left)
// - RotateRight90 -> [0,-1] (Up)
// - RotateRight90 -> [1,0] (Right) - back to start

TankInfo::TankInfo(size_t x, size_t y, int dx, int dy, std::unique_ptr<TankAlgorithm> algo, size_t width, size_t height, int playerId, int creationOrder, int numShells)
    : MovableObject(x, y, width, height), isAlive(true), shootCooldown(0),
      algorithm(std::move(algo)), playerId(playerId), creationOrder(creationOrder),
      backwardMoveCounter(0), isMovingBackward(false), numShells(numShells) {
    clearRoundInfo();
    roundInfo.isAlive = true;
    // Set the initial direction
    setDirection(dx, dy);
    std::cout << "Tank " << creationOrder << " has " << numShells << " shells" << std::endl;
}

// Tank-specific getters
bool TankInfo::getIsAlive() const { return isAlive; }
int TankInfo::getShootCooldown() const { return shootCooldown; }
TankAlgorithm* TankInfo::getAlgorithm() { return algorithm.get(); }

// Tank-specific actions
void TankInfo::killTank() { 
    isAlive = false; 
    roundInfo.isAlive = false;
    roundInfo.wasKilled = true;
}
void TankInfo::startShootCooldown() { shootCooldown = 4; }  // Assuming 2 turns cooldown
void TankInfo::updateShootCooldown() { if (shootCooldown > 0) shootCooldown--; }

void TankInfo::rotate(ActionRequest action) {
    switch (action) {
        case ActionRequest::RotateLeft90:
            rotate90(false);
            break;
        case ActionRequest::RotateRight90:
            rotate90(true);
            break;
        case ActionRequest::RotateLeft45:
            rotate45(false);
            break;
        case ActionRequest::RotateRight45:
            rotate45(true);
            break;
        default:
            break;
    }
}

void TankInfo::startBackwardMove() {
    if (!isMovingBackward) {
        isMovingBackward = true;
        backwardMoveCounter = 0;
    }
}

void TankInfo::cancelBackwardMove() {
    isMovingBackward = false;
    backwardMoveCounter = 0;
}

void TankInfo::updateBackwardMove() {
    if (isMovingBackward) {
        backwardMoveCounter++;
    }
}


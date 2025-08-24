#include "DefensiveTankAlgorithm.h"
#include "../UserCommon/SatelliteBattleInfo.h"
#include <cmath>
#include <iostream>
#include "../common/ActionRequest.h"
#include "../UserCommon/BoardConstants.h"

using namespace UserCommon_208000547_208000547;

DefensiveTankAlgorithm::DefensiveTankAlgorithm() 
    : boardWidth(0), boardHeight(0), turnCounter(0), tankX(-1), tankY(-1),
      dirX(0), dirY(0), directionInitialized(false)
{
    // Initialize defensive strategy
}

DefensiveTankAlgorithm::~DefensiveTankAlgorithm()
{
    // Cleanup if needed
}

bool DefensiveTankAlgorithm::shouldGetBattleInfo() const {
    // Get battle info on first turn (turnCounter == 0) and every 4 turns after
    return turnCounter == 0 || turnCounter % 4 == 0;
}

bool DefensiveTankAlgorithm::hasLineOfSight(int x1, int y1, int x2, int y2) const {
    // Check if there's a wall between two points
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    // If points are the same, there's line of sight
    if (dx == 0 && dy == 0) return true;
    
    // Check horizontal line
    if (dy == 0) {
        int step = (dx > 0) ? 1 : -1;
        for (int x = x1 + step; x != x2; x += step) {
            if (board[y1][x] == WALL) return false;  // Assuming '#' is wall
        }
        return true;
    }
    
    // Check vertical line
    if (dx == 0) {
        int step = (dy > 0) ? 1 : -1;
        for (int y = y1 + step; y != y2; y += step) {
            if (board[y][x1] == WALL) return false;
        }
        return true;
    }
    
    // For diagonal lines, check both horizontal and vertical paths
    return hasLineOfSight(x1, y1, x2, y1) && hasLineOfSight(x2, y1, x2, y2);
}

bool DefensiveTankAlgorithm::isShellNearby(int tankX, int tankY) const {
    // Check in a 5x5 area around the tank (±2 in each direction)
    for (int y = std::max(0, tankY - 2); y <= std::min(boardHeight - 1, tankY + 2); y++) {
        for (int x = std::max(0, tankX - 2); x <= std::min(boardWidth - 1, tankX + 2); x++) {
            if (board[y][x] == SHELL && hasLineOfSight(tankX, tankY, x, y)) {  // Assuming 'O' is shell
                return true;
            }
        }
    }
    return false;
}

bool DefensiveTankAlgorithm::isInDanger() const {
    // Check in 8 directions (horizontal, vertical, and diagonal)
    const int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},  // top-left, top, top-right
        {0, -1},           {0, 1},   // left, right
        {1, -1},  {1, 0},  {1, 1}    // bottom-left, bottom, bottom-right
    };

    for (const auto& dir : directions) {
        // Check up to 2 spaces in each direction
        for (int distance = 1; distance <= 2; distance++) {
            int checkY = (tankY + dir[0] * distance + boardHeight) % boardHeight;
            int checkX = (tankX + dir[1] * distance + boardWidth) % boardWidth;
            
            char cell = board[checkY][checkX];
            bool isShell = cell == SHELL;
            
            if (isShell) {
                return true;
            }
            
            // If we hit a wall, stop checking in this direction
            if (cell == WALL || cell == DAMAGED_WALL) {
                break;
            }
        }
    }
    return false;
}

void DefensiveTankAlgorithm::updateDirection(ActionRequest action) {
    int newDirX, newDirY;
    switch (action) {
        case ActionRequest::MoveForward:
        case ActionRequest::MoveBackward:
        case ActionRequest::Shoot:
        case ActionRequest::GetBattleInfo:
        case ActionRequest::DoNothing:
            // No direction change needed for these actions
            break;
        case ActionRequest::RotateLeft45:
            // Rotate 45 degrees left: (x,y) -> ((x-y)/√2, (x+y)/√2)
            // Since we only use -1,0,1, we can simplify to:
            if (dirX == 0) {
                dirX = -dirY;
                dirY = 0;
            } else if (dirY == 0) {
                dirY = dirX;
                dirX = 0;
            } else {
                // For diagonal directions, we need to handle both components
                newDirX = (dirX == dirY) ? -dirX : 0;
                newDirY = (dirX == dirY) ? 0 : dirX;
                dirX = newDirX;
                dirY = newDirY;
            }
            break;

        case ActionRequest::RotateRight45:
            // Rotate 45 degrees right: (x,y) -> ((x+y)/√2, (y-x)/√2)
            // Since we only use -1,0,1, we can simplify to:
            if (dirX == 0) {
                dirX = dirY;
                dirY = 0;
            } else if (dirY == 0) {
                dirY = -dirX;
                dirX = 0;
            } else {
                // For diagonal directions, we need to handle both components
                newDirX = (dirX == -dirY) ? 0 : dirY;
                newDirY = (dirX == -dirY) ? dirX : 0;
                dirX = newDirX;
                dirY = newDirY;
            }
            break;

        case ActionRequest::RotateLeft90:
            // Rotate 90 degrees left: (x,y) -> (y,-x)
            newDirX = dirY;
            newDirY = -dirX;
            dirX = newDirX;
            dirY = newDirY;
            break;

        case ActionRequest::RotateRight90:
            // Rotate 90 degrees right: (x,y) -> (-y,x)
            newDirX = -dirY;
            newDirY = dirX;
            dirX = newDirX;
            dirY = newDirY;
            break;
    }
}

void DefensiveTankAlgorithm::updateBattleInfo(BattleInfo& info)
{
    // Cast to SatelliteBattleInfo to access its methods
    SatelliteBattleInfo& satelliteInfo = static_cast<SatelliteBattleInfo&>(info);
    
    // Store board dimensions
    boardWidth = satelliteInfo.getColumns();
    boardHeight = satelliteInfo.getRows();
    
    // Get a copy of the board directly
    board = satelliteInfo.getBoard();
    
    // Update tank position
    tankX = satelliteInfo.getTankX();
    tankY = satelliteInfo.getTankY();

    // Update player index
    playerIndex = satelliteInfo.getPlayerIndex();

    // Initialize direction if not done yet
    if (!directionInitialized) {
        // Player 1 starts pointing left, Player 2 starts pointing right
        dirX = (playerIndex == 1) ? -1 : 1;
        dirY = 0;
        directionInitialized = true;
    }
}

ActionRequest DefensiveTankAlgorithm::wrapMoveForward() {
    // Calculate next position based on current direction
    int nextX = (tankX + dirX + boardWidth) % boardWidth;
    int nextY = (tankY + dirY + boardHeight) % boardHeight;
    
    // Update board: current position becomes empty, next position becomes tank
    board[tankY][tankX] = ' ';
    board[nextY][nextX] = REQUESTING_TANK;
    
    // Update tank position
    tankX = nextX;
    tankY = nextY;
    
    turnCounter++;
    return ActionRequest::MoveForward;
}

ActionRequest DefensiveTankAlgorithm::wrapRotateLeft45() {
    updateDirection(ActionRequest::RotateLeft45);
    turnCounter++;
    return ActionRequest::RotateLeft45;
}

ActionRequest DefensiveTankAlgorithm::wrapRotateRight45() {
    updateDirection(ActionRequest::RotateRight45);
    turnCounter++;
    return ActionRequest::RotateRight45;
}

ActionRequest DefensiveTankAlgorithm::wrapRotateLeft90() {
    updateDirection(ActionRequest::RotateLeft90);
    turnCounter++;
    return ActionRequest::RotateLeft90;
}

ActionRequest DefensiveTankAlgorithm::wrapRotateRight90() {
    updateDirection(ActionRequest::RotateRight90);
    turnCounter++;
    return ActionRequest::RotateRight90;
}

bool DefensiveTankAlgorithm::isAllyTankInDirection() const {
    // Check up to 2 spaces in the current direction
    for (int distance = 1; distance <= std::max(boardHeight, boardWidth); distance++) {
        int checkY = (tankY + dirY * distance + boardHeight) % boardHeight;
        int checkX = (tankX + dirX * distance + boardWidth) % boardWidth;
        
        char cell = board[checkY][checkX];
        std::cout << "Checking cell: " << checkX << ", " << checkY << " with value: " << cell << std::endl;
        // Check for ally tank based on player index
        if ((playerIndex == 1 && cell == PLAYER1_TANK) ||
            (playerIndex == 2 && cell == PLAYER2_TANK)) {
            return true;
        }
        
        // If we hit a wall, stop checking
        if (cell == WALL || cell == DAMAGED_WALL) {
            break;
        }
    }
    return false;
}

ActionRequest DefensiveTankAlgorithm::getAction()
{
    // Check if we should get battle info based on turn counter
    if (shouldGetBattleInfo()) {
        turnCounter++;
        return ActionRequest::GetBattleInfo;
    }

    // Check if we're in danger
    if (isInDanger()) {
        // Calculate next position based on current direction
        int nextX = (tankX + dirX + boardWidth) % boardWidth;
        int nextY = (tankY + dirY + boardHeight) % boardHeight;
        
        // Check if next cell is safe (not a mine or wall)
        char nextCell = board[nextY][nextX];
        if (nextCell != MINE && 
            nextCell != WALL && 
            nextCell != DAMAGED_WALL &&
            nextCell != SHELL) {
            return wrapMoveForward();
        }
        else {
            return wrapRotateRight90();
        }
    }
    
    // Check for shooting opportunity
    if (!isAllyTankInDirection()) {
        // If no ally tank in direction, try to shoot
        if (turnCounter >= nextShootTurn) {
            nextShootTurn = turnCounter + 5;
            turnCounter++;
            return ActionRequest::Shoot;
        }
        else {
            return wrapRotateRight45();
        }
    }
    
    // If no shooting opportunity or ally tank in way, rotate right 45 degrees
    return wrapRotateRight45();
} 
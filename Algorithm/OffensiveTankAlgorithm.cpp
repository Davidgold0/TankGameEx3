#include "OffensiveTankAlgorithm.h"
#include "PathFinder.h"
#include "../UserCommon/SatelliteBattleInfo.h"
#include "../common/ActionRequest.h"
#include "../UserCommon/BoardConstants.h"
#include "../common/TankAlgorithmRegistration.h"
#include <iostream>
#include <climits>
#include <limits>

using namespace UserCommon_208000547_208000547;

namespace Algorithm_208000547_208000547 {
OffensiveTankAlgorithm::OffensiveTankAlgorithm(int player_index, int tank_index) : boardWidth(0), boardHeight(0), turnCounter(0), tankX(-1), tankY(-1),
    dirX(0), dirY(0), directionInitialized(false), playerIndex(player_index), tankIndex(tank_index), currentMode(OperationsMode::Regular)
{
    // Initialize offensive strategy
}

OffensiveTankAlgorithm::~OffensiveTankAlgorithm()
{
    // Cleanup if needed
}

bool OffensiveTankAlgorithm::shouldGetBattleInfo() const {
    // Get battle info on first turn (turnCounter == 0) and every 4 turns after
    return turnCounter == 0 || turnCounter % 4 == 0;
}

void OffensiveTankAlgorithm::updateDirection(ActionRequest action) {
    const std::vector<std::pair<int, int>> directions = {
        {1, 0}, {1, 1}, {0, 1}, {-1, 1},
        {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
    };

    // Find current direction index
    int currentIndex = -1;
    for (int i = 0; i < 8; ++i) {
        if (directions[i].first == dirX && directions[i].second == dirY) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex == -1) {
        throw std::runtime_error("Invalid current direction");
    }

    int offset = 0;
    switch (action) {
        case ActionRequest::RotateLeft45:  offset = -1; break;
        case ActionRequest::RotateRight45: offset = +1; break;
        case ActionRequest::RotateLeft90:  offset = -2; break;
        case ActionRequest::RotateRight90: offset = +2; break;
        default: return; // no direction change
    }

    int newIndex = (currentIndex + offset + 8) % 8;
    dirX = directions[newIndex].first;
    dirY = directions[newIndex].second;
}


void OffensiveTankAlgorithm::updateBattleInfo(BattleInfo& info)
{
    std::cout << "OffensiveTank: Updating battle info" << std::endl;
    
    // Cast to SatelliteBattleInfo to access its methods
    SatelliteBattleInfo& satelliteInfo = static_cast<SatelliteBattleInfo&>(info);
    
    // Update the board first to ensure it's populated
    satelliteInfo.updateBoard();
    
    // Store board dimensions
    boardWidth = satelliteInfo.getColumns();
    boardHeight = satelliteInfo.getRows();
    std::cout << "OffensiveTank: Board dimensions - Width: " << boardWidth << ", Height: " << boardHeight << std::endl;
    
    // Get a copy of the board directly
    board = satelliteInfo.getBoard();
    
    // Update tank position
    tankX = satelliteInfo.getTankX();
    tankY = satelliteInfo.getTankY();
    std::cout << "OffensiveTank: Current position - X: " << tankX << ", Y: " << tankY << std::endl;

    // Update player index
    playerIndex = satelliteInfo.getPlayerIndex();
    std::cout << "OffensiveTank: Player index: " << playerIndex << std::endl;

    // Initialize direction if not done yet
    if (!directionInitialized) {
        // Player 1 starts pointing left, Player 2 starts pointing right
        dirX = (playerIndex == 1) ? -1 : 1;
        dirY = 0;
        directionInitialized = true;
        std::cout << "OffensiveTank: Initialized direction - dirX: " << dirX << ", dirY: " << dirY << std::endl;
    }

    // find path to closest enemy
    Point start = {tankX, tankY};
    std::vector<Point> closestPath;
    size_t minPathLength = std::numeric_limits<size_t>::max();

    // Determine enemy tank character based on player index
    char enemyTankChar = (playerIndex == 1) ? '2' : '1';  // Player 1 looks for '2', Player 2 looks for '1'
    std::cout << "OffensiveTank: Looking for enemy tank character: " << enemyTankChar << std::endl;

    // Find all enemy tanks on the board
    for (int y = 0; y < boardHeight; y++) {
        for (int x = 0; x < boardWidth; x++) {
            // Check if this is an enemy tank
            if (board[y][x] == enemyTankChar) {
                Point enemyPos = {x, y};
                std::cout << "OffensiveTank: Found enemy at position - X: " << x << ", Y: " << y << std::endl;
                // Find path to this enemy
                std::vector<Point> path = bfsPathfinder(board, start, enemyPos, false);
                
                // If we found a valid path and it's shorter than our current closest
                if (!path.empty() && path.size() < minPathLength) {
                    minPathLength = path.size();
                    closestPath = path;
                    std::cout << "OffensiveTank: Found new closest path with length: " << path.size() << std::endl;
                }
            }
        }
    }

    // Save the path to the closest enemy
    if (!closestPath.empty()) {
        pathToClosestEnemy = closestPath;
        std::cout << "OffensiveTank: Updated path to closest enemy with " << closestPath.size() << " steps" << std::endl;
    } else {
        std::cout << "OffensiveTank: No valid path found to any enemy" << std::endl;
    }
}

ActionRequest OffensiveTankAlgorithm::wrapMoveForward() {
    // Calculate next position based on current direction
    int nextX = (tankX + dirX + boardWidth) % boardWidth;
    int nextY = (tankY + dirY + boardHeight) % boardHeight;
    
    // Update board: current position becomes empty, next position becomes tank
    board[tankY][tankX] = ' ';
    board[nextY][nextX] = '%';
    
    // Update tank position
    tankX = nextX;
    tankY = nextY;
    
    turnCounter++;
    return ActionRequest::MoveForward;
}

ActionRequest OffensiveTankAlgorithm::wrapRotateLeft45() {
    updateDirection(ActionRequest::RotateLeft45);
    turnCounter++;
    return ActionRequest::RotateLeft45;
}

ActionRequest OffensiveTankAlgorithm::wrapRotateRight45() {
    updateDirection(ActionRequest::RotateRight45);
    turnCounter++;
    return ActionRequest::RotateRight45;
}

ActionRequest OffensiveTankAlgorithm::wrapRotateLeft90() {
    updateDirection(ActionRequest::RotateLeft90);
    turnCounter++;
    return ActionRequest::RotateLeft90;
}

ActionRequest OffensiveTankAlgorithm::wrapRotateRight90() {
    updateDirection(ActionRequest::RotateRight90);
    turnCounter++;
    return ActionRequest::RotateRight90;
}

ActionRequest OffensiveTankAlgorithm::wrapShoot() {
    turnCounter++;
    return ActionRequest::Shoot;
}

ActionRequest OffensiveTankAlgorithm::turnToAction(Turn t) {
    switch (t)
    {
    case Turn::RIGHT_90 :
    case Turn::RIGHT_135 :
    case Turn::COMPLETE_180 :
        return wrapRotateRight90();
    case Turn::RIGHT_45 :
        return wrapRotateRight45();
    case Turn::LEFT_90:
    case Turn::LEFT_135:
        return wrapRotateLeft90();
    case Turn::LEFT_45:
        return wrapRotateLeft45();
    default:
        return ActionRequest::DoNothing;
    }
}

ActionRequest OffensiveTankAlgorithm::getAction()
{
    // Check if we should get battle info based on turn counter
    if (shouldGetBattleInfo()) {
        turnCounter++;
        return ActionRequest::GetBattleInfo;
    }
    
    // If in panic mode, always shoot
    if (currentMode == OperationsMode::Panic) {
        return wrapShoot();
    }

    // If we have a path to the enemy, check if it's straight
    if (!pathToClosestEnemy.empty()) {
        if (isPathStraight(pathToClosestEnemy, boardHeight, boardWidth)) {
            // Calculate direction to enemy
            array<int,2> pathDir = calcDirection(pathToClosestEnemy, boardHeight, boardWidth);
            array<int,2> currentDir = {dirX, dirY};

            // If we're facing the enemy, shoot
            if (pathDir[0] == currentDir[0] && pathDir[1] == currentDir[1]) {
                return wrapShoot();
            }
            // Otherwise, turn to face the enemy
            else {
                Turn t = rotation(currentDir, pathDir);
                return turnToAction(t);
            }
        }
        // If path is not straight, follow it
        else {
            return followPath();
        }
    }
    turnCounter++;
    return ActionRequest::GetBattleInfo;
}

ActionRequest OffensiveTankAlgorithm::followPath() {
    std::cout << "OffensiveTank: Following path with " << pathToClosestEnemy.size() << " steps remaining" << std::endl;
    
    Point start = pathToClosestEnemy[0];
    Point next = pathToClosestEnemy[1];
    std::cout << "OffensiveTank: Current position - X: " << start.x << ", Y: " << start.y << std::endl;
    std::cout << "OffensiveTank: Next target - X: " << next.x << ", Y: " << next.y << std::endl;
    
    array<int,2> dir = directionBetweenPoints(start, next);
    array<int,2> currentDir = {dirX, dirY};
    std::cout << "OffensiveTank: Required direction - X: " << dir[0] << ", Y: " << dir[1] << std::endl;
    std::cout << "OffensiveTank: Current direction - X: " << currentDir[0] << ", Y: " << currentDir[1] << std::endl;

    // If we're facing the correct direction
    if (dir[0] == currentDir[0] && dir[1] == currentDir[1]) {
        std::cout << "OffensiveTank: Facing correct direction, checking next tile" << std::endl;
        // Check next tile
        int nextX = (tankX + dirX + boardWidth) % boardWidth;
        int nextY = (tankY + dirY + boardHeight) % boardHeight;
        char tile = board[nextY][nextX];
        std::cout << "OffensiveTank: Next tile at X: " << nextX << ", Y: " << nextY << " contains: '" << tile << "'" << std::endl;

        // If there's a wall, shoot
        if (tile == WALL || tile == DAMAGED_WALL) {
            std::cout << "OffensiveTank: Wall detected, shooting" << std::endl;
            return wrapShoot();
        }
        // If path is clear, move forward
        else if (tile == ' ') {
            std::cout << "OffensiveTank: Path clear, moving forward" << std::endl;
            pathToClosestEnemy.erase(pathToClosestEnemy.begin());
            return wrapMoveForward();
        }
    }
    // Otherwise, turn to face the correct direction
    else {
        std::cout << "OffensiveTank: Need to adjust direction" << std::endl;
        Turn t = rotation(currentDir, dir);
        std::cout << "OffensiveTank: Calculating turn: " << static_cast<int>(t) << std::endl;
        return turnToAction(t);
    }
    std::cout << "OffensiveTank: No valid action found, requesting battle info" << std::endl;
    turnCounter++;
    return ActionRequest::GetBattleInfo;
}
}

using TA_208000547_208000547 = Algorithm_208000547_208000547::OffensiveTankAlgorithm;
REGISTER_TANK_ALGORITHM(TA_208000547_208000547);
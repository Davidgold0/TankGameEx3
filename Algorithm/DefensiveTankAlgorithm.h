#pragma once

#include <cstddef>
#include <functional>
#include <vector>
#include <array>
#include <memory>

#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"

class DefensiveTankAlgorithm : public TankAlgorithm
{
public:
    DefensiveTankAlgorithm();
    ~DefensiveTankAlgorithm() override;
    
    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;

private:
    std::vector<std::vector<char>> board;
    int boardWidth;
    int boardHeight;
    int turnCounter;
    int tankX;
    int tankY;
    int playerIndex;
    int nextShootTurn;
    // Direction tracking
    int dirX;  // -1 for left, 1 for right
    int dirY;  // -1 for up, 1 for down
    bool directionInitialized;

    // Helper functions
    bool isShellNearby(int tankX, int tankY) const;
    bool hasLineOfSight(int x1, int y1, int x2, int y2) const;
    bool shouldGetBattleInfo() const;
    bool isInDanger() const;
    bool isAllyTankInDirection() const;
    void updateDirection(ActionRequest action);
    ActionRequest wrapMoveForward();
    ActionRequest wrapRotateLeft45();
    ActionRequest wrapRotateRight45();
    ActionRequest wrapRotateLeft90();
    ActionRequest wrapRotateRight90();
}; 
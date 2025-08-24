#pragma once
#include <cstddef>
#include <functional>
#include <vector>
#include <array>
#include <memory>

#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"

#include "PathFinder.h"
namespace Algorithm_208000547_208000547 {
    class OffensiveTankAlgorithm : public TankAlgorithm
    {
    public:
        enum class OperationsMode {
            Regular,
            Panic
        };

        OffensiveTankAlgorithm();
        ~OffensiveTankAlgorithm() override;
        
        ActionRequest getAction() override;
        void updateBattleInfo(BattleInfo& info) override;

    private:
        std::vector<std::vector<char>> board;
        int boardWidth;
        int boardHeight;
        int turnCounter;
        int tankX;
        int tankY;
        int dirX;
        int dirY;
        bool directionInitialized;
        int playerIndex;
        OperationsMode currentMode;
        std::vector<Point> pathToClosestEnemy;

        // Helper functions for movement and rotation
        bool shouldGetBattleInfo() const;
        ActionRequest wrapMoveForward();
        ActionRequest wrapRotateLeft45();
        ActionRequest wrapRotateRight45();
        ActionRequest wrapRotateLeft90();
        ActionRequest wrapRotateRight90();
        ActionRequest wrapShoot();
        ActionRequest turnToAction(Turn t);
        void updateDirection(ActionRequest action);
        ActionRequest followPath();
    }; 
}
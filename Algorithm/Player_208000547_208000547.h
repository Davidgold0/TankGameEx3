#pragma once

#include <cstddef>
#include <memory>
#include <functional>
#include "../common/BattleInfo.h"
#include "../common/SatelliteView.h"
#include "../common/ActionRequest.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../UserCommon/SatelliteBattleInfo.h"

namespace Algorithm_208000547_208000547 {
    using namespace UserCommon_208000547_208000547;
    
    class Player_208000547_208000547: public Player {
    private:
        int player_index;
        size_t x;
        size_t y;
        size_t max_steps;
        size_t num_shells;
    public:
        Player_208000547_208000547(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells) 
            : player_index(player_index), x(x), y(y), max_steps(max_steps), num_shells(num_shells) {}
        
        void updateTankWithBattleInfo
            (TankAlgorithm& tank, SatelliteView& satellite_view) override {
            SatelliteBattleInfo battle_info(&satellite_view, player_index);
            battle_info.updateBoard();
            tank.updateBattleInfo(battle_info);
        }
    };
    
}
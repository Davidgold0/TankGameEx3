#pragma once

#include <cstddef>
#include <memory>
#include <functional>
#include "../common/BattleInfo.h"
#include "../common/SatelliteView.h"
#include "../common/ActionRequest.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"

namespace Algorithm_208000547_208000547 {

    class Player_208000547_208000547: public Player {
    private:
        int player_index;
        size_t x;
        size_t y;
        size_t max_steps;
        size_t num_shells;
    public:
        Player_208000547_208000547(int, size_t, size_t, size_t, size_t) {}
        void updateTankWithBattleInfo
            (TankAlgorithm&, SatelliteView&) override {
        }
    };
    
}
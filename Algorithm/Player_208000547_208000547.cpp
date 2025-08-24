#include "Player_208000547_208000547.h"
#include "../common/PlayerRegistration.h"
#include "../UserCommon/SatelliteBattleInfo.h"
#include <iostream>

using namespace UserCommon_208000547_208000547;

namespace Algorithm_208000547_208000547{
Player_208000547_208000547::Player_208000547_208000547(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
    : player_index(player_index), x(x), y(y), max_steps(max_steps), num_shells(num_shells) {}

void Player_208000547_208000547::updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &satellite_view) {
    SatelliteBattleInfo battle_info(&satellite_view, player_index);
    battle_info.updateBoard();
    tank.updateBattleInfo(battle_info);
} 
}


using Player_208000547_208000547 = Algorithm_208000547_208000547::Player_208000547_208000547;
REGISTER_PLAYER(Player_208000547_208000547);
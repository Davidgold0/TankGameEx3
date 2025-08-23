#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/SatelliteView.h"
#include "../UserCommon/SatelliteBattleInfo.h"
#include <memory>

class GamePlayer : public Player {
private:
    int playerIndex;

public:
    GamePlayer(int player_index)
        : playerIndex(player_index) {}
    
    virtual ~GamePlayer() override {}
    
    virtual void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override {
        SatelliteBattleInfo battleInfo(&satellite_view, playerIndex);
        tank.updateBattleInfo(battleInfo);
    }
};

// Factory function to create a GamePlayer instance
std::unique_ptr<Player> createGamePlayer(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells) {
    return std::make_unique<GamePlayer>(player_index, x, y, max_steps, num_shells);
}

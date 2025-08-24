#include <string>
#include <cstddef>
#include <functional>
#include "../common/SatelliteView.h"
#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/GameResult.h"
#include "../common/AbstractGameManager.h"
#include "../common/GameManagerRegistration.h"
#include "registrars.h"
#include <memory>

// Implementation of GameManagerRegistration constructor
// This connects the common header declaration with the Simulator's registration system
GameManagerRegistration::GameManagerRegistration(GameManagerFactory f) {
    GameManagerRegistrar::get().setFactoryOnLast(std::move(f));
}

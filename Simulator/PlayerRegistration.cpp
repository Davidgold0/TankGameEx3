#include <string>
#include <cstddef>
#include <functional>
#include <memory>
#include "../common/SatelliteView.h"
#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/PlayerRegistration.h"
#include "registrars.h"
#include <memory>

// Implementation of PlayerRegistration constructor
// This connects the common header declaration with the Simulator's registration system
PlayerRegistration::PlayerRegistration(PlayerFactory f) {
    AlgorithmRegistrar::get().addPlayerFactory(std::move(f));
}

#include <string>
#include <cstddef>
#include <functional>
#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/TankAlgorithm.h"
#include "../common/TankAlgorithmRegistration.h"
#include "registrars.h"
#include <memory>

// Implementation of TankAlgorithmRegistration constructor
// This connects the common header declaration with the Simulator's registration system
TankAlgorithmRegistration::TankAlgorithmRegistration(TankAlgorithmFactory f) {
    AlgorithmRegistrar::get().addTankFactory(std::move(f));
}

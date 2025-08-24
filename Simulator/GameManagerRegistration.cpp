#include "../common/GameManagerRegistration.h"
#include "registrars.h"
#include <memory>

// Implementation of GameManagerRegistration constructor
// This connects the common header declaration with the Simulator's registration system
GameManagerRegistration::GameManagerRegistration(GameManagerFactory f) {
    GameManagerRegistrar::get().setFactoryOnLast(std::move(f));
}

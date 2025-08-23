#pragma once
#include "../common/ActionRequest.h"

struct RoundInfo {
    bool isAlive;
    ActionRequest action;
    bool wasActionIgnored;
    bool wasKilled;
}; 
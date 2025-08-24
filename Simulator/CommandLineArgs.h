#pragma once

#include <string>
#include <vector>

struct CommandLineArgs {
    bool comparative = false;
    bool competition = false;
    bool verbose = false;
    std::string gameMap;
    std::string gameManagersFolder;
    std::string gameManager;
    std::string algorithmsFolder;
    std::string algorithm1;
    std::string algorithm2;
    int numThreads = 1;
    
    std::vector<std::string> unsupportedArgs;
    std::vector<std::string> missingArgs;
};

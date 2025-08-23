#include "GameManagerRegistration.h"
#include "../common/AbstractGameManager.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>

// Global registry for GameManager factories
// This is a simple global registry that the Simulator can access
namespace {
    std::unordered_map<std::string, std::function<std::unique_ptr<AbstractGameManager>(bool)>> gameManagerRegistry;
    std::string currentLibraryName;
    
    void setCurrentLibraryName(const std::string& name) {
        currentLibraryName = name;
    }
}

// Function to set the current library name (called by Simulator)
extern "C" void setGameManagerLibraryName(const char* name) {
    if (name) {
        setCurrentLibraryName(name);
    }
}

// Function to get a GameManager factory by name
extern "C" std::function<std::unique_ptr<AbstractGameManager>(bool)> getGameManagerFactory(const char* name) {
    if (name && gameManagerRegistry.find(name) != gameManagerRegistry.end()) {
        return gameManagerRegistry[name];
    }
    return nullptr;
}

// Function to get all registered GameManager names
extern "C" const char** getGameManagerNames(int* count) {
    static std::vector<const char*> namePtrs;
    static std::vector<std::string> names;
    
    names.clear();
    namePtrs.clear();
    
    for (const auto& pair : gameManagerRegistry) {
        names.push_back(pair.first);
    }
    
    // Create pointers to the string data
    for (const auto& name : names) {
        namePtrs.push_back(name.c_str());
    }
    
    *count = static_cast<int>(names.size());
    return names.empty() ? nullptr : namePtrs.data();
}

GameManagerRegistration::GameManagerRegistration(std::function<std::unique_ptr<AbstractGameManager>()> factory) {
    if (!currentLibraryName.empty()) {
        // Convert the factory to the expected signature (bool verbose)
        auto convertedFactory = [factory](bool verbose) -> std::unique_ptr<AbstractGameManager> {
            return factory(); // Ignore verbose parameter as per header signature
        };
        
        // Register the converted factory with the current library name
        gameManagerRegistry[currentLibraryName] = convertedFactory;
        std::cout << "Registered GameManager factory for: " << currentLibraryName << std::endl;
    } else {
        std::cerr << "Warning: No library name set for GameManager registration" << std::endl;
    }
}

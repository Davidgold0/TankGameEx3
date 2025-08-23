#include "../common/TankAlgorithmRegistration.h"
#include "../common/TankAlgorithm.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>

// Global registry for TankAlgorithm factories
// This is a simple global registry that the Simulator can access
namespace {
    std::unordered_map<std::string, std::function<std::unique_ptr<TankAlgorithm>(int, int)>> tankAlgorithmRegistry;
    std::string currentLibraryName;
    
    void setCurrentLibraryName(const std::string& name) {
        currentLibraryName = name;
    }
}

// Function to set the current library name (called by Simulator)
extern "C" void setTankAlgorithmLibraryName(const char* name) {
    if (name) {
        setCurrentLibraryName(name);
    }
}

// Function to get a TankAlgorithm factory by name
extern "C" std::function<std::unique_ptr<TankAlgorithm>(int, int)> getTankAlgorithmFactory(const char* name) {
    if (name && tankAlgorithmRegistry.find(name) != tankAlgorithmRegistry.end()) {
        return tankAlgorithmRegistry[name];
    }
    return nullptr;
}

// Function to get all registered TankAlgorithm names
extern "C" const char** getTankAlgorithmNames(int* count) {
    static std::vector<const char*> namePtrs;
    static std::vector<std::string> names;
    
    names.clear();
    namePtrs.clear();
    
    for (const auto& pair : tankAlgorithmRegistry) {
        names.push_back(pair.first);
    }
    
    // Create pointers to the string data
    for (const auto& name : names) {
        namePtrs.push_back(name.c_str());
    }
    
    *count = static_cast<int>(names.size());
    return names.empty() ? nullptr : namePtrs.data();
}

TankAlgorithmRegistration::TankAlgorithmRegistration(TankAlgorithmFactory factory) {
    if (!currentLibraryName.empty()) {
        // Register the factory with the current library name
        tankAlgorithmRegistry[currentLibraryName] = factory;
        std::cout << "Registered TankAlgorithm factory for: " << currentLibraryName << std::endl;
    } else {
        std::cerr << "Warning: No library name set for TankAlgorithm registration" << std::endl;
    }
}
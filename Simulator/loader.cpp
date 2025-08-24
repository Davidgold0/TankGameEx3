#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include "registrars.h"
#include "loader.h"
namespace fs = std::filesystem;

// Destructor implementation for SharedLib
SharedLib::~SharedLib() { 
    if(handle) dlclose(handle); 
}

std::string soBaseName(const fs::path& p) {
    // "Algorithm_123.so" -> "Algorithm_123"
    return p.stem().string();
}

// Helper function to load a single shared library with common logic
template<typename RegistrarType>
SharedLib loadSingleSO(const fs::path& filePath, RegistrarType& registrar, const std::string& typeName) {
    if (!fs::is_regular_file(filePath) || filePath.extension() != ".so") {
        throw std::runtime_error("Invalid file path or not a .so file: " + filePath.string());
    }
    
    const std::string base = soBaseName(filePath);
    registrar.beginRegistration(base);
    
    SharedLib lib;
    lib.path = filePath.string();
    lib.handle = dlopen(lib.path.c_str(), RTLD_NOW | RTLD_LOCAL);
    
    if (!lib.handle) {
        std::string error = dlerror();
        registrar.removeLast();
        throw std::runtime_error("dlopen failed for " + lib.path + ": " + error);
    }
    
    try {
        registrar.validateLast();
        std::cout << "Loaded " << typeName << " so: " << base << "\n";
        return lib;
    } catch (const std::exception& ex) {
        std::cerr << "Bad " << typeName << " registration in " << base << " (" << ex.what() << ")\n";
        registrar.removeLast();
        dlclose(lib.handle);
        throw std::runtime_error("Bad " + typeName + " registration in " + base + ": " + ex.what());
    }
}

SharedLib loadAlgorithmSO(const fs::path& filePath) {
    return loadSingleSO(filePath, AlgorithmRegistrar::get(), "Algorithm");
}

SharedLib loadGameManagerSO(const fs::path& filePath) {
    return loadSingleSO(filePath, GameManagerRegistrar::get(), "GameManager");
}

std::vector<SharedLib> loadAlgorithmSOs(const fs::path& dir) {
    std::vector<SharedLib> handles;
    for(const auto& de: fs::directory_iterator(dir)) {
        if(!de.is_regular_file() || de.path().extension() != ".so") continue;
        try {
            handles.push_back(loadAlgorithmSO(de.path()));
        } catch(const std::exception& ex) {
            // Error already logged by loadAlgorithmSO
            continue;
        }
    }
    return handles;
}

std::vector<SharedLib> loadGameManagerSOs(const fs::path& dir) {
    std::vector<SharedLib> handles;
    for(const auto& de: fs::directory_iterator(dir)) {
        if(!de.is_regular_file() || de.path().extension() != ".so") continue;
        try {
            handles.push_back(loadGameManagerSO(de.path()));
        } catch(const std::exception& ex) {
            // Error already logged by loadGameManagerSO
            continue;
        }
    }
    return handles;
}
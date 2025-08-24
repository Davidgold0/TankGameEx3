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
    std::cout << "  loadSingleSO: Starting to load " << typeName << " from " << filePath.filename().string() << std::endl;
    
    if (!fs::is_regular_file(filePath) || filePath.extension() != ".so") {
        std::cerr << "  loadSingleSO: ERROR - Invalid file path or not a .so file: " << filePath.string() << std::endl;
        throw std::runtime_error("Invalid file path or not a .so file: " + filePath.string());
    }
    
    const std::string base = soBaseName(filePath);
    std::cout << "  loadSingleSO: Base name extracted: " << base << std::endl;
    
    std::cout << "  loadSingleSO: Beginning registration for " << base << std::endl;
    registrar.beginRegistration(base);
    
    SharedLib lib;
    lib.path = filePath.string();
    std::cout << "  loadSingleSO: Attempting dlopen with path: " << lib.path << std::endl;
    lib.handle = dlopen(lib.path.c_str(), RTLD_NOW | RTLD_LOCAL);
    
    if (!lib.handle) {
        std::string error = dlerror();
        std::cerr << "  loadSingleSO: ERROR - dlopen failed for " << lib.path << ": " << error << std::endl;
        std::cout << "  loadSingleSO: Removing last registration due to dlopen failure" << std::endl;
        registrar.removeLast();
        throw std::runtime_error("dlopen failed for " + lib.path + ": " + error);
    }
    
    std::cout << "  loadSingleSO: dlopen successful, handle: " << lib.handle << std::endl;
    
    try {
        std::cout << "  loadSingleSO: Validating registration for " << base << std::endl;
        registrar.validateLast();
        std::cout << "  loadSingleSO: Registration validation successful for " << base << std::endl;
        std::cout << "Loaded " << typeName << " so: " << base << "\n";
        return lib;
    } catch (const std::exception& ex) {
        std::cerr << "  loadSingleSO: ERROR - Bad " << typeName << " registration in " << base << " (" << ex.what() << ")" << std::endl;
        std::cout << "  loadSingleSO: Removing last registration due to validation failure" << std::endl;
        registrar.removeLast();
        std::cout << "  loadSingleSO: Closing handle due to validation failure" << std::endl;
        dlclose(lib.handle);
        throw std::runtime_error("Bad " + typeName + " registration in " + base + ": " + ex.what());
    }
}

SharedLib loadAlgorithmSO(const fs::path& filePath) {
    std::cout << "--- Loading single Algorithm SO: " << filePath.filename().string() << " ---" << std::endl;
    std::cout << "Full path: " << filePath.string() << std::endl;
    
    SharedLib result = loadSingleSO(filePath, AlgorithmRegistrar::get(), "Algorithm");
    
    std::cout << "--- Successfully loaded Algorithm: " << filePath.filename().string() << " ---" << std::endl;
    return result;
}

SharedLib loadGameManagerSO(const fs::path& filePath) {
    std::cout << "--- Loading single GameManager SO: " << filePath.filename().string() << " ---" << std::endl;
    std::cout << "Full path: " << filePath.string() << std::endl;
    
    SharedLib result = loadSingleSO(filePath, GameManagerRegistrar::get(), "GameManager");
    
    std::cout << "--- Successfully loaded GameManager: " << filePath.filename().string() << " ---" << std::endl;
    return result;
}

std::vector<SharedLib> loadAlgorithmSOs(const fs::path& dir) {
    std::cout << "=== Starting to load Algorithm SOs from directory: " << dir.string() << " ===" << std::endl;
    
    if (!fs::exists(dir)) {
        std::cerr << "ERROR: Directory does not exist: " << dir.string() << std::endl;
        return {};
    }
    
    if (!fs::is_directory(dir)) {
        std::cerr << "ERROR: Path is not a directory: " << dir.string() << std::endl;
        return {};
    }
    
    std::vector<SharedLib> handles;
    size_t filesFound = 0;
    size_t filesProcessed = 0;
    size_t filesLoaded = 0;
    
    std::cout << "Scanning directory for .so files..." << std::endl;
    
    for(const auto& de: fs::directory_iterator(dir)) {
        std::cout << "Found entry: " << de.path().string() << " (is_file: " << de.is_regular_file() 
                  << ", extension: " << de.path().extension().string() << ")" << std::endl;
        
        if(!de.is_regular_file() || de.path().extension() != ".so") {
            std::cout << "Skipping entry (not a regular .so file)" << std::endl;
            continue;
        }
        
        filesFound++;
        std::cout << "Processing .so file: " << de.path().filename().string() << std::endl;
        
        try {
            std::cout << "Attempting to load: " << de.path().string() << std::endl;
            handles.push_back(loadAlgorithmSO(de.path()));
            filesLoaded++;
            std::cout << "SUCCESS: Successfully loaded Algorithm: " << de.path().filename().string() << std::endl;
        } catch(const std::exception& ex) {
            std::cerr << "ERROR: Failed to load Algorithm " << de.path().filename().string() 
                      << ": " << ex.what() << std::endl;
            // Error already logged by loadAlgorithmSO
            continue;
        }
        
        filesProcessed++;
    }
    
    std::cout << "=== Algorithm SO loading summary ===" << std::endl;
    std::cout << "Directory: " << dir.string() << std::endl;
    std::cout << "Files found: " << filesFound << std::endl;
    std::cout << "Files processed: " << filesProcessed << std::endl;
    std::cout << "Files successfully loaded: " << filesLoaded << std::endl;
    std::cout << "Total handles returned: " << handles.size() << std::endl;
    std::cout << "===================================" << std::endl;
    
    return handles;
}

std::vector<SharedLib> loadGameManagerSOs(const fs::path& dir) {
    std::cout << "=== Starting to load GameManager SOs from directory: " << dir.string() << " ===" << std::endl;
    
    if (!fs::exists(dir)) {
        std::cerr << "ERROR: Directory does not exist: " << dir.string() << std::endl;
        return {};
    }
    
    if (!fs::is_directory(dir)) {
        std::cerr << "ERROR: Path is not a directory: " << dir.string() << std::endl;
        return {};
    }
    
    std::vector<SharedLib> handles;
    size_t filesFound = 0;
    size_t filesProcessed = 0;
    size_t filesLoaded = 0;
    
    std::cout << "Scanning directory for .so files..." << std::endl;
    
    for(const auto& de: fs::directory_iterator(dir)) {
        std::cout << "Found entry: " << de.path().string() << " (is_file: " << de.is_regular_file() 
                  << ", extension: " << de.path().extension().string() << ")" << std::endl;
        
        if(!de.is_regular_file() || de.path().extension() != ".so") {
            std::cout << "Skipping entry (not a regular .so file)" << std::endl;
            continue;
        }
        
        filesFound++;
        std::cout << "Processing .so file: " << de.path().filename().string() << std::endl;
        
        try {
            std::cout << "Attempting to load: " << de.path().string() << std::endl;
            handles.push_back(loadGameManagerSO(de.path()));
            filesLoaded++;
            std::cout << "SUCCESS: Successfully loaded GameManager: " << de.path().filename().string() << std::endl;
        } catch(const std::exception& ex) {
            std::cerr << "ERROR: Failed to load GameManager " << de.path().filename().string() 
                      << ": " << ex.what() << std::endl;
            // Error already logged by loadGameManagerSO
            continue;
        }
        
        filesProcessed++;
    }
    
    std::cout << "=== GameManager SO loading summary ===" << std::endl;
    std::cout << "Directory: " << dir.string() << std::endl;
    std::cout << "Files found: " << filesFound << std::endl;
    std::cout << "Files processed: " << filesProcessed << std::endl;
    std::cout << "Files successfully loaded: " << filesLoaded << std::endl;
    std::cout << "Total handles returned: " << handles.size() << std::endl;
    std::cout << "=====================================" << std::endl;
    
    return handles;
}
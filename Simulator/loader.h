#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// Structure to hold shared library information and handle
struct SharedLib {
    std::string path;
    void* handle = nullptr;
    
    // Destructor automatically closes the library handle
    ~SharedLib();
};

// Utility function to extract base name from shared library path
// "Algorithm_123.so" -> "Algorithm_123"
std::string soBaseName(const fs::path& p);

// Load all algorithm shared libraries from a directory
// Returns vector of SharedLib objects with loaded handles
std::vector<SharedLib> loadAlgorithmSOs(const fs::path& dir);

// Load all game manager shared libraries from a directory
// Returns vector of SharedLib objects with loaded handles
std::vector<SharedLib> loadGameManagerSOs(const fs::path& dir);

// Load a single algorithm shared library from a file path
// Returns SharedLib object with loaded handle, or throws on failure
SharedLib loadAlgorithmSO(const fs::path& filePath);

// Load a single game manager shared library from a file path
// Returns SharedLib object with loaded handle, or throws on failure
SharedLib loadGameManagerSO(const fs::path& filePath);

// Helper function to load a single shared library with common logic
// This is an internal implementation detail, not meant for external use
template<typename RegistrarType>
SharedLib loadSingleSO(const fs::path& filePath, RegistrarType& registrar, const std::string& typeName);
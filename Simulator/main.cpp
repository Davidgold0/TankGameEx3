#include "Simulator.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

// Command line argument parsing structure
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

void printUsage(const std::string& programName, const CommandLineArgs& args) {
    std::cout << "Usage:" << std::endl;
    
    if (args.comparative) {
        std::cout << "  " << programName << " -comparative game_map=<game_map_filename> "
                  << "game_managers_folder=<game_managers_folder> "
                  << "algorithm1=<algorithm_so_filename> "
                  << "algorithm2=<algorithm_so_filename> "
                  << "[num_threads=<num>] [-verbose]" << std::endl;
    } else if (args.competition) {
        std::cout << "  " << programName << " -competition game_maps_folder=<game_maps_folder> "
                  << "game_manager=<game_manager_so_filename> "
                  << "algorithms_folder=<algorithms_folder> "
                  << "[num_threads=<num>] [-verbose]" << std::endl;
    } else {
        std::cout << "  " << programName << " -comparative game_map=<game_map_filename> "
                  << "game_managers_folder=<game_managers_folder> "
                  << "algorithm1=<algorithm_so_filename> "
                  << "algorithm2=<algorithm_so_filename> "
                  << "[num_threads=<num>] [-verbose]" << std::endl;
        std::cout << "  " << programName << " -competition game_maps_folder=<game_maps_folder> "
                  << "game_manager=<game_manager_so_filename> "
                  << "algorithms_folder=<algorithms_folder> "
                  << "[num_threads=<num>] [-verbose]" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Note: All arguments can appear in any order." << std::endl;
    std::cout << "      The = sign can appear with any number of spaces around." << std::endl;
    std::cout << "      num_threads is optional (default: 1)." << std::endl;
}

void printErrors(const CommandLineArgs& args) {
    if (!args.unsupportedArgs.empty()) {
        std::cout << "Error: Unsupported command line arguments:" << std::endl;
        for (const auto& arg : args.unsupportedArgs) {
            std::cout << "  " << arg << std::endl;
        }
        std::cout << std::endl;
    }
    
    if (!args.missingArgs.empty()) {
        std::cout << "Error: Missing command line arguments:" << std::endl;
        for (const auto& arg : args.missingArgs) {
            std::cout << "  " << arg << std::endl;
        }
        std::cout << std::endl;
    }
}

CommandLineArgs parseCommandLine(int argc, char* argv[]) {
    CommandLineArgs args;
    std::vector<std::string> arguments(argv + 1, argv + argc);
    
    for (const auto& arg : arguments) {
        if (arg == "-comparative") {
            args.comparative = true;
        } else if (arg == "-competition") {
            args.competition = true;
        } else if (arg == "-verbose") {
            args.verbose = true;
        } else if (arg.find("game_map=") == 0) {
            args.gameMap = arg.substr(9); // Remove "game_map=" prefix
        } else if (arg.find("game_managers_folder=") == 0) {
            args.gameManagersFolder = arg.substr(22); // Remove "game_managers_folder=" prefix
        } else if (arg.find("game_manager=") == 0) {
            args.gameManager = arg.substr(13); // Remove "game_manager=" prefix
        } else if (arg.find("algorithms_folder=") == 0) {
            args.algorithmsFolder = arg.substr(18); // Remove "algorithms_folder=" prefix
        } else if (arg.find("algorithm1=") == 0) {
            args.algorithm1 = arg.substr(11); // Remove "algorithm1=" prefix
        } else if (arg.find("algorithm2=") == 0) {
            args.algorithm2 = arg.substr(11); // Remove "algorithm2=" prefix
        } else if (arg.find("num_threads=") == 0) {
            try {
                args.numThreads = std::stoi(arg.substr(12)); // Remove "num_threads=" prefix
                if (args.numThreads < 1) {
                    args.numThreads = 1;
                }
            } catch (const std::exception&) {
                args.numThreads = 1;
            }
        } else {
            args.unsupportedArgs.push_back(arg);
        }
    }
    
    // Validate mode selection
    if (!args.comparative && !args.competition) {
        args.missingArgs.push_back("mode (-comparative or -competition)");
    } else if (args.comparative && args.competition) {
        args.unsupportedArgs.push_back("both modes specified (use only one)");
    }
    
    // Validate comparative mode arguments
    if (args.comparative) {
        if (args.gameMap.empty()) args.missingArgs.push_back("game_map");
        if (args.gameManagersFolder.empty()) args.missingArgs.push_back("game_managers_folder");
        if (args.algorithm1.empty()) args.missingArgs.push_back("algorithm1");
        if (args.algorithm2.empty()) args.missingArgs.push_back("algorithm2");
    }
    
    // Validate competition mode arguments
    if (args.competition) {
        if (args.gameMapsFolder.empty()) args.missingArgs.push_back("game_maps_folder");
        if (args.gameManager.empty()) args.missingArgs.push_back("game_manager");
        if (args.algorithmsFolder.empty()) args.missingArgs.push_back("algorithms_folder");
    }
    
    return args;
}

bool validatePaths(const CommandLineArgs& args) {
    // Validate comparative mode paths
    if (args.comparative) {
        if (!fs::exists(args.gameMap)) {
            std::cout << "Error: Game map file does not exist: " << args.gameMap << std::endl;
            return false;
        }
        
        if (!fs::is_directory(args.gameManagersFolder)) {
            std::cout << "Error: Game managers folder does not exist or is not a directory: " 
                      << args.gameManagersFolder << std::endl;
            return false;
        }
        
        if (!fs::exists(args.algorithm1)) {
            std::cout << "Error: Algorithm 1 file does not exist: " << args.algorithm1 << std::endl;
            return false;
        }
        
        if (!fs::exists(args.algorithm2)) {
            std::cout << "Error: Algorithm 2 file does not exist: " << args.algorithm2 << std::endl;
            return false;
        }
    }
    
    // Validate competition mode paths
    if (args.competition) {
        if (!fs::is_directory(args.gameMapsFolder)) {
            std::cout << "Error: Game maps folder does not exist or is not a directory: " 
                      << args.gameMapsFolder << std::endl;
            return false;
        }
        
        if (!fs::exists(args.gameManager)) {
            std::cout << "Error: Game manager file does not exist: " << args.gameManager << std::endl;
            return false;
        }
        
        if (!fs::is_directory(args.algorithmsFolder)) {
            std::cout << "Error: Algorithms folder does not exist or is not a directory: " 
                      << args.algorithmsFolder << std::endl;
            return false;
        }
        
        // Check if algorithms folder has at least 2 algorithms
        int algorithmCount = 0;
        for (const auto& entry : fs::directory_iterator(args.algorithmsFolder)) {
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                algorithmCount++;
            }
        }
        
        if (algorithmCount < 2) {
            std::cout << "Error: Algorithms folder must contain at least 2 algorithm files (.so)" << std::endl;
            return false;
        }
        
        // Check if game maps folder has at least 1 map
        int mapCount = 0;
        for (const auto& entry : fs::directory_iterator(args.gameMapsFolder)) {
            if (entry.is_regular_file()) {
                mapCount++;
            }
        }
        
        if (mapCount == 0) {
            std::cout << "Error: Game maps folder must contain at least 1 map file" << std::endl;
            return false;
        }
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Error: No arguments provided" << std::endl;
        printUsage(argv[0], CommandLineArgs{});
        return 1;
    }
    
    // Parse command line arguments
    CommandLineArgs args = parseCommandLine(argc, argv);
    
    // Check for errors
    if (!args.unsupportedArgs.empty() || !args.missingArgs.empty()) {
        printErrors(args);
        printUsage(argv[0], args);
        return 1;
    }
    
    // Validate file/folder paths
    if (!validatePaths(args)) {
        return 1;
    }
    
    try {
        // Create simulator instance
        Simulator simulator;
        
        bool success = false;
        
        // Run appropriate mode
        if (args.comparative) {
            success = simulator.runComparativeMode(
                args.gameMap,
                args.gameManagersFolder,
                args.algorithm1,
                args.algorithm2,
                args.numThreads,
                args.verbose
            );
        } else if (args.competition) {
            success = simulator.runCompetitionMode(
                args.gameMapsFolder,
                args.gameManager,
                args.algorithmsFolder,
                args.numThreads,
                args.verbose
            );
        }
        
        if (success) {
            std::cout << "Simulation completed successfully." << std::endl;
            return 0;
        } else {
            std::cout << "Simulation failed." << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

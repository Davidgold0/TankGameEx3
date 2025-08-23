# Simulator Project - Tank Game Assignment 3

This is the Simulator component of the Tank Game assignment. The Simulator is responsible for running multiple Tank Games concurrently using multithreading in two modes: comparative and competitive.

## Features

- **Comparative Mode**: Run multiple GameManagers on a single game map with two algorithms
- **Competition Mode**: Run a single GameManager on multiple game maps with multiple algorithms in tournament-style competition
- **Multithreading Support**: Configurable number of threads for parallel game execution
- **Dynamic Library Loading**: Loads GameManager and Algorithm libraries (.so files) at runtime
- **Comprehensive Error Handling**: Validates command line arguments and file paths

## Building

### Prerequisites

- GCC/G++ compiler with C++17 support
- Make utility
- Linux/Unix environment (for .so file support)

### Build Commands

```bash
# Build the simulator
make

# Clean build artifacts
make clean

# Show available make targets
make help
```

### Target Executable

The build will create an executable named `simulator_<submitter_ids>` (e.g., `simulator_123456789_987654321`).

**Important**: Update the `TARGET` variable in the Makefile with your actual submitter IDs before building.

## Usage

### Command Line Syntax

#### Comparative Mode
```bash
./simulator_<submitter_ids> -comparative \
    game_map=<game_map_filename> \
    game_managers_folder=<game_managers_folder> \
    algorithm1=<algorithm_so_filename> \
    algorithm2=<algorithm_so_filename> \
    [num_threads=<num>] \
    [-verbose]
```

#### Competition Mode
```bash
./simulator_<submitter_ids> -competition \
    game_maps_folder=<game_maps_folder> \
    game_manager=<game_manager_so_filename> \
    algorithms_folder=<algorithms_folder> \
    [num_threads=<num>] \
    [-verbose]
```

### Parameters

- **Mode Selection**: Use `-comparative` or `-competition` (mutually exclusive)
- **File Paths**: All paths can have spaces around the `=` sign
- **num_threads**: Optional, defaults to 1 (single-threaded)
- **-verbose**: Optional flag for verbose output

### Examples

#### Comparative Mode Example
```bash
./simulator_123456789_987654321 -comparative \
    game_map=./maps/simple_map.txt \
    game_managers_folder=./game_managers \
    algorithm1=./algorithms/algorithm1.so \
    algorithm2=./algorithms/algorithm2.so \
    num_threads=4
```

#### Competition Mode Example
```bash
./simulator_123456789_987654321 -competition \
    game_maps_folder=./maps \
    game_manager=./game_managers/game_manager.so \
    algorithms_folder=./algorithms \
    num_threads=8 \
    -verbose
```

## Output Files

### Comparative Mode Output
- **Location**: Inside the `game_managers_folder`
- **Filename**: `comparative_results_<timestamp>.txt`
- **Content**: Game results grouped by outcome, including final game states

### Competition Mode Output
- **Location**: Inside the `algorithms_folder`
- **Filename**: `competition_<timestamp>.txt`
- **Content**: Algorithm scores sorted by total points (3 for win, 1 for tie, 0 for loss)

## Architecture

### Core Classes

- **Simulator**: Main class managing the simulation execution
- **AlgorithmRegistrar**: Manages algorithm and player factory registrations
- **LoadedLibrary**: Wrapper for dynamically loaded .so files

### Key Components

1. **Dynamic Library Loading**: Uses `dlopen`/`dlclose` for runtime library loading
2. **Factory Pattern**: Creates instances through factory functions from loaded libraries
3. **Threading Model**: Worker threads for parallel game execution
4. **Result Management**: Thread-safe result collection and output generation

## Error Handling

The simulator provides comprehensive error handling for:
- Invalid command line arguments
- Missing required parameters
- Non-existent files or directories
- Insufficient algorithms or game maps
- Library loading failures

## Threading Model

- **Single Thread**: `num_threads=1` or omitted (main thread only)
- **Multi-threaded**: `num_threads>=2` creates additional worker threads
- **Thread Safety**: Results are collected using mutex-protected data structures

## Dependencies

- **Standard C++17**: Uses modern C++ features
- **POSIX Threads**: For multithreading support
- **Dynamic Linking**: For .so file loading
- **Filesystem**: For directory traversal and file operations

## Development Notes

### Current Implementation Status

This is a **shell implementation** that provides the basic structure and command-line interface. The following areas need to be completed:

1. **Game Execution Logic**: Implement actual game running in `runSingleGame()`
2. **Library Symbol Discovery**: Enhance dynamic library loading to find and register factories
3. **Result Processing**: Implement proper result grouping and analysis
4. **Threading Implementation**: Complete the worker thread system
5. **Integration**: Connect with the registration system properly

### Next Steps

1. Implement the actual game execution logic
2. Complete the dynamic library symbol discovery
3. Implement proper result processing and output formatting
4. Add comprehensive testing
5. Optimize threading performance

## Troubleshooting

### Common Issues

1. **Library Loading Failures**: Check file permissions and library dependencies
2. **Compilation Errors**: Ensure C++17 support and all dependencies are available
3. **Runtime Errors**: Verify file paths and .so file compatibility

### Debug Mode

Build with debug information:
```bash
make clean
make CXXFLAGS="-std=c++17 -Wall -Wextra -O0 -g -DDEBUG"
```

## License

This project is part of the TAU Advanced Topics in Programming course assignment.

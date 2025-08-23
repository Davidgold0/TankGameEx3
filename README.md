# Tank Game Assignment 3 - Advanced Topics in Programming

This repository contains the implementation of Assignment 3 for the TAU Advanced Topics in Programming course. The project implements a Tank Game simulator that can run multiple games concurrently using multithreading.

## Project Structure

The project is organized into three main components, each built separately with its own makefile:

```
tankgame_ex3_v2/
├── Simulator/           # Simulator project (runs multiple games)
├── Algorithm/           # Algorithm project (Player and TankAlgorithm logic)
├── GameManager/         # GameManager project (runs single games)
├── common/              # Common header files (provided by course staff)
├── UserCommon/          # User-defined common files
├── Makefile             # Root makefile for building all projects
├── students.txt         # Submitter information
└── README.md            # This file
```

## Components

### 1. Simulator
- **Purpose**: Runs multiple Tank Games concurrently using multithreading
- **Modes**: Comparative and Competitive
- **Output**: Generates result files with game outcomes
- **Executable**: `simulator_<submitter_ids>`

### 2. Algorithm
- **Purpose**: Implements Player and TankAlgorithm logic
- **Output**: Shared library (`.so`) file
- **Naming**: `Algorithm_<submitter_ids>.so`
- **Namespace**: `Algorithm_<submitter_ids>`

### 3. GameManager
- **Purpose**: Manages and runs individual Tank Games
- **Output**: Shared library (`.so`) file
- **Naming**: `GameManager_<submitter_ids>.so`
- **Namespace**: `GameManager_<submitter_ids>`

## Building

### Prerequisites
- GCC/G++ compiler with C++17 support
- Make utility
- Linux/Unix environment (for .so file support)

### Build Commands

```bash
# Build all projects
make

# Build individual projects
make simulator
make algorithm
make game_manager

# Clean all projects
make clean

# Show available targets
make help
```

## Usage

### Simulator Modes

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

## Features

- **Multithreading**: Configurable number of threads for parallel execution
- **Dynamic Loading**: Loads GameManager and Algorithm libraries at runtime
- **Two Operation Modes**: Comparative and Competitive
- **Comprehensive Error Handling**: Validates inputs and provides clear error messages
- **Timestamped Output**: Generates unique output files with timestamps
- **Thread Safety**: Safe concurrent execution and result collection

## Implementation Status

### ✅ Completed
- Project structure and organization
- Command-line argument parsing and validation
- Basic simulator architecture
- Dynamic library loading framework
- Multithreading infrastructure
- Error handling and validation
- Build system (Makefiles)

### 🔄 In Progress / To Complete
- Game execution logic implementation
- Library symbol discovery and registration
- Result processing and analysis
- Threading implementation details
- Integration with registration system

## Submission Requirements

### Files to Submit
1. **Simulator folder** - Complete simulator implementation
2. **Algorithm folder** - Algorithm implementation with .so file
3. **GameManager folder** - GameManager implementation with .so file
4. **common folder** - Common header files (as provided)
5. **UserCommon folder** - User-defined common files
6. **Makefiles** - One per project folder + root makefile
7. **students.txt** - Submitter information
8. **README.md** - Project documentation

### Naming Conventions
- **Executable**: `simulator_<submitter_ids>`
- **Algorithm library**: `Algorithm_<submitter_ids>.so`
- **GameManager library**: `GameManager_<submitter_ids>.so`
- **Namespaces**: Match the library names

## Development Notes

This is a **shell implementation** that provides the complete structure and interface for the Tank Game simulator. The core game execution logic and some integration details need to be completed.

### Key Implementation Areas
1. **Game Execution**: Implement actual game running in the simulator
2. **Library Integration**: Complete the dynamic library symbol discovery
3. **Result Processing**: Implement proper result analysis and output formatting
4. **Threading**: Complete the worker thread system
5. **Testing**: Add comprehensive testing for all components

## Troubleshooting

### Common Issues
- **Library Loading**: Check file permissions and dependencies
- **Compilation**: Ensure C++17 support and all dependencies
- **Runtime**: Verify file paths and .so file compatibility

### Debug Mode
```bash
make clean
make CXXFLAGS="-std=c++17 -Wall -Wextra -O0 -g -DDEBUG"
```

## License

This project is part of the TAU Advanced Topics in Programming course assignment.

## Contact

For questions or issues, please refer to the course forum as specified in the assignment document.

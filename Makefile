# Root Makefile for Tank Game Assignment 3
# Builds all three projects: Simulator, Algorithm, and GameManager

# Default target
all: simulator algorithm game_manager

# Build the Simulator project
simulator:
	@echo "Building Simulator project..."
	@cd Simulator && make
	@echo "Simulator build completed."

# Build the Algorithm project
algorithm:
	@echo "Building Algorithm project..."
	@cd Algorithm && make
	@echo "Algorithm build completed."

# Build the GameManager project
game_manager:
	@echo "Building GameManager project..."
	@cd GameManager && make
	@echo "GameManager build completed."

# Clean all projects
clean:
	@echo "Cleaning all projects..."
	@cd Simulator && make clean
	@cd Algorithm && make clean
	@cd GameManager && make clean
	@echo "All projects cleaned."

# Install all projects (optional)
install: all
	@echo "Installing all projects..."
	@cd Simulator && make install
	@cd Algorithm && make install
	@cd GameManager && make install
	@echo "All projects installed."

# Uninstall all projects
uninstall:
	@echo "Uninstalling all projects..."
	@cd Simulator && make uninstall
	@cd Algorithm && make uninstall
	@cd GameManager && make uninstall
	@echo "All projects uninstalled."

# Show help
help:
	@echo "Available targets:"
	@echo "  all         - Build all three projects"
	@echo "  simulator   - Build only the Simulator project"
	@echo "  algorithm   - Build only the Algorithm project"
	@echo "  game_manager- Build only the GameManager project"
	@echo "  clean       - Clean all projects"
	@echo "  install     - Build and install all projects (requires sudo)"
	@echo "  uninstall   - Uninstall all projects (requires sudo)"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Individual project makefiles are located in their respective directories:"
	@echo "  Simulator/Makefile"
	@echo "  Algorithm/Makefile"
	@echo "  GameManager/Makefile"

# Phony targets
.PHONY: all simulator algorithm game_manager clean install uninstall help

# Show build status
status:
	@echo "Build status:"
	@echo "Simulator:"
	@cd Simulator && make -n 2>/dev/null && echo "  Ready to build" || echo "  Build issues detected"
	@echo "Algorithm:"
	@cd Algorithm && make -n 2>/dev/null && echo "  Ready to build" || echo "  Build issues detected"
	@echo "GameManager:"
	@cd GameManager && make -n 2>/dev/null && echo "  Ready to build" || echo "  Build issues detected"

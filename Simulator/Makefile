# Makefile for Simulator Project
# Assignment 3 - Tank Game Simulator

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
LDFLAGS = -ldl -pthread

# Target executable name (replace with actual submitter IDs)
TARGET = simulator_123456789_987654321

# Source files
SOURCES = main.cpp Simulator.cpp TankAlgorithmRegistration.cpp GameManagerRegistration.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = Simulator.h ../common/GameManagerRegistration.h ../common/TankAlgorithmRegistration.h ../common/AbstractGameManager.h ../common/Player.h ../common/TankAlgorithm.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Built $(TARGET) successfully"

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned build artifacts"

# Install target (optional)
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin/"
	@sudo cp $(TARGET) /usr/local/bin/ || echo "Installation failed - check permissions"

# Uninstall target
uninstall:
	@echo "Removing $(TARGET) from /usr/local/bin/"
	@sudo rm -f /usr/local/bin/$(TARGET) || echo "Uninstallation failed - check permissions"

# Run tests (placeholder)
test: $(TARGET)
	@echo "Running tests..."
	@echo "No tests implemented yet"

# Show help
help:
	@echo "Available targets:"
	@echo "  all        - Build the simulator executable"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install to /usr/local/bin/ (requires sudo)"
	@echo "  uninstall  - Remove from /usr/local/bin/ (requires sudo)"
	@echo "  test       - Run tests (placeholder)"
	@echo "  help       - Show this help message"

# Phony targets
.PHONY: all clean install uninstall test help

# Dependencies
main.o: main.cpp Simulator.h
Simulator.o: Simulator.cpp Simulator.h ../common/GameManagerRegistration.h ../common/TankAlgorithmRegistration.h
GameManagerRegistration.o: GameManagerRegistration.cpp GameManagerRegistration.h
TankAlgorithmRegistration.o: TankAlgorithmRegistration.cpp TankAlgorithmRegistration.h

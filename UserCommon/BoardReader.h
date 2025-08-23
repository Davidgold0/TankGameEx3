#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include "BoardConstants.h"

struct BoardData {
    std::string mapName;
    size_t maxStep;
    size_t numShells;
    size_t rows;
    size_t columns;
    std::vector<std::vector<char>> board;
    size_t player1TankCount;
    size_t player2TankCount;
};

class BoardReader {
private:
    static void logError(const std::string& errorMessage);
    static size_t extractVal(std::ifstream &f, const std::string& param);
    static void buildBoard(std::ifstream &f, BoardData& data);
    static void validateTanks(BoardData& data);
    
    // Helper functions for buildBoard
    static void processBoardLine(const std::string& line, int line_number, BoardData& data);
    static char validateAndProcessChar(char c, int line_number, size_t position);
    static void fillMissingRows(BoardData& data);
    
    // Helper functions for extractVal
    static std::string getValueAfterEquals(const std::string& line);
    static size_t parseValue(const std::string& value, const std::string& param, const std::string& line);

public:
    static BoardData readBoard(const std::string& fileName);
}; 
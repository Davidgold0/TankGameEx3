#pragma once
#include "../common/BattleInfo.h"
#include "../common/SatelliteView.h"
#include <vector>
#include <cstddef> 

class SatelliteBattleInfo : public BattleInfo {
private:
    SatelliteView* satelliteView;
    std::vector<std::vector<char>> board;
    size_t rows;
    size_t columns;
    int tankX;
    int tankY;
    int playerIndex;

public:
    SatelliteBattleInfo(SatelliteView* view, int player_index) : satelliteView(view), playerIndex(player_index) {
        // Initialize board dimensions based on the view
        rows = 0;
        columns = 0;
        tankX = -1;
        tankY = -1;
        // We'll populate the board when needed
    }
    virtual ~SatelliteBattleInfo() {}

    char getObjectAt(size_t x, size_t y) const {
        return satelliteView->getObjectAt(x, y);
    }

    // New methods to access board information
    const std::vector<std::vector<char>>& getBoard() const { return board; }
    size_t getRows() const { return rows; }
    size_t getColumns() const { return columns; }
    
    // Tank position getters
    int getTankX() const { return tankX; }
    int getTankY() const { return tankY; }
    
    // Player index getter
    int getPlayerIndex() const { return playerIndex; }

    // Method to update the board
    void updateBoard() {
        // Find the dimensions by checking the view
        size_t maxX = 0, maxY = 0;
        for (size_t y = 0; ; y++) {
            bool rowHasContent = false;
            for (size_t x = 0; ; x++) {
                char obj = satelliteView->getObjectAt(x, y);
                if (obj == '&') { // '&' indicates out of bounds
                    break;
                }
                rowHasContent = true;
                maxX = std::max(maxX, x);
            }
            if (!rowHasContent) {
                break;
            }
            maxY = y;
        }

        // Update dimensions
        rows = maxY + 1;
        columns = maxX + 1;

        // Reset tank position
        tankX = -1;
        tankY = -1;

        // Resize and populate the board
        board.resize(rows, std::vector<char>(columns));
        for (size_t y = 0; y < rows; y++) {
            for (size_t x = 0; x < columns; x++) {
                board[y][x] = satelliteView->getObjectAt(x, y);
                // Track tank position
                if (board[y][x] == '%') {
                    tankX = x;
                    tankY = y;
                }
            }
        }
    }
}; 
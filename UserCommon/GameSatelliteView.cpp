#include "GameSatelliteView.h"

namespace UserCommon_208000547_208000547 {

GameSatelliteView::GameSatelliteView(const vector<vector<char>>& board, size_t rows, size_t columns,
                                   size_t requestingTankX, size_t requestingTankY)
    : board(board), rows(rows), columns(columns), 
      requestingTankX(requestingTankX), requestingTankY(requestingTankY) {}

GameSatelliteView::~GameSatelliteView() {}

char GameSatelliteView::getObjectAt(size_t x, size_t y) const {
    // Check if coordinates are out of bounds
    if (x >= columns || y >= rows) {
        return INVALID_LOCATION;
    }

    // If this is the requesting tank's position, return '%'
    if (x == requestingTankX && y == requestingTankY) {
        return '%';
    }

    // Get the character at the specified location
    char cell = board[y][x];

    // Map the cell character to the satellite view character according to rules
    switch (cell) {
        case WALL:
        case DAMAGED_WALL:
            return '#';
        case PLAYER1_TANK:
            return '1';
        case PLAYER2_TANK:
            return '2';
        case MINE:
            return '@';
        case SHELL:
        case MINE_SHELL_COLLISION:
            return '*';
        case EMPTY_SPACE:
            return ' ';
        default:
            return ' ';  // Default to empty space for any other characters
    }
}

}
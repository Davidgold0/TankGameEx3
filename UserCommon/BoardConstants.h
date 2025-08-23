#pragma once

namespace BoardConstants {
    // Basic board elements
    const char WALL = '#';
    const char DAMAGED_WALL = '-';
    const char MINE = '@';
    const char EMPTY_SPACE = ' ';
    const char PLAYER1_TANK = '1';
    const char PLAYER2_TANK = '2';
    const char REQUESTING_TANK = '%';
    const char SHELL = '*';
    const char INVALID_LOCATION = '&';

    // Collision states
    const char TANK_SHELL_COLLISION = 'X';
    const char WALL_SHELL_COLLISION = 'W';
    const char SHELL_SHELL_COLLISION = 'S';
    const char MINE_SHELL_COLLISION = 'M';
    const char TANK_MINE_COLLISION = 'T';
    const char TANK_TANK_COLLISION = 'C';

    // Helper function to check if a character represents a collision
    inline bool isCollision(char c) {
        return c != WALL && c != DAMAGED_WALL && c != MINE && 
               c != EMPTY_SPACE && c != PLAYER1_TANK && c != PLAYER2_TANK && 
               c != SHELL;
    }
} 
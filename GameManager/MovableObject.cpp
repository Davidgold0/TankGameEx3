#include "MovableObject.h"
#include <stdexcept>
#include <vector>

MovableObject::MovableObject(size_t x, size_t y, size_t width, size_t height)
    : x(x), y(y), direction({-1, 0}), boardWidth(width), boardHeight(height) {}

// Getters
size_t MovableObject::getX() const { return x; }
size_t MovableObject::getY() const { return y; }
const std::array<int, 2>& MovableObject::getDirection() const { return direction; }

// Movement methods
void MovableObject::move() {
    // Store current position before moving
    previousPosition = {x, y};
    
    auto [newX, newY] = getPotentialMove();
    x = newX;
    y = newY;
}

void MovableObject::moveBackwards() {
    // Store current position before moving
    previousPosition = {x, y};
    
    auto [newX, newY] = getPotentialMoveBackwards();
    x = newX;
    y = newY;
}

std::pair<size_t, size_t> MovableObject::getPotentialMove() const {
    // Calculate next position based on current direction
    size_t nextX = (x + boardWidth + direction[0]) % boardWidth;
    size_t nextY = (y + boardHeight + direction[1]) % boardHeight;
    return {nextX, nextY};
}

std::pair<size_t, size_t> MovableObject::getPotentialMoveBackwards() const {
    // Calculate next position based on opposite direction
    size_t nextX = (x + boardWidth - direction[0]) % boardWidth;
    size_t nextY = (y + boardHeight - direction[1]) % boardHeight;
    return {nextX, nextY};
}

void MovableObject::setPosition(size_t newX, size_t newY) {
    x = newX;
    y = newY;
}

// Direction methods
void MovableObject::setDirection(int dx, int dy) {
    direction = {dx, dy};
}

void MovableObject::rotate90(bool clockwise) {
    int x = direction[0];
    int y = direction[1];
    
    if (clockwise) {
        direction = {-y, x};
    } else {
        direction = {y, -x};
    }
}

void MovableObject::rotate45(bool clockwise) {
    int x = direction[0];
    int y = direction[1];
    
    static const std::vector<std::pair<int, int>> directions = {
        {1, 0}, {1, 1}, {0, 1}, {-1, 1},
        {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
    };

    int currentIndex = -1;
    for (int i = 0; i < 8; ++i) {
        if (directions[i].first == x && directions[i].second == y) {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex == -1) {
        throw std::runtime_error("Invalid direction");
    }

    int offset = clockwise ? 1 : -1;
    int newIndex = (currentIndex + offset + 8) % 8;
    direction = {directions[newIndex].first, directions[newIndex].second};
}

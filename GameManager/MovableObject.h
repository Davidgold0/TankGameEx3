#pragma once
#include <array>
#include <utility>
#include <cstddef>  // for size_t
#include <optional>
#include <vector>

class MovableObject {
protected:
    size_t x;
    size_t y;
    std::array<int, 2> direction;  // [x, y] direction vector
    size_t boardWidth;
    size_t boardHeight;
    std::optional<std::pair<size_t, size_t>> previousPosition;  // Store previous position for swap detection

public:
    MovableObject(size_t x, size_t y, size_t width, size_t height);
    virtual ~MovableObject() = default;

    // Getters
    size_t getX() const;
    size_t getY() const;
    const std::array<int, 2>& getDirection() const;
    std::optional<std::pair<size_t, size_t>> getPreviousPosition() const { return previousPosition; }
    void clearPreviousPosition() { previousPosition = std::nullopt; }

    // Movement methods
    virtual void move();
    virtual void moveBackwards();
    virtual std::pair<size_t, size_t> getPotentialMove() const;
    virtual std::pair<size_t, size_t> getPotentialMoveBackwards() const;
    virtual void setPosition(size_t newX, size_t newY);

    // Direction methods
    virtual void setDirection(int dx, int dy);
    virtual void rotate90(bool clockwise);
    virtual void rotate45(bool clockwise);
}; 
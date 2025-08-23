#include "Shell.h"

Shell::Shell(size_t x, size_t y, int dx, int dy, size_t width, size_t height)
    : MovableObject(x, y, width, height) {
    // Set the initial direction
    setDirection(dx, dy);
} 
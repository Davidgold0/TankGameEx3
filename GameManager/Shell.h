#pragma once
#include "MovableObject.h"

class Shell : public MovableObject {
public:
    // Constructor takes initial position, direction, and board dimensions
    Shell(size_t x, size_t y, int dx, int dy, size_t width, size_t height);
}; 
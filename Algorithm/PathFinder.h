#pragma once

#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <cmath>
#include "../UserCommon/BoardConstants.h"
#include "../common/ActionRequest.h"

using namespace std;
using namespace UserCommon_208000547_208000547;

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Node {
    Point pt;
    int dist;
};

enum Turn {
    RIGHT_90 = 2,
    RIGHT_45 = 1,
    LEFT_45 = -1,
    LEFT_90 = -2,
    RIGHT_135 = 3,
    LEFT_135 = -3,
    COMPLETE_180 = 4,
    NONE_0 = 0
};

// Function declarations
Turn rotation(array<int,2> currDir, array<int,2> newDir);
int inverseMap(array<int, 2> d);
int getRotation45(array<int, 2> from, array<int, 2> to);
array<int,2> directionBetweenPoints(Point &start, Point &end);

bool isValid(int x, int y, const vector<vector<char>>& grid, const vector<vector<bool>>& visited, bool includeWalls);
Point wrapPoint(int x, int y, int rows, int cols);
vector<Point> bfsPathfinder(const vector<vector<char>>& grid, Point start, Point end, bool includeWalls);
int dist(Point p1, Point p2, int rows, int cols);
int distArr(array<int,2> p1, array<int,2> p2, int rows, int cols);
void updatePathEnd(vector<Point> &path, Point &newEnd, int rows, int cols);
void updatePathStart(vector<Point> &path, Point &newStart, int rows, int cols);
bool isPathStraight(vector<Point> &path, int rows, int columns);
bool isPathClear(vector<Point> &path, const vector<vector<char>>& grid);
array<int,2> calcDirection(vector<Point> &path, int rows, int columns);
void printPath(const vector<Point>& path);
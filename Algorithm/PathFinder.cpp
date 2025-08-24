#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <cmath>
#include <array>
#include "../UserCommon/BoardConstants.h"
#include "PathFinder.h"
#include "../common/ActionRequest.h"

using namespace std;
using namespace UserCommon_208000547_208000547;

const int directions[8][2] = {
    {-1, 0}, //UP
    {-1, 1}, //UP_RIGHT
    {0, 1}, //RIGHT
    {1, 1}, //DOWN_RIGHT
    {1, 0}, //DOWN
    {1, -1}, //DOWN_LEFT
    {0, -1}, //LEFT
    {-1, -1}, //UP_LEFT
};

bool isValid(int x, int y, const vector<vector<char>>& grid, const vector<vector<bool>>& visited, bool includeWalls) {
    if (x < 0 || x >= static_cast<int>(grid[0].size()) || y < 0 || y >= static_cast<int>(grid.size())) {
        return false;
    }
    if (visited[y][x]) {
        return false;
    }
    if (grid[y][x] == EMPTY_SPACE) {
        return true;
    }
    if (includeWalls && 
        (grid[y][x] == WALL || grid[y][x] == DAMAGED_WALL)) {
            return true;
        }
    if (grid[y][x] == PLAYER1_TANK || grid[y][x] == PLAYER2_TANK) {
        return true;
    }
    return false;
}

Point wrapPoint(int x, int y, int rows, int cols) {
    x = (x + rows) % rows;
    y = (y + cols) % cols;
    return {x, y};
}

vector<Point> bfsPathfinder(const vector<vector<char>>& grid, Point start, Point end, bool includeWalls) {
    cout << "Starting BFS pathfinding from (" << start.x << "," << start.y << ") to (" << end.x << "," << end.y << ")" << endl;
    cout << "Include walls: " << (includeWalls ? "true" : "false") << endl;

    cout << "Debug: About to get grid dimensions" << endl;
    int rows = grid.size();
    cout << "Debug: Got rows = " << rows << endl;
    int cols = grid[0].size();
    cout << "Debug: Got cols = " << cols << endl;

    // Validate start and end points
    cout << "Debug: Validating start and end points" << endl;
    if (start.y < 0 || start.y >= rows || start.x < 0 || start.x >= cols) {
        cout << "ERROR: Start point (" << start.x << "," << start.y << ") is out of bounds!" << endl;
        cout << "Grid bounds: rows=" << rows << ", cols=" << cols << endl;
        return {};
    }
    if (end.y < 0 || end.y >= rows || end.x < 0 || end.x >= cols) {
        cout << "ERROR: End point (" << end.x << "," << end.y << ") is out of bounds!" << endl;
        cout << "Grid bounds: rows=" << rows << ", cols=" << cols << endl;
        return {};
    }

    cout << "Debug: Creating visited array" << endl;
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    cout << "Debug: Creating parent array" << endl;
    vector<vector<Point>> parent(rows, vector<Point>(cols, {-1, -1}));

    cout << "Debug: Creating queue" << endl;
    queue<Node> q;
    cout << "Debug: Setting start position as visited" << endl;
    cout << "Debug: Start coordinates - x: " << start.x << ", y: " << start.y << endl;
    visited[start.y][start.x] = true;
    cout << "Debug: Successfully set start position as visited" << endl;
    cout << "Debug: Pushing start node to queue" << endl;
    q.push({start, 0});
    cout << "Initialized BFS with grid size: " << rows << "x" << cols << endl;

    while (!q.empty()) {
        Node current = q.front();
        q.pop();

        Point pt = current.pt;
        cout << "Exploring node at (" << pt.x << "," << pt.y << ") with distance " << current.dist << endl;

        if (pt.x == end.x && pt.y == end.y) {
            cout << "Found path to destination!" << endl;
            // Reconstruct path
            vector<Point> path;
            while (!(pt.x == -1 && pt.y == -1)) {
                path.push_back(pt);
                pt = parent[pt.y][pt.x];
            }
            reverse(path.begin(), path.end());
            cout << "Path length: " << path.size() << " steps" << endl;
            return path;
        }

        for (const auto& dir : directions) {
            Point neighbor = wrapPoint(pt.x + dir[1], pt.y + dir[0], cols, rows);
            if (isValid(neighbor.x, neighbor.y, grid, visited, includeWalls)) {
                visited[neighbor.y][neighbor.x] = true;
                parent[neighbor.y][neighbor.x] = pt;
                q.push({neighbor, current.dist + 1});
                cout << "Added valid neighbor at (" << neighbor.x << "," << neighbor.y << ")" << endl;
            }
        }
    }

    cout << "No path found without walls, retrying with walls included" << endl;
    if (includeWalls) {
        return {};
    }

    return bfsPathfinder(grid, start, end, true);
}

int dist(Point p1, Point p2, int rows, int cols) {
    int dx = min(abs(p1.x - p2.x), rows - abs(p1.x - p2.x));
    int dy = min(abs(p1.y - p2.y), cols - abs(p1.y - p2.y));
    return max(dx ,dy);
}

int distArr(array<int,2> p1, array<int,2> p2, int rows, int cols) {
    return dist({p1[0], p1[1]}, {p2[0], p2[1]}, rows, cols);
}

void updatePathEnd(vector<Point> &path, Point &newEnd, int rows, int cols) {
    Point end = path.back();
    path.pop_back();
    if (newEnd == end) {
        path.push_back(newEnd);
    }
    else if (path.size() > 1){
        Point nearEnd = path.back();
        path.pop_back();
        if (!(newEnd == nearEnd)) {
            if (dist(end, newEnd, rows, cols) >= dist(nearEnd, newEnd, rows, cols)) {
                if (!path.empty()) {
                    Point nearNearEnd = path.back();
                    if (dist(newEnd, nearNearEnd, rows, cols) > 1) {
                        path.push_back(nearEnd);
                    }
                }
            }
            else {
                path.push_back(nearEnd);
                path.push_back(end);
            }
        }
        path.push_back(newEnd);
    } else {
        if(dist(newEnd, path.front(), rows, cols) > 1)
            path.push_back(end);
        path.push_back(newEnd);
    }
}

void updatePathStart(vector<Point> &path, Point &newStart, int rows, int cols) {
    Point start = path.front();
    path.erase(path.begin());
    if (newStart == start) {
        path.insert(path.begin(), start);
    }
    else if (path.size() > 1) {
        Point nearStart = path.front();
        path.erase(path.begin());
        if (!(newStart == nearStart)) {
            if (dist(start, newStart, rows, cols) >= dist(nearStart, newStart, rows, cols)) {
                if (!path.empty()) {
                    Point nearNearStart = path.front();
                    if (dist(newStart, nearNearStart, rows, cols) > 1) {
                        path.insert(path.begin(), nearStart);
                    }
                }
            }
            else {
                path.insert(path.begin(), nearStart);
                path.insert(path.begin(), start);
            }
        }
        path.insert(path.begin(), newStart);
    } else {
        if(dist(newStart, path.back(), rows, cols) > 1)
            path.insert(path.begin(), start);
        path.insert(path.begin(), newStart);
    }
}

array<int,2> calcDirection(vector<Point> &path, int rows, int columns) {
    Point *start = &path[0];
    Point *next = &path[1];
    int dx, dy;
    if (start->x == next->x)
        dy = 0;
    else if (start->x == 0 && next->x == rows - 1) 
        dy = -1;
    else if (start->x == rows - 1 && next->x == 0)
        dy = 1;
    else
        dy = next->x - start->x;

    if (start->y == next->y)
        dx = 0;
    else if (start->y == 0 && next->y == columns - 1) 
        dx = -1;
    else if (start->y == columns - 1 && next->y == 0)
        dx = 1;
    else
        dx = next->y - start->y;
    
    return {dy, dx};
}

bool isPathStraight(vector<Point> &path, int rows, int columns) {
    if (path.size() == 1) {
        return true;
    }
    array<int,2> dir = calcDirection(path, rows, columns);

    for (size_t i = 1; i < path.size() - 1; ++i) {
        Point *start = &path[i];
        Point *next = &path[i+1];
        if (!((next->x - start->x)%columns == dir[0]%columns)) {
            return false;
        }
        if (!((next->y - start->y)%rows == dir[1]%rows)) {
            return false;
        }
    }
    return true;
}

bool isPathClear(vector<Point> &path, const vector<vector<char>>& grid) {
    for (Point &p: path) {
        char tile = grid[p.y][p.x];
        if (tile == WALL ||
                tile == DAMAGED_WALL ||
                tile == MINE ||
                tile == SHELL)
                return false;
    }
    return true;
}

void printPath(const vector<Point>& path) {
    for (const auto& p : path) {
        cout << "(" << p.x << "," << p.y << ") ";
    }
    cout << endl;
}


int inverseMap(array<int, 2> d) {
    for(int i = 0; i < 8; ++i) {
        if (directions[i][0] == d[0] && directions[i][1] == d[1]) {
            return i;
        }
    }
    return -1; // Return -1 if direction not found
}


int getRotation45(array<int, 2> from, array<int, 2> to) {
    // All 8 directions in clockwise order
    const vector<array<int, 2>> directions = {
        {1, 0},   // 0 - right
        {1, 1},   // 1 - down-right
        {0, 1},   // 2 - down
        {-1, 1},  // 3 - down-left
        {-1, 0},  // 4 - left
        {-1, -1}, // 5 - up-left
        {0, -1},  // 6 - up
        {1, -1}   // 7 - up-right
    };

    int fromIndex = -1, toIndex = -1;
    for (int i = 0; i < 8; ++i) {
        if (directions[i] == from) fromIndex = i;
        if (directions[i] == to) toIndex = i;
    }

    if (fromIndex == -1 || toIndex == -1) {
        throw std::invalid_argument("Invalid direction vector");
    }

    int delta = toIndex - fromIndex;
    if (delta > 4) delta -= 8;
    if (delta < -4) delta += 8;
    return delta;
}


Turn rotation(array<int,2> currDir, array<int,2> newDir) {

    return Turn(getRotation45(currDir, newDir));
}


array<int,2> directionBetweenPoints(Point &start, Point &end) {
    int dy = end.x - start.x; //this is not a mistake
    int dx = end.y - start.y;
    if (dy > 1) 
        dy = -1;
    else if (dy < -1)
        dy = 1;
    if (dx > 1) 
        dx = -1;
    else if (dx < -1)
        dx = 1; 
    return {dy, dx};
}
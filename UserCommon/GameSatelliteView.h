#pragma once
#include "../common/SatelliteView.h"
#include <vector>
#include "BoardConstants.h"

using namespace std;

class GameSatelliteView : public SatelliteView {
private:
    const vector<vector<char>>& board;
    const size_t rows;
    const size_t columns;
    const size_t requestingTankX;
    const size_t requestingTankY;

public:
    GameSatelliteView(const vector<vector<char>>& board, size_t rows, size_t columns, 
                     size_t requestingTankX, size_t requestingTankY);
    virtual ~GameSatelliteView() override;
    virtual char getObjectAt(size_t x, size_t y) const override;
}; 
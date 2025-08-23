#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "../common/ActionRequest.h"
#include "RoundInfo.h"

class OutputWriter {
private:
    std::ofstream outputFile;
    
    // 2D vector matrix to store tank history
    // First dimension: tank ID
    // Second dimension: round number
    std::vector<std::vector<RoundInfo>> tankHistory;

    void writeRoundToFile(const std::vector<RoundInfo>& currentRound);
    std::string actionToString(ActionRequest action) const;

public:
    OutputWriter(const std::string& fileName);
    ~OutputWriter();

    static constexpr int ZERO_SHELLS_STEPS = 40;

    void addRoundForTank(int tankId, const RoundInfo& info);
    void writeOutputFile();
    void writeCurrentRound();
    
    // Game end conditions
    void writeGameEnd(int winner, int remainingTanks);
    void writeMaxStepsTie(int maxSteps, int player1Tanks, int player2Tanks);
    void writeZeroShellsTie();
}; 
#include "OutputWriter.h"
#include <iostream>

OutputWriter::OutputWriter(const std::string& fileName, bool verbose) : verbose(verbose) {
    if (verbose) {
        outputFile.open(fileName);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Could not open output file: " + fileName);
        }
    }
    tankHistory = std::vector<std::vector<RoundInfo>>();
}

OutputWriter::~OutputWriter() {
    if (verbose && outputFile.is_open()) {
        outputFile.close();
    }
}

std::string OutputWriter::actionToString(ActionRequest action) const {
    switch (action) {
        case ActionRequest::MoveForward: return "MoveForward";
        case ActionRequest::MoveBackward: return "MoveBackward";
        case ActionRequest::RotateLeft90: return "RotateLeft90";
        case ActionRequest::RotateRight90: return "RotateRight90";
        case ActionRequest::RotateLeft45: return "RotateLeft45";
        case ActionRequest::RotateRight45: return "RotateRight45";
        case ActionRequest::Shoot: return "Shoot";
        case ActionRequest::GetBattleInfo: return "GetBattleInfo";
        case ActionRequest::DoNothing: return "DoNothing";
        default: return "Unknown";
    }
}

void OutputWriter::addRoundForTank(int tankId, const RoundInfo& info) {
    std::cout << "Adding round for Tank " << tankId << std::endl;
    // Ensure the tankHistory vector is large enough
    if (static_cast<size_t>(tankId) >= tankHistory.size()) {
        std::cout << "Tank " << tankId << " not found in history, creating new entry" << std::endl;
        tankHistory.resize(static_cast<size_t>(tankId) + 1);
    }
    
    std::cout << "Tank " << tankId << " found in history, adding new round" << std::endl;
    tankHistory[static_cast<size_t>(tankId)].push_back(info);
    std::cout << "Added round for Tank " << tankId << ", history size: " << tankHistory[static_cast<size_t>(tankId)].size() << std::endl;
}

void OutputWriter::writeRoundToFile(const std::vector<RoundInfo>& currentRound) {
    if (!verbose) {
        return;
    }
    
    bool firstAction = true;
    
    // Write actions in order of tank IDs
    for (size_t tankId = 0; tankId < currentRound.size(); ++tankId) {
        if (!firstAction) {
            outputFile << ", ";
        }
        firstAction = false;

        const auto& info = currentRound[tankId];
        if (!info.isAlive && !info.wasKilled) {
            // Tank was already dead in previous rounds
            outputFile << "killed";
        } else {
            // Tank is either alive or was just killed this round
            outputFile << actionToString(info.action);
            
            if (info.wasActionIgnored) {
                outputFile << " (ignored)";
            }
            
            if (info.wasKilled) {
                outputFile << " (killed)";
            }
        }
    }
    outputFile << std::endl;
}

void OutputWriter::writeOutputFile() {
    if (!verbose) {
        return;
    }
    
    // Get the number of rounds from any tank's history
    if (tankHistory.empty()) {
        return;
    }
    
    size_t numRounds = tankHistory[0].size();
    
    // For each round, collect the info for all tanks
    for (size_t round = 0; round < numRounds; ++round) {
        std::vector<RoundInfo> currentRound;
        currentRound.reserve(tankHistory.size());
        
        // Collect info for all tanks in this round
        for (size_t tankId = 0; tankId < tankHistory.size(); ++tankId) {
            if (round < tankHistory[tankId].size()) {
                currentRound.push_back(tankHistory[tankId][round]);
            }
        }
        
        writeRoundToFile(currentRound);
    }
}

void OutputWriter::writeGameEnd(int winner, int remainingTanks) {
    if (!verbose) {
        return;
    }
    
    if (winner == 0) {
        outputFile << "Tie, both players have zero tanks" << std::endl;
    } else {
        outputFile << "Player " << winner << " won with " << remainingTanks << " tanks still alive" << std::endl;
    }
}

void OutputWriter::writeMaxStepsTie(int maxSteps, int player1Tanks, int player2Tanks) {
    if (!verbose) {
        return;
    }
    
    outputFile << "Tie, reached max steps = " << maxSteps 
               << ", player 1 has " << player1Tanks 
               << " tanks, player 2 has " << player2Tanks << " tanks" << std::endl;
}

void OutputWriter::writeZeroShellsTie() {
    if (!verbose) {
        return;
    }
    
    outputFile << "Tie, both players have zero shells for <" << ZERO_SHELLS_STEPS << "> steps" << std::endl;
}

void OutputWriter::writeCurrentRound() {
    if (!verbose) {
        return;
    }
    
    if (tankHistory.empty()) {
        return;
    }
    
    size_t currentRoundNum = tankHistory[0].size() - 1;
    
    // Collect info for all tanks in this round
    std::vector<RoundInfo> currentRound;
    currentRound.reserve(tankHistory.size());
    
    for (size_t tankId = 0; tankId < tankHistory.size(); ++tankId) {
        if (currentRoundNum < tankHistory[tankId].size()) {
            currentRound.push_back(tankHistory[tankId][currentRoundNum]);
        }
    }
    
    writeRoundToFile(currentRound);
} 
#include "BoardReader.h"
#include <iostream>

using namespace std;

namespace UserCommon_208000547_208000547 {

void BoardReader::logError(const string& errorMessage) {
    ofstream errorFile("board_errors.txt", ios::app);
    if (errorFile.is_open()) {
        errorFile << errorMessage << endl;
        errorFile.close();
    }
}

string BoardReader::getValueAfterEquals(const string& line) {
    size_t equal_sign = line.find("=");
    if (equal_sign == string::npos) {
        logError("Error: Invalid format: " + line);
        throw runtime_error("Invalid format: " + line);
    }
    return line.substr(equal_sign + 1);
}

size_t BoardReader::parseValue(const string& value, const string& param, const string& line) {
    size_t space = value.find_last_of(' ');
    try {
        size_t returnVal;
        if (space == string::npos) 
            returnVal = stoi(value);
        else
            returnVal = stoi(value.substr(space + 1));
        cout << param << " = " << returnVal << endl;
        return returnVal;
    }
    catch (const invalid_argument &e) {
        logError("Error: Invalid format: Expecting " + param + " to be unsigned int" + line);
        throw runtime_error("Error: Invalid format: Expecting " + param + " to be unsigned int" + line);
    }
}

size_t BoardReader::extractVal(ifstream &f, const string& param) {
    string line;
    if (!getline(f, line)) {
        logError("Error: " + param + " not specified");
        throw runtime_error(param + " not specified");
    }
    string value = getValueAfterEquals(line);
    return parseValue(value, param, line);
}

char BoardReader::validateAndProcessChar(char c, int line_number, size_t position) {
    if (c != WALL && c != MINE && c != PLAYER1_TANK && c != PLAYER2_TANK && c != EMPTY_SPACE) {
        logError("Warning: Invalid character '" + string(1, c) + "' at line " + 
                to_string(line_number) + ", position " + to_string(position) + 
                ". Replacing with empty space.");
        return EMPTY_SPACE;
    }
    return c;
}

void BoardReader::processBoardLine(const string& line, int line_number, BoardData& data) {
    vector<char> row;
    for (size_t i = 0; i < data.columns; i++) {
        char c;
        if (i < line.length()) {
            c = validateAndProcessChar(line[i], line_number, i);
            // Count tanks while processing
            if (c == PLAYER1_TANK) {
                data.player1TankCount++;
            } else if (c == PLAYER2_TANK) {
                data.player2TankCount++;
            }
        } else {
            c = EMPTY_SPACE;
            logError("Warning: Line " + to_string(line_number) + 
                    " is shorter than specified width. Adding empty spaces.");
        }
        row.push_back(c);
    }
    data.board.push_back(row);
}

void BoardReader::fillMissingRows(BoardData& data) {
    while (data.board.size() < data.rows) {
        logError("Warning: File has fewer rows than specified height. Adding empty rows.");
        vector<char> emptyRow(data.columns, EMPTY_SPACE);
        data.board.push_back(emptyRow);
    }
}

void BoardReader::buildBoard(ifstream &f, BoardData& data) {
    int line_number = 6;
    string s;

    cout << "Reading board contents..." << endl;
    while(getline(f,s)) {
        cout << "Reading line " << line_number << ": " << s << endl;

        if (data.board.size() >= data.rows) {
            cout << "Reached maximum board height, stopping." << endl;
            logError("Warning: File has more rows than specified height. Extra rows will be ignored.");
            break;
        }

        processBoardLine(s, line_number, data);
        ++line_number;
    }

    fillMissingRows(data);
    std::cout << "Finished constructing board" << std::endl;
}

void BoardReader::validateTanks(BoardData& data) {
    // Only validate and warn about tank counts
    if (data.player1TankCount == 0 && data.player2TankCount == 0) {
        logError("Warning: No tanks found for either player. This will result in an immediate tie.");
    } else if (data.player1TankCount == 0) {
        logError("Warning: No tanks found for Player 1. Player 1 will lose immediately.");
    } else if (data.player2TankCount == 0) {
        logError("Warning: No tanks found for Player 2. Player 2 will lose immediately.");
    }
}

BoardData BoardReader::readBoard(const string& fileName) {
    BoardData data;
    // Initialize tank counts
    data.player1TankCount = 0;
    data.player2TankCount = 0;
    
    ifstream f(fileName);
    if (!f.is_open()) {
        logError("Error: Could not open file: " + fileName);
        throw runtime_error("Could not open file: " + fileName);
    }

    string firstLine;
    if (!getline(f, firstLine)) {
        logError("Error: File is empty: " + fileName);
        throw runtime_error("File is empty: " + fileName);
    }
    // First line is map name/description - we ignore it as per guidelines
    data.maxStep = extractVal(f, "MaxSteps");
    data.numShells = extractVal(f, "NumShells");
    data.rows = extractVal(f, "Rows");
    data.columns = extractVal(f, "Columns");
    buildBoard(f, data);
    validateTanks(data);
    return data;
}

} 
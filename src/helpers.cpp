#include "helpers.hpp"

// Utility functions
std::string getCarName(const std::string& modelId) {
    if (modelId == "01") return "kourai";
    else if (modelId == "02") return "boson";
    else if (modelId == "03") return "rho";
    else if (modelId == "04") return "katal";
    else if (modelId == "05") return "hadion";
    else if (modelId == "06") return "spektrix";
    else if (modelId == "07") return "corax";
    else if (modelId == "08") return "groundshock";
    else if (modelId == "09") return "skull";
    else if (modelId == "0A") return "thermo";
    else if (modelId == "0B") return "nuke";
    else if (modelId == "0C") return "guardian";
    else if (modelId == "0E") return "bigbang";
    else if (modelId == "0F") return "freewheel";
    else if (modelId == "10") return "x52";
    else if (modelId == "11") return "x52ice";
    else if (modelId == "12") return "mammoth";
    else if (modelId == "13") return "dynamo";
    else if (modelId == "14") return "ghost";
    else return "unknown";
}

// Basic track name utility function
std::string getTrackName(uint8_t trackId) {
    switch (trackId) {
        case 36: case 39: case 40: case 51: return "Straight";
        case 17: case 18: case 20: case 23: return "Curve";
        case 34: return "Pre-Finish Line";
        case 33: return "Start/Finish";
        case 57: return "FnF Powerup";
        case 10: return "Intersection";
        default: return "Unknown";
    }
}

std::vector<std::tuple<std::string, bool, int>> filterDuplicates(const std::vector<std::tuple<std::string, bool, int>>& readings) {
    std::vector<std::tuple<std::string, bool, int>> filteredReadings;

    int i = 0;
    while (i < readings.size()) {
        auto current = readings[i];
        std::string currentType = std::get<0>(current);
        int currentPosition = std::get<2>(current);

        filteredReadings.push_back(current);

        int j = i + 1;
        bool foundValidSuccessor = false;
        while (j < readings.size()) {
            auto next = readings[j];
            std::string nextType = std::get<0>(next);
            int nextPosition = std::get<2>(next);

            if (nextType == currentType) {
                if (nextPosition >= currentPosition) {
                    foundValidSuccessor = true;
                    break;
                } else {
                    j++;
                }
            } else {
                break;
            }
        }

        if (!foundValidSuccessor) {
            filteredReadings.pop_back();
            filteredReadings.push_back(current);
        }

        i = j;
    }

    return filteredReadings;
}
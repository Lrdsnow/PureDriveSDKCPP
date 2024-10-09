#include "VehicleDelegate.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>

std::vector<std::tuple<std::string, bool, int>> VehicleDelegate::scanTrack() {
    setSpeed(700, 700);
    
    int startFinishCount = 0;
    std::vector<std::tuple<std::string, bool, int>> trackLog;
    int lastPos = -1;

    loggedTracks.clear();

    while (startFinishCount < 3) {
        if (!loggedTracks.empty() && lastPos != std::get<2>(loggedTracks.back())) {
            auto lastTrack = loggedTracks.back();
            if (startFinishCount == 1) {
                trackLog.push_back(lastTrack);
            }
            if (std::get<0>(lastTrack) == "Start/Finish") {
                std::cout << startFinishCount << std::endl;
                startFinishCount++;
                if (startFinishCount == 1) {
                    loggedTracks.clear();
                }
            }
            if (startFinishCount == 1 && std::get<0>(lastTrack) == "Pre-Finish Line") {
                setSpeed(0, 2800);
                break;
            }
            lastPos = std::get<2>(lastTrack);
        }
    }

    bool finish_clockwise = !trackLog.empty() ? std::get<1>(trackLog.back()) : false;
    trackLog.erase(std::remove_if(trackLog.begin(), trackLog.end(),
        [](const std::tuple<std::string, bool, int>& track) {
            return std::get<0>(track) == "Start/Finish" || std::get<0>(track) == "Pre-Finish Line";
        }), trackLog.end());

    trackLog.push_back(std::make_tuple("Pre-Finish Line", finish_clockwise, 0));
    trackLog.push_back(std::make_tuple("Start/Finish", finish_clockwise, 0));

    auto newTrackLog = filterDuplicates(trackLog);
    loggedTracks = newTrackLog;
    return newTrackLog;
}
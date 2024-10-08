#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>
#include <gattlib.h>
#include <sstream>

// get car name
std::string getCarName(uint8_t modelId);

// get track name
std::string getTrackName(uint8_t trackId);

// gattlib error handling
std::string getGattlibErrorString(int error_code);
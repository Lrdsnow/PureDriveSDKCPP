#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>
#include <simpleble/SimpleBLE.h>
#include <sstream>
#include <tuple>


// get car name
std::string getCarName(const std::string& modelId);

// get track name
std::string getTrackName(uint8_t trackId);

// filter duplicate tracks
std::vector<std::tuple<std::string, bool, int>> filterDuplicates(const std::vector<std::tuple<std::string, bool, int>>& readings);

enum class PeripheralState {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting
};

struct VehicleAdvData {
public:
    std::string carName;
    std::string address;
    PeripheralState state;

    VehicleAdvData(const std::string& carName, const SimpleBLE::BluetoothUUID address, PeripheralState state) 
        : carName(carName), address(address), state(state) {

    }

    friend std::ostream& operator<<(std::ostream& os, const VehicleAdvData& v) {
        os << "Car Name: " << v.carName << ", Address: " << v.address << ", State: " << static_cast<int>(v.state);
        return os;
    }
};
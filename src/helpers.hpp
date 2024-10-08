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
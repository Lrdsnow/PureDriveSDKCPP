#ifndef PURE_DRIVE_HPP
#define PURE_DRIVE_HPP

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <cstring>
#include <tuple>
#include <string>

#include <simpleble/SimpleBLE.h>
#include "anki_sdk/protocol.h"

// Forward declaration of VehicleDelegate
class VehicleDelegate;

// BluetoothManager Class Definition
class BluetoothManager {
private:
    std::thread scanThread;
    std::atomic<bool> isScanning;
    std::vector<std::unique_ptr<VehicleDelegate>> discoveredVehicles;
    
    // UUID for the Anki service
    SimpleBLE::BluetoothUUID ankiServiceUUID{"be15beef-6186-407e-8381-0bd89c4d8df4"};

    // Scanning loop for Bluetooth devices
    void scanLoop();

public:
    BluetoothManager();
    ~BluetoothManager();
    
    // Start scanning for Bluetooth devices
    void startScanning();
    
    // Stop scanning for Bluetooth devices
    void stopScanning();
};

// VehicleDelegate Class Definition
class VehicleDelegate {
public:
    SimpleBLE::Peripheral peripheral;
    VehicleAdvData advData;
    std::optional<SimpleBLE::Characteristic> readChannel;
    std::optional<SimpleBLE::Characteristic> writeChannel;
    std::optional<SimpleBLE::Service> rwService;
    void* controller;
    bool active = false;
    uint32_t identifier = 0;
    uint8_t modelIdentifier = 0;
    std::string version = "0x0000";
    bool debug_mode = true;
    std::vector<std::tuple<std::string, bool, int>> loggedTracks;

    SimpleBLE::BluetoothUUID ankiChrReadUUID{"be15bee0-6186-407e-8381-0bd89c4d8df4"};
    SimpleBLE::BluetoothUUID ankiChrWriteUUID{"be15bee1-6186-407e-8381-0bd89c4d8df4"};

    // Enumerations
    enum class VehicleLightChannel : uint32_t {
        Red = 0,
        Tail = 1,
        Blue = 2,
        Green = 3,
        FrontL = 4,
        FrontR = 5,
        Count = 6
    };

    enum class VehicleLightEffect : uint32_t {
        Steady = 0,
        Fade = 1,
        Throb = 2,
        Flash = 3,
        Random = 4,
        Count = 5
    };

    // Constructor
    VehicleDelegate(void* controller, SimpleBLE::Peripheral peripheral, VehicleAdvData advData);

    // Public methods
    bool isActive() const;
    void setSDKMode(bool on, uint8_t flags);
    void setOffsetFromRoadCenter(float offset);
    void changeLane(uint16_t horizontalSpeed, uint16_t horizontalAccel, float offset);
    void setLights(uint8_t mask);
    void setLightsPattern(VehicleLightChannel channel, VehicleLightEffect effect, uint8_t start, uint8_t end, uint16_t cyclesPerMin);
    void setEngineLight(uint8_t r, uint8_t g, uint8_t b, VehicleLightEffect effect, uint16_t cycles);
    void disconnect();
    void ping();
    void getVersion();
    void getBatteryLevel();
    void cancelLaneChange();
    void turn180();
    void setSpeed(int speed, int acceleration);
    void sendMessage(void* buffer, uint8_t len);

    void onConnected();

    std::vector<std::tuple<std::string, bool, int>> scanTrack();

private:
    void onMessageReceived(SimpleBLE::ByteArray payload);
};

#endif // PURE_DRIVE_HPP

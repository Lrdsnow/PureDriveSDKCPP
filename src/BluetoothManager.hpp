#ifndef BLUETOOTH_MANAGER_HPP
#define BLUETOOTH_MANAGER_HPP

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <map>
#include <memory>
#include <simpleble/SimpleBLE.h>
#include "anki_sdk/protocol.h"
#include "VehicleDelegate.hpp"

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

#endif // BLUETOOTH_MANAGER_HPP

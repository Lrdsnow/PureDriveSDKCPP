#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <map>
#include <memory>
#include <simpleble/SimpleBLE.h>
#include "helpers.hpp"
#include "anki_sdk/protocol.h"
#include "VehicleDelegate.hpp"

class BluetoothManager {
private:
    std::thread scanThread;
    std::atomic<bool> isScanning;
    std::vector<std::unique_ptr<VehicleDelegate>> discoveredVehicles;
    
    SimpleBLE::BluetoothUUID ankiServiceUUID{"be15beef-6186-407e-8381-0bd89c4d8df4"};

public:
    BluetoothManager() : isScanning(false) {}

    ~BluetoothManager() {
        stopScanning();
    }

    void startScanning() {
        if (isScanning) {
            std::cerr << "Scan already in progress." << std::endl;
            return;
        }

        isScanning = true;
        scanThread = std::thread(&BluetoothManager::scanLoop, this);
    }

    void stopScanning() {
        if (isScanning) {
            isScanning = false;
            if (scanThread.joinable()) {
                scanThread.join();
            }
        }
    }

    void scanLoop() {
        auto adapters = SimpleBLE::Adapter::get_adapters();
        if (adapters.empty()) {
            std::cerr << "No Bluetooth adapter found." << std::endl;
            return;
        }

        SimpleBLE::Adapter adapter = adapters.front();
        std::cout << "Using adapter: " << adapter.identifier() << std::endl;

        adapter.set_callback_on_scan_found([this](SimpleBLE::Peripheral peripheral) {
            auto services = peripheral.services();

            bool containsAnkiServiceUUID = std::any_of(services.begin(), services.end(),
                [this](SimpleBLE::Service service) {
                    return service.uuid() == ankiServiceUUID;
                });

            if (!containsAnkiServiceUUID) {
                return;
            }

            //std::cout << "debug:\naddr: " << peripheral.address() << "\nname: " << peripheral.identifier() << "\n" << std::endl;

            std::map<short unsigned int, kvn::bytearray> manufacturerDataMap = peripheral.manufacturer_data();
            
            if (!manufacturerDataMap.empty()) {
                auto it = manufacturerDataMap.begin();
                const kvn::bytearray& byteArray = it->second;
                std::vector<uint8_t> manufacturerData(byteArray.begin(), byteArray.end());
                uint8_t modelData = (manufacturerData.size() > 2) ? manufacturerData[2] : 0;
                if (modelData != 0x00) {
                    std::cout << "Found Car: " << getCarName(modelData) << " @ " << peripheral.address() << std::endl;
                }
            } else {
                std::cout << "No manufacturer data available." << std::endl;
            }
        });

        adapter.scan_for(10000);  // Scan for 10 seconds (or use another condition)
    }
};

// Debugging
int main() {
    BluetoothManager manager;
    manager.startScanning();

    std::this_thread::sleep_for(std::chrono::seconds(10));  // Simulate scanning for 10 seconds

    manager.stopScanning();
    return 0;
}

#include "BluetoothManager.hpp"

BluetoothManager::BluetoothManager() : isScanning(false) {}

BluetoothManager::~BluetoothManager() {
    stopScanning();
}

void BluetoothManager::startScanning() {
    if (isScanning) {
        std::cerr << "Scan already in progress." << std::endl;
        return;
    }

    isScanning = true;
    scanThread = std::thread(&BluetoothManager::scanLoop, this);
}

void BluetoothManager::stopScanning() {
    if (isScanning) {
        isScanning = false;
        if (scanThread.joinable()) {
            scanThread.join();
        }
    }
}

void BluetoothManager::scanLoop() {
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

        std::map<short unsigned int, kvn::bytearray> manufacturerDataMap = peripheral.manufacturer_data();
        
        if (!manufacturerDataMap.empty()) {
            auto it = manufacturerDataMap.begin();
            const SimpleBLE::ByteArray byteArray = it->second;
            std::istringstream iss(byteArray.toHex(true));
            std::vector<std::string> words;
            std::string word;
            while (iss >> word) {
                words.push_back(word);
            }
            if (words.size() >= 2) {
                std::string secondWord = words[1];
                std::string carName = getCarName(secondWord);
                if (carName != "unknown") {
                    std::cout << "Found Car: " << carName << " @ " << peripheral.address() << std::endl;
                    VehicleAdvData advData = VehicleAdvData(carName, peripheral.address(), PeripheralState::Disconnected);
                    discoveredVehicles.push_back(std::make_unique<VehicleDelegate>(this, peripheral, advData));
                }
            }
        } else {
            std::cout << "No manufacturer data available." << std::endl;
        }
    });

    adapter.scan_for(10000000);
}

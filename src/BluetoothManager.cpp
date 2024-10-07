#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include "helpers.hpp"
#include "anki_sdk/protocol.h"
#include "anki_sdk/advertisement.h"
#include "anki_sdk/vehicle_gatt_profile.h"
#include "anki_sdk/uuid.h"
#include "VehicleDelegate.hpp"

class BluetoothManager {
private:
    int deviceId;
    int socket;
    std::thread scanThread;
    std::atomic<bool> isScanning;
    std::vector<std::unique_ptr<VehicleDelegate>> discoveredVehicles;

public:
    BluetoothManager() {
        deviceId = hci_get_route(nullptr);
        socket = hci_open_dev(deviceId);
        if (deviceId < 0 || socket < 0) {
            std::cerr << "Failed to open Bluetooth device." << std::endl;
            exit(1);
        }
        isScanning = false;
    }

    ~BluetoothManager() {
        stopScanning();
        if (socket >= 0) {
            close(socket);
        }
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
        // Setup the filter for HCI events
        struct hci_filter nf;
        hci_filter_clear(&nf);
        hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
        hci_filter_set_event(EVT_LE_META_EVENT, &nf);
        setsockopt(socket, SOL_HCI, HCI_FILTER, &nf, sizeof(nf));

        // Configure scanning parameters
        uint16_t interval = htobs(0x0010); // 10 ms scan interval
        uint16_t window = htobs(0x0010);   // 10 ms scan window
        int ret = hci_le_set_scan_parameters(socket, 0x01, interval, window, 0x00, 0x00, 1000);
        if (ret < 0) {
            perror("Failed to set scan parameters");
            //return;
        }

        // Enable scanning
        hci_le_set_scan_enable(socket, 1, 1, 1000);

        // Allow non-blocking reads
        int flags = fcntl(socket, F_GETFL, 0);
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);

        while (isScanning) {
            unsigned char buffer[HCI_MAX_EVENT_SIZE];
            int len = read(socket, buffer, sizeof(buffer));
            if (len < 0) {
                if (errno == EAGAIN || errno == EINTR) {
                    continue;
                }
                perror("Error reading from socket");
                break;
            }

            evt_le_meta_event *meta = (evt_le_meta_event *)(buffer + (1 + HCI_EVENT_HDR_SIZE));
            if (meta->subevent != EVT_LE_ADVERTISING_REPORT) {
                continue;
            }

            processAdvertisement(meta->data, len - (1 + HCI_EVENT_HDR_SIZE));
        }

        hci_le_set_scan_enable(socket, 0, 0x01, 1000);
    }

    void processAdvertisement(const uint8_t* scanData, size_t scanDataLen) {
        le_advertising_info *info = (le_advertising_info *)(scanData + 1);

        char addr[18];
        ba2str(&info->bdaddr, addr);

        for (const auto& vehiclePtr : discoveredVehicles) {
            const VehicleDelegate* vehicle = vehiclePtr.get();
            if (strcmp(vehicle->addr, addr) == 0) {
                return;
            }
        }

        anki_vehicle_adv_t vehicleAdv;
        uint8_t parseResult = anki_vehicle_parse_adv_record(info->data, info->length, &vehicleAdv);

        if (parseResult != 0) {
            return;
        }

        std::string carName = getCarName(vehicleAdv.mfg_data.model_id);

        if (!carName.empty() && carName != "Unknown") {
            std::cout << "Found car: " << carName << " @ " << addr << std::endl;
            VehicleDelegate vehicleDelegate(vehicleAdv, carName, addr);
            discoveredVehicles.push_back(std::make_unique<VehicleDelegate>(vehicleAdv, carName, addr));
        }
    }
};

// Debugging
int main() {
    std::cout << "Scanning..." << std::endl;
    BluetoothManager manager;
    manager.startScanning();

    std::this_thread::sleep_for(std::chrono::seconds(10));  // Simulate scanning for 10 seconds

    manager.stopScanning();
    return 0;
}

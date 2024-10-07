#ifndef VEHICLE_DELEGATE_H
#define VEHICLE_DELEGATE_H

#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <gattlib.h>
#include "anki_sdk/protocol.h"
#include "anki_sdk/advertisement.h"
#include "anki_sdk/vehicle_gatt_profile.h"

class VehicleDelegate {
public:
    // Existing constructor
    VehicleDelegate(const anki_vehicle_adv_t &vehicleAdv, const std::string &name, char addr[18]);
    
    // Move constructor
    VehicleDelegate(VehicleDelegate&& other) noexcept
        : addr(), name(std::move(other.name)), modelIdentifier(other.modelIdentifier), 
          active(other.active.load()), writeChannel(other.writeChannel), readChannel(other.readChannel),
          loggedTracks(std::move(other.loggedTracks)) {
        strncpy(this->addr, other.addr, sizeof(this->addr));
        this->addr[sizeof(this->addr) - 1] = '\0'; // Ensure null-termination
    }
    
    // Move assignment operator
    VehicleDelegate& operator=(VehicleDelegate&& other) noexcept {
        if (this != &other) {
            strncpy(this->addr, other.addr, sizeof(this->addr));
            this->addr[sizeof(this->addr) - 1] = '\0'; // Ensure null-termination
            name = std::move(other.name);
            modelIdentifier = other.modelIdentifier;
            active.store(other.active.load());
            writeChannel = other.writeChannel;
            readChannel = other.readChannel;
            loggedTracks = std::move(other.loggedTracks);
        }
        return *this;
    }

    char addr[18]; // Bluetooth address of the vehicle
    std::string name; // Name of the vehicle
    uint8_t modelIdentifier; // Model identifier of the vehicle
    std::string version; // Version of the vehicle
    std::vector<std::tuple<std::string, bool, int>> loggedTracks; // Tracks logged by the vehicle
    
    bool isActive() const {
        return active.load(); // Use atomic load to get active status
    }
    
    void setSpeed(int speed, int acceleration);
    void spin180();
    void setEngineLight(uint8_t red, uint8_t green, uint8_t blue);
    void sendMessage(const uint8_t* buffer, uint8_t len);

    void handleServiceDiscovery();
    void handleCharacteristicDiscovery();
    void handleValueUpdate(const uint8_t* data, size_t length);
    void handleWriteConfirmation();

    void connectToVehicle();
    void discoverServices();
    void discoverCharacteristics();

private:
    std::atomic<bool> active; // Indicates if the vehicle is active

    std::string deviceAddress;
    gatt_connection_t* connection;
    gattlib_characteristic_t* writeChannel;
    gattlib_characteristic_t* readChannel;

    static void connectionCallback(void *adapter, const char *dst, gatt_connection_t* connection, int error, void* user_data);

    void logTrack(const std::string& trackName, bool isClockwise, int reserved);
};

#endif // VEHICLE_DELEGATE_H

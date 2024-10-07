#include "VehicleDelegate.hpp"
#include <cstring>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/socket.h>
#include <unistd.h>

// Define the constructor
VehicleDelegate::VehicleDelegate(const anki_vehicle_adv_t &vehicleAdv, const std::string &name, char addr[18])
    : name(name), modelIdentifier(vehicleAdv.mfg_data.identifier), active(false), writeChannel(0), readChannel(0) {
    
    // Copy the address, ensure null termination
    strncpy(this->addr, addr, sizeof(this->addr));
    this->addr[sizeof(this->addr) - 1] = '\0'; // Ensure null-termination

    // Attempt to connect to the vehicle and retrieve the channels
    connectToVehicle();
}

void VehicleDelegate::connectToVehicle() {
    int result = gattlib_connect(nullptr, deviceAddress.c_str(), 0, connectionCallback, this);

    if (result != GATTLIB_SUCCESS) {
        std::cerr << "Failed to initiate connection to vehicle at " << deviceAddress << ", error code: " << result << std::endl;
    } else {
        std::cout << "Connection initiated for vehicle at " << deviceAddress << std::endl;
    }
}

void VehicleDelegate::discoverServices() {
    // Simulate discovering services
    std::cout << "Discovered services (example)..." << std::endl;
    discoverCharacteristics();
}

void VehicleDelegate::connectionCallback(void *adapter, const char *dst, gatt_connection_t* conn, int error, void* user_data) {
    if (error != GATTLIB_SUCCESS) {
        std::cerr << "Connection error to vehicle at " << dst << ", error code: " << error << std::endl;
        return;
    }

    std::cout << "Connected to vehicle at " << dst << std::endl;

    // Cast user_data back to VehicleDelegate
    VehicleDelegate* vehicleDelegate = static_cast<VehicleDelegate*>(user_data);
    vehicleDelegate->connection = conn; // Store the connection for further use

    // Proceed to discover services or characteristics as needed
    vehicleDelegate->discoverServices(); // Implement this method as needed
}

void read_signal_handler(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {
    // Process the notification
    printf("Received notification!\n");
    printf("UUID: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", uuid);
    }
    printf("\nData: ");
    for (size_t i = 0; i < data_length; i++) {
        printf("%02x ", data[i]);
    }
    printf("\nData length: %zu\n", data_length);
}

// Helper function to convert a UUID string to the custom uuid_t
void stringToUuid(const char* uuid_str, uuid_t& uuid) {
    uint8_t bytes[16];
    sscanf(uuid_str, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
           &bytes[0], &bytes[1], &bytes[2], &bytes[3],
           &bytes[4], &bytes[5], &bytes[6], &bytes[7],
           &bytes[8], &bytes[9], &bytes[10], &bytes[11],
           &bytes[12], &bytes[13], &bytes[14], &bytes[15]);

    uuid.type = 0; // Assuming type 0 for 128-bit UUID
    memcpy(&uuid.value.uuid128, bytes, sizeof(bytes)); // Copy the bytes into uuid128
}

// Function to compare two uuid_t values
bool compareUuids(const uuid_t& uuid1, const uuid_t& uuid2) {
    return memcmp(&uuid1.value.uuid128, &uuid2.value.uuid128, sizeof(uuid1.value.uuid128)) == 0;
}

void VehicleDelegate::discoverCharacteristics() {
    int characteristics_count = 0;
    gattlib_characteristic_t** characteristics = nullptr;
    
    // Discover characteristics
    int result = gattlib_discover_char(connection, characteristics, &characteristics_count);
    if (result != GATTLIB_SUCCESS) {
        std::cerr << "Failed to discover characteristics." << std::endl;
        return;
    }

    uuid_t read_uuid, write_uuid;
    stringToUuid(ANKI_STR_CHR_READ_UUID, read_uuid);
    stringToUuid(ANKI_STR_CHR_WRITE_UUID, write_uuid);

    // Inside discoverCharacteristics, update the comparison
    for (int i = 0; i < characteristics_count; ++i) {
        if (compareUuids(characteristics[i]->uuid, write_uuid)) {
            writeChannel = characteristics[i];
            std::cout << "Discovered write channel" << std::endl;
        }
        if (compareUuids(characteristics[i]->uuid, read_uuid)) {
            readChannel = characteristics[i];
            std::cout << "Discovered read channel" << std::endl;
            gattlib_register_notification(connection, read_signal_handler, nullptr); // Register notification
        }
    }

    // Free each characteristic value
    for (int i = 0; i < characteristics_count; ++i) {
        gattlib_characteristic_free_value(characteristics[i]);
    }

    // Free the array of characteristics if allocated
    free(characteristics); // Ensure to free the characteristics array if necessary
}

void VehicleDelegate::setSpeed(int speed, int acceleration) {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_set_speed(&msg, static_cast<uint16_t>(speed), static_cast<uint16_t>(acceleration));
    sendMessage(reinterpret_cast<uint8_t*>(&msg), size);
}

void VehicleDelegate::spin180() {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_turn_180(&msg);
    sendMessage(reinterpret_cast<uint8_t*>(&msg), size);
}

void VehicleDelegate::setEngineLight(uint8_t red, uint8_t green, uint8_t blue) {
    // Construct the lights pattern message
    std::vector<uint8_t> lightsPatternMessage = {0x11, 0x33, 0x03, 0x00, 0x00,
                                                 red, red, 0x00, 0x03, 0x00,
                                                 green, green, 0x00, 0x02, 0x00,
                                                 blue, blue, 0x00};

    // Assuming writeChannel has been set and is valid
    // Peripheral write function to be implemented
}

void VehicleDelegate::sendMessage(const uint8_t* buffer, uint8_t len) {
    if (writeChannel) { // Check if writeChannel is valid
        // Send the message through the peripheral
        // Peripheral write function to be implemented
    }
}

void VehicleDelegate::handleServiceDiscovery() {
    // Implement service discovery logic
}

void VehicleDelegate::handleCharacteristicDiscovery() {
    // Implement characteristic discovery logic
}

void VehicleDelegate::handleValueUpdate(const uint8_t* data, size_t length) {
    anki_vehicle_msg_t msg;
    memcpy(&msg, data, std::min(length, sizeof(msg)));

    // Handle different message types
    switch (msg.msg_id) {
        case ANKI_VEHICLE_MSG_V2C_PING_RESPONSE:
            std::cout << "Ping received from vehicle" << std::endl;
            break;

        case ANKI_VEHICLE_MSG_V2C_VERSION_RESPONSE:
            anki_vehicle_msg_version_response_t versionMsg;
            memcpy(&versionMsg, data, sizeof(versionMsg));
            version = "0x" + std::to_string(versionMsg.version);
            std::cout << "Version response: " << version << std::endl;
            break;

        case ANKI_VEHICLE_MSG_V2C_LOCALIZATION_POSITION_UPDATE: {
            //anki_vehicle_msg_localization_position_update_t updateMsg;
            //memcpy(&updateMsg, data, sizeof(updateMsg));
            // Process localization position update
            //std::string trackName = getTrackName(updateMsg._reserved.1);
            //logTrack(trackName, updateMsg.is_clockwise == 71, updateMsg._reserved.0);
            break;
        }

        case ANKI_VEHICLE_MSG_V2C_LOCALIZATION_TRANSITION_UPDATE:
            // Handle localization transition update
            break;

        case ANKI_VEHICLE_MSG_V2C_VEHICLE_DELOCALIZED:
            std::cout << "Warning: vehicle delocalized id=0x" << std::hex << addr << std::endl;
            break;

        default:
            std::cout << "Unknown message received - 0x" << std::hex << msg.msg_id << std::endl;
            break;
    }
}

void VehicleDelegate::handleWriteConfirmation() {
    // Implement write confirmation logic
}

void VehicleDelegate::logTrack(const std::string& trackName, bool isClockwise, int reserved) {
    loggedTracks.emplace_back(trackName, isClockwise, reserved);
}

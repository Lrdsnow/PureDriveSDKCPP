#include "VehicleDelegate.hpp"
#include <cstring>
#include <vector>
#include <iostream>

// Constructor
VehicleDelegate::VehicleDelegate(void* controller, SimpleBLE::Peripheral peripheral, VehicleAdvData advData)
    : controller(controller), peripheral(peripheral), advData(advData) {
    peripheral.set_callback_on_connected([this]() {
        this->onConnected();
    });
    peripheral.connect();
}

// Method implementations
bool VehicleDelegate::isActive() const {
    return active;
}

void VehicleDelegate::setSDKMode(bool on, uint8_t flags) {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_set_sdk_mode(&msg, on ? 1 : 0, flags);
    sendMessage(&msg, size);
}

void VehicleDelegate::setOffsetFromRoadCenter(float offset) {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_set_offset_from_road_center(&msg, offset);
    sendMessage(&msg, size);
}

void VehicleDelegate::changeLane(uint16_t horizontalSpeed, uint16_t horizontalAccel, float offset) {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_change_lane(&msg, horizontalSpeed, horizontalAccel, offset);
    sendMessage(&msg, size);
}

void VehicleDelegate::setLights(uint8_t mask) {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_set_lights(&msg, mask);
    sendMessage(&msg, size);
}

void VehicleDelegate::setLightsPattern(VehicleLightChannel channel, VehicleLightEffect effect, uint8_t start, uint8_t end, uint16_t cyclesPerMin) {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_lights_pattern(&msg,
        static_cast<anki_vehicle_light_channel_t>(static_cast<uint32_t>(channel)),
        static_cast<anki_vehicle_light_effect_t>(static_cast<uint32_t>(effect)),
        start, end, cyclesPerMin);
    sendMessage(&msg, size);
}

void VehicleDelegate::setEngineLight(uint8_t r, uint8_t g, uint8_t b, VehicleLightEffect effect, uint16_t cycles) {
    switch (effect) {
        case VehicleLightEffect::Throb:
        case VehicleLightEffect::Flash:
            setLightsPattern(VehicleLightChannel::Red, effect, 0, r, cycles);
            setLightsPattern(VehicleLightChannel::Green, effect, 0, g, cycles);
            setLightsPattern(VehicleLightChannel::Blue, effect, 0, b, cycles);
            break;
        case VehicleLightEffect::Fade:
            setLightsPattern(VehicleLightChannel::Red, effect, r, 0, cycles);
            setLightsPattern(VehicleLightChannel::Green, effect, g, 0, cycles);
            setLightsPattern(VehicleLightChannel::Blue, effect, b, 0, cycles);
            break;
        default:
            setLightsPattern(VehicleLightChannel::Red, effect, r, r, 0);
            setLightsPattern(VehicleLightChannel::Green, effect, g, g, 0);
            setLightsPattern(VehicleLightChannel::Blue, effect, b, b, 0);
            break;
    }
}

void VehicleDelegate::disconnect() {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_disconnect(&msg);
    sendMessage(&msg, size);
}

void VehicleDelegate::ping() {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_ping(&msg);
    sendMessage(&msg, size);
}

void VehicleDelegate::getVersion() {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_get_version(&msg);
    sendMessage(&msg, size);
}

void VehicleDelegate::getBatteryLevel() {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_get_battery_level(&msg);
    sendMessage(&msg, size);
}

void VehicleDelegate::cancelLaneChange() {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_cancel_lane_change(&msg);
    sendMessage(&msg, size);
}

void VehicleDelegate::turn180() {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_turn_180(&msg);
    sendMessage(&msg, size);
}

void VehicleDelegate::setSpeed(int speed, int acceleration) {
    anki_vehicle_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    uint8_t size = anki_vehicle_msg_set_speed(&msg, static_cast<uint16_t>(speed), static_cast<uint16_t>(acceleration));
    sendMessage(&msg, size);
}

void VehicleDelegate::sendMessage(void* buffer, uint8_t len) {
    if (!writeChannel || !rwService) {
        return;
    }
    SimpleBLE::Characteristic &writeChar = writeChannel.value();
    SimpleBLE::Service &rwServ = rwService.value();
    if (writeChar.can_write_command()) {
        SimpleBLE::ByteArray valData(static_cast<uint8_t*>(buffer), len);
        peripheral.write_command(rwServ.uuid(), writeChar.uuid(), valData);
        if (debug_mode) {
            std::cout << "Sent: ";
            for (int i = 0; i < len; ++i) {
                std::cout << std::hex << (static_cast<uint8_t*>(buffer)[i]) << " ";
            }
            std::cout << std::endl;
        }
    }
}

void VehicleDelegate::onConnected() {
    // Discover services on connection
    auto services = peripheral.services();
    for (auto& service : services) {
        std::cout << "Discovered service: " << service.uuid() << std::endl;
        auto characteristics = service.characteristics();

        for (auto& characteristic : characteristics) {
            std::cout << "Discovered characteristic: " << characteristic.uuid() << std::endl;
            if (characteristic.uuid() == ankiChrWriteUUID) {
                writeChannel = characteristic;
                rwService = service;
                printf("set sdk mode...\n");
                setSDKMode(true, ANKI_VEHICLE_SDK_OPTION_OVERRIDE_LOCALIZATION);
                printf("set speed...\n");
                setSpeed(500, 500);
            } else if (characteristic.uuid() == ankiChrReadUUID) {
                readChannel = characteristic;
                rwService = service;
                peripheral.notify(service.uuid(), characteristic.uuid(), onMessageReceived);
            }
        }
    }
}

void VehicleDelegate::onMessageReceived(SimpleBLE::ByteArray payload) {

}


// void VehicleDelegate::onDiscoverServices(SimpleBLE::Peripheral& peripheral, const std::string& error) {
//     if (!error.empty()) {
//         std::cout << "Discovered services for " << peripheral.getName() << " with error: " << error << std::endl;
//         return;
//     }

//     auto services = peripheral.getServices();
//     if (services.empty()) return;

//     std::cout << "Discovered service, Id=" << services[0].getUUID() << std::endl;
//     peripheral.discoverCharacteristics(services[0]);
// }

// void VehicleDelegate::onDiscoverCharacteristics(SimpleBLE::Peripheral& peripheral, SimpleBLE::Service& service, const std::string& error) {
//     if (!error.empty()) {
//         std::cout << "Discovered characteristics for " << service.getUUID() << " with error: " << error << std::endl;
//         return;
//     }

//     auto characteristics = service.getCharacteristics();
//     for (auto& characteristic : characteristics) {
//         if (characteristic.getUUID() == ankiChrWriteUUID) {
//             writeChannel = characteristic;
//             std::cout << "Discovered write channel" << std::endl;
//         }
//         if (characteristic.getUUID() == ankiChrReadUUID) {
//             readChannel = characteristic;
//             std::cout << "Discovered read channel" << std::endl;
//         }
//     }

//     peripheral.setNotify(readChannel, true);

//     anki_vehicle_msg_t msg;
//     memset(&msg, 0, sizeof(msg));
//     uint8_t size = anki_vehicle_msg_set_sdk_mode(&msg, 1, static_cast<uint8_t>(ANKI_VEHICLE_SDK_OPTION_OVERRIDE_LOCALIZATION));
//     sendMessage(&msg, size);

//     memset(&msg, 0, sizeof(msg));
//     anki_vehicle_msg_get_version(&msg);
//     sendMessage(&msg, size);

//     active = true;
// }

// void VehicleDelegate::onUpdateValue(SimpleBLE::Peripheral& peripheral, SimpleBLE::Characteristic& characteristic, const SimpleBLE::Data& data, const std::string& error) {
//     if (!error.empty()) {
//         std::cout << "Error updating value for characteristic " << characteristic.getUUID() << " error: " << error << std::endl;
//         return;
//     }

//     if (data.size() == 0) {
//         std::cout << "No data available for characteristic " << characteristic.getUUID() << std::endl;
//         return;
//     }

//     anki_vehicle_msg_t msg;
//     memset(&msg, 0, sizeof(msg));
//     uint8_t len = data.size();
//     memcpy(msg.data, data.data(), len);

//     // Handle the message
//     handleIncomingMessage(msg);
// }

// void VehicleDelegate::handleIncomingMessage(anki_vehicle_msg_t& msg) {
//     // Process the incoming message
//     // Implementation depends on the specifics of anki_vehicle_msg_t
// }

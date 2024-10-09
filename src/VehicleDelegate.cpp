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
    std::cout << "Connected" << std::endl;
    auto services = peripheral.services();
    for (auto& service : services) {
        std::cout << "Discovered service: " << service.uuid() << std::endl;
        auto characteristics = service.characteristics();

        for (auto& characteristic : characteristics) {
            std::cout << "Discovered characteristic: " << characteristic.uuid() << std::endl;
            if (characteristic.uuid() == ankiChrWriteUUID) {
                writeChannel = characteristic;
                rwService = service;
                setSDKMode(true, ANKI_VEHICLE_SDK_OPTION_OVERRIDE_LOCALIZATION);
                getBatteryLevel();
            } else if (characteristic.uuid() == ankiChrReadUUID) {
                readChannel = characteristic;
                rwService = service;
                peripheral.notify(service.uuid(), characteristic.uuid(), 
                    [this](SimpleBLE::ByteArray payload) { 
                        this->onMessageReceived(payload); 
                });
            }
        }
    }
}

void VehicleDelegate::onMessageReceived(SimpleBLE::ByteArray data) {
    anki_vehicle_msg_t msg;
    std::memcpy(&msg, data.data(), std::min(data.size(), sizeof(anki_vehicle_msg_t)));

    switch (msg.msg_id) {
        case ANKI_VEHICLE_MSG_V2C_PING_RESPONSE:
            std::cout << "Ping received from vehicle" << std::endl;
            break;

        case ANKI_VEHICLE_MSG_V2C_VERSION_RESPONSE: {
            anki_vehicle_msg_version_response_t versionMsg;
            std::memcpy(&versionMsg, data.data(), std::min(data.size(), sizeof(anki_vehicle_msg_version_response_t)));
            std::string version = "0x" + kvn::bytearray::fromHex(std::to_string(versionMsg.version)).toHex();
            std::cout << "Version response: " << version << std::endl;
            break;
        }

        case ANKI_VEHICLE_MSG_V2C_LOCALIZATION_POSITION_UPDATE: {
            anki_vehicle_msg_localization_position_update_t updateMsg;
            std::memcpy(&updateMsg, data.data(), std::min(data.size(), sizeof(anki_vehicle_msg_localization_position_update_t)));
            std::cout << "Position Update:\n";
            std::cout << "Size: " << static_cast<int>(updateMsg.size) << std::endl;
            std::cout << "Msg ID: " << static_cast<int>(updateMsg.msg_id) << std::endl;
            std::cout << "Reserved: [" 
                    << static_cast<int>(updateMsg._reserved[0]) << ", " 
                    << static_cast<int>(updateMsg._reserved[1]) << "]" << std::endl;
            std::cout << "Offset From Road Center (mm): " << updateMsg.offset_from_road_center_mm << std::endl;
            std::cout << "Speed (mm/s): " << updateMsg.speed_mm_per_sec << std::endl;
            std::cout << "Is Clockwise: " << static_cast<int>(updateMsg.is_clockwise) << std::endl;
            std::string track_name = getTrackName(updateMsg._reserved[1]);
            std::cout << "Track name: " << track_name << std::endl;
            break;
        }

        case ANKI_VEHICLE_MSG_V2C_LOCALIZATION_TRANSITION_UPDATE: {
            anki_vehicle_msg_localization_transition_update_t transitionMsg;
            std::memcpy(&transitionMsg, data.data(), std::min(data.size(), sizeof(anki_vehicle_msg_localization_transition_update_t)));
            std::cout << "Localization Transition Update:\n";
            std::cout << "Size: " << static_cast<int>(transitionMsg.size) << std::endl;
            std::cout << "Msg ID: " << static_cast<int>(transitionMsg.msg_id) << std::endl;
            std::cout << "Reserved: " << static_cast<int>(transitionMsg._reserved) << std::endl;
            std::cout << "Offset From Road Center (mm): " << transitionMsg.offset_from_road_center_mm << std::endl;
            std::cout << "Is Clockwise: " << static_cast<int>(transitionMsg.is_clockwise) << std::endl;
            break;
        }

        case ANKI_VEHICLE_MSG_V2C_VEHICLE_DELOCALIZED:
            std::cout << "Warning: vehicle delocalized" << std::endl;
            break;

        case ANKI_VEHICLE_MSG_V2C_BATTERY_LEVEL_RESPONSE: {
            anki_vehicle_msg_battery_level_response_t batteryMsg;
            std::memcpy(&batteryMsg, data.data(), std::min(data.size(), sizeof(anki_vehicle_msg_battery_level_response_t)));
            std::cout << "Battery Level: " << batteryMsg.battery_level << std::endl;
            break;
        }

        case ANKI_VEHICLE_MSG_V2C_STATUS_UPDATE: {
            anki_vehicle_msg_status_update_t statusUpdateMsg;
            std::memcpy(&statusUpdateMsg, data.data(), std::min(data.size(), sizeof(anki_vehicle_msg_status_update_t)));
            std::cout << "Status Update:\n";
            std::cout << "Size: " << static_cast<int>(statusUpdateMsg.size) << std::endl;
            std::cout << "Msg ID: " << static_cast<int>(statusUpdateMsg.msg_id) << std::endl;
            std::cout << "Reserved: " << static_cast<int>(statusUpdateMsg.reserved0) << std::endl;
            std::cout << "On Charger: " << static_cast<int>(statusUpdateMsg.on_charger) << std::endl;
            std::cout << "Battery Low: " << static_cast<int>(statusUpdateMsg.battery_low) << std::endl;
            std::cout << "Battery Full: " << static_cast<int>(statusUpdateMsg.battery_full) << std::endl;
            break;
        }

        case ANKI_VEHICLE_MSG_V2C_CAR_MESSAGE_CYCLE_OVERTIME:
            // Handle cycle overtime if needed
            break;

        case ANKI_VEHICLE_MSG_V2C_CAR_COLLISION:
            std::cout << "Collision!" << std::endl;
            break;

        case ANKI_VEHICLE_MSG_V2C_CAR_ERROR:
            std::cout << "Car Error" << std::endl;
            break;

        default:
            std::cout << "Unknown message received - 0x" << std::hex << msg.msg_id
                      << ", size " << msg.size << std::endl;
            std::cout << "Message: " << data.toHex(true) << std::endl;
            break;
    }
}

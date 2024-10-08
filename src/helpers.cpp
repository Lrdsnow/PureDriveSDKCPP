#include "helpers.hpp"

// Utility functions
std::string getCarName(uint8_t modelId) {
    switch (modelId) {
        case 0x01: return "kourai";
        case 0x02: return "boson";
        case 0x03: return "rho";
        case 0x04: return "katal";
        case 0x05: return "hadion";
        case 0x06: return "spektrix";
        case 0x07: return "corax";
        case 0x08: return "groundshock";
        case 0x09: return "skull";
        case 0x0A: return "thermo";
        case 0x0B: return "nuke";
        case 0x0C: return "guardian";
        case 0x0E: return "bigbang";
        case 0x0F: return "freewheel";
        case 0x10: return "x52";
        case 0x11: return "x52ice";
        case 0x12: return "mammoth";
        case 0x13: return "dynamo";
        case 0x14: return "ghost";
        default: return "unknown";
    }
}

// Basic track name utility function
std::string getTrackName(uint8_t trackId) {
    switch (trackId) {
        case 36: case 39: case 40: case 51: return "Straight";
        case 17: case 18: case 20: case 23: return "Curve";
        case 34: return "Pre-Finish Line";
        case 33: return "Start/Finish";
        case 57: return "FnF Powerup";
        case 10: return "Intersection";
        default: return "Unknown";
    }
}

std::string getGattlibErrorString(int error_code) {
    switch (error_code) {
        case GATTLIB_SUCCESS:
            return "Success";
        case GATTLIB_INVALID_PARAMETER:
            return "Invalid Parameter";
        case GATTLIB_NOT_FOUND:
            return "Not Found";
        case GATTLIB_TIMEOUT:
            return "Timeout";
        case GATTLIB_OUT_OF_MEMORY:
            return "Out of Memory";
        case GATTLIB_NOT_SUPPORTED:
            return "Not Supported";
        case GATTLIB_DEVICE_ERROR:
            return "Device Error";
        case GATTLIB_DEVICE_NOT_CONNECTED:
            return "Device Not Connected";
        case GATTLIB_NO_ADAPTER:
            return "No Adapter";
        case GATTLIB_BUSY:
            return "Busy";
        case GATTLIB_UNEXPECTED:
            return "Unexpected Error";
        case GATTLIB_ADAPTER_CLOSE:
            return "Adapter Closed";
        case GATTLIB_DEVICE_DISCONNECTED:
            return "Device Disconnected";
        default:
            if (error_code & GATTLIB_ERROR_MODULE_MASK) {
                if (error_code & GATTLIB_ERROR_DBUS) {
                    return "DBus Error";
                } else if (error_code & GATTLIB_ERROR_BLUEZ) {
                    return "BlueZ Error";
                } else if (error_code & GATTLIB_ERROR_UNIX) {
                    return "Unix Error";
                } else if (error_code & GATTLIB_ERROR_INTERNAL) {
                    return "Internal Error";
                } else {
                    return "Unknown Module Error";
                }
            } else {
                std::ostringstream oss;
                oss << "Unknown Error (code: " << error_code << ")";
                return oss.str();
            }
    }
}
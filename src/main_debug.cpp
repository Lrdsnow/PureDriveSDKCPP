#include "BluetoothManager.hpp"

int main() {
    BluetoothManager manager;
    manager.startScanning();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    manager.stopScanning();
    return 0;
}

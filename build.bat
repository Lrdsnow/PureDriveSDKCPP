cl /c src/anki_sdk/protocol.c src/BluetoothManager.cpp src/helpers.cpp src/main_debug.cpp src/Track.cpp src/VehicleDelegate.cpp /I"C:\Program Files (x86)\simpleble\include" /std:c++17
link /OUT:build/bin/PureDrive.exe protocol.obj BluetoothManager.obj helpers.obj main_debug.obj Track.obj VehicleDelegate.obj /link /LIBPATH:"C:\Program Files (x86)\simpleble\lib" simpleble-debug.lib simpleble-c-debug.lib
del /Q *.obj
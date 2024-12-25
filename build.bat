mkdir build
cl /c src/anki_sdk/protocol.c src/BluetoothManager.cpp src/helpers.cpp src/main_debug.cpp src/Track.cpp src/VehicleDelegate.cpp /Ilib\include /std:c++17
link /OUT:build/PureDrive.exe protocol.obj BluetoothManager.obj helpers.obj main_debug.obj Track.obj VehicleDelegate.obj /link /LIBPATH:lib\lib simpleble.lib simpleble-c.lib
del /Q *.obj
copy lib\bin\* build
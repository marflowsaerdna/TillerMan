#include "MyBleAdvCallbacks.h"
#include "MyBleClient.h"

void MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
    MyBleClient::deviceFoundStatic(advertisedDevice);
}
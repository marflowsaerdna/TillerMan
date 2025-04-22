#ifndef MYADVERTISEDDEVICECALLBACKS_H
#define MYADVERTISEDDEVICECALLBACKS_H

#include <BLEDevice.h>
#include "MyBleClient.h"

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {

public:
    void onResult(BLEAdvertisedDevice advertisedDevice) override;
};

#endif
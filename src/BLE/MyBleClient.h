#ifndef MYBLECLIENT_H
#define MYBLECLIENT_H

#include <Arduino.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include "../UI/MyIoPort.h"
#include "../BLE/MyBleAdvCallbacks.h"


// put function declarations here:
#define DEVICE_NAME "TillerDevice"

class MyBleClient {

    public:
        enum BleStatus
        {
            CRASH,
            INIT,
            SCANNING,
            CONNECTING, // 1
            CONNECTED,
            DATA_TRANSFER
        };

        enum BleWorkToDo
        {
            DATA_RECEIVED,
            STATE_MACHINE
        };

        struct __attribute__((packed)) ServerData {
            short MsgCounter;
            short courseChange;
            short AWAsoll;
            short AWA; 
            short TWA;
            short tackAngle;
        };

        ServerData serverData;

        #define BLE_TIMEOUT 10000           // 10s

        static MyBleClient* instance;
        static boolean led_state;
        BleStatus connStatus;

        boolean deviceAndServiceFound;
        boolean connected;

        typedef void (*CallbackFunctionStatic)(uint8_t *pData); // Definiere den Typ für den Funktionszeiger

        MyBleClient();
        void sendMessage(String message);
        void sendData(uint8_t *pData, int length);
        static void loopTaskStatic(void *pvParameters);
        static void dataNotifyStatic(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
        static void deviceFoundStatic(BLEAdvertisedDevice advertisedDevice);
        void deviceFound(BLEAdvertisedDevice advertisedDevice);
        void begin();
        void end();
        void loop();


    private:
        int value;
        unsigned long timeStamp;


        BLEServer *pServer;
        BLEClient *pClient;
        BLEAdvertising *pAdvertising;

        TaskHandle_t  bleTaskHandle;  // Handle für die Task

        uint8_t dataByteBuffer[20]; // Statischer Bytebuffer


        BLERemoteCharacteristic *pRemoteCharacteristic;




        void initBLE();
        void startScan();
        void connectToServer();
        void startCommunication();
        bool checkTimeout(void);
        void resetTimeout(void);

        BLEAdvertisedDevice *myDevice;
        CallbackFunctionStatic mainCallback; // Speichert den Callback


};

#endif
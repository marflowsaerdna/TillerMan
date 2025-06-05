#include <Arduino.h>
#include "MyBleClient.h"
#include "ServerDataFifo.h"
#include "hw_def.h"

static BLEUUID serviceUUID("00001810-0000-1000-8000-00805F9B34FB"); // Ersetze mit der Service-UUID
static BLEUUID charUUID("00002A6E-0000-1000-8000-00805F9B34FB");    // Ersetze mit der Charakteristik-UUID

static bool doSomething = false;

MyBleClient* MyBleClient::instance = nullptr;

MyBleClient::MyBleClient()
{ 

    instance = this;
    connStatus = INIT;

    timeStamp = millis();

}

void MyBleClient::end() {

}

bool MyBleClient::checkTimeout(void)
{
    if ((millis() - timeStamp) > BLE_TIMEOUT)
        return true;
    else
        return false;
}

void MyBleClient::resetTimeout(void)
{
    timeStamp = millis();
}

void MyBleClient::loop()
{
    if (connStatus != DATA_TRANSFER)
    {

        delay(1000);
        Serial.println("No Connection");
    }
    if (connStatus == CRASH)
    {
       // ESP.restart();
    }
    if (connStatus == INIT)
    {
        Serial.println("Init BLE...");
        resetTimeout();
        BLEDevice::deinit();
        delay(1000);
        BLEDevice::init(DEVICE_NAME);  // BLE neu starten
        pServer = BLEDevice::createServer();

        startScan();
        connStatus = SCANNING;
    }
    if (connStatus == SCANNING)
    {
        if (checkTimeout())
        {
            Serial.println("❌ Device and Service not found!");
            connStatus = INIT;
        }
    }
    if (instance->connStatus == CONNECTING)
    {
        Serial.println("✅ Passendes Gerät gefunden!");
        connectToServer();
        if (checkTimeout())
        {
            Serial.println("❌ Timeout!");
            connStatus = INIT;
        }
    }
    if (connStatus == CONNECTED)
    {
        startCommunication();
        if (checkTimeout())
        {
            Serial.println("❌ Timeout!");
            pClient->disconnect();
            connStatus = INIT;
        }
        else{
            connStatus = DATA_TRANSFER;
        }
    }
    if (connStatus == DATA_TRANSFER)
    {
        if (!pClient->isConnected())
        {
            Serial.println("❌ Verbindung verloren! - starte neu");
            connStatus = CONNECTING;
        }

        if (checkTimeout())
        {
            Serial.println("❌ Timeout!");
            resetTimeout();
            pClient->disconnect();
            connStatus = CONNECTED;
        }
    }
}


void MyBleClient::deviceFoundStatic(BLEAdvertisedDevice advertisedDevice)
{
    if (instance) {
        instance->deviceFound(advertisedDevice);
    }
}

void MyBleClient::deviceFound(BLEAdvertisedDevice advertisedDevice)
{
    // Serial.print("Gefundenes Gerät: ");
    // Serial.println(advertisedDevice.toString().c_str());

    resetTimeout();

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {
        BLEDevice::getScan()->stop();
        delay(1000);
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        // Serial.println("✅ Passendes Gerät gefunden!");
        connStatus = CONNECTING;
    }
    else{
        myDevice = nullptr;
    }
}

void MyBleClient::dataNotifyStatic(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    // Serial.println("Daten empfangen!");
    if (instance) {
        instance->resetTimeout();
        bool returnwert = ServerDataFifo::getInstance().set(*pData); // Daten in FIFO speichern

        instance->connStatus = DATA_TRANSFER;
    }
}

void MyBleClient::startScan()
{
    Serial.println("Start scan...");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

void MyBleClient::sendMessage(String message)
{
    if (connStatus == DATA_TRANSFER)
    {
        if (pRemoteCharacteristic->canWrite()) {
            pRemoteCharacteristic->writeValue(("T" + message).c_str());
            // Serial.println("✅ Nachricht gesendet!");
        } else {
            Serial.println("❌ Charakteristik unterstützt kein Schreiben!");
        }
    }
    else
    {
        Serial.println("❌ Transfer nicht aktiv!");
    }
}

void MyBleClient::sendData(uint8_t *pData, int length)
{
    if (connStatus == DATA_TRANSFER)
    {
        if (pRemoteCharacteristic->canWrite()) {
            pRemoteCharacteristic->writeValue(pData, length);
            // Serial.println("✅ Daten gesendet!");
        } else {
            Serial.println("❌ Charakteristik unterstützt kein Schreiben!");
        }
    }
    else
    {
        Serial.println("❌ Transfer nicht aktiv!");
    }
}

void MyBleClient::connectToServer()
{
    Serial.println("Verbindungsversuch...");
    if (myDevice)
    {
        pClient = BLEDevice::createClient();
        Serial.println("Client erstellt");

        if (pClient->isConnected())
        {
            Serial.println("✅ Verbindung aktiv!");
            connStatus = CONNECTED;
        }  
        else{
            Serial.println("Verbinde mit Gerät...");
            if (!pClient->connect(myDevice))
            {
                Serial.println("❌ Verbindung fehlgeschlagen!");
                pServer->disconnect(pServer->getConnId());
                connStatus = INIT; // CRASH;
                return;
            }        
            else
            {
                Serial.println("✅ Verbindung aktiv!");
                connStatus = CONNECTED;
            }   
        }
    }
    else{
        Serial.println("❌ Device verloren!");
        connStatus = SCANNING;
    }


}

void MyBleClient::startCommunication()
{
    if (!pClient->isConnected())
    {
        connStatus = SCANNING;
        return;
    }


    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService != nullptr)
    {
        Serial.println("✅ Service gefunden, suche Charakteristik !");
    }
    else
    {
        Serial.println("❌ Service nicht gefunden!");
        return;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic != nullptr)
    {
        Serial.println("✅ Charakteristik gefunden, hole cccd !");
    }
    else
    {
        Serial.println("❌ Charakteristik nicht gefunden!");
        return;
    }

    /*
    BLERemoteDescriptor *p2902 = pRemoteCharacteristic->getDescriptor(BLEUUID("00002902-0000-1000-8000-00805f9b34fb"));
    if (p2902 != nullptr)
    {
        uint8_t notifyEnable[] = {0x01, 0x00}; // Aktiviert Notifications
        p2902->writeValue(notifyEnable, 2);
        Serial.println("✅ CCCD-Descriptor erfolgreich gesetzt!");
    }
    else
    {
        Serial.println("❌ Kein CCCD-Descriptor gefunden!");
    }
    */
    if (pRemoteCharacteristic->canNotify())
    {
        Serial.println("✅ Charakteristik unterstützt notify, register callback!");
        pRemoteCharacteristic->registerForNotify(dataNotifyStatic);
        Serial.println("✅ Für Notifications registriert!");
    }
    else
    {
        Serial.println("❌ Charakteristik unterstützt kein Notify!");
        return;
    }
}
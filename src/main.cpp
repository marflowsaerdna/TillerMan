#include "hw_def.h"
#include <Arduino.h>
#include "BLE/MyBleClient.h"
#include "BLE/ServerDataFifo.h"
#include "UI/MyIoPort.h"
#include "UI/StdbyWatch.h"
#include "UI/InputParser.h"
// #include "BLE/SerialWrapper.h"
#include "TillerMan.h"

extern "C" {
    #include "esp_debug_helpers.h"
  }

#ifdef BOOT_WITH_OTA
    #include "OTA/MyOta.h"
#endif

MyBleClient* bleClient;
InputParser* inputParser;
TillerMan*   tillerMan;
ServerDataFifo* serverDataFifo; // FIFO für Server-Daten, Größe 10
// DebugOutput* debugOut;

struct MyBleClient::ServerData  serverData;
InputParser::UserInput userInput;
MyIoPort* indicatorLed;
short msgCount = 0;



void onUserInput(InputParser::UserInput input)        // Callback für Unser Input, wird aus einer anderen Task aufgerufen
{
    tillerMan->setUserInput(input);
}

void myMainTask(void *pvParameters)
{
    inputParser = new InputParser(bleClient, onUserInput);
    tillerMan = new TillerMan(inputParser);

    while(true)
    {
        delay(100);

        if (bleClient->connStatus == MyBleClient::DATA_TRANSFER)
        {
            // Serial.printf("Heap: %d, Min: %d\n", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
            tillerMan->mainLoop();
            // Serial.println("TillerLoop finish");
        }
    }
}

void myBlinkTask(void *pvParameters)
{
    MyIoPort* indicatorLed  = new MyIoPort(ACTIVE_LED, OUTPUT, HIGH, LOW);  // Indicator LED

    while(true)
    {
        delay(100);

        if (bleClient->connStatus != MyBleClient::DATA_TRANSFER)
        {
            indicatorLed->pulse(1000);
            delay(2000);
        }
        else
        {
            indicatorLed->pulse(250);
            delay(500);
        }
    }
}

void loop()
{  
        // OTA in Hauptschleife, damit OTA auch funktioniert, wenn keine Verbindung besteht
        #ifdef BOOT_WITH_OTA
            MyOtaLoop();
            // Serial.println("OTALoop");
        #endif

        bleClient->loop();
        // Serial.println("BLELoop finish");
        // Serial.printf("FIFO Info: %d Elemente vorhanden\n", ServerDataFifo::getInstance().getInfo());
        delay(100);
}

void setup()
{    // put your setup code here, to run once:

    delay(500);
    Serial.begin(115200);   

    #ifdef BOOT_WITH_OTA
        MyOtaSetup();
        Serial.println("OTA Setup");
    #endif

    // BLE muss zuerst initialisiert werden, weil debug-Ausgaben evtl. über BLE laufen
    bleClient = new MyBleClient();
    
    serverDataFifo = new ServerDataFifo(10); // Initialisiere die FIFO Instanz mit Größe 10
    // Debug.initDebugOutput(bleClient);
    
    // Starte eigene Task mit größerem Stack
    xTaskCreatePinnedToCore(
        myMainTask,      // Task-Funktion
        "MainTask",      // Name
        4096,            // Stackgröße in Bytes (Minimum: 2048)
        NULL,            // Parameter
        1,               // Priorität
        NULL,            // Handle (optional)
        0                // Core 0 oder 1
    );

        // Starte eigene Task mit größerem Stack
    xTaskCreatePinnedToCore(
        myBlinkTask,      // Task-Funktion
        "BlinkTask",      // Name
        2048,            // Stackgröße in Bytes (Minimum: 2048)
        NULL,            // Parameter
        1,               // Priorität
        NULL,            // Handle (optional)
        1                // Core 0 oder 1
    );
    
}


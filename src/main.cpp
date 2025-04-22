#include "hw_def.h"
#include <Arduino.h>
#include "BLE/MyBleClient.h"
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
// DebugOutput* debugOut;

struct MyBleClient::ServerData  serverData;
InputParser::UserInput userInput;
MyIoPort* indicatorLed;
short msgCount = 0;


void onNewBleData(uint8_t *pData)        // Callback für neue BLE Daten, wird aus einer anderen Task aufgerufen
{
    memcpy(&serverData, pData, sizeof(MyBleClient::ServerData)); 

    tillerMan->setServerData(serverData);

    if ((serverData.MsgCounter - msgCount) > 1)
    {
        Serial.println("Daten wurden verloren !");
    }
    msgCount = serverData.MsgCounter;

    Serial.print(serverData.MsgCounter);  
    Serial.print(":  Cch:");
    Serial.print(serverData.courseChange); 
    Serial.print("  Ach:");
    Serial.print(serverData.awsChange);
    Serial.print("  AWA:");
    Serial.print(serverData.AWA);
    Serial.print("  TWA:");
    Serial.print(serverData.TWA);
    Serial.print("  Tck:");
    Serial.println(serverData.tackAngle);

    short MsgCounter;
    short courseChange;
    short awsChange;
    short AWA; 
    short TWA;
    short tackAngle;
}



void onUserInput(InputParser::UserInput input)        // Callback für Unser Input, wird aus einer anderen Task aufgerufen
{
    tillerMan->setUserInput(input);
}


void setup()
{    // put your setup code here, to run once:

    delay(500);
    Serial.begin(115200);   

    // gdb_start();

    // BLE muss zuerst initialisiert werden, weil debug-Ausgaben evtl. über BLE laufen
    bleClient = new MyBleClient(onNewBleData);
    
    // Debug.initDebugOutput(bleClient);

    #ifdef BOOT_WITH_OTA
        MyOtaSetup();
    #endif

    inputParser = new InputParser(bleClient, onUserInput);
    tillerMan = new TillerMan(inputParser);
 
}

void loop()
{  
    #ifdef BOOT_WITH_OTA
        MyOtaLoop();
    #endif

    delay(100);

    bleClient->loop();

    if (bleClient->connStatus == MyBleClient::DATA_TRANSFER)
    {
        tillerMan->mainLoop();
    }
}
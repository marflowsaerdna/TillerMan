 #include "MyOta.h"
 
 #ifdef BOOT_WITH_OTA
    // WLAN-Zugangsdaten für den Access Point
    const char* ssid = "ESP32_OTA_AP";
    const char* password = "12345678";  // Mindestens 8 Zeichen


    // Webserver auf Port 80
    WebServer server(80);
#endif

 void MyOtaSetup() {

 #ifdef BOOT_WITH_OTA


    Serial.println("\nStarte ESP32 Access Point für OTA...");

    // ESP32 als Access Point einrichten
    WiFi.softAP(ssid, password);
    Serial.print("Access Point gestartet! IP-Adresse: ");
    Serial.println(WiFi.softAPIP());

    // Root-Seite
    server.on("/", []() {
        server.send(200, "text/html", "<h1>ESP32 OTA Update</h1><p><a href='/update'>Update durchführen</a></p>");
    });

    // ElegantOTA aktivieren
    ElegantOTA.begin(&server);
    server.begin();
    Serial.println("Webserver gestartet!"); 
#endif
}

void MyOtaLoop() 
{
#ifdef BOOT_WITH_OTA
    server.handleClient();
    ElegantOTA.loop();
#endif
}

#include <Arduino.h>
#include <WiFi.h>
#include <setUp/rgbControler/rgbControler.h>
#include <setUp/wifiSetUp/WifiSetUp.h>
#include "MinecraftFunctions.h"

constexpr int SERVER_PORT = 25565;
constexpr unsigned long TICK_INTERVAL = 50; 
WiFiServer server(SERVER_PORT);
unsigned long lastTick = 0;

void setup() {
    Serial.begin(115200);
    WiFIsetUP(); 
    server.begin();
    InitChunksAroundZero();
    Serial.print("Serwer nasłuchuje na porcie: ");
    Serial.println(SERVER_PORT);
}

bool enabled = true;
void loop() {
    unsigned long now = millis();
    if (now - lastTick >= TICK_INTERVAL) {
        unsigned long tickStart = millis();
        lastTick = now;

       
        MinecraftServerTick(server);

        
        unsigned long tickEnd = millis();
        unsigned long tickDuration = tickEnd - tickStart;

        if (tickDuration < TICK_INTERVAL) {
            delay(TICK_INTERVAL - tickDuration);
        }

        float cpuUsage = (tickDuration / float(TICK_INTERVAL)) * 100.0;
        if (cpuUsage > 25.0&&enabled == true) {
            Serial.print("Tick duration: "); Serial.print(tickDuration);
            Serial.print(" ms | CPU usage: "); Serial.print(cpuUsage); Serial.println("%");
            if (cpuUsage > 100) {
                Serial.println("Tick overrun! Server is lagging.");
            }
        }
    }
}

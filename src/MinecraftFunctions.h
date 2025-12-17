#ifndef MINECRAFT_FUNCTIONS_H
#define MINECRAFT_FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <string>

struct Player {
    String name = "";
    int connectionState = -1;
};

struct Connection {
    uint32_t id;            
    Player* player = nullptr; 
    WiFiClient client;       // klient jest teraz dostępny globalnie
    unsigned long connectionStart;
    int8_t connectionState = -1;
    uint32_t packetCounter;
};

extern std::vector<Connection> Connections;
extern uint32_t nextConnectionId;

void HandlePacket(Connection &conn, const uint8_t* data, size_t length); 
void MinecraftServerTick(WiFiServer &server);

#endif

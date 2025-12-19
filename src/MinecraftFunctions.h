#ifndef MINECRAFT_FUNCTIONS_H
#define MINECRAFT_FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <string>

struct Player {
    String name = "";
    String uuid = "";
    int connectionState = -1;
    int entityId =-1;


    //inf0
    String brand = "";
    String locale = "";
    int8_t viewDistance = -1;
    int chatMode = 0;
    bool chatColors = true;
    uint8_t displayedSkinParts = 0;
    int mainHand = -1;
    bool textFiltering = false;
    bool allowServerListings;
    int particleStatus = 0;

    int gamemode = -1;;
};

struct Connection {
    uint32_t id;            
    Player* player = nullptr; 
    WiFiClient client;    
    unsigned long connectionStart;
    int8_t connectionState = -1;
    uint32_t packetCounter;
    std::vector<uint8_t> recvBuffer;

    uint32_t lastKeepAliveSent = 0;   
    uint32_t lastKeepAliveRecv = 0; 
    int64_t  lastKeepAliveId   = 0;  
};

//##################CHUNKS##################
#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Z 16
#define CHUNK_HEIGHT 255
#define CHUNK_VOLUME (CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_HEIGHT)

#define SIMULATION_DISTANCE 2  
#define CHUNK_RADIUS (SIMULATION_DISTANCE - 1)
#define CHUNK_COUNT ((CHUNK_RADIUS * 2 + 1) * (CHUNK_RADIUS * 2 + 1))

struct Chunk {
    int32_t posX;
    int32_t posZ;
    uint32_t tickingTime;
    std::vector<uint8_t> data;

    Chunk() : posX(0), posZ(0), tickingTime(0), data(CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_HEIGHT, 0) {}
};

extern std::vector<Chunk> chunks;


void InitChunksAroundZero();
Chunk* getChunk(int32_t x, int32_t z);
//##################CHUNKS##################

extern std::vector<Connection> Connections;
extern uint32_t nextConnectionId;

void HandlePacket(Connection &conn, const uint8_t* data, size_t length); 
void MinecraftServerTick(WiFiServer &server);

#endif

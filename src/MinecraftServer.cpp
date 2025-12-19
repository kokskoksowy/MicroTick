#include "MinecraftFunctions.h"
#include <registries/registries.h>
#include <settings.h>
#include "functions/functions.h"
#include <settings.h>
// globals
std::vector<Connection> Connections;
uint32_t nextConnectionId = 0;
const String statusJson = R"json({
    "version": {
        "name": "1.21.8",
        "protocol": 772
    },
    "players": {
        "max": 2137,
        "online": 69,
        "sample": [
            {
                "name": "wbijaj kórwo",
                "id": "0541ed27-7595-4e6a-9101-6c07f879b7b5"
            }
        ]
    },
    "description": {
        "text": "niGGGGGGGGEr\npierdol się"
    },
    "favicon": "data:image/png;base64,<data>",
    "enforcesSecureChat": false
})json";



void sendStatusResponse(WiFiClient &client) {
    Serial.print("(sendStatusResponse) ");
    std::vector<uint8_t> payload;
    appendVarInt(payload, 0); // packet ID = 0x00
    appendVarInt(payload, statusJson.length());
    for (size_t i = 0; i < statusJson.length(); ++i) payload.push_back((uint8_t)statusJson[i]);

    std::vector<uint8_t> packet;
    appendVarInt(packet, payload.size());
    packet.insert(packet.end(), payload.begin(), payload.end());

    client.write(packet.data(), packet.size());
    client.flush();
}
void handleHandshakePacket(Connection &conn, const uint8_t* data, size_t length) {
    size_t offset = 0;

    int32_t protocolVer = readVarInt(data,  offset);
    int32_t hostLen     = readVarInt(data,  offset);

    offset += hostLen; // host string
    offset += 2;       // port (uint16)

    int32_t nextState = readVarInt(data,  offset);

    Serial.print("(Handshake , Connection = ");
    Serial.print(nextState);
    Serial.print(") ");

    conn.connectionState = nextState;
}

void dumpBinary(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            Serial.print((data[i] >> bit) & 1);
        }
        Serial.print(' ');
    }
    Serial.println();
}

void sendPongResponse(WiFiClient &client, const uint8_t* data, size_t length) {
    if (length < 8) return;

    int64_t timestamp = readLong(data, 0);

    std::vector<uint8_t> buffer;
    appendVarInt(buffer, 0x01); // Packet ID = 0x01

 
    for (int i = 7; i >= 0; i--) {
        buffer.push_back((timestamp >> (i * 8)) & 0xFF);
    }

    std::vector<uint8_t> finalBuffer;
    appendVarInt(finalBuffer, buffer.size());
    finalBuffer.insert(finalBuffer.end(), buffer.begin(), buffer.end());

    client.write(finalBuffer.data(), finalBuffer.size());
    client.flush();
    Serial.print("(Sent PONG: ");
    Serial.print(timestamp);
    Serial.print(") ");
}





void parseLoginStart(const uint8_t* data, size_t length, Player* &player) {
    size_t offset = 0;

    String username = readString(data, offset);
    String uuid     = readUUID(data, offset);

    if (!player) {
        player = new Player();
    }
    player->name = username;
    player->uuid = uuid;

    Serial.print("(loginStart user: ");
    Serial.print(username);
    Serial.print(" UUID: ");
    Serial.print(uuid);
    Serial.println(")");
}



void sendLoginSuccess(WiFiClient &client, Player* player) {
    std::vector<uint8_t> packet;

    // Packet ID = 0x02 (Login Success)
    appendVarInt(packet, 0x02);

    // UUID
    appendUUID(packet, player->uuid);

    // Username
    appendString(packet, player->name);

    // Properties
    appendVarInt(packet, 0); // number of properties = 0

  
    std::vector<uint8_t> finalPacket;
    appendVarInt(finalPacket, packet.size()); 
    finalPacket.insert(finalPacket.end(), packet.begin(), packet.end());

    client.write(finalPacket.data(), finalPacket.size());

    Serial.print("(Sent Login Success: ");
    Serial.print(player->name);
    Serial.print(" UUID: ");
    Serial.print(player->uuid);
    Serial.print(") ");
}


void handleBrand(WiFiClient &client, const uint8_t* data, size_t len, Player* &player) {
    size_t offset = 0;
    int32_t channelLen = readVarInt(data, offset);


    String channelName;
    for (int i = 0; i < channelLen; i++) {
        channelName += (char)data[offset++];
    }


    if (channelName == "minecraft:brand") {
        int32_t brandLen = readVarInt(data,  offset);
        String brand;
            for (int i = 0; i < brandLen; i++) {
                brand += (char)data[offset++];
            }
        Serial.print("(brand: ");
        Serial.print(brand);
        Serial.print(") ");
        player->brand = brand;
    }
}
void handleClientInformation(WiFiClient &client, const uint8_t* data, size_t len, Player* &player) {
    size_t offset = 0;

    // Locale (String)
    int32_t localeLen = readVarInt(data,  offset);
    for (int i = 0; i < localeLen && offset < len; i++) player->locale += (char)data[offset++];

    // View Distance (Byte)
    if (offset < len) player->viewDistance = data[offset++];

    // Chat Mode (VarInt)
    player->chatMode = readVarInt(data,  offset);

    // Chat Colors (Boolean)
    if (offset < len) player->chatColors = data[offset++] != 0;

    // Displayed Skin Parts (Unsigned Byte)
    if (offset < len) player->displayedSkinParts = data[offset++];

    // Main Hand (VarInt)
    player->mainHand = readVarInt(data, offset);

    // Enable Text Filtering (Boolean)
    if (offset < len) player->textFiltering = data[offset++] != 0;

    // Allow Server Listings (Boolean)
    if (offset < len) player->allowServerListings = data[offset++] != 0;

    // Particle Status (VarInt)
    player->particleStatus = readVarInt(data,  offset);

    // ================== Print ==================
    Serial.print("(Player Info");
    Serial.print("Name:"); Serial.print(player->name);
    Serial.print(" Brand:"); Serial.print(player->brand);
    Serial.print(" Locale:"); Serial.print(player->locale);
    Serial.print(" View Distance:"); Serial.print(player->viewDistance);
    Serial.print(" Chat Mode:"); Serial.print(player->chatMode);
    Serial.print(" Chat Colors:"); Serial.print(player->chatColors ? "true" : "false");
    Serial.print(" Displayed Skin Parts:"); Serial.print(player->displayedSkinParts);
    Serial.print(" Main Hand:"); Serial.print(player->mainHand);
    Serial.print(" Text Filtering:"); Serial.print(player->textFiltering ? "true" : "false");
    Serial.print(" Allow Server Listings:"); Serial.print(player->allowServerListings ? "true" : "false");
    Serial.print(" Particle Status:"); Serial.print(player->particleStatus);
    Serial.print(") ");
}
void sendSelectKnownPacks(WiFiClient &client) {
    std::vector<uint8_t> payload;

    appendVarInt(payload, 0x0E); // packet ID = select_known_packs

    appendVarInt(payload, 1);

 
    String ns = "minecraft";
    String pid = "core";
    String ver = "1.21.8";

    appendString(payload,ns);
    appendString(payload,pid);
    appendString(payload,ver);

 
    std::vector<uint8_t> packet;
    appendVarInt(packet, payload.size());
    packet.insert(packet.end(), payload.begin(), payload.end());

    client.write(packet.data(), packet.size());
    client.flush();

    Serial.print("(Sent select_known_packs) ");
}
void handleServerboundKnownPacks(const uint8_t* buffer, size_t len) {
    size_t offset = 0;

 
    int32_t packCount = readVarInt(buffer, offset);
    Serial.print("(Serverbound Known Packs count: #");
    Serial.print(packCount);

    for (int i = 0; i < packCount; i++) {
        
        int32_t nsLen = readVarInt(buffer, offset);
        String ns = "";
        for (int j = 0; j < nsLen && offset < len; j++) ns += (char)buffer[offset++];

        
        int32_t idLen = readVarInt(buffer, offset);
        String pid = "";
        for (int j = 0; j < idLen && offset < len; j++) pid += (char)buffer[offset++];

        
        int32_t verLen = readVarInt(buffer, offset);
        String ver = "";
        for (int j = 0; j < verLen && offset < len; j++) ver += (char)buffer[offset++];

        
        Serial.println("");
        Serial.print("Pack #"); Serial.println(i + 1);
        Serial.print("Namespace: "); Serial.println(ns);
        Serial.print("ID: "); Serial.println(pid);
        Serial.print("Version: "); Serial.println(ver);
        Serial.println("-------------------");
    }
    Serial.print(") ");
}
void sendRegistryData(WiFiClient &client) {
    if (!client.connected()) return;

    
    size_t totalRegistries = sizeof(registries_bin);
    size_t sent = 0;
    while (sent < totalRegistries) {
        size_t n = client.write(registries_bin + sent, totalRegistries - sent);
        if (n == 0) break; // error
        sent += n;
    }

    // Wysyłamy pakiet tagów
    size_t totalTags = sizeof(tags_bin);
    sent = 0;
    while (sent < totalTags) {
        size_t n = client.write(tags_bin + sent, totalTags - sent);
        if (n == 0) break; // error
        sent += n;
    }

 
    Serial.print("(Sent ");
    Serial.print(totalRegistries);
    Serial.print(" bites i ");
    Serial.print(totalTags);
    Serial.print(" tags bites) ");

}
void sendFinishConfiguration(WiFiClient &client) {
    if (!client.connected()) return;

    std::vector<uint8_t> payload;
    appendVarInt(payload, 0x03); // ID 
    std::vector<uint8_t> packet;
    appendVarInt(packet, payload.size());
    packet.insert(packet.end(), payload.begin(), payload.end());
    client.write(packet.data(), packet.size());


    Serial.print("(Sent Finish Configuration) ");
}
int32_t nextEntityId = 0;
void sendLoginPlay(WiFiClient &client,Player* &Player) {
    std::vector<uint8_t> payload;

    // ================== Packet ID ==================
    appendVarInt(payload, 0x2B); // Packet ID = 0x2B (Login - Play)

    // ================== Entity ID ==================
    nextEntityId++;
    int32_t entityId = nextEntityId;
    payload.push_back((entityId >> 24) & 0xFF);
    payload.push_back((entityId >> 16) & 0xFF);
    payload.push_back((entityId >> 8) & 0xFF);
    payload.push_back(entityId & 0xFF);
    Player->entityId = entityId;

    // ================== Is hardcore ==================
    bool isHardcore = false; // true/false
    payload.push_back(isHardcore ? 1 : 0);

    // ================== Dimension Names ==================
    std::vector<String> dimensionNames = {"minecraft:overworld", "minecraft:the_nether", "minecraft:the_end"};
    appendVarInt(payload, dimensionNames.size());
    for (const auto &dim : dimensionNames) {
        appendVarInt(payload, dim.length()); 
        for (size_t i = 0; i < dim.length(); ++i) payload.push_back((uint8_t)dim[i]);
    }

    // ================== Max Players ==================
    int32_t maxPlayers = 0; // ignored now
    appendVarInt(payload, maxPlayers);

    // ================== View Distance ==================
    int32_t viewDistance = 10; // render distance
    appendVarInt(payload, viewDistance);

    // ================== Simulation Distance ==================
    int32_t simulationDistance = 10;
    appendVarInt(payload, simulationDistance);

    // ================== Reduced Debug Info ==================
    bool reducedDebugInfo = false;
    payload.push_back(reducedDebugInfo ? 1 : 0);

    // ================== Enable Respawn Screen ==================
    bool enableRespawnScreen = true;
    payload.push_back(enableRespawnScreen ? 1 : 0);

    // ================== Do Limited Crafting ==================
    bool limitedCrafting = false;
    payload.push_back(limitedCrafting ? 1 : 0);

    // ================== Dimension Type ==================
    int32_t dimensionTypeId = 0; // ID in registry_data
    appendVarInt(payload, dimensionTypeId);

    // ================== Dimension Name ==================
    String dimensionName = "minecraft:overworld";
    appendVarInt(payload, dimensionName.length());
    for (size_t i = 0; i < dimensionName.length(); ++i) payload.push_back((uint8_t)dimensionName[i]);

    // ================== Hashed Seed ==================
    int64_t hashedSeed = 0x1234567890ABCDEF; // example seed
    for (int i = 7; i >= 0; --i) payload.push_back((uint8_t)((hashedSeed >> (8 * i)) & 0xFF));

    // ================== Game Mode ==================
    uint8_t gameMode = 1; 
    Player->gamemode = 1;
    payload.push_back(gameMode);

    // ================== Previous Game Mode ==================
    int8_t previousGameMode = -1; // -1 = undefined
    payload.push_back((uint8_t)previousGameMode);

    // ================== Is Debug ==================
    bool isDebug = false;
    payload.push_back(isDebug ? 1 : 0);

    // ================== Is Flat ==================
    bool isFlat = false;
    payload.push_back(isFlat ? 1 : 0);

    // ================== Has Death Location ==================
    bool hasDeathLocation = false; // false -> brak death location
    payload.push_back(hasDeathLocation ? 1 : 0);

    // ================== Portal Cooldown ==================
    int32_t portalCooldown = 0;
    appendVarInt(payload, portalCooldown);

    // ================== Sea Level ==================
    int32_t seaLevel = 64;
    appendVarInt(payload, seaLevel);

    // ================== Enforces Secure Chat ==================
    bool enforcesSecureChat = false;
    payload.push_back(enforcesSecureChat ? 1 : 0);

    // ================== Prefix całego pakietu VarInt długością ==================
    std::vector<uint8_t> packet;
    appendVarInt(packet, payload.size());
    packet.insert(packet.end(), payload.begin(), payload.end());

    client.write(packet.data(), packet.size());
    client.flush();

    Serial.print("(Sent Login/Play) ");
}






void appendUint32BE(std::vector<uint8_t>& payload, uint32_t value) {
    payload.push_back((uint8_t)(value >> 24));
    payload.push_back((uint8_t)(value >> 16));
    payload.push_back((uint8_t)(value >> 8));
    payload.push_back((uint8_t)(value));
}
int TeleportID = 0;
void SendTeleport(WiFiClient &client) {
    std::vector<uint8_t> payload;
    appendVarInt(payload,0x41);

    appendVarInt(payload,TeleportID);
    TeleportID++;

    appendDouble(payload,5.0);
    appendDouble(payload,65.0);
    appendDouble(payload,5.0);

    appendDouble(payload,0.0);
    appendDouble(payload,0.0);
    appendDouble(payload,0.0);

    appendFloat(payload,0.0f);
    appendFloat(payload,0.0f);

    appendUint32BE(payload, 0);

    std::vector<uint8_t> packet;
    appendVarInt(packet, payload.size());
    packet.insert(packet.end(), payload.begin(), payload.end());
    client.write(packet.data(), packet.size());

    Serial.print("(Sent Teleport 5,65,5) ");
}



void sendKeepAlive(WiFiClient &client, int64_t id) {
    std::vector<uint8_t> payload;
    appendVarInt(payload, 0x26);   // KeepAlive Packet ID (PLAY)
    appendLong(payload, id);

    std::vector<uint8_t> packet;
    appendVarInt(packet, payload.size());
    packet.insert(packet.end(), payload.begin(), payload.end());

    client.write(packet.data(), packet.size());
}


void sendSetChunkCenter_Custom(WiFiClient &client) {
    if (!client.connected()) return;

    std::vector<uint8_t> payload;

    // Packet ID: 0x57 (Set Chunk Cache Center)
    appendVarInt(payload, 0x57);

    // Chunk X = 0
    appendVarInt(payload, 0);

    // Chunk Z = 0
    appendVarInt(payload, 0);

    // Wrap packet with length
    std::vector<uint8_t> packet;
    appendVarInt(packet, (int)payload.size());
    packet.insert(packet.end(), payload.begin(), payload.end());

    client.write(packet.data(), packet.size());
    Serial.print("(Sent CenterChunk) ");
}
void sendStartWaitingChunks_Custom(WiFiClient &client) {
    if (!client.connected()) return;

    std::vector<uint8_t> payload;

    // Packet ID (0x22)
    appendVarInt(payload, 0x22);

    // Event = 13 (Start waiting for level chunks)
    payload.push_back(13);

    // Value = 0.0f (Float)
    union { float f; uint8_t b[4]; } val;
    val.f = 0.0f;
    payload.insert(payload.end(), val.b, val.b + 4);

    // Wrap packet with length
    std::vector<uint8_t> packet;
    appendVarInt(packet, (int)payload.size());
    packet.insert(packet.end(), payload.begin(), payload.end());

    // Send to client
    client.write(packet.data(), packet.size());
    client.flush();
    Serial.println("AwaitChunks packet sent");
}

void sendSingleChunkFromMemory(WiFiClient &client, int _x, int _z) {
    Serial.printf("(chunk %dx%d) ", _x, _z);
   
    if (!client.connected()) return;

  
    Chunk* c = getChunk(_x, _z);
    if (c == nullptr) {
        Serial.print("(No Chunk in memory!) ");
        return;
    }

    uint8_t chunk_section[4096];
    std::vector<uint8_t> network_block_palette;


    auto appendVarIntToVec = [&](std::vector<uint8_t> &v, int value) {
        uint32_t val = (uint32_t)value;
        do {
            uint8_t temp = val & 0x7F;
            val >>= 7;
            if (val != 0) temp |= 0x80;
            v.push_back(temp);
        } while (val != 0);
    };

    for (int i = 0; i < 256; ++i) appendVarIntToVec(network_block_palette, i);


    std::vector<uint8_t> payload;
    auto appendByte = [&](uint8_t b) { payload.push_back(b); };
    auto appendBytes = [&](const uint8_t *buf, size_t len) { payload.insert(payload.end(), buf, buf + len); };
    auto appendVarInt = [&](int value) {
        uint32_t val = (uint32_t)value;
        do {
            uint8_t temp = val & 0x7F;
            val >>= 7;
            if (val != 0) temp |= 0x80;
            payload.push_back(temp);
        } while (val != 0);
    };
    auto appendUint16 = [&](uint16_t v) {
        payload.push_back((v >> 8) & 0xFF);
        payload.push_back(v & 0xFF);
    };
    auto appendUint32 = [&](uint32_t v) {
        payload.push_back((v >> 24) & 0xFF);
        payload.push_back((v >> 16) & 0xFF);
        payload.push_back((v >> 8) & 0xFF);
        payload.push_back(v & 0xFF);
    };
    auto appendUint64 = [&](uint64_t v) {
        for (int i = 7; i >= 0; i--) payload.push_back((v >> (8 * i)) & 0xFF);
    };

    auto task_yield = [&]() { yield(); };

    int sizeVarInt256 = 2;
    
    int chunk_data_size = (4101 + sizeVarInt256 + (int)network_block_palette.size()) * 20 + 6 * 12; 

    appendByte(0x27); // packet ID: level_chunk_with_light
    appendUint32((uint32_t)_x);
    appendUint32((uint32_t)_z);
    appendVarInt(0); // omit heightmaps
    appendVarInt(chunk_data_size);


    for (int i = 0; i < 4; i++) {
        appendUint16(4096);
        appendByte(0);
        appendVarInt(0);
        appendByte(0);
        appendByte(0);
    }
    task_yield();
    const int SECTION_HEIGHT = 16;

    for (int i = 0; i < 20; i++) {
        int baseY = i * SECTION_HEIGHT; // 0, 16, 32, ...

        appendUint16(4096);
        appendByte(8);
        appendVarInt(256);
        appendBytes(network_block_palette.data(), network_block_palette.size());


        for (int k = 0; k < 4096; ++k) {
            
   
            int localY = k >> 8;           
            int localXZ = k & 0xFF;        
            int localZ = localXZ >> 4;     
            int localX = localXZ & 0x0F;   
            
            int worldY = baseY + localY;
            int chunk_idx = worldY * CHUNK_SIZE_X * CHUNK_SIZE_Z
              + localZ * CHUNK_SIZE_X
              + localX;


            int output_index = k ^ 7;
                    
            if (worldY < CHUNK_HEIGHT) {
                chunk_section[output_index] = block_palette[
                    c->data[chunk_idx]
                ];
            } else {
                chunk_section[output_index] = 0;
            }

        }
        // =======================================================================================

        appendBytes(chunk_section, 4096);
        appendByte(0);
        appendByte(0); // biome
        task_yield();
    }


    for (int i = 0; i < 8; i++) {
        appendUint16(4096);
        appendByte(0);
        appendVarInt(0);
        appendByte(0);
        appendByte(0);
    }

    appendVarInt(0); // block entities omitted

    // light map (idk what im doing)
    appendVarInt(1);
    appendUint64(0b11111111111111111111111111ULL);
    appendVarInt(0);
    appendVarInt(0);
    appendVarInt(0);
    appendVarInt(26);


    for (int i = 0; i < 4096; ++i) chunk_section[i] = 0xFF; 
    for (int i = 0; i < 8; i++) {
        appendVarInt(2048);
        appendBytes(chunk_section, 2048);
    }
    for (int i = 0; i < 18; i++) {
        appendVarInt(2048);
        appendBytes(chunk_section, 2048); // Wysyła 0xFF
    }


    appendVarInt(0); // no block light


    const int block_changes_count = 0;

    // --- sent
    auto writeVarInt = [&](int value) {
        uint32_t val = (uint32_t)value;
        uint8_t tmp[5];
        int idx = 0;
        do {
            uint8_t t = val & 0x7F;
            val >>= 7;
            if (val != 0) t |= 0x80;
            tmp[idx++] = t;
        } while (val != 0 && idx < 5);
        client.write(tmp, idx);
    };
    auto send_all = [&](const uint8_t *buf, size_t len) {
        size_t off = 0;
        while (off < len) {
            size_t wrote = client.write(buf + off, len - off);
            if (wrote == 0) break;
            off += wrote;
        }
    };

    writeVarInt(payload.size());
    send_all(payload.data(), payload.size());
    client.flush();


}









































void HandleKeepAlive(Connection &conn, const uint8_t* payload, size_t length) {
    size_t offset = 0;
    int64_t recvId = readLong(payload, offset);  

    if (conn.lastKeepAliveId == recvId) {
        // reset timer
        conn.lastKeepAliveRecv = millis(); 
    } else {
        
        Serial.print("(Fake KeepAlive, disconnect) ");
        conn.client.stop();
    }
}





void HandlePacket(Connection &conn, const uint8_t* data, size_t length) {
    size_t offset = 0;
    int32_t packetLength = readVarInt(data,  offset);
    int32_t packetId = readVarInt(data,  offset);
    String playerName = conn.player ? conn.player->name : "Unknown";
    const uint8_t* payload = data + offset;
    size_t payloadLength = length - offset;
    bool handled = false;

    int8_t& connectionState = conn.connectionState;
    const uint32_t& packetCounter = conn.packetCounter; 

    //#################KEEPALIVE###################
    if (packetId == 0x1B) { // KeepAlive Response
        HandleKeepAlive(conn, payload, payloadLength); 
        handled = true;
    }


    if (connectionState == -1 && packetId == 0x00) {
        handleHandshakePacket(conn, payload, payloadLength);
        handled = true;
    }
    ////##############################PLAY##############################
    if (connectionState == 3) {
        if (packetId == 0xC) {//client Tick
            handled = true;
        } else if (packetId == 0x1D) {//Set Player Position
            handled = true;
        } else if (packetId == 0x1E) {//Set Position & rotation
            handled = true;
        } else if (packetId == 0x1F) {//rotation
            handled = true;
        } else if (packetId == 0x00) {//most likely Confirm Teleportation but there are 5 more packet with id 0x00
            handled = true;
        } else if (packetId == 0x20) {//Set Player Movement Flags
            handled = true;
        } else if (packetId == 0x2B) { // Player Loaded
            handled = true;
        }
    }

    //##############################LOGIN##############################
    if (connectionState == 2) {
        if (packetId == 0x00 && packetCounter == 1) { //LOGIN START
            parseLoginStart(payload, payloadLength,conn.player);
            
            sendLoginSuccess(conn.client, conn.player);
            handled = true;
        } else if (packetId == 0x3 && packetCounter == 2) { //Login Acknowledged
            handled = true;
        } else if (packetId == 0x2 &&packetCounter == 3) {
            handleBrand(conn.client,payload,payloadLength,conn.player);
            handled = true;
        } else if (packetId == 0x0 && packetCounter == 4) {
            handleClientInformation(conn.client,payload,payloadLength,conn.player);
            sendSelectKnownPacks(conn.client);
            handled = true;
        } else if (packetId == 0x7 && packetCounter == 5) {
            //handleServerboundKnownPacks(data,length); broken LOL
            sendRegistryData(conn.client);
            sendFinishConfiguration(conn.client);
            
            handled = true;
        } else if (packetId == 0x3 && packetCounter == 6) {//Acknowledge Finish Configuration
            sendLoginPlay(conn.client,conn.player);

            sendStartWaitingChunks_Custom(conn.client);
            sendSetChunkCenter_Custom(conn.client);


            uint32_t start = millis();
            sendSingleChunkFromMemory(conn.client,-1,-1);
            sendSingleChunkFromMemory(conn.client,0,-1);
            sendSingleChunkFromMemory(conn.client,1,-1);

            sendSingleChunkFromMemory(conn.client,-1,0);
            sendSingleChunkFromMemory(conn.client,0,0);
            sendSingleChunkFromMemory(conn.client,1,0);

            sendSingleChunkFromMemory(conn.client,-1,1);
            sendSingleChunkFromMemory(conn.client,0,1);
            sendSingleChunkFromMemory(conn.client,1,1);

            SendTeleport(conn.client);
            connectionState = 3;

            Serial.print("(TOOK: ");
            Serial.print(millis() - start);
            Serial.print("ms.) ");
            handled = true;
            
        }// 0x03 Finish Configuration
    }
    //##############################PING##############################
    if (connectionState == 1) {
        if (packetId == 0x00&& packetCounter == 0) { //PING
            sendStatusResponse(conn.client);
            handled = true;
        } else if (packetId == 0x1) { //PONG
            sendPongResponse(conn.client, payload, payloadLength);
            handled = true;
        } else if (packetId == 0x00) {
            handled = true;  //legacy shit
        }
    }
   
    //handled = false;


    if (!handled) {
        Serial.print("Klient ID: ");
        Serial.print(conn.id);
        Serial.print(" (");
        Serial.print(playerName);
        Serial.print(") ");

        Serial.print("PacketID: 0x");
        Serial.print(packetId, HEX);

        Serial.print(" | PacketCounter: ");
        Serial.print(packetCounter);

        Serial.print(" | Payload (");
        Serial.print(payloadLength);
        Serial.print(" bytes): ");
        dumpBinary(payload, payloadLength);
        Serial.println("");
        Serial.println("");
    }
}

















































void MinecraftServerTick(WiFiServer &server) {
    WiFiClient newClient = server.available();
    if (newClient) {
        Connection conn;
        conn.id = nextConnectionId++;
        conn.client = newClient;
        conn.connectionState = -1;
        conn.packetCounter = 0;
        conn.player = nullptr;

        conn.lastKeepAliveSent = millis();
        conn.lastKeepAliveRecv = millis(); 

        Connections.push_back(conn);
        Serial.print("Nowy klient połączony! ID: ");
        Serial.println(conn.id);
    }
    

    for (size_t i = 0; i < Connections.size(); i++) {
        Connection &conn = Connections[i];

        uint32_t now = millis();

        //########keepalive##########
        if (now - conn.lastKeepAliveSent >= KEEPALIVE_SEND_INTERVAL) {
            conn.lastKeepAliveId = esp_random(); 
            sendKeepAlive(conn.client, conn.lastKeepAliveId);
            conn.lastKeepAliveSent = now;
        }
        if (now - conn.lastKeepAliveRecv >= KEEPALIVE_TIMEOUT) {
            Serial.println("(KeepAlive timeout, disconnect )");
            conn.client.stop();
            Connections.erase(Connections.begin() + i);
            i--;
            continue;
        }


        if (!conn.client.connected()) {
            Serial.print("Klient ID: ");
            Serial.print(conn.id);
            Serial.println(" rozłączony.");
            conn.client.stop();
            Connections.erase(Connections.begin() + i);
            i--;
            continue;
        }

        while (conn.client.available()) {
            


            uint8_t buffer[256];
            size_t len = conn.client.read(buffer, sizeof(buffer));
            if (len > 0) {
                conn.recvBuffer.insert(conn.recvBuffer.end(), buffer, buffer + len);

             
                size_t processed = 0;
                while (true) {
                    size_t varIntSize = 0;
                    int32_t packetLen = tryReadVarInt(conn.recvBuffer, processed, varIntSize);
                    if (packetLen <= 0) break;
                    if (conn.recvBuffer.size() - processed - varIntSize < (size_t)packetLen) break;

                    size_t totalPacketSize = varIntSize + packetLen;
                    HandlePacket(conn, conn.recvBuffer.data() + processed, totalPacketSize);

                    conn.packetCounter++;
                    processed += totalPacketSize;
                }

                if (processed > 0) {
                    conn.recvBuffer.erase(conn.recvBuffer.begin(), conn.recvBuffer.begin() + processed);
                }
            }
        }
    }
}
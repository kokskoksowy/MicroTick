#include "MinecraftFunctions.h"
#include "functions/functions.h"
#include <settings.h>
// Definicje globalnych zmiennych
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


// Funkcja obsługi pakietu
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

    int64_t timestamp = readLong(data, 0); // odczyt 8 bajtów

    std::vector<uint8_t> buffer;
    appendVarInt(buffer, 0x01); // Packet ID = 0x01

    // Dodajemy timestamp jako **8 bajtów big-endian**, nie VarLong
    for (int i = 7; i >= 0; i--) {
        buffer.push_back((timestamp >> (i * 8)) & 0xFF);
    }

    std::vector<uint8_t> finalBuffer;
    appendVarInt(finalBuffer, buffer.size()); // długość pakietu
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

    // Properties - pusta lista
    appendVarInt(packet, 0); // number of properties = 0

    // Teraz pakiet gotowy, opakowujemy długość VarInt
    std::vector<uint8_t> finalPacket;
    appendVarInt(finalPacket, packet.size()); // długość pakietu
    finalPacket.insert(finalPacket.end(), packet.begin(), packet.end());

    // Wyślij do klienta
    client.write(finalPacket.data(), finalPacket.size());

    Serial.print("(Sent Login Success: ");
    Serial.print(player->name);
    Serial.print(" UUID: ");
    Serial.print(player->uuid);
    Serial.println(")");
}

void HandlePacket(Connection &conn, const uint8_t* data, size_t length) {
    size_t offset = 0;

    // 1️⃣ Packet Length
    int32_t packetLength = readVarInt(data,  offset);

    // 2️⃣ Packet ID
    int32_t packetId = readVarInt(data,  offset);

    String playerName = conn.player ? conn.player->name : "Unknown";

    
    //print binary
    

    const uint8_t* payload = data + offset;
    size_t payloadLength = length - offset;

    bool handled = false;
    // 3️⃣ HANDSHAKE → przekazujemy tylko PAYLOAD
    int8_t& connectionState = conn.connectionState;    // referencja do connectionState
    const uint32_t& packetCounter = conn.packetCounter;     // referencja do packetCounter



    if (connectionState == -1 && packetId == 0x00) {
        handleHandshakePacket(conn, payload, payloadLength);
        handled = true;
    }

    if (connectionState == 2) {
        if (packetId == 0x00 && packetCounter == 1) { //LOGIN START
            parseLoginStart(payload, payloadLength,conn.player);
            sendLoginSuccess(conn.client, conn.player);
            handled = true;
            handled = false;
        }
    }

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

















































// Funkcja ticku serwera
void MinecraftServerTick(WiFiServer &server) {
    WiFiClient newClient = server.available();
    if (newClient) {
        Connection conn;
        conn.id = nextConnectionId++;
        conn.client = newClient;
        conn.connectionState = -1;
        conn.packetCounter = 0;
        conn.player = nullptr;

        Connections.push_back(conn);
        Serial.print("Nowy klient połączony! ID: ");
        Serial.println(conn.id);
    }

    for (size_t i = 0; i < Connections.size(); i++) {
        Connection &conn = Connections[i];

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

                // Wyciąganie wszystkich pełnych pakietów
                size_t processed = 0;
                while (true) {
                    size_t varIntSize = 0;
                    int32_t packetLen = tryReadVarInt(conn.recvBuffer, processed, varIntSize);
                    if (packetLen <= 0) break; // niekompletny lub błąd
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
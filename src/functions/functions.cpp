#include "Arduino.h"
#include <vector>
uint32_t readVarInt(const uint8_t* data, size_t &offset) {
    uint32_t num = 0;
    int numRead = 0;
    uint8_t read;
    do {
        read = data[offset++];
        uint32_t value = read & 0b01111111;
        num |= value << (7 * numRead);

        numRead++;
        if (numRead > 5) {
            Serial.println("VarInt za duży!");
            break;
        }
    } while (read & 0b10000000);
    return num;
}
void appendVarInt(std::vector<uint8_t>& buffer, int32_t value) {
    do {
        uint8_t temp = value & 0b01111111;
        value >>= 7;
        if (value != 0) {
            temp |= 0b10000000;
        }
        buffer.push_back(temp);
    } while (value != 0);
}
void appendString(std::vector<uint8_t>& buffer, const String& str) {
    // długość w bajtach UTF-8
    int len = str.length();
    appendVarInt(buffer, len);
    for (int i = 0; i < len; i++) {
        buffer.push_back(str[i]);
    }
}
void appendUUID(std::vector<uint8_t>& buf, const String& uuid) {
    // UUID w formacie "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
    String clean = uuid;
    clean.replace("-", "");
    for (int i = 0; i < 16; i++) {
        String byteStr = clean.substring(i*2, i*2 + 2);
        buf.push_back((uint8_t) strtoul(byteStr.c_str(), nullptr, 16));
    }
}

int64_t readLong(const uint8_t* data, size_t offset) {
    int64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value <<= 8;
        value |= data[offset + i];
    }
    return value;
}

int32_t tryReadVarInt(const std::vector<uint8_t>& buf, size_t start, size_t &outVarIntSize) {
    int32_t result = 0;
    int numRead = 0;
    size_t idx = start;
    while (idx < buf.size()) {
        uint8_t byte = buf[idx++];
        result |= (int32_t)(byte & 0x7F) << (7 * numRead);
        numRead++;
        if ((byte & 0x80) == 0) {
            outVarIntSize = numRead;
            return result;
        }
        if (numRead >= 5) return -2; // VarInt za długi
    }
    return -1; // niekompletny
}

String readUUID(const uint8_t* data, size_t &offset) {
    if (offset + 16 > 256) return "ERROR"; // bezpieczeństwo
    String uuid = "";
    for (int i = 0; i < 16; i++) {
        if (i == 4 || i == 6 || i == 8 || i == 10) uuid += "-";
        if (data[offset] < 16) uuid += "0";
        uuid += String(data[offset], HEX);
        offset++;
    }
    return uuid;
}
String readString(const uint8_t* data, size_t &offset) {
    int32_t length = readVarInt(data, offset); // długość w bajtach
    if (length < 0) return "ERROR";

    char buf[256]; // tymczasowy bufor
    if (length > 255) length = 255; // bezpieczeństwo
    for (int i = 0; i < length; i++) {
        buf[i] = (char)data[offset++];
    }
    buf[length] = '\0';
    return String(buf);
}
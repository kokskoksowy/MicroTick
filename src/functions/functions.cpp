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
int64_t readLong(const uint8_t* data, size_t offset) {
    int64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value <<= 8;
        value |= data[offset + i];
    }
    return value;
}
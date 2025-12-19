#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <Arduino.h>
#include <vector>
uint32_t readVarInt(const uint8_t* data, size_t &offset);
void appendVarInt(std::vector<uint8_t>& buffer, int32_t value);
int64_t readLong(const uint8_t* data, size_t offset);
int32_t tryReadVarInt(const std::vector<uint8_t>& buf, size_t start, size_t &outVarIntSize);
String readUUID(const uint8_t* data, size_t &offset);
String readString(const uint8_t* data, size_t &offset);
void appendString(std::vector<uint8_t>& buffer, const String& str);
void appendUUID(std::vector<uint8_t>& buf, const String& uuid);
void appendFloat(std::vector<uint8_t>& buf, float value);
void appendDouble(std::vector<uint8_t>& buf, double value);
void appendLong(std::vector<uint8_t>& buf, int64_t value);
#endif
#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <Arduino.h>
#include <vector>
uint32_t readVarInt(const uint8_t* data, size_t &offset);
void appendVarInt(std::vector<uint8_t>& buffer, int32_t value);
int64_t readLong(const uint8_t* data, size_t offset);
#endif
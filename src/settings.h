#pragma once
#include <Arduino.h>

// KeepAlive interwały w milisekundach
constexpr uint32_t KEEPALIVE_SEND_INTERVAL = 5000;   // wysyłanie co 5 sekund
constexpr uint32_t KEEPALIVE_TIMEOUT       = 25000;  // disconnect po 10 sekundach braku odpowiedzi

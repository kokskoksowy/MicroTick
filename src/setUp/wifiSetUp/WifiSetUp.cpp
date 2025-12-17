#include <WiFi.h>
#include <setUp/rgbControler/rgbControler.h>
#include <secrets.h>

constexpr int bright = 255;

void WiFIsetUP() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA); // tryb stacji

    while (true) {  // główna pętla prób połączenia
        Serial.println("Czekam na sieć...");
        setRGB(255, 0, 0, bright);// 🔴 Czerwony - start i brak połączenia
        WiFi.disconnect();   

        
        

        // Skanowanie sieci
        setRGB(255, 255, 0, bright);// 🟡szuka sieci
        int networks = WiFi.scanNetworks();
        bool found = false;
        for (int i = 0; i < networks; i++) {
            if (WiFi.SSID(i) == WIFI_SSID) {
                found = true;
                break;
            }
        }

        if (!found) {
            Serial.println("Sieć nie znaleziona, wracam do czerwonego.");
            setRGB(255, 0, 0, bright);
            delay(1000);  // krótka pauza przed ponownym skanem
            continue;     // wracamy na początek pętli
        }

        setRGB(0, 255, 0, bright);// 🟢 znalazło
        Serial.println("Sieć znaleziona! Łączenie...");
       
        
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        unsigned long startTime = millis();
        const unsigned long timeout = 5000; // timeout 10s na połączenie
        bool connected = false;

        while (millis() - startTime < timeout) {
            if (WiFi.status() == WL_CONNECTED) {
                connected = true;
                break;
            }
            delay(100);
        }

        if (!connected) {
            Serial.println("Nie udało się połączyć. Wracam na czerwony.");
            setRGB(255, 0, 0, bright);
            delay(1000);
            continue; // wracamy na początek pętli
        }

        // 🟢 Połączono - mrugamy zielonym 3 razy
        Serial.println("Połączono z Wi-Fi!");
        for (int i = 0; i < 2; i++) {
            setRGB(0, 255, 0, 255);
            delay(250);
            setRGB(0, 0, 0, 0);
            delay(250);
        }

        break; // zakończ po udanym połączeniu
    }
}
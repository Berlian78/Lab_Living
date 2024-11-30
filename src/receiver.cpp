#include <ESP8266WiFi.h>
#include <espnow.h>

// Data structures for different transmitters
struct SHT20Data {
    float temp;
    float hum;
};

struct DS18B20Data {
    float temp;
};

void handleReceivedData(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
    Serial.print("Received data from MAC: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac_addr[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

    if (len == sizeof(SHT20Data)) {
        SHT20Data sht20Data;
        memcpy(&sht20Data, incomingData, sizeof(sht20Data));
        Serial.printf("SHT20 - Temp: %.2f°C, Hum: %.2f%%\n", sht20Data.temp, sht20Data.hum);
    } else if (len == sizeof(DS18B20Data)) {
        DS18B20Data dsData;
        memcpy(&dsData, incomingData, sizeof(dsData));
        Serial.printf("DS18B20 - Temp: %.2f°C\n", dsData.temp);
    } else {
        Serial.println("Unknown data format");
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != 0) {
        Serial.println("ESP-NOW Initialization Failed");
        return;
    }

    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(handleReceivedData);

    Serial.println("Waiting for data...");
}

void loop() {
    // No loop logic needed; the callback handles everything
}

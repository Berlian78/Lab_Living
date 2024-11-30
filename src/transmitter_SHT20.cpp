#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include "SHTSensor.h"
#include <esp_sleep.h>

#define WAKEUP_GPIO GPIO_NUM_33  // Button on GPIO33
RTC_DATA_ATTR int bootCount = 0;

// Replace with your ESP-01 MAC address
uint8_t receiverMACAddress[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};

void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Wakeup caused by external signal using RTC_IO");
    } else {
        Serial.printf("Wakeup not caused by deep sleep: %d\n", wakeup_reason);
    }
}

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

    print_wakeup_reason();

    // Initialize WiFi and ESP-NOW
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Initialization Failed");
        return;
    }
    esp_now_register_send_cb(onSent);

    // Register peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, receiverMACAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    // Initialize SHT20 sensor
    SHT2x.begin();
    float temperature = SHT2x.GetTemperature();
    float humidity = SHT2x.GetHumidity();

    // Send data via ESP-NOW
    struct {
        float temp;
        float hum;
    } dataToSend;

    dataToSend.temp = temperature;
    dataToSend.hum = humidity;

    esp_err_t result = esp_now_send(receiverMACAddress, (uint8_t *)&dataToSend, sizeof(dataToSend));
    Serial.println(result == ESP_OK ? "Data sent successfully" : "Error sending data");

    // Prepare to go back to sleep
    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 1);  // Wake up on HIGH
    Serial.println("Going to sleep...");
    delay(1000);
    esp_deep_sleep_start();
}

void loop() {
    // Not used
}

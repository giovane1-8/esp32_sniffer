#include <WiFi.h>
extern "C" {
  #include "esp_wifi.h"
}

typedef struct {
    uint8_t bssid[6];
    char ssid[33];
} bssid_map_t;

bssid_map_t known_aps[50];
int ap_count = 0;

void add_ap(const uint8_t *bssid, const char *ssid) {
    for (int i = 0; i < ap_count; i++) {
        if (memcmp(known_aps[i].bssid, bssid, 6) == 0) return;
    }
    memcpy(known_aps[ap_count].bssid, bssid, 6);
    strncpy(known_aps[ap_count].ssid, ssid, 32);
    ap_count++;
}

const char *find_ssid(const uint8_t *bssid) {
    for (int i = 0; i < ap_count; i++) {
        if (memcmp(known_aps[i].bssid, bssid, 6) == 0)
            return known_aps[i].ssid;
    }
    return "(SSID desconhecido)";
}

unsigned long lastPrint = 0;
const unsigned long printInterval = 200;  // printa a cada 200ms

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {

    unsigned long now = millis();
    if (now - lastPrint < printInterval) return;
    lastPrint = now;

    const wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    const uint8_t *data = pkt->payload;

    uint16_t frame_ctrl = data[0] | (data[1] << 8);
    uint8_t frame_type = (frame_ctrl & 0x0C) >> 2;
    uint8_t frame_subtype = (frame_ctrl & 0xF0) >> 4;

    // BEACON
    if (frame_type == 0 && frame_subtype == 8) {

        const uint8_t *bssid = data + 16;

        int pos = 36;
        uint8_t tag = data[pos];
        uint8_t len = data[pos + 1];

        char ssid[33] = {0};

        if (tag == 0 && len < 32) {
            memcpy(ssid, data + pos + 2, len);

            add_ap(bssid, ssid);

            Serial.printf("[BEACON] SSID: %s | BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
                          ssid,
                          bssid[0], bssid[1], bssid[2],
                          bssid[3], bssid[4], bssid[5]);
        }
    }

    // DATA
    if (frame_type == 2) {

        const uint8_t *bssid = data + 16;
        const char *ssid = find_ssid(bssid);

        Serial.printf("[DATA] BSSID: %02X:%02X:%02X:%02X:%02X:%02X (%s)\n",
                      bssid[0], bssid[1], bssid[2],
                      bssid[3], bssid[4], bssid[5], ssid);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_MODE_NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_start();

    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL
    };

    esp_wifi_set_promiscuous_filter(&filter);
    esp_wifi_set_promiscuous_rx_cb(sniffer);
    esp_wifi_set_promiscuous(true);

    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);

    Serial.println("Sniffer iniciado no canal 6...");
}

void loop() {
    // Arduino exige loop(), mas não precisamos dele
}
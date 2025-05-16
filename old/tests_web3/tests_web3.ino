#include <WiFi.h>
#include <esp_now.h>
#include <esp_camera.h>
#include "camera_pins.h"  // Assurez-vous d'avoir ce fichier dans le même dossier que votre code

// Définition de la structure d'image
typedef struct {
    uint8_t jpg_buffer[40000];  // Tampon pour l'image (ajuster selon la résolution)
    size_t jpg_length;
} esp_now_frame_t;

esp_now_peer_info_t peerInfo;
uint8_t receiverMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}; // Remplace avec l'adresse MAC de la manette

void sendImage() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Erreur capture image !");
        return;
    }

    esp_now_frame_t frame;
    frame.jpg_length = fb->len;
    memcpy(frame.jpg_buffer, fb->buf, fb->len);

    esp_now_send(receiverMAC, (uint8_t *)&frame, sizeof(frame));
    Serial.println("Image envoyée !");
    esp_camera_fb_return(fb);
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    esp_now_init();

    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    // Configuration de la caméra
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Échec initialisation caméra !");
        return;
    }

    Serial.println("Caméra initialisée !");
}

void loop() {
    sendImage();
    delay(66); // 15 FPS
}

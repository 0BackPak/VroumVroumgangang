#include <WiFi.h>
#include <esp_now.h>
#include <esp_camera.h>

// Sélection du modèle de caméra utilisé
#define CAMERA_MODEL_WROVER_KIT

#include "camera_pins.h"

// Définition de la structure pour l'image envoyée
esp_now_peer_info_t peerInfo;
uint8_t receiverMAC[] = {0x08, 0xA6, 0xF7, 0x12, 0xB3, 0x00}; // Adresse MAC de la manette
#define MAX_PAYLOAD_SIZE 250  // Limite de taille de paquet pour ESP-NOW (approximativement)

void setup() {
    Serial.begin(115200);

    // Initialisation du Wi-Fi en mode Station
    WiFi.mode(WIFI_STA);

    // Initialisation d'ESP-NOW
    esp_err_t initResult = esp_now_init();
    if (initResult != ESP_OK) {
        Serial.printf("Erreur d'initialisation d'ESP-NOW : %d\n", initResult);
        return;
    }
    Serial.println("ESP-NOW initialisé avec succès.");

    // Ajout du pair (récepteur)
    memcpy(peerInfo.peer_addr, receiverMAC, sizeof(receiverMAC));
    peerInfo.channel = 0;  // Le canal doit correspondre avec celui du récepteur
    peerInfo.encrypt = false;  // Aucun chiffrement

    esp_err_t addPeerResult = esp_now_add_peer(&peerInfo);
    if (addPeerResult != ESP_OK) {
        Serial.printf("Erreur lors de l'ajout du pair : %d\n", addPeerResult);
        return;
    }
    Serial.println("Pair ajouté avec succès.");

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
    config.frame_size = FRAMESIZE_QVGA;  // Taille de l'image (QVGA)
    config.jpeg_quality = 100;  // Qualité de l'image JPEG
    config.fb_count = 1;  // Nombre de buffers d'images

    // Initialisation de la caméra
    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Échec de l'initialisation de la caméra !");
        return;
    }
    Serial.println("Caméra initialisée avec succès.");
}

void loop() {
    // Capture une image
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Erreur lors de la capture de l'image !");
        return;
    } else {
      Serial.println("Image capturée !");
    }

    // Affichage de la taille de l'image capturée
    Serial.printf("Image capturée : taille = %d bytes\n", fb->len);

    // Envoi de l'image par morceaux
    size_t offset = 0;
    while (offset < fb->len) {
        size_t remaining = fb->len - offset;
        size_t chunkSize = (remaining > MAX_PAYLOAD_SIZE) ? MAX_PAYLOAD_SIZE : remaining;

        esp_err_t sendResult = esp_now_send(receiverMAC, fb->buf + offset, chunkSize);
        if (sendResult != ESP_OK) {
            Serial.print("Erreur d'envoi du paquet : ");
            Serial.println(sendResult);
            if (sendResult == ESP_ERR_NO_MEM) {
                Serial.println("Erreur : Pas assez de mémoire.");
            } else if (sendResult == ESP_ERR_INVALID_ARG) {
                Serial.println("Erreur : Argument invalide.");
            } else if (sendResult == ESP_ERR_TIMEOUT) {
                Serial.println("Erreur : Délai d'attente dépassé.");
            } else if (sendResult == ESP_ERR_INVALID_STATE) {
                Serial.println("Erreur : État invalide.");
            } else if(sendResult == ESP_ERR_ESPNOW_NO_MEM){
                Serial.println("Erreur mémoire");
                esp_camera_fb_return(fb);
            } else {
                Serial.println("Erreur non spécifiée.");
            }
        } else {
            Serial.println("Envoi du paquet réussi !");
        }

        // Mise à jour de l'offset pour le prochain paquet
        offset += chunkSize;

        // Attente de 10ms entre les envois pour éviter la surcharge
        delay(10);
    }

    // Message de confirmation après l'envoi complet de l'image
    Serial.println("Image envoyée avec succès !");

    // Libération du buffer de l'image après l'envoi
    esp_camera_fb_return(fb);

    delay(500); // Attente de 1 seconde avant la prochaine capture d'image
}

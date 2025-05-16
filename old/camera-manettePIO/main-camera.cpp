/**************************************************
 * ESPNowCam video Transmitter
 * by @hpsaturn Copyright (C) 2024
 * This file is part ESPNowCam project:
 * https://github.com/hpsaturn/ESPNowCam
 **************************************************/

 #include <Arduino.h>
 #include <esp_camera.h>
 #include <ESPNowCam.h>
 #include <Utils.h>
 
 #define CAMERA_MODEL_WROVER_KIT
 #include "camera_pins.h"  // Inclure le fichier camera_pins.h qui contient la configuration des pins
 
 ESPNowCam radio;  // Création d'un objet pour la communication via ESP-NOW
 camera_fb_t* fb;  // Pointeur vers un frame buffer pour l'image capturée
 
 bool has_psram = false;  // Vérifier la présence de PSRAM
 
 // Paramètres de la caméra
 int jpeg_quality = 40;  // Qualité de l'image JPEG ( 100 ~13FPS)
 
 // Macro pour ajuster la qualité du JPEG
 #define SET_JPEG_QUALITY(quality_value) \
   jpeg_quality = quality_value;
 
 unsigned long lastTime = 0;  // Temps de la dernière capture
 unsigned int frameCount = 0;  // Compteur d'images envoyées
 
 // Configuration de la caméra avec les pins définis dans camera_pins.h
 camera_config_t camera_config = {
     .pin_pwdn = PWDN_GPIO_NUM,
     .pin_reset = RESET_GPIO_NUM,
     .pin_xclk = XCLK_GPIO_NUM,
     .pin_sscb_sda = SIOD_GPIO_NUM,
     .pin_sscb_scl = SIOC_GPIO_NUM,
     .pin_d7 = Y9_GPIO_NUM,
     .pin_d6 = Y8_GPIO_NUM,
     .pin_d5 = Y7_GPIO_NUM,
     .pin_d4 = Y6_GPIO_NUM,
     .pin_d3 = Y5_GPIO_NUM,
     .pin_d2 = Y4_GPIO_NUM,
     .pin_d1 = Y3_GPIO_NUM,
     .pin_d0 = Y2_GPIO_NUM,
     .pin_vsync = VSYNC_GPIO_NUM,
     .pin_href = HREF_GPIO_NUM,
     .pin_pclk = PCLK_GPIO_NUM,
 
     .xclk_freq_hz = 20000000,  // Fréquence d'horloge XCLK
     .ledc_timer   = LEDC_TIMER_0,
     .ledc_channel = LEDC_CHANNEL_0,
 
     .pixel_format  = PIXFORMAT_JPEG,  // Format JPEG pour la capture d'images
     .frame_size = FRAMESIZE_QVGA, // Taille de l'image 
     .jpeg_quality  = jpeg_quality,  // Qualité JPEG modifiable via la macro
     .fb_count      = 1,  // Nombre de buffers pour le framebuffer
     .fb_location   = CAMERA_FB_IN_DRAM,  // Utilisation de la DRAM pour stocker les images
     .grab_mode     = CAMERA_GRAB_WHEN_EMPTY,  // Capture d'image quand le buffer est vide
 };
 
 // Fonction d'initialisation de la caméra
 bool CameraBegin() {
   esp_err_t err = esp_camera_init(&camera_config);
   if (err != ESP_OK) {
     Serial.println("Erreur d'initialisation de la caméra.");
     return false;  // Si l'initialisation échoue
   }
   return true;
 }
 
 // Fonction pour obtenir une image
 bool CameraGet() {
   fb = esp_camera_fb_get();
   if (!fb) {
     Serial.println("Erreur: Impossible de capturer une image.");
     return false;  // Si l'image ne peut pas être capturée
   }
   return true;
 }
 
 // Fonction pour libérer le framebuffer après usage
 bool CameraFree() {
   if (fb) {
     esp_camera_fb_return(fb);  // Libère la mémoire du framebuffer
     return true;
   }
   return false;
 }
 
 // Fonction pour traiter et envoyer l'image
 void processFrame() {
   // Capturer l'image
   if (CameraGet()) {
     frameCount++;  // Incrémenter le compteur d'images envoyées
     if (has_psram) {
       // Si PSRAM est disponible, on convertit le frame en JPEG et l'envoie
       uint8_t *out_jpg = NULL;
       size_t out_jpg_len = 0;
       frame2jpg(fb, jpeg_quality, &out_jpg, &out_jpg_len);  // Conversion en JPEG avec qualité modifiable
       radio.sendData(out_jpg, out_jpg_len);  // Envoi de l'image via ESP-NOW
       free(out_jpg);  // Libération de la mémoire du JPEG
     }
     else {
       // Si PSRAM n'est pas disponible, envoi direct du buffer de l'image
       radio.sendData(fb->buf, fb->len);  // Envoi de l'image
     }
     CameraFree();  // Libération de la mémoire de l'image
 
     // Afficher le nombre d'images envoyées toutes les secondes
     if (millis() - lastTime >= 1000) {
       Serial.printf("Images envoyées: %u\r\n", frameCount);
       frameCount = 0;
       lastTime = millis();
     }
 
     printFPS("\tCAM:");  // Affichage des FPS pour débogage
   }
 }
 
 void setup() {
   Serial.begin(115200);  // Initialisation du port série
   delay(5000);  // Délai pour le débogage
 
   if (psramFound()) {
     has_psram = true;  // Si PSRAM est disponible, activer les paramètres PSRAM
     size_t psram_size = esp_spiram_get_size() / 1048576;
     Serial.printf("PSRAM size: %dMb\r\n", psram_size);
     // Configuration optimisée pour PSRAM
     camera_config.pixel_format = PIXFORMAT_RGB565;
     camera_config.fb_location = CAMERA_FB_IN_PSRAM;
     camera_config.fb_count = 2;
   } else {
     Serial.println("PSRAM non trouvé ! Utilisation d'un framebuffer de base.");
     // Configuration de base sans PSRAM
     camera_config.pixel_format = PIXFORMAT_JPEG;  // Utilise JPEG pour limiter la taille
     camera_config.fb_location = CAMERA_FB_IN_DRAM; // Utilisation de la DRAM pour la mémoire image
     camera_config.fb_count = 1;  // Nombre de buffers de frame
   }
 
   radio.init();  // Initialisation du module ESP-NOW
 
   if (!CameraBegin()) {
     Serial.println("Erreur d'initialisation de la caméra.");
     delay(1000);
     ESP.restart();  // Redémarrage en cas d'échec d'initialisation
   }
   delay(500);  // Petite pause pour stabiliser
 }
 
 void loop() {
   processFrame();  // Capture et envoi de l'image
 }
 
#include <WiFi.h>
#include <esp_now.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>


#define CAMERA_MODEL_WROVER_KIT

#include "camera_pins.h"
#include "FS.h"
#include "SPIFFS.h"

// Écran TFT
TFT_eSPI tft = TFT_eSPI();

// Joystick
const int buttonPin = 32;
const int xPin = 34;
const int yPin = 35;

int buttonState = 0;
int xValue = 0;
int yValue = 0;

// MACs
uint8_t cameraMAC[] = {0x08, 0xA6, 0xF7, 0x12, 0xB3, 0x00};
uint8_t carMAC[]    = {0x08, 0xA6, 0xF7, 0x12, 0xB3, 0x68};

// Structure pour envoyer les données
typedef struct struct_message {
  float vitesse;
  float direction;
} struct_message;

struct_message message;

// Envoi vers la voiture
void sendJoystickData() {
  message.vitesse = map(yValue, 0, 4095, -100, 100) / 100.0;
  message.direction = map(xValue, 0, 4095, -100, 100) / 100.0;

  esp_err_t result = esp_now_send(carMAC, (uint8_t*)&message, sizeof(message));
  if (result == ESP_OK) {
    Serial.print("Y: ");
    Serial.print(yValue);
    Serial.print(" | X: ");
    Serial.println(xValue);
  } else {
    Serial.print("Erreur envoi : ");
    Serial.println(result);
  }
}

// Réception image
void onReceiveData(const uint8_t *mac, const uint8_t *data, int len) {
  Serial.println("Image reçue");

  File file = SPIFFS.open("/image.jpg", FILE_WRITE);
  if (!file) {
    Serial.println("Erreur ouverture fichier image");
    return;
  }

  file.write(data, len);
  file.close();

  Serial.println("Affichage image...");
  TJpgDec.drawFsJpg(0, 0, "/image.jpg");
}



// Setup
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur init ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW OK");

  // Ajouter la caméra comme peer
  esp_now_peer_info_t peerCam = {};
  memcpy(peerCam.peer_addr, cameraMAC, 6);
  peerCam.channel = 0;
  peerCam.encrypt = false;
  if (esp_now_add_peer(&peerCam) != ESP_OK) {
    Serial.println("Erreur ajout peer caméra");
  }

  // Ajouter la voiture comme peer
  esp_now_peer_info_t peerCar = {};
  memcpy(peerCar.peer_addr, carMAC, 6);
  peerCar.channel = 0;
  peerCar.encrypt = false;
  if (esp_now_add_peer(&peerCar) != ESP_OK) {
    Serial.println("Erreur ajout peer voiture");
  }

  // Callback de réception image
  esp_now_register_recv_cb(onReceiveData);

  // TFT
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Erreur SPIFFS !");
    return;
  }

  pinMode(buttonPin, INPUT_PULLUP);
}

// Boucle
void loop() {
  // Lire le joystick
  buttonState = digitalRead(buttonPin);
  xValue = analogRead(xPin);
  yValue = analogRead(yPin);

  // Afficher sur TFT
  tft.fillRect(0, 0, 128, 160, TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.printf("Joystick X: %d\n", xValue);
  tft.printf("Joystick Y: %d\n", yValue);
  tft.setCursor(10, 120);
  tft.setTextColor(buttonState == LOW ? TFT_RED : TFT_GREEN);
  tft.println(buttonState == LOW ? "Bouton Pressé" : "Bouton Relâché");

  // Envoyer à la voiture
  sendJoystickData();

  delay(100);
}


#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <math.h>

uint8_t carMAC[] = {0x08, 0xD1, 0xF9, 0x35, 0x50, 0x7C};

const int buttonPin = 32;
const int xPin = 34;
const int yPin = 35;

int buttonState = 0;
int xValue = 0;
int yValue = 0;

typedef struct __attribute__((packed)) {
  float vitesse;
  float direction;
} Message;

Message message;


const int redPin = 5;
const int greenPin = 18;
const int bluePin = 19;
const int pilePin = 39; // 39 ici pour pas avoir deux fois le 34 !

const float error = 0.13;

enum EtatBatterie { ROUGE, JAUNE, VERT };
EtatBatterie etatActuel = ROUGE;


float nonlinearMap(int val) {
  float norm = ((float)val - 2047.5) / 2047.5;
  if (abs(norm + 0.1) < 0.2) return 0.0;
  float scale = 2.5;
  return tanh(scale * norm) * 3.55;
}

void sendJoystickData() {
  xValue = analogRead(xPin);
  yValue = analogRead(yPin);
  buttonState = digitalRead(buttonPin); // Non utilisé pour l'instant

  message.vitesse = nonlinearMap(yValue);
  message.direction = nonlinearMap(xValue);

  esp_err_t result = esp_now_send(carMAC, (uint8_t*)&message, sizeof(message));
  if (result == ESP_OK) {
    Serial.print("X: "); Serial.print(xValue);
    Serial.print(" | Y: "); Serial.print(yValue);
    Serial.print(" -> Dir: "); Serial.print(message.direction);
    Serial.print(" | Vit: "); Serial.println(message.vitesse);
  } else {
    Serial.print("Erreur envoi : ");
    Serial.println(result);
  }
}

void verifierBatterie() {
  int raw = analogRead(pilePin);
  float v_adc = ((raw / 4095.0) * 3.3) - error;
  float v_batt = v_adc * ((3.28 + 1.96) / 1.96); 

  Serial.print("Tension batterie : ");
  Serial.print(v_batt);
  Serial.println(" V");

  switch (etatActuel) {
    case VERT:
      if (v_batt < 7.2) etatActuel = JAUNE;
      break;
    case JAUNE:
      if (v_batt >= 7.5) etatActuel = VERT;
      else if (v_batt < 6.1) etatActuel = ROUGE;
      break;
    case ROUGE:
      if (v_batt >= 6.5) etatActuel = JAUNE;
      break;
  }

 
  switch (etatActuel) {
    case VERT:
      setColor(LOW, HIGH, LOW);
      break;
    case JAUNE:
      setColor(HIGH, HIGH, LOW);
      break;
    case ROUGE:
      setColor(HIGH, LOW, LOW);
      break;
  }
}

void setColor(int redValue, int greenValue, int blueValue) {
  digitalWrite(redPin, redValue);
  digitalWrite(greenPin, greenValue);
  digitalWrite(bluePin, blueValue);
}

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(pilePin, INPUT);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur init ESP-NOW");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, carMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Erreur ajout peer");
    return;
  }
}

void loop() {
  sendJoystickData();
  verifierBatterie();
  delay(500);
}

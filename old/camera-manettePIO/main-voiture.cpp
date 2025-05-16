#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pins des moteurs
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Moteur A (Droit)
#define pinMotorENA 27
#define pinMotorIN1 14
#define pinMotorIN2 21

// Moteur B (Gauche)
#define pinMotorENB 33
#define pinMotorIN3 32
#define pinMotorIN4 35

// PWM
#define pwm_channel_A 0
#define frequency_A 20000
#define resolution_A 10

#define pwm_channel_B 1
#define frequency_B 20000
#define resolution_B 10

// Encodeurs
#define interruptPinAC1 26
#define interruptPinAC2 25
#define interruptPinBC1 18
#define interruptPinBC2 19

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables globales pour les moteurs et PID
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
volatile int countA = 0;
volatile int countB = 0;

volatile int cmdPA = 0;
volatile int cmdPB = 0;

volatile float cmdA = 0.0;
volatile float cmdB = 0.0;
volatile float max_vit = 2.0;

volatile int pwmA = 0;
volatile int pwmB = 0;
volatile int max_pwmA = 600;
volatile int max_pwmB = 600;


typedef struct struct_message {
  float vitesse;  
  float direction;
} struct_message;

struct_message message;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback de réception des données
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void onDataReceived(const uint8_t * mac_addr, const uint8_t *data, int len) {
  memcpy(&message, data, sizeof(message));

  // Affichage des valeurs reçues
  Serial.print("Reçu -> Vitesse (X) : ");
  Serial.print(message.vitesse);
  Serial.print(" | Direction (Y) : ");
  Serial.println(message.direction);

  // Commenté pour ne pas contrôler les moteurs pendant les tests
  // cmdA = message.vitesse + message.direction * 0.5;
  // cmdB = message.vitesse - message.direction * 0.5;
}

// Fonctions de contrôle des moteurs
void sens_moteurA() {
  if (cmdA > 0) {
    digitalWrite(pinMotorIN1, HIGH);
    digitalWrite(pinMotorIN2, LOW);
  } else {
    digitalWrite(pinMotorIN1, LOW);
    digitalWrite(pinMotorIN2, HIGH);
  }
}

void sens_moteurB() {
  if (cmdB > 0) {
    digitalWrite(pinMotorIN3, HIGH);
    digitalWrite(pinMotorIN4, LOW);
  } else {
    digitalWrite(pinMotorIN3, LOW);
    digitalWrite(pinMotorIN4, HIGH);
  }
}

void asserv_vit_motA() {
  pwmA = constrain(cmdA * max_pwmA, 0, max_pwmA);
  ledcWrite(pwm_channel_A, pwmA);
}

void asserv_vit_motB() {
  pwmB = constrain(cmdB * max_pwmB, 0, max_pwmB);
  ledcWrite(pwm_channel_B, pwmB);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.println("Initialisation Wi-Fi en mode station");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erreur d'initialisation d'ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataReceived);

  // Pins moteurs
  pinMode(pinMotorENA, OUTPUT);
  pinMode(pinMotorIN1, OUTPUT);
  pinMode(pinMotorIN2, OUTPUT);

  pinMode(pinMotorENB, OUTPUT);
  pinMode(pinMotorIN3, OUTPUT);
  pinMode(pinMotorIN4, OUTPUT);

  ledcSetup(pwm_channel_A, frequency_A, resolution_A);
  ledcAttachPin(pinMotorENA, pwm_channel_A);

  ledcSetup(pwm_channel_B, frequency_B, resolution_B);
  ledcAttachPin(pinMotorENB, pwm_channel_B);

  pinMode(interruptPinAC1, INPUT);
  pinMode(interruptPinAC2, INPUT);
  pinMode(interruptPinBC1, INPUT);
  pinMode(interruptPinBC2, INPUT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loop principal
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Juste attendre les données
  delay(100);

  /*
  // Contrôle moteur désactivé pour test
  sens_moteurA();
  sens_moteurB();

  asserv_vit_motA();
  asserv_vit_motB();

  static unsigned long previousDebugMillis = 0;
  const long debugInterval = 2000;
  unsigned long currentMillis = millis();

  if (currentMillis - previousDebugMillis >= debugInterval) {
    previousDebugMillis = currentMillis;

    Serial.print("Commande A: ");
    Serial.print(cmdA);
    Serial.print(" | Commande B: ");
    Serial.println(cmdB);

    Serial.print("PWM A: ");
    Serial.print(pwmA);
    Serial.print(" | PWM B: ");
    Serial.println(pwmB);
  }
  */
}

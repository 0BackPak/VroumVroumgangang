#include <WiFi.h>
#include <esp_now.h>

// Structure pour l'image reçue
typedef struct {
    size_t jpg_length;  // Longueur de l'image
    uint8_t *jpg_data;  // Données de l'image
} esp_now_frame_t;

bool videoStreamActive = false;

// Définition des pins du joystick et du bouton
const int buttonPin = 32;
const int xPin = 34; // VRX pin
const int yPin = 35; // VRY pin

int buttonState = 0;
int xValue = 0;
int yValue = 0;

// Variable globale pour compter les images reçues
unsigned int imagesReceivedCount = 0;

// Variables globales pour stocker les informations sur l'image reçue
#define MAX_PAYLOAD_SIZE 250  // Taille maximale d'un paquet, ajuster si nécessaire

#define MAX_IMAGE_SIZE 10000  // Réduire la taille du tampon d'image
uint8_t imageBuffer[MAX_IMAGE_SIZE];


size_t imageSize = 0;  // Taille totale de l'image reçue
size_t expectedImageSize = 0;  // Taille de l'image attendue (à initialiser à la réception du premier morceau)
bool imageReceivedComplete = false;  // Indique si l'image est complète

void onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
    esp_now_frame_t *frame = (esp_now_frame_t *)data;

    // Si c'est le premier morceau, on initialise la taille de l'image attendue
    if (imageSize == 0) {
        expectedImageSize = frame->jpg_length;
        Serial.printf("Image attendue de taille: %d bytes\n", expectedImageSize);
    }

    // Vérifie si le morceau reçu est valide
    if (frame->jpg_length > 0 && frame->jpg_length <= MAX_PAYLOAD_SIZE) {
        Serial.printf("Image reçue, taille du morceau: %d bytes\n", len);

        // Copie les données du morceau dans le tampon de réception
        memcpy(imageBuffer + imageSize, frame->jpg_data, len);

        // Mise à jour de la taille de l'image reçue
        imageSize += len;

        // Vérifie si l'image est complète
        if (imageSize == expectedImageSize) {
            imageReceivedComplete = true;
            Serial.println("Image complète reçue !");
            // Traiter l'image complète ici (par exemple, l'afficher)
        }
    } else {
        Serial.println("Erreur : Taille du morceau invalide.");
    }

    // Incrémente le compteur d'images reçues
    imagesReceivedCount++;

    // Si le flux vidéo n'est pas déjà actif, on l'active
    if (!videoStreamActive) {
        videoStreamActive = true;
        Serial.println("Flux vidéo connecté !");
    }

    delay(1000);
}


// Définir l'adresse MAC du récepteur (manette)
esp_now_peer_info_t peerInfo;

void setup() {
    Serial.begin(115200);
    Serial.print("setup() running on core ");
    Serial.println(xPortGetCoreID());

    WiFi.mode(WIFI_STA);
    esp_err_t initResult = esp_now_init();
    if (initResult != ESP_OK) {
        Serial.printf("Erreur d'initialisation d'ESP-NOW: %d\n", initResult);
        return;
    }
    Serial.println("ESP-NOW initialisé avec succès.");

    // Enregistrement du callback de réception
    esp_now_register_recv_cb(onDataReceived);

    Serial.print("Adresse MAC du récepteur : ");
    Serial.println(WiFi.macAddress());

    // Configuration des entrées
    pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  /*
    Serial.print("loop() running on core ");
    Serial.println(xPortGetCoreID());
    */

    /*
    if (videoStreamActive) {
        Serial.println("Le flux vidéo est actif !");
    } else {
        Serial.println("Pas de connexion vidéo active.");
    }
    */
    
    // Affiche le nombre d'images reçues
    Serial.printf("Nombre d'images reçues: %d\n", imagesReceivedCount);

    // Lecture des valeurs du joystick et du bouton
    buttonState = digitalRead(buttonPin);
    xValue = analogRead(xPin);
    yValue = analogRead(yPin);

    /* Affichage des valeurs du joystick et du bouton
    Serial.print("vx : ");
    Serial.print(xValue);
    Serial.print("\tvy : ");
    Serial.print(yValue);
    Serial.print("\tBouton : ");
    Serial.println(buttonState);
    */

    delay(1000); // Ajuste le délai si nécessaire
}

#include <WiFi.h>
#include <WebServer.h>
#include "page_web.h"

const char* ssid = "Galaxy A12F446";  // Mettre le ssid de votre réseau Wifi
const char* password = "Jaidelacoetpastoi";   // Mettre le mot de passe de votre réseau Wifi
WebServer server(80);  // Permet l'affichage de la page d'accueil


void handleRoot() {
    server.send(200, "text/html", PAGE_HTML);
}

void goForward() {
    Serial.println("I go forward");
}

void goBackward() {
    Serial.println("I go backward");
}

void goLeft() {
    Serial.println("I go left");
}

void goRight() {
    Serial.println("I go right");
}

void handleDirection(String direction) {
    Serial.println(direction); // Affiche la direction reçue dans la console

    // Utilisation d'un switch pour appeler la fonction appropriée
    switch (direction.c_str()[0]) {  // Utilisation de la première lettre de la direction pour déterminer l'action
        case 'f':
            goForward();
            server.send(200, "text/plain", "Commande reçue : Aller en avant");
            break;
        case 'b':
            goBackward();
            server.send(200, "text/plain", "Commande reçue : Aller en arrière");
            break;
        case 'l':
            goLeft();
            server.send(200, "text/plain", "Commande reçue : Aller à gauche");
            break;
        case 'r':
            goRight();
            server.send(200, "text/plain", "Commande reçue : Aller à droite");
            break;
        default:
            server.send(400, "text/plain", "Commande invalide");
            break;
    }
}


void handleNotFound() {  // Page Not found
    server.send(404, "text/plain", "404: Not found");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n");

  WiFi.begin(ssid, password);  // Initialisation du Wifi
  Serial.print("Attente de connexion ...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\n");
  Serial.println("Connexion réussi");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/forward", []() { handleDirection("forward"); });
  server.on("/backward", []() { handleDirection("backward"); });
  server.on("/left", []() { handleDirection("left"); });
  server.on("/right", []() { handleDirection("right"); });
  server.onNotFound(handleNotFound);  // Page Not found

  server.begin();  // Initialisation du serveur web
  Serial.println("Serveur web actif");
}

void loop() {
  server.handleClient();  // Attente de demande du client
}

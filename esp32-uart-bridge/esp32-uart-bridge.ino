/*
 * USB CDC (PC) <-> UART (routeur Tenda MW6), transparent et bidirectionnel.
 *
 * Dans Arduino IDE, activer « USB CDC On Boot » pour que Serial soit le port
 * USB natif de la carte. Serial1 reste l'UART matériel vers le routeur.
 * Les broches ci-dessous ne sont que des valeurs de départ : vérifier le
 * brochage et les fonctions déjà utilisées par le modèle exact de carte.
 */

#include <Arduino.h>

#ifndef ARDUINO_USB_CDC_ON_BOOT
#error "Sélectionnez une cible ESP32 avec USB CDC natif et activez USB CDC On Boot"
#elif !ARDUINO_USB_CDC_ON_BOOT
#error "Activez Tools > USB CDC On Boot pour utiliser Serial sur l'USB natif"
#endif

// Valeurs par défaut éditables ; voir README.md avant le câblage.
#ifndef UART_RX_PIN
#if defined(CONFIG_IDF_TARGET_ESP32C6)
#define UART_RX_PIN 10
#elif defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)
#define UART_RX_PIN 4
#else
#error "Ce sketch est prévu pour ESP32-S3, ESP32-C3 ou ESP32-C6"
#endif
#endif

#ifndef UART_TX_PIN
#if defined(CONFIG_IDF_TARGET_ESP32C6)
#define UART_TX_PIN 11
#elif defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3)
#define UART_TX_PIN 5
#else
#error "Ce sketch est prévu pour ESP32-S3, ESP32-C3 ou ESP32-C6"
#endif
#endif

constexpr uint32_t ROUTER_BAUD = 115200;
constexpr size_t BUFFER_SIZE = 64;

void setup() {
  Serial.begin(115200);  // USB CDC ; le débit indiqué par le terminal n'est pas un débit UART
  Serial1.begin(ROUTER_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
}

void loop() {
  uint8_t buffer[BUFFER_SIZE];

  int available = Serial.available();
  if (available > 0) {
    size_t count = static_cast<size_t>(available);
    if (count > sizeof(buffer)) {
      count = sizeof(buffer);
    }
    count = Serial.read(buffer, count);
    Serial1.write(buffer, count);
  }

  available = Serial1.available();
  if (available > 0) {
    size_t count = static_cast<size_t>(available);
    if (count > sizeof(buffer)) {
      count = sizeof(buffer);
    }
    count = Serial1.read(buffer, count);
    Serial.write(buffer, count);
  }
}

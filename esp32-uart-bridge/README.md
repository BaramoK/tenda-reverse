# Pont USB-CDC ↔ UART ESP32 pour le Tenda MW6

Ce sketch Arduino transforme une carte **ESP32-S3, ESP32-C3 ou ESP32-C6 avec USB natif** en adaptateur série bidirectionnel. Il relaie les octets entre le port USB CDC visible sur l'ordinateur et `Serial1`, raccordé à la console UART J4 du MW6 en **115200 bauds, 8N1**. Il ne programme pas la flash et ne produit aucun message ajouté au flux série.

> Cette fonction est distincte du programmateur SPI ESP8266/Arduino dans [`../arduino-spi-flasher/`](../arduino-spi-flasher/), qui sert à lire une mémoire SPI et ne fournit pas la console UART.

## Compiler et téléverser

1. Installer Arduino IDE et le paquet de cartes **Arduino ESP32 par Espressif**. Choisir le modèle de carte correspondant au SoC et au matériel réels ; ne pas supposer que toutes les cartes S3/C3/C6 ont le même brochage ou le même connecteur USB.
2. Ouvrir `esp32-uart-bridge.ino` dans l'IDE. Le fichier peut être ouvert directement depuis ce dossier.
3. Dans **Tools**, activer **USB CDC On Boot**. `Serial` désigne alors le CDC natif et `Serial1` l'UART matériel.
   - **ESP32-S3** : la carte peut exposer l'USB Serial/JTAG ou l'USB-OTG. Si le menu **USB Mode** est proposé, choisir le mode relié au connecteur de la carte. Le mode USB-OTG (TinyUSB) et le mode Hardware CDC and JTAG sont deux périphériques USB différents ; le choix utilisable dépend du câblage de la carte et du port branché. En mode USB-OTG, garder le CDC activé au démarrage.
   - **ESP32-C3 / ESP32-C6** : le CDC natif passe par le périphérique USB Serial/JTAG du SoC, pas par USB-OTG/TinyUSB. Utiliser le connecteur USB relié aux broches USB du SoC et activer **USB CDC On Boot**.
4. Sélectionner le port de la carte et téléverser. Après le redémarrage, sélectionner le port série USB CDC réapparu pour le moniteur série. L'énumération et le nom du port dépendent de la carte et du système d'exploitation.

Le débit affiché dans le moniteur série USB (par exemple **115200**) ne règle pas le débit physique USB ; le débit UART vers le MW6 est fixé à 115200 dans le sketch.

## Broches par défaut et configuration

Les broches sont définies au début du sketch par `UART_RX_PIN` et `UART_TX_PIN`. Valeurs proposées :

| SoC | RX ESP32 (entrée depuis TX routeur) | TX ESP32 (vers RX routeur) | Remarque |
|---|---:|---:|---|
| ESP32-S3 | GPIO4 | GPIO5 | Vérifier que le module/carte ne réserve pas ces GPIO. |
| ESP32-C3 | GPIO4 | GPIO5 | Ces GPIO peuvent aussi servir au JTAG ; choisir d'autres pins si celui-ci est utilisé ou si la carte les réserve. |
| ESP32-C6 | GPIO10 | GPIO11 | GPIO4/5 sont des broches de strapping sur le C6 ; ne pas reprendre sans vérification les valeurs proposées pour S3/C3. |

Ces valeurs ne sont **pas universelles** : examiner le schéma et le brochage de la carte exacte, les broches exposées, les fonctions réservées (flash/PSRAM, USB, LED, périphériques intégrés) et les contraintes de démarrage. Adapter les deux macros si nécessaire. L'UART est routable sur différents GPIO, mais toutes les broches ne sont pas nécessairement utilisables sur une carte donnée.

## Câblage au connecteur J4

Le dépôt décrit J4, en partant de la broche 1 du côté de la flash, comme **VCC, RX, TX, GND**. Routeur éteint pendant le câblage :

| ESP32 | Tenda MW6 J4 |
|---|---|
| GND | GND |
| GPIO configuré `UART_TX_PIN` | RX routeur |
| GPIO configuré `UART_RX_PIN` | TX routeur |
| **Ne rien connecter** | **VCC** |

Les lignes TX/RX sont croisées. Le routeur doit être alimenté normalement ; **ne reliez jamais le VCC de J4** à la carte ESP32 ou à son alimentation. Reliez les masses et vérifiez la compatibilité de niveau logique avant branchement (UART 3,3 V, pas RS-232). Évitez de connecter en même temps un autre adaptateur sur les mêmes lignes TX/RX.

## Test de bouclage avant le routeur

1. Téléverser le sketch, ouvrir le moniteur série USB et choisir 115200 bauds.
2. Sans routeur branché, relier temporairement avec un cavalier le GPIO configuré `UART_TX_PIN` au GPIO `UART_RX_PIN` de l'ESP32.
3. Taper quelques caractères dans le moniteur : ils doivent revenir à l'écran par `Serial1` et être retransmis en USB. Retirer le cavalier avant le câblage du routeur.

## Utilisation et dépannage

- Fermer puis rouvrir le moniteur série si le port USB change après téléversement ou redémarrage. Sur Linux, le périphérique est souvent `/dev/ttyACM0` (parfois un autre nom) ; le consulter avec `arduino-cli board list` ou l'outil de ports de l'IDE. Exemple : `picocom -b 115200 /dev/ttyACM0`.
- Si le téléversement échoue, fermer tout moniteur qui occupe le port, débrancher/rebrancher la carte et sélectionner le nouveau port. Au besoin, suivre la procédure de récupération propre à la carte : généralement maintenir **BOOT**, réinitialiser ou reconnecter l'USB, puis relâcher BOOT quand le port de téléchargement apparaît. Les boutons et la séquence exacte varient selon le modèle.
- Si la carte ne réapparaît pas, vérifier **USB CDC On Boot**, le mode USB S3 le cas échéant, le port physique connecté au périphérique natif, le câble USB de données et le brochage exact.
- Pas de réponse du routeur : vérifier masse commune, croisement TX/RX, niveaux logiques et réglage 115200 8N1. Ne pas connecter VCC. Le mot de passe de maintenance et les limites décrites dans le [guide rapide](../QUICKSTART.md) restent applicables.

Le sketch a besoin du cœur Arduino ESP32 ; aucun outil de compilation Arduino n'est requis par le reste du dépôt. Pour les détails d'API et la prise en charge USB par SoC, voir la [documentation USB CDC Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/usb_cdc.html) et la [documentation USB Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/usb.html). La console donne accès à l'équipement : ne l'utilisez que sur un appareil que vous êtes autorisé à administrer.

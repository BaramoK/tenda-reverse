# Démarrage rapide — Tenda MW6

Ce guide résume les informations du dépôt pour le MW6 hw2.1 avec le firmware `MESH_V1.0.0.28(1887_5834_815)_cn`. Il s'agit d'une recherche inverse communautaire, pas d'une procédure officielle Tenda. Les observations ne sont pas garanties sur d'autres versions ou modèles.

> **Accès root : pas besoin de flasher ni de dessouder la mémoire.** Le chemin décrit ici utilise la console UART et le mot de passe Wi-Fi. N'appliquez les commandes qu'à un appareil qui vous appartient ou que vous êtes autorisé à administrer.

## 1. Obtenir la console UART et un shell root

Le connecteur J4 expose la console série `ttyS0`, configurée à **115200 bauds, 8 bits, sans parité, 1 bit d'arrêt (115200 8N1)**. La photo du PCB et du connecteur est dans [`images/tenda-mw6-board-uart.png`](images/tenda-mw6-board-uart.png). D'après le dépôt, en partant de la broche 1 du côté de la flash, l'ordre est VCC, RX, TX, GND.

1. Utilisez un adaptateur **USB–TTL 3,3 V**, pas un adaptateur RS-232. Routeur alimenté, reliez la masse (GND) et les lignes TX/RX croisées (TX de l'adaptateur vers RX du routeur et inversement). **Ne raccordez pas VCC** si le routeur est déjà alimenté. Une carte ESP32-S3/C3/C6 à USB natif peut aussi servir de pont série avec le [sketch USB-CDC ↔ UART](esp32-uart-bridge/README.md) ; ses broches et son mode USB dépendent du modèle de carte.
2. Ouvrez le terminal série, par exemple :

   ```sh
   picocom -b 115200 /dev/ttyUSB0
   ```

   Remplacez `/dev/ttyUSB0` par le port attribué à votre adaptateur.
3. Au démarrage, le système demande le mot de passe du mode maintenance (`sulogin`). Le dépôt rapporte que le processus `netctrl` remplace le mot de passe root au démarrage par le mot de passe Wi-Fi encodé en Base64. Calculez cette valeur sur votre ordinateur :

   ```sh
   printf %s 'MOT_DE_PASSE_WIFI' | base64
   ```

   Saisissez le résultat au prompt. Après une réinitialisation usine, le mot de passe Wi-Fi imprimé sur l'étiquette peut être celui configuré ; sinon, prenez le mot de passe Wi-Fi actuel. La ligne ne s'affiche pas nécessairement pendant la saisie.

Le dépôt montre une session aboutissant à `System Maintenance Mode` et à un shell `root` (voir [`README.md`](README.md), section « I'm in » et [`inside_tenda.log`](inside_tenda.log)). Si l'authentification échoue, vérifiez que la clé Wi-Fi est exacte et que la version du firmware correspond ; ne modifiez pas la flash pour contourner l'échec.

### À quoi sert l'UART ?

C'est une console série pour voir les messages de démarrage et interagir avec le système ; elle n'est ni un accès réseau, ni un programmeur SPI, ni du JTAG. Le dépôt documente également le prompt du bootloader RealTek sur cette console, mais ses commandes d'écriture flash sont risquées. La console UART ne nécessite pas de flasher ni de remplacer la ROM.

## 2. Sauvegarde de flash (facultatif, avant une opération sur la flash)

Cette étape **n'est pas nécessaire** pour obtenir le shell root ni pour les réglages `cfm` ci-dessous. Elle nécessite un programmateur SPI externe et des compétences en soudure/électronique. Le dépôt indique que la puce BOHONG d'origine n'est pas reconnue correctement par la version de `flashrom` testée ; l'auteur a obtenu de meilleurs résultats avec une puce Winbond W25Q64FV de remplacement. Ne tentez pas de lire ou écrire une puce alimentée simultanément par le routeur et le programmateur. Faites une sauvegarde vérifiée avant toute écriture ; une erreur peut rendre le routeur inutilisable.

Avec une puce et un programmateur compatibles, la commande de lecture illustrée dans le dépôt est :

```sh
flashrom -p ft2232_spi:type=2232H,port=A -r dump.bin
```

La flash fait 8 Mio. Les principales régions sont listées dans [`mtd_blocks.md`](mtd_blocks.md). Les offsets/tailles sont propres à l'image analysée ; vérifiez le modèle et la version avant d'extraire une région. Pour inspecter les signatures d'une sauvegarde :

```sh
binwalk dump.bin
```

Dans le dump du dépôt, le RootFS SquashFS commence à l'offset `0x228012`, et la partition MTD RootFS se termine à `0x5c0000`. Pour en extraire les octets avec GNU `dd` :

```sh
dd if=dump.bin of=RootFS.bin bs=1 skip=$((0x228012)) count=$((0x5c0000-0x228012))
unsquashfs -d RootFS-unpacked-local RootFS.bin
```

Les tailles/offsets correspondent aux notes de [`README-ru.md`](README-ru.md) et à [`mtd_blocks.md`](mtd_blocks.md), pas à une procédure universelle.

## 3. Désactiver le serveur DHCP

Depuis le shell root du routeur, inspectez d'abord la configuration existante et notez la valeur actuelle :

```sh
cfm get ^dhcps
```

Le dépôt rapporte que régler `dhcps.listnum` à `0` empêche la création de `/etc/dhcps.conf` au démarrage, désactivant ainsi le serveur DHCP :

```sh
cfm set dhcps.listnum 0
```

Ce réglage est persistant. Pour revenir en arrière, restaurez la valeur d'origine notée précédemment ou effectuez une réinitialisation usine (qui efface aussi les autres réglages). Vérifiez que le réseau conserve un autre serveur DHCP avant de désactiver celui du Tenda.

## 4. Configurer la DMZ

Le README indique que l'application limite les règles de redirection de ports à huit. Pour définir une adresse IP DMZ, remplacez `<IP_CIBLE>` par une adresse adaptée à votre réseau :

```sh
cfm set wan1.dmzip <IP_CIBLE>
cfm set wan1.dmzen 1
reboot
```

La DMZ expose largement la machine cible aux connexions entrantes : utilisez-la avec prudence, sur un appareil dont le pare-feu est correctement configuré.

## 5. Limites et précautions

- La tentative décrite dans le README de modifier puis reconstruire le RootFS n'a pas démarré : le bootloader a signalé `rootfs checksum error at 00228012`. Recompresser en SquashFS XZ et compléter le fichier avec des octets `0xFF` ne suffit pas à recalculer ce contrôle d'intégrité.
- Le journal mentionne une fenêtre d'upload réseau/TFTP dans le bootloader, mais ce n'est pas une procédure de restauration complète validée dans ce dépôt. Ne supposez pas qu'elle récupérera une image arbitraire.
- Un appui sur Reset d'environ 3 secondes est rapporté comme activant le service telnet ; environ 6 secondes réinitialisent les paramètres d'usine. Les identifiants telnet ne sont pas établis ici. Ce n'est pas le chemin recommandé pour le shell root.
- Les commandes du bootloader telles que `FLW`, `FLERASE` ou `ERASECHIP` peuvent écraser la flash. Ne les utilisez pas sans sauvegarde complète, procédure de restauration vérifiée et matériel approprié.
- Ce dépôt n'a pas de build global ni de suite de tests. Le sketch ESP32 de pont UART est autonome et se compile séparément avec le cœur Arduino ESP32 ; voir [`esp32-uart-bridge/README.md`](esp32-uart-bridge/README.md).

## Pour approfondir

- [`README.md`](README.md) — récit principal et manipulations documentées.
- [`README-ru.md`](README-ru.md) — notes détaillées, notamment les commandes d'extraction.
- [`boot.log`](boot.log), [`inside_tenda.log`](inside_tenda.log), [`flashrom.log`](flashrom.log) — traces capturées.
- [`bootloader options.md`](<bootloader options.md>), [`mtd_blocks.md`](mtd_blocks.md) — commandes et disposition mémoire.
- [`esp32-uart-bridge/README.md`](esp32-uart-bridge/README.md) — sketch ESP32-S3/C3/C6 pour faire le pont USB-CDC ↔ console UART J4 ; **VCC de J4 non connecté**.
- [`arduino-spi-flasher/README.md`](arduino-spi-flasher/README.md) — matériel Arduino/ESP8266 de lecture SPI, distinct du pont de console UART, avec client .NET indiqué pour Windows.

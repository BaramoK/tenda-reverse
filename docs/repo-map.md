# Carte du dépôt

> Carte rapide du dépôt `tenda-reverse`, consultée le 16 novembre 2026. Pour les commandes pratiques, voir [`../QUICKSTART.md`](../QUICKSTART.md).

## Objectif

Notes et artefacts de rétro-ingénierie du routeur mesh Tenda MW6 « Nova » (notamment hw2.1 / firmware `MESH_V1.0.0.28(1887_5834_815)_cn`). L'enquête cherche entre autres à comprendre les services réseau et à désactiver le DHCP en mode bridge. C'est un dépôt de documentation et d'artefacts, pas un logiciel applicatif à construire.

## Organisation

| Chemin | Rôle |
|---|---|
| [`../README.md`](../README.md) | Récit principal : matériel, accès série, firmware, root, DHCP et DMZ. |
| [`../README-ru.md`](../README-ru.md) | Version russe détaillée, avec des commandes d'analyse supplémentaires. |
| [`../QUICKSTART.md`](../QUICKSTART.md) | Recettes pratiques en français : UART, sauvegarde, `cfm`. |
| `firmware-dump-tenda-mw6-hw2.1-fw-1.0.0.28(5834).bin` | Dump binaire de 8 Mio ; l'original à préserver. |
| `image-unpacked-as-per-mtd/` | Régions de flash découpées selon les notes MTD. |
| `RootFS-unpacked/` | Arborescence extraite du RootFS : `etc_ro/`, exécutables, bibliothèques et modules. |
| `change-to-do/` | Essai de modification d'`inittab` ayant échoué au contrôle CRC au démarrage. |
| `arduino-spi-flasher/` | Archives et notes pour un programmateur SPI basé sur Arduino/ESP8266. |
| `GPL/` | Documentation minimale ; l'archive des sources GPL est gérée en Git LFS et absente de ce clone. |
| `boot.log`, `inside_tenda.log`, `flashrom.log` | Captures de démarrage, shell et essais de lecture de flash. |
| `mtd_blocks.md`, `bootloader options.md`, `_reverse__log.md` | Notes sur la flash, le bootloader RealTek et le reverse engineering. |
| `images/`, `BH25Q64BSSIG_C691572.pdf` | Photos et datasheet de la mémoire SPI BOHONG. |
| `firmware-version.md` | Identifiant de version noté par l'auteur. |

## Points d'entrée et commandes

- **Comprendre le projet** : commencer par [`../README.md`](../README.md), puis consulter [`../QUICKSTART.md`](../QUICKSTART.md).
- **Inspecter l'image** : `binwalk firmware-dump-tenda-mw6-hw2.1-fw-1.0.0.28\(5834\).bin` (échapper les parenthèses dans un shell) ; les offsets sont documentés dans `mtd_blocks.md`.
- **Accéder à un shell sur le routeur** : console UART J4, 115200 8N1 ; le dépôt rapporte un mot de passe root calculé depuis le mot de passe Wi-Fi encodé en Base64. Ce chemin ne nécessite pas de reflash.
- **Modifier la configuration persistante** : sur l'appareil avec accès root, utilitaire `cfm` ; le README montre `cfm get ^dhcps`, `cfm set dhcps.listnum 0` et les paramètres DMZ. Le stockage de configuration est associé à `/dev/mtd5`.
- **Lire la flash** : exemple documenté `flashrom -p ft2232_spi:type=2232H,port=A -r dump.bin`. La BOHONG d'origine a posé problème dans la version testée ; voir le README avant d'essayer.
- **Analyser le RootFS** : `unsquashfs` ; les notes de reconstruction indiquent SquashFS/XZ, mais l'image modifiée a échoué au contrôle CRC du bootloader.

## Architecture et dépendances clés

Le firmware observé utilise un SoC Realtek RTL8197F avec CPU MIPS 24Kc, Linux 3.10.90, uClibc et BusyBox. `netctrl` appelle la logique de changement de mot de passe décrite dans `libcommonprod.so`. `cfm`/`cfmd` manipulent la configuration persistante. Les outils d'analyse mentionnés dans le dépôt sont `binwalk`, `unsquashfs`/`mksquashfs` et `flashrom`; le matériel de lecture comprend FT2232H ou ESP8266/Arduino.

## Conventions et limites

- Pas de build, tests automatisés, manifeste de dépendances ni CI identifiés ; les commandes utiles sont des outils externes et des commandes exécutées sur le routeur.
- Le clone local suit le dépôt amont `latonita/tenda-reverse` sur `master`; développer les ajouts sur une branche dédiée du fork `BaramoK/tenda-reverse`.
- Le dump et plusieurs artefacts sont binaires : éviter de les modifier ou republier sans raison. Faire une copie vérifiée avant toute expérience d'écriture.
- La restauration réseau/TFTP est mentionnée dans les journaux, mais aucune procédure complète vérifiée n'est fournie ici.
- Les observations sont propres aux appareils/firmwares testés. Une erreur de flash peut rendre le routeur inutilisable ; ne pas exécuter les commandes d'effacement du bootloader sans sauvegarde et plan de récupération.

# Pissed Off - Mechatronic Cocktail Art Installation

(c) 2025, Christian Schüler, hello@christianschueler.at

```
Gips gegossen in 1,5 mm PLA 3D Druck Form, 6 mm Sperrholz Box, 8 mm Aluminiumrohr, 
2,5 mm Silikonkautschukschlauch, ESP32-S3 Mikrocontroller, LM2596 DC-DC Spannungswandler, 
Wägezelle 1 kg mit Dehnmessstreifen, HX711 24 Bit A/D-Wandler, DG600F Münzprüfer, 12 V 
Peristaltikpumpe mit Controllerboard, 1-Kanal Relais, Waagschale PLA 3D Druck, 10 l Wasserkanister. 
```

See /esp-32-s3 subfolder for microcontroller source code.

# Pissed Off ESP32-S3 Code

This ESP-32 code controls the following hardware:

- load cell
- peristaltic pump
- coin acceptor

## Loop - wait for donation and cup, then dispense a cocktail; repeat

- count coins, measure load
- INSERT_COIN: when coins reach price for a defined duration AND load = empty cup:
  - switch on peristaltic pump
  - state -> DISPENSING
- DISPENSING: 
  - measure dispensed
  - when load = full cup OR timeout (empty supply) OR load = 0 (someone took cup or cup fell):
    - switch off peristaltic pump
    - state -> TAKE_CUP or INSERT_COIN if someone took the cup
- TAKE_CUP: when load = empty:
    - reset coins (only here when full cocktail has been dispensed, otherwise credits remain valid)
    - tare
    - state -> INSERT_COIN

## Connect to ESP32

powershell:
```
usbipd list
usbipd attach --wsl --auto-attach --busid 10-2
```

wsl:
```
ls /dev
```
look for ttyACM0 (on my machine) and set in esp-idf config

## Ideas and Todo

- nive to have: count all donated coins across the session and store overall coint coin, preferably by day + have a way to output the amount
  - could use NVS to store an int 32 per day, marked by key name, e.g. key = "coins_2025-11-23", value = 4000 meaning EUR 40,-
  - would have to use NVS iterator to get all coins donated OR simply try to get keys for certain dates
- nice to have: drink size depending on amount donated, e.g. 2 sizes

# BrewDuino
BrewDuino ist eine einfache Brau-Steuerungs-Software für Hobbybrauer.

## Funktionen

### Maischen
* Definition und Ausführung Des Kompletten Maischeplans.
* Optische und akustische Warung bei Erreichen der Einmaischtemperatur
* Unterbrechung des Maischeplans, bis eingemaischt wurde
* Optische und akustische Warung bei Erreichen der Abmaischtemperatur

### Kochen
* Freie Definition der Hopfengaben
* Sekundengenaue Messung der Kochzeit
* Optische und akustische Warung für Hopfengaben
* Optische und akustische Warung nach Ablauf der Kochzeit

### Heizen
* Heizt auf eine eingegebene Temperatur
* Hält diese Temperatur dauerhaft

### Timer
* Countdown einer eingegebenen Zeit
* Optische und akustische Warung bei Ablauf der Zeit

### Einstellungen (noch nicht implementiert)
* Live Anpassung aller Betriebsparameter:
  * Ein-Ausschalt Temperaturoffset
  * Intervall der Temperaturmessungen

##Hardware
* Prozessor: ATmega 328P, Arduino-Bootloader
* Display: Nokia 5110 Display (PCD8544)
* SainSmart Relaiskarte
* DS18B20 Temperatursensor
* Rotary Encoder mit Button

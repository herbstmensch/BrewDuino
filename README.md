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
* (noch nicht implementiert)
* Heizt auf, und hält eine eingegebene Temperatur

### Einstellungen
* (noch nicht implementiert)
* Live Anpassung aller Betriebsparameter:
  * Ein-Ausschalt Temperaturoffset
  * Standard Rasten Anzahl
  * Standard Hopfengaben Anzahl
  * 

##Hardware
* Prozessor: AtMega 328P, mit Arduino-Bootloader,
* Display: Nokia 5110 Display (PCD8544),
* SainSmart Relaiskarte zur Ansteuerung eines Einkochers oder einer Kochplatte
* DS18B20 Temperatursensor
* Rotary Encoder mit Button zur Eingabe

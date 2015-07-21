# BrewDuino
BrewDuino ist eine einfache Brau-Steuerungs-Software für Hobbybrauer.

## Inhalt
- [Funktionen](#)
	- [Maischen](#)
	- [Kochen](#)
	- [Heizen](#)
	- [Timer](#)
	- [Einstellungen (noch nicht implementiert)](#)
- [Hardware](#)
	- [Schaltplan](#)
	- [Board](#)
	- [Bauteile](#)
	- [Anschlüsse](#)
- [Bilder](#)
	- [Board unbestückt](#)
	- [Board bestückt](#)

### Funktionen

#### Maischen
* Definition und Ausführung des Kompletten Maischeplans.
* Optische und akustische Warung bei Erreichen der Einmaischtemperatur
* Unterbrechung des Maischeplans, bis eingemaischt wurde
* Optische und akustische Warung bei Erreichen der Abmaischtemperatur

#### Kochen
* Freie Definition der Hopfengaben
* Sekundengenaue Messung der Kochzeit
* Optische und akustische Warung für Hopfengaben
* Optische und akustische Warung nach Ablauf der Kochzeit

#### Heizen
* Heizt auf eine eingegebene Temperatur
* Hält diese Temperatur dauerhaft

#### Timer
* Countdown einer eingegebenen Zeit
* Optische und akustische Warung bei Ablauf der Zeit

#### Einstellungen (noch nicht implementiert)
* Anpassung aller Betriebsparameter:
  * Ein-Ausschalt Temperaturoffset
  * Intervall der Temperaturmessungen
  * Ein- / Ausschalten der akustischen Warnungen
  * Ein- / Ausschalten der optischen Warnungen

### Hardware
* Prozessor: ATmega 328P, Arduino-Bootloader
* Display: Nokia 5110 Display (PCD8544)
* SainSmart Relaiskarte
* DS18B20 Temperatursensor
* Rotary Encoder mit Button
 
#### Schaltplan
![Schaltplan](/Hardware/BrewDuino_Schaltplan.png)

#### Board
![PCB](/Hardware/BrewDuino_Leiterplatte.png)

#### Bauteile
Bezeichnung | Bauteil
------------ | -------------
C1 | Kondensator 22 uF
C2 | Kondensator 22 uF
C3 | Kondensator 100 nF
C4 | Kondensator 100 nF
U2 | Spannungsregulator
D1 | Diode
JX | PinHeader
XTAL2 | Quartz 16 Mhz
R1 | Wiederstand 220 Ohm

#### Anschlüsse
Bezeichung | Anschluß
-----------|---------
VCC  | Versorgungsspannung 5V
GND  | MAsse
SCE  | Nokia 5110 Display SCE
RST  | Nokia 5110 Display RST
D/C  | Nokia 5110 Display D/C
MOSI | Nokia 5110 Display MOSI
SCLK | Nokia 5110 Display SCLK
RST  | Reset (Arduino Reset)
TX   | Tranceive (Arduino Serial Programming)
RX   | Receive (Arduino Serial Programming)
RT1  | Rotary Encoder
RT2  | Rotary Encoder
BTN  | Button zur Eingabe
RL1  | Relais 1 (Kocher oder Heizplatte)
RL2  | Relais 2 (Rührwerk)
TMP  | Temperatursensor Data
BZZ  | Summer

### Bilder

#### Board unbestückt
![Schaltplan](/Hardware/Board.jpg)

#### Board bestückt
![Schaltplan](/Hardware/Board_components.jpg)

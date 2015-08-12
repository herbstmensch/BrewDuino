#define CREATOR "Herbstmensch"
#define PROG_NAME "BrewDuino"
#define PROG_VERSION "v0.9"
#define COPYRIGHT "(c) 2015 - "
#define COPYRIGHT2 "Tim Herbst"
#include <LCD5110_Basic.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include "FiniteStateMachine.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//Pins benennen
#define PIN_ROTARY_1 2
#define PIN_ROTARY_2 3
#define PIN_ROTARY_BUTTON 4
#define PIN_BG_LIGHT A0
#define PIN_RUEHRER 5
#define PIN_HEATER 6
#define PIN_THERMOMETER 7
#define PIN_BUZZER 8
#define PIN_LCD_SCLK 9
#define PIN_LCD_MOSI 10
#define PIN_LCD_DC 11
#define PIN_LCD_RST 12
#define PIN_LCD_SCE 13

#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

//Konfigurationsparameter festlegen
#define TEMP_OFFSET 1
#define MIN_TEMP -999
#define TMP_SMOOTH 5
#define MAX_LONG 2147483647L;
#define ENC_HALFSTEP
#define SERIAL N

//Fehler festlegen
#define ERROR_NO_THERMOMETER 1

int errors = 0, lastErrors = 0;

//Display einrichten
//Old LCD5110 lcd(8, 9, 10, 11, 12);
LCD5110 lcd(PIN_LCD_SCLK, PIN_LCD_MOSI, PIN_LCD_DC, PIN_LCD_RST, PIN_LCD_SCE);
extern uint8_t SmallFont[];

//Encoder einrichten;
ClickEncoder *encoder;
int16_t encoderValue;

//Temperatursensor DS1820 Initialisierung
OneWire oneWire(PIN_THERMOMETER);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer;

//Main FSM und States definieren
State stateMenu = State(enterMenu, menu, leaveMenu);
State stateError = State(enterError, error, NULL);
State stateMaischen = State(enterMaischen, maischen, leaveMaischen);
State stateKochen = State(enterKochen, kochen, leaveKochen);
State stateHeizen = State(enterHeizen, heizen, leaveHeizen);
State stateTimer = State(enterTimer, timer, NULL);
//State stateSettings = State(enterSettings, settings, NULL);
FSM fsmMain = FSM(stateMenu);

//Maischen FSM und States definieren
State stateEnterEinmaischTemp = State(forceFirstDisplay, enterEinmaischTemp, NULL);
State stateEnterAnzahlRasten = State(forceFirstDisplay, enterAnzahlRasten, NULL);
State stateDefineRast = State(forceFirstDisplay, defineRast, NULL);
State stateEnterAbmaischTemp = State(forceFirstDisplay, enterAbmaischTemp, NULL);
State stateReachEinmaischTemp = State(prepareReachEinmaischTemp, reachEinmaischTemp, NULL);
State stateEinmaischen = State(einmaischen);
State stateReachRastTemp = State(reachRastTemp);
State stateWaitRastDauer = State(prepareRastDauer, waitRastDauer, NULL);
State stateReachAbmaischTemp = State(NULL, reachAbmaischTemp, NULL);
FSM fsmMaischen = FSM(stateEnterEinmaischTemp);

//Kochen FSM und States definieren
State stateEnterKochzeit = State(forceFirstDisplay, enterKochzeit, NULL);
State stateEnterAnzahlHopfengaben = State(forceFirstDisplay, enterAnzahlHopfengaben, NULL);
State stateDefineHopfengaben = State(forceFirstDisplay, defineHopfengaben, NULL);
State stateDoKochen = State(enterDoKochen, doKochen, NULL);
FSM fsmKochen = FSM(stateEnterKochzeit);

//Timer FSM und States definieren
State stateEnterTimerTime = State(forceFirstDisplay, enterTimerTime, NULL);
State stateDoTimer = State(prepareDoTimer, doTimer, NULL);
FSM fsmTimer = FSM(stateEnterTimerTime);

//Variablen initialisieren

int selectedMenuEntry = 0, menuOffset = 0;
//char* menuEntrys[] = {"Maischen","Kochen","Heizen","Timer","Einstell."};
char* menuEntrys[] = {"Maischen", "Kochen", "Heizen", "Timer"};
float lastTemps[TMP_SMOOTH] = {0, 0, 0, 0, 0};
float tempTotal = 0;
int temp, lastTemp, sollTemp, lastHeatCheckTemp, tmpIndex = 0;

unsigned long lastRest = -1;
unsigned long dauer = 0;
unsigned long storedSystemMillis = 0;
unsigned long lastTempMillis;
unsigned long alertMillis = 0;

bool first = false;
bool isHeating;

void setup()   {
  //LCD Setup
  lcd.InitLCD();
  lcd.clrScr();
  lcd.setFont(SmallFont);
  printRow(CREATOR, LEFT, 0);
  printRow(PROG_NAME" "PROG_VERSION, LEFT, 16);
  printRow(COPYRIGHT, LEFT, 32);
  printRow(COPYRIGHT2, RIGHT, 40);

  #if SERIAL != N
  Serial.begin(9600);
  #endif

  pinMode(PIN_BG_LIGHT, OUTPUT);
  pinMode(PIN_HEATER, OUTPUT);
  digitalWrite(PIN_BG_LIGHT, LOW);
  digitalWrite(PIN_HEATER, HIGH);

  //Encoder und Interrupt Setup
  encoder = new ClickEncoder(PIN_ROTARY_1, PIN_ROTARY_2, PIN_ROTARY_BUTTON, 4);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  //Temperaturen und Heizung initialisieren
  setSollTemp(MIN_TEMP);
  isHeating = false;

  initThermometer();

  checkForErrors();
  delay(2000);
  if (errors > 0) {
    fsmMain.immediateTransitionTo(stateError);
    alarm();
  } else {
    fsmMain.immediateTransitionTo(stateMenu);
  }
}

void loop() {
  fsmMain.update();
  
  if ( encoder->getButton() == ClickEncoder::Held ) {
    fsmMain.transitionTo(stateMenu);
  }

  readTemperature();
  checkHeatingStatus(false);
  addTemperature(false);

  if (alertMillis != 0)
    doAlert();
}

void initThermometer(){
  sensors.begin();
  sensors.getAddress(thermometer, 0);
  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(thermometer, 9);
  //Init Temp array
  for (int i = 0; i < TMP_SMOOTH; i++){
    sensors.setWaitForConversion(true);
    sensors.requestTemperatures();
     //Letzte Temperatur dieser Position aus Summe nehmen
    tempTotal -= lastTemps[tmpIndex];
    //Neue Temperatur dieser Position schreiben
    lastTemps[tmpIndex] = sensors.getTempC(thermometer);
    //Neue Temperatur dieser Position zur Summe hinzunehmen und Position erhöhen
    tempTotal += lastTemps[tmpIndex++];

    //Durchschnitt errechnen
    temp = tempTotal / TMP_SMOOTH;
  }
  
  tmpIndex = 0;
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
}

void enterMenu() {
  encoder->setAccelerationEnabled(false);
  //selectedMenuEntry = 0;
  //menuOffset = 0;
  setSollTemp(MIN_TEMP);
  checkHeatingStatus(true);
  clrScr(true, false);
  printRow(PROG_NAME" "PROG_VERSION, CENTER, 0);
  forceFirstDisplay();
}

void leaveMenu() {
  encoder->setAccelerationEnabled(true);
}

void enterError() {
  setSollTemp(MIN_TEMP);
  checkHeatingStatus(true);
  clrScr(true, true);
  printRow("--Fehler--", CENTER, 0);
  forceFirstDisplay();
}

void enterMaischen() {
  encoder->setAccelerationEnabled(true);
  fsmMaischen.immediateTransitionTo(stateEnterEinmaischTemp);
  clrScr(true, false);
  printRow("--Maischen--", CENTER, 0);
}

void leaveMaischen() {
  setSollTemp(MIN_TEMP);
}

void enterKochen() {
  encoder->setAccelerationEnabled(true);
  setSollTemp(100);
  checkHeatingStatus(true);
  fsmKochen.immediateTransitionTo(stateEnterKochzeit);
  clrScr(true, false);
  printRow("--Kochen--", CENTER, 0);
}

void leaveKochen() {
  setSollTemp(MIN_TEMP);
}

void enterHeizen() {
  encoder->setAccelerationEnabled(true);
  setSollTemp(temp);
  clrScr(true, false);
  printRow("--Heizen--", CENTER, 0);
  forceFirstDisplay();
}

void leaveHeizen() {
  setSollTemp(MIN_TEMP);
}

void enterTimer() {
  encoder->setAccelerationEnabled(true);
  fsmTimer.immediateTransitionTo(stateEnterTimerTime);
  clrScr(true, false);
  printRow("--Timer--", CENTER, 0);
}

/*void enterSettings(){
  fsmSettings.immediateTransitionTo();
}*/

void menu() {
  encoderValue = encoder->getValue();

  if (encoderValue != 0 || first) {
    first = false;
    if (encoderValue > 0)
      for (int i = 0; i < encoderValue; i++)
        menuUp();
    if (encoderValue < 0)
      for (int i = encoderValue; i < 0; i++)
        menuDown();

    clrScr(false, false);
    if (selectedMenuEntry == 0 + menuOffset)
      printRow("=>", 10, 16);
    lcd.print(menuEntrys[0 + menuOffset], 25, 16);
    if (selectedMenuEntry == 1 + menuOffset)
      printRow("=>", 10, 24);
    lcd.print(menuEntrys[1 + menuOffset], 25, 24);
  }

  if (buttonClicked()) {
    if (selectedMenuEntry == 0)
      fsmMain.transitionTo(stateMaischen);
    if (selectedMenuEntry == 1)
      fsmMain.transitionTo(stateKochen);
    if (selectedMenuEntry == 2)
      fsmMain.transitionTo(stateHeizen);
    if (selectedMenuEntry == 3)
      fsmMain.transitionTo(stateTimer);
    //if(selectedMenuEntry == 4)
    //fsmMain.transitionTo(stateSettings);
    //alarm();
  }
}

void menuUp() {
  selectedMenuEntry -= 1;
  if (selectedMenuEntry < 0)
    selectedMenuEntry = 0;
  if (selectedMenuEntry < menuOffset)
    menuOffset--;
}

void menuDown() {
  selectedMenuEntry += 1;
  if (selectedMenuEntry > NUMITEMS(menuEntrys) - 1)
    selectedMenuEntry = NUMITEMS(menuEntrys) - 1;
  if (selectedMenuEntry > menuOffset + 1)
    menuOffset += 1;
}

void error() {
  if(lastErrors != errors || first){

    lastErrors = errors;

    if(errors >= ERROR_NO_THERMOMETER){
      errors -= ERROR_NO_THERMOMETER;
      lcd.print("Kein Therm.", 0, 16);
    }
    
  }
}

void maischen() {
  fsmMaischen.update();
}

void kochen() {
  fsmKochen.update();
}

void heizen() {
  doHeizen();
}

void timer() {
  fsmTimer.update();
}
/*void settings(){
  fsmSettings.update();
}*/

void readTemperature() {
  
  //Gelesen wir nur alle 2,5 sec.
  if (millis() - lastTempMillis > 2500 && sensors.isConversionAvailable(0)) {
    lastTempMillis = millis();

    //Letzte Temperatur dieser Position aus Summe nehmen
    tempTotal -= lastTemps[tmpIndex];
    //Neue Temperatur dieser Position schreiben
    lastTemps[tmpIndex] = sensors.getTempC(thermometer);
    //Neue Temperatur dieser Position zur Summe hinzunehmen und Position erhöhen
    tempTotal += lastTemps[tmpIndex++];

    //Durchschnitt errechnen
    temp = tempTotal / TMP_SMOOTH;

    //Ggf. Position korrigieren
    if (tmpIndex >= TMP_SMOOTH)
      tmpIndex = 0;

    //Neue Temperatur anfordern
    sensors.requestTemperatures();
  }
}

void setSollTemp(int t) {
  sollTemp = t;
  checkHeatingStatus(true);
}

void checkHeatingStatus(bool force) {
  if (temp != lastHeatCheckTemp || force) {
    lastHeatCheckTemp = temp;
    //Ein oder ausschalten des Heizelementes.
    //Evtl. schaltfrequent sicherstellen

    //-127 heißt kein Thermometer
    if (temp != -127) {
      if (isHeating) {
        if (temp >= sollTemp + TEMP_OFFSET)
          turnOffHeating();
      } else {
        if (temp <= sollTemp - TEMP_OFFSET)
          turnOnHeating();
      }
    }
  }
}

void turnOffHeating() {
  //Toggle Relais
  isHeating = false;
  digitalWrite(PIN_HEATER, HIGH);
  addTemperature(true);
}

void turnOnHeating() {
  //Toggle Relais
  isHeating = true;
  digitalWrite(PIN_HEATER, LOW);
  addTemperature(true);
}

void addTemperature(boolean force) {
  //Wenn die aktuelle Temperatur nciht der angezeigten entspricht, aktualisieren.
  if (temp != lastTemp || force) {
    lastTemp = temp;
    lcd.clrRow(5);
    String s = (isHeating ? "H " : "");
    s += "ist: ";
    s += temp;
    s += "~ C";
    char buf[14];
    s.toCharArray(buf, 14);
    printRow(buf, RIGHT, 40);
  }
}

void clrScr(bool clearFirst, bool clearLast) {
  if (clearFirst && clearLast) {
    lcd.clrScr();
    return;
  }
  if (clearFirst) {
    lcd.clrRow(0);
  }
  lcd.clrRow(1);
  lcd.clrRow(2);
  lcd.clrRow(3);
  lcd.clrRow(4);
  if (clearLast) {
    lcd.clrRow(5);
  }
}

void printRow(char* text, int pos, int row) {
  if (row > 5)
    row = row / 8;
  lcd.clrRow(row);
  lcd.print(text, pos, row * 8);
}

void timerIsr() {
  encoder->service();
}

void forceFirstDisplay() {
  first = true;
}

void alarm() {
  alertMillis = millis();
}

void doAlert() {
  unsigned long dur = millis() - alertMillis;

  if ((dur / 250) % 4 == 0) {
    tone(PIN_BUZZER, 800, 250);
  } else if ((dur / 250) % 2 == 0) {
    tone(PIN_BUZZER, 970, 250);
  } else {
    noTone(PIN_BUZZER);
  }
  if ((dur / 500) % 2 == 0) {
    digitalWrite(PIN_BG_LIGHT, LOW);
  } else {
    digitalWrite(PIN_BG_LIGHT, HIGH);
  }

  //Nach spät. 10 sec. den Alarm Abbrechen
  if (dur >= 10000) {
    cancelAlarm();
  }
}

void cancelAlarm() {
  alertMillis = 0;
  noTone(PIN_BUZZER);
  digitalWrite(PIN_BG_LIGHT, LOW);
}

bool buttonClicked() {
  if (encoder->getButton() == ClickEncoder::Clicked )
    return true;
  return false;
}

void checkForErrors() {
  if (-128 < temp && -126 > temp) {
    //Kein Thermometer
    errors += ERROR_NO_THERMOMETER;
  }
}

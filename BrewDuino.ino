#define CREATOR "Herbstmensch"
#define PROG_NAME "BrewDuino"
#define PROG_VERSION "v0.1"
#define COPYRIGHT "(c) 2015 Tim"
#define COPYRIGHT2 "Herbst"
#include <LCD5110_Basic.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include "FiniteStateMachine.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_ROTARY_1 2
#define PIN_ROTARY_2 3
#define PIN_ROTARY_BUTTON 4
#define PIN_THERMOMETER 7
#define PIN_BG_LIGHT 5
#define PIN_HEATER 6
#define PIN_BUZZER 8
#define PIN_LCD_SCLK 9
#define PIN_LCD_MOSI 10
#define PIN_LCD_DC 11
#define PIN_LCD_RST 12
#define PIN_LCD_SCE 13

//Konfigurationsparameter festlegen
#define TEMP_OFFSET 1
#define MIN_TEMP -999
#define MAX_LONG 2147483647L;

//Display einrichten 
//Old LCD5110 lcd(8, 9, 10, 11, 12);
LCD5110 lcd(PIN_LCD_SCLK, PIN_LCD_MOSI, PIN_LCD_DC, PIN_LCD_RST, PIN_LCD_SCE);
extern uint8_t SmallFont[];

//Encoder einrichten;
#define ENC_HALFSTEP
ClickEncoder *encoder;
int16_t lastEncoderValue, encoderValue;

//Temperatursensor DS1820 Initialisierung
OneWire oneWire(PIN_THERMOMETER);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer;

//Main FSM und States definieren
State stateMenu = State(enterMenu, menu, leaveMenu);
State stateMaischen = State(enterMaischen, maischen, leaveMaischen);
State stateKochen = State(enterKochen, kochen, leaveKochen);
//State stateSettings = State(enterSettings, settings, NULL);
FSM fsmMain = FSM(stateMenu);

//Maischen FSM und States definieren
State stateEnterEinmaischTemp = State(storeEncoderValue,enterEinmaischTemp,NULL);
State stateEnterAnzahlRasten = State(storeEncoderValue,enterAnzahlRasten,NULL);
State stateDefineRast = State(storeEncoderValue,defineRast,NULL);
State stateEnterAbmaischTemp = State(storeEncoderValue,enterAbmaischTemp,NULL);
State stateDoMaischen = State(enterDoMaischen, doMaischen,NULL);
FSM fsmMaischen = FSM(stateEnterEinmaischTemp);

//Kochen FSM und States definieren
State stateEnterKochzeit = State(storeEncoderValue,enterKochzeit,NULL);
State stateEnterAnzahlHopfengaben = State(storeEncoderValue,enterAnzahlHopfengaben,NULL);
State stateDefineHopfengaben = State(storeEncoderValue,defineHopfengaben,NULL);
State stateDoKochen = State(enterDoKochen,doKochen,NULL);
FSM fsmKochen = FSM(stateEnterKochzeit);

/* 
//Settings FSM und States definieren
State enterEinmaischTemp;
FSM fsmSettings = FSM(enterEinmaischTemp);
*/

//Variablen initialisieren
int selectedMenuEntry, lastSelectedMenuEntry;
float lastTemps[]={0,0,0,0,0,0,0,0,0,0};
int temp, lastTemp, sollTemp;
int lastReadIndex=0;

long lastRest = -1;
long dauer = 0;
long storedSystemMillis = 0;
long lastTempMillis;

bool isHeating;

void setup()   {
  Serial.begin(9600);
  
  pinMode(PIN_BG_LIGHT, OUTPUT);
  pinMode(PIN_HEATER, OUTPUT);
  digitalWrite(PIN_BG_LIGHT, LOW);
  digitalWrite(PIN_HEATER, HIGH);
  
  //LCD Setup 
  lcd.InitLCD();
  lcd.clrScr();
  lcd.setFont(SmallFont);
  lcd.print(CREATOR, LEFT, 0);
  lcd.print(PROG_NAME" "PROG_VERSION, LEFT, 16);
  lcd.print(COPYRIGHT, LEFT, 32);
  lcd.print(COPYRIGHT2, 42, 40);
  
  //Encoder und Interrupt Setup
  encoder = new ClickEncoder(PIN_ROTARY_1, PIN_ROTARY_2, PIN_ROTARY_BUTTON, 4);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  lastEncoderValue = -1;
  
  //Temperaturen und Heizung initialisieren
  sollTemp = 0;
  isHeating = false;
  
  //Temperatursensor initialisieren
  sensors.begin();
  sensors.getAddress(thermometer, 0);
  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(thermometer, 9);
  //---------------------------------------------------------------

  delay(2000);
}

void loop() {
  fsmMain.update();
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Held ){
      fsmMain.transitionTo(stateMenu);
    }
  } 
  
  readTemperature();
  checkHeatingStatus();
  addTemperature(false);
}

void enterMenu(){
  encoder->setAccelerationEnabled(false);
  selectedMenuEntry = 0;
  lastSelectedMenuEntry = -1;
  sollTemp = MIN_TEMP;
  clrScr(true,false);
  lcd.print(PROG_NAME" "PROG_VERSION, CENTER, 0);
}

void leaveMenu(){
  encoder->setAccelerationEnabled(true);
}

void enterMaischen(){
  encoder->setAccelerationEnabled(true);
  fsmMaischen.immediateTransitionTo(stateEnterEinmaischTemp);
  clrScr(true,false);
  lcd.print("--Maischen--",CENTER,0);
}

void leaveMaischen(){
  sollTemp = MIN_TEMP;
}

void enterKochen(){
  encoder->setAccelerationEnabled(true);
  sollTemp = 100;
  fsmKochen.immediateTransitionTo(stateEnterKochzeit);
  clrScr(true,false);
  lcd.print("--Kochen--",CENTER,0);
}

void leaveKochen(){
  sollTemp = MIN_TEMP;
}

/*void enterSettings(){
  fsmSettings.immediateTransitionTo();
}*/

void menu() {
  encoderValue += encoder->getValue();
    Serial.println(encoder->getValue());
    Serial.println(encoder->getValue());
    Serial.println(encoder->getValue());
  if(encoderValue != lastEncoderValue){
    lastEncoderValue = encoderValue;
    Serial.print("Encoder Value: ");
    Serial.println(encoderValue);
    selectedMenuEntry = (encoderValue*encoderValue)%2;
    Serial.print("Entry: ");
    Serial.println(selectedMenuEntry);
  }
  
  if(selectedMenuEntry != lastSelectedMenuEntry){
    lastSelectedMenuEntry = selectedMenuEntry;
    clrScr(false,false);
    lcd.print("=>", 10, 16+selectedMenuEntry*8);
    lcd.print("Maischen", 25, 16);
    lcd.print("Kochen", 25, 24);
    //lcd.print("Parameter", 30, 28);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        if(selectedMenuEntry == 0)
          fsmMain.transitionTo(stateMaischen);
        if(selectedMenuEntry == 1)
          fsmMain.transitionTo(stateKochen);
       // if(selectedMenuEntry == 2)
         // fsmMain.transitionTo(stateSettings);
    }
  } 
}

void maischen(){
  fsmMaischen.update();
}

void kochen(){
  fsmKochen.update();
}

/*void settings(){
  fsmSettings.update();
}*/

void storeEncoderValue(){
  lastEncoderValue = -999;
  Serial.print("LastEncoderValue: ");
  Serial.println(lastEncoderValue);
  //Die Werte dürfen nicht gleich sein, sonst bauen sich die Menüs erst nach veränderung auf.
  encoderValue = 0;
  Serial.print("encoderValue: ");
  Serial.println(encoderValue);
}

void readTemperature(){
  if(millis()-lastTempMillis > 500){
    //Aktuelle Temperatur lesen.
    sensors.requestTemperatures(); 
    lastTemps[lastReadIndex++] = sensors.getTempC(thermometer);
    
    float sum = 0;
    for(int i = 0; i < 10; i++)
      sum += lastTemps[i];
      
    //Tatsächliche Temperatur ist das Mittel über die letzten 10 gelesenen Temperaturen
    temp = sum / 10;
    
    if(lastReadIndex >= 10)
      lastReadIndex = 0;
    
    lastTempMillis = millis();
  }
}

void checkHeatingStatus(){
  //Ein oder ausschalten des Heizelementes.
  //Evtl. schaltfrequent sicherstellen
  if(isHeating){
    if(temp >= sollTemp + TEMP_OFFSET)
      turnOffHeating();
  } else {
     if(temp - TEMP_OFFSET <= sollTemp - TEMP_OFFSET)
      turnOnHeating();
  }
}

void turnOffHeating(){
  //Toggle Relais
  isHeating = false;
  digitalWrite(PIN_HEATER,HIGH);
  addTemperature(true);
}

void turnOnHeating(){
  //Toggle Relais
  isHeating = true;
  digitalWrite(PIN_HEATER,LOW);
  addTemperature(true);
}

void addTemperature(boolean force) {
  //Wenn die aktuelle Temperatur nciht der angezeigten entspricht, aktualisieren.
  if(temp != lastTemp || force){
    lastTemp = temp;
    lcd.clrRow(5);
    String s = (isHeating ? "H " : "");
    s += "ist: ";
    s += temp;
    s += "~ C";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, RIGHT, 40);
  }
}

void clrScr(bool clearFirst, bool clearLast){
  if(clearFirst && clearLast){
    lcd.clrScr();
    return;
  }
  if(clearFirst){
    lcd.clrRow(0);
  }
  lcd.clrRow(1);
  lcd.clrRow(2);
  lcd.clrRow(3);
  lcd.clrRow(4);
  if(clearLast){
    lcd.clrRow(5);
  }
}

void timerIsr() {
  encoder->service();
}

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

#define BG_LIGHT_PIN 7

//Konfigurationsparameter festlegen
int tempOffset = 1;

//Display einrichten 
LCD5110 lcd(8, 9, 10, 11, 12);
//Change in LCD5110 lcd(9,10,11,12,13);
extern uint8_t SmallFont[];

//Encoder einrichten;
#define ENC_HALFSTEP
ClickEncoder *encoder;
int16_t lastEncoderValue, encoderValue, savedEncoderValue;

//Temperatursensor DS1820 Initialisierung
OneWire oneWire(5);
DallasTemperature sensors(&oneWire);

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
State stateEnterKochzeit = State(enterKochzeit);
State stateEnterAnzahlHopfengaben = State(enterAnzahlHopfengaben);
State stateDefineHopfengaben = State(defineHopfengaben);
State stateDoKochen = State(doKochen);
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

int* rastTemp=0;
int* rastDauer=0;

long storedSystemMillies = 0;

bool isHeating;

void setup()   {
  Serial.begin(9600);
  
  pinMode(BG_LIGHT_PIN, OUTPUT);
  digitalWrite(BG_LIGHT_PIN, LOW);
  
  //LCD Setup 
  lcd.InitLCD();
  lcd.clrScr();
  lcd.setFont(SmallFont);
  lcd.print(CREATOR, LEFT, 0);
  lcd.print(PROG_NAME" "PROG_VERSION, LEFT, 16);
  lcd.print(COPYRIGHT, LEFT, 32);
  lcd.print(COPYRIGHT2, 42, 40);
  
  //Encoder und Interrupt Setup
  encoder = new ClickEncoder(2, 3, 4, 4);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  lastEncoderValue = -1;
  
  //Temperaturen und Heizung initialisieren
  sollTemp = 0;
  isHeating = false;
  
  //Temperatursensor initialisieren
  sensors.begin();
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
  
  addTemperature();
}

void enterMenu(){
  encoder->setAccelerationEnabled(false);
  selectedMenuEntry = 0;
  lastSelectedMenuEntry = -1;
  sollTemp = 0;
  clrScr(true,false);
  lcd.print(PROG_NAME" "PROG_VERSION, CENTER, 0);
}

void leaveMenu(){
  encoder->setAccelerationEnabled(true);
}

void enterMaischen(){
  //fsmMaischen.immediateTransitionTo();
  clrScr(true,false);
  lcd.print("--Maischen--",CENTER,0);
}

void leaveMaischen(){
  sollTemp = 0;
}

void enterKochen(){
  sollTemp = 100;
  //fsmMaischen.immediateTransitionTo();
  clrScr(true,false);
  lcd.print("--Kochen--",CENTER,0);
}

void leaveKochen(){
  sollTemp = 0;
}

/*void enterSettings(){
  fsmSettings.immediateTransitionTo();
}*/

void menu() {
  encoderValue += encoder->getValue();
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
  savedEncoderValue = encoderValue;
}

void readTemperature(){
  //Aktuelle Temperatur lesen.
  sensors.requestTemperatures(); 
  lastTemps[lastReadIndex++] = sensors.getTempCByIndex(0);
  float sum = 0;
  for(int i = 0; i < 9; i++)
    sum += lastTemps[i];
    
  //Tatsächliche Temperatur ist das Mittel über die letzten 10 gelesenen Temperaturen
  temp = sum / 10;
  if(lastReadIndex >= 10)
    lastReadIndex = 0;
}

void checkHeatingStatus(){
  //Ein oder ausschalten des Heizelementes.
  //Evtl. schaltfrequent sicherstellen
  if(isHeating){
    if(temp + tempOffset >= sollTemp)
      turnOffHeating();
  } else {
     if(temp + tempOffset <= sollTemp)
      turnOnHeating();
  }
}

void turnOffHeating(){
  //Toggle Relais
  isHeating = false;
}

void turnOnHeating(){
  //Toggle Relais
  isHeating = true;
}

void addTemperature() {
  //Wenn die aktuelle Temperatur nciht der angezeigten entspricht, aktualisieren.
  //if(temp != lastTemp){
    lastTemp = temp;
    lcd.clrRow(5);
    String s = (isHeating ? "H " : "");
    s += "ist: ";
    s += temp;
    s += "~ C";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, RIGHT, 40);
 // }
}

void clrScr(bool clearFirst, bool clearLast){
  Serial.print("dd");
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

#define CREATOR "Herbstmensch"
#define PROG_NAME "BrewDuino"
#define PROG_VERSION "v0.5"
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
#define PIN_BG_LIGHT 5
#define PIN_HEATER 6
#define PIN_THERMOMETER 7
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
#define ENC_HALFSTEP
#define SERIAL N

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
State stateMaischen = State(enterMaischen, maischen, leaveMaischen);
State stateKochen = State(enterKochen, kochen, leaveKochen);
State stateHeizen = State(enterHeizen, heizen, leaveHeizen);
//State stateSettings = State(enterSettings, settings, NULL);
FSM fsmMain = FSM(stateMenu);

//Maischen FSM und States definieren
State stateEnterEinmaischTemp = State(forceFirstDisplay, enterEinmaischTemp, NULL);
State stateEnterAnzahlRasten = State(forceFirstDisplay, enterAnzahlRasten, NULL);
State stateDefineRast = State(forceFirstDisplay, defineRast, NULL);
State stateEnterAbmaischTemp = State(forceFirstDisplay, enterAbmaischTemp, NULL);
State stateDoMaischen = State(enterDoMaischen, doMaischen,NULL);
FSM fsmMaischen = FSM(stateEnterEinmaischTemp);

//Kochen FSM und States definieren
State stateEnterKochzeit = State(forceFirstDisplay, enterKochzeit, NULL);
State stateEnterAnzahlHopfengaben = State(forceFirstDisplay, enterAnzahlHopfengaben, NULL);
State stateDefineHopfengaben = State(forceFirstDisplay, defineHopfengaben, NULL);
State stateDoKochen = State(enterDoKochen,doKochen, NULL);
FSM fsmKochen = FSM(stateEnterKochzeit);

//Variablen initialisieren

int selectedMenuEntry = 0, menuOffset = 0;
char menuEntrys[][] = {"Maischen","Kochen","Heizen","Einstell."};
float lastTemps[]={0,0,0,0};
int temp, lastTemp, sollTemp, lastHeatCheckTemp;
int lastReadIndex=0;

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
  lcd.print(CREATOR, LEFT, 0);
  lcd.print(PROG_NAME" "PROG_VERSION, LEFT, 16);
  lcd.print(COPYRIGHT, LEFT, 32);
  lcd.print(COPYRIGHT2, RIGHT, 40);
  
  #if SERIAL != N
    Serial.begin(9600);
  #endif
  
  pinMode(PIN_BG_LIGHT, OUTPUT);
  pinMode(PIN_HEATER, OUTPUT);
  digitalWrite(PIN_BG_LIGHT, HIGH);
  digitalWrite(PIN_HEATER, HIGH);
  
  //Encoder und Interrupt Setup
  encoder = new ClickEncoder(PIN_ROTARY_1, PIN_ROTARY_2, PIN_ROTARY_BUTTON, 4);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  //Temperaturen und Heizung initialisieren
  sollTemp = 0;
  isHeating = false;
  
  //Temperatursensor initialisieren
  sensors.begin();
  sensors.getAddress(thermometer, 0);
  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(thermometer, 9);
  //Init Temp array
  for(int i = 0; i < TEMP_ARRAY_SIZE; i++)  
    readTemperature(true);
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
  
  readTemperature(false);
  checkHeatingStatus();
  addTemperature(false);

  if(alertMillis != 0)
    doAlert();
}

void enterMenu(){
  encoder->setAccelerationEnabled(false);
  selectedMenuEntry = 0;
  lastSelectedMenuEntry = -1;
  sollTemp = MIN_TEMP;
  clrScr(true,false);
  lcd.print(PROG_NAME" "PROG_VERSION, CENTER, 0);
  first = true;
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

void enterHeizen(){
  encoder->setAccelerationEnabled(true);
  sollTemp = temp;
  clrScr(true,false);
  lcd.print("--Heizen--",CENTER,0);
  first = true;
}

void leaveHeizen(){
  sollTemp = MIN_TEMP;
}

/*void enterSettings(){
  fsmSettings.immediateTransitionTo();
}*/

void menu() {
  encoderValue = encoder->getValue();
  
  if(encoderValue != 0 || first){
    first = false;
    if(encoderValue > 0)
      for(int i = 0; i < encoderValue; i++)
        menuUp();
    if(encoderValue < 0)
      for(int i = encoderValue; i > 0; i--)
        menuDown();
        
    clrScr(false,false);
    
    if(selectedMenuEntry == 0+menuOffset)
      lcd.print("=>", 10, 16);
    lcd.print(menuEntrys[0+menuOffset], 25,16);
    if(selectedMenuEntry == 1+menuOffset)
      lcd.print("=>", 10, 24);
    lcd.print(menuEntrys[1+menuOffset], 25,24);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        if(selectedMenuEntry == 0)
          fsmMain.transitionTo(stateMaischen);
        if(selectedMenuEntry == 1)
          fsmMain.transitionTo(stateKochen);
        if(selectedMenuEntry == 2)
          fsmMain.transitionTo(stateHeizen);
        if(selectedMenuEntry == 3)
          //fsmMain.transitionTo(stateSettings);
          alarm();
    }
  } 
}

void menuUp() {
	selectedMenuEntry -= 1;
	if(selectedMenuEntry < 0)
		selectedMenuEntry = 0;
	if(selectedMenuEntry < menuOffset)
		menuOffset--;
}
	
void menuDown() {
	selectedMenuEntry += 1;
	if(selectedMenuEntry > menuEntrys.length-1)
		selectedMenuEntry = menuEntrys.length-1;
	if(selectedMenuEntry > menuOffset+1)
		menuOffset += 1;	
}

void maischen(){
  fsmMaischen.update();
}

void kochen(){
  fsmKochen.update();
}

void heizen(){
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    sollTemp += encoderValue;
    if(sollTemp > 101)
      sollTemp = 101;
    if(sollTemp < 0)
      sollTemp = 0;
  
    clrScr(false,false);
    lcd.print("Heizen auf:",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(sollTemp),15,24);
    lcd.invertText(false);
    lcd.print("~ C",29,24);
  }
}

/*void settings(){
  fsmSettings.update();
}*/

void readTemperature(bool force){
  if(millis()-lastTempMillis > 1000 || force){
    //Aktuelle Temperatur lesen.
    sensors.requestTemperatures(); 
    lastTemps[lastReadIndex++] = sensors.getTempC(thermometer);
    
    float sum = 0;
    for(int i = 0; i < lastTemps.length; i++)
      sum += lastTemps[i];
      
    //Tatsächliche Temperatur ist das Mittel über die letzten 10 gelesenen Temperaturen
    temp = sum / lastTemps.length;
    
    if(lastReadIndex >= lastTemps.length)
      lastReadIndex = 0;
    
    lastTempMillis = millis();
  }
}

void checkHeatingStatus(){
  if(temp != lastHeatCheckTemp){
    lastHeatCheckTemp = temp;
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

void forceFirstDisplay(){
  first = true;
}

void alarm(){
  alertMillis = millis();
}

void doAlert(){
  long dur = millis()-alertMillis;
  
  if((millis()-alertMillis/500)%2==0){
    tone(PIN_BUZZER, 262, 250);
  } else {
    noTone(PIN_BUZZER);
  }
  if((dur/1000)%2==0){
    digitalWrite(PIN_BG_LIGHT,LOW);
  } else {
    digitalWrite(PIN_BG_LIGHT,HIGH);
  }
  
  //Nach spät. 10 sec. den Alarm Abbrechen
  if(dur >= 10000){
    cancelAlarm();
  }
}

void cancelAlarm(){
  alertMillis = 0;
  noTone(PIN_BUZZER);
  digitalWrite(PIN_BG_LIGHT,HIGH);
}

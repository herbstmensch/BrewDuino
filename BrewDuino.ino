#define CREATOR "Herbstmensch"
#define PROG_NAME "BrewDuino"
#define PROG_VERSION "v0.1"
#define COPYRIGHT "(c) 2015 Tim"
#define COPYRIGHT2 "Herbst"
#include <LCD5110_Basic.h>
#include <ClickEncoder.h>
#include <TimerOne.h>


//Konfigurationsparameter festlegen
int tempOffset = 1;

//Display einrichten 
LCD5110 lcd(8, 9, 10, 11, 12);
extern uint8_t SmallFont[];

//Encoder einrichten;
ClickEncoder *encoder;
int16_t lastEncoderValue, encoderValue;

//Zustände festlegen
#define STATE_INIT 0
#define STATE_MENU 1
#define STATE_MAISCHEN 2
#define STATE_KOCHEN 3
#define STATE_SETTINGS 4
#define STATE_MAISCHEN_START 0
#define STATE_MAISCHEN_EMT 1
#define STATE_MAISCHEN_AR 2
#define STATE_MAISCHEN_R 3
#define STATE_MAISCHEN_AMT 4
#define STATE_KOCHEN_KZ 1 
#define STATE_KOCHEN_AHG 2
#define STATE_KOCHEN_HG 3
#define STATE_KOCHEN_WT 4
#define STATE_KOCHEN_SKT 5
#define STATE_KOCHEN_KT 6

//Variablen initialisieren
int state = STATE_MENU;
int selectedMenuEntry = 0;
int lastSelectedMenuEntry = -1;

int temp, lastTemp, sollTemp;
bool isHeating;

void timerIsr() {
  encoder->service();
  readTemperature();
  checkHeatingStatus();
}

void setup()   {
  Serial.begin(9600);
  
  //LCD Setup 
  lcd.InitLCD();
  lcd.clrScr();
  lcd.setFont(SmallFont);
  lcd.print(CREATOR, LEFT, 0);
  lcd.print(PROG_NAME" "PROG_VERSION, LEFT, 16);
  lcd.print(COPYRIGHT, LEFT, 32);
  lcd.print(COPYRIGHT2, 42, 40);
  
  //Encoder und Interruot Setup
  encoder = new ClickEncoder(2, 3, 4, 2);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  lastEncoderValue = -1;
  
  //Temperaturen und Heizung initialisieren
  sollTemp = 0;
  isHeating = false;

  delay(5000);
}

void loop() {
  encoderValue += encoder->getValue();
  if(encoderValue != lastEncoderValue){
    lastEncoderValue = encoderValue;
    Serial.print("Encoder Value: ");
    Serial.println(encoderValue);
    selectedMenuEntry = (encoderValue*encoderValue)%2;
    Serial.print("Entry: ");
    Serial.println(selectedMenuEntry);
  }
    
    switch(state){
      case STATE_MENU:
        menu();
        break;
      case STATE_MAISCHEN:
        maischen();
        break;
      case STATE_KOCHEN:
        kochen();
        break;
      case STATE_SETTINGS:
        settings();
        break;
    }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        if(selectedMenuEntry == 0)
          state = STATE_MAISCHEN;
        if(selectedMenuEntry == 1)
          state = STATE_KOCHEN;
        if(selectedMenuEntry == 2)
          state = STATE_SETTINGS;
    }
  } 
  
  addTemperature();
}

void readTemperature(){
  temp = 59+random(1);
}

void checkHeatingStatus(){
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
  if(temp != lastTemp){
    lastTemp = temp;
    lcd.clrRow(5,80);
    String s = " ist: ";
    s += temp;
    s += "~ C";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, 0, 40);
  }
}

void menu() {
  if(selectedMenuEntry != lastSelectedMenuEntry){
  lastSelectedMenuEntry = selectedMenuEntry;
  lcd.clrScr();
  lcd.print(PROG_NAME" "PROG_VERSION, CENTER, 0);
  lcd.print("=>", 5, 12+selectedMenuEntry*8);
  lcd.print("Maischen", 30, 12);
  lcd.print("Kochen", 30, 20);
  lcd.print("Parameter", 30, 28);
  }
}

void maischen(){
  lcd.clrScr();
  lcd.print("--Maischen--",CENTER,0);
  
  int stateMaischen = STATE_MAISCHEN_EMT;
  
  while(state == STATE_MAISCHEN){
    switch(stateMaischen){
      case STATE_MAISCHEN_EMT: //Einmaischtemp festlegen
        break;
      case STATE_MAISCHEN_AR: //Anzahl Rasten festlegen
        break;
      case STATE_MAISCHEN_R: //Rasten festlegen
        break;
      case STATE_MAISCHEN_AMT: //Abmaischtemperatur festlegen
        break;
      case STATE_MAISCHEN_WEMT: //Warte auf Abmaischtemperatur
        break;
    } 
    
    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {
      if( b == ClickEncoder::Held ){
          returnToMenu();
      }
    }   
    
  }
}

void kochen(){
  sollTemp = 100;
  lcd.clrScr();
  lcd.print("--Kochen--",CENTER,0);
  
  int stateKochen = STATE_KOCHEN_KZ;
  
  while(state == STATE_KOCHEN){
    switch(stateKochen){
      case STATE_KOCHEN_KZ: //Einmaischtemp festlegen
        break;
      case STATE_KOCHEN_AHG: //Anzahl Hopfengaben festlegen
        break;
      case STATE_KOCHEN_HG: //Hopfengaben festlegen
        break;
      case STATE_KOCHEN_WT: //Warten auf Temperatur
        break;
      case STATE_KOCHEN_SKT: //Starte Kochtimer
        break;
      case STATE_KOCHEN_KT: //Starte Kochtimer
        break;
    } 
    
    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {
      if( b == ClickEncoder::Held ){
          returnToMenu();
      }
    }   
    
  }  
}

void returnToMenu(){
  sollTemp = 0;
  selectedMenuEntry = 0;
  lastSelectedMenuEntry = -1;
  state = STATE_MENU; 
}

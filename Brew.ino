
#define CREATOR "Herbstmensch"
#define PROG_NAME "ArduBrew"
#define PROG_VERSION "v0.1"
#define COPYRIGHT "(c) 2015 Tim"
#define COPYRIGHT2 "Herbst"
#include <LCD5110_Basic.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

//Display einrichten 
LCD5110 lcd(8, 9, 10, 11, 12);
extern uint8_t SmallFont[];

//Encoder einrichten;
ClickEncoder *encoder;
int16_t last, value;

void timerIsr() {
  encoder->service();
}

//Zustände festlegen
#define STATE_INIT 0
#define STATE_MENU 1
#define STATE_MAISCHEN 2
#define STATE_KOCHEN 3
#define MAISCHEN_EMT 1
#define MAISCHEN_AR 2
#define MAISCHEN_R 3
#define MAISCHEN_AMT 4

//Variablen initialisieren
int state = STATE_MENU;
int selectedMenuEntry = 0;
int lastSelectedMenuEntry = -1;

int temp, lastTemp;

void setup()   {
  Serial.begin(9600);
  encoder = new ClickEncoder(2, 3, 4, 2);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  last = -1;

//LCD Setup ------------------------------------------------------
  lcd.InitLCD();
  lcd.clrScr();
  lcd.setFont(SmallFont);
  lcd.print(CREATOR, LEFT, 0);
  lcd.print(PROG_NAME" "PROG_VERSION, LEFT, 16);
  lcd.print(COPYRIGHT, LEFT, 32);
  lcd.print(COPYRIGHT2, 42, 40);
  delay(5000);
}

void loop() {
  value += encoder->getValue();
  if(value != last){
    last = value;
    Serial.print("Encoder Value: ");
    Serial.println(value);
    selectedMenuEntry = (value*value)%2;
    Serial.print("Enntry: ");
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
        kochen;
        break;
    }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      
        if(selectedMenuEntry == 0)
          state = STATE_MAISCHEN;
    }
  }   
}

void addTemperature() {
  temp = 50 + random(4); //Richtig auslesen
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
  lcd.print("=>", 5, (selectedMenuEntry+2)*8);
  lcd.print("Maischen", 30, 16);
  lcd.print("Kochen", 30, 24);
  }
  addTemperature();
}

void maischen(){
  //Werte laden
  int stateMaischen = MAISCHEN_EMT;
  while(state == STATE_MAISCHEN){
    lcd.clrScr();
    lcd.print("--Maischen--",CENTER,0);
    switch(stateMaischen){
      case MAISCHEN_EMT: //Einmaischtemp festlegen
        break;
      case MAISCHEN_AR: //Anzahl Rasten festlegen
        break;
      case MAISCHEN_R: //Rasten festlegen
        break;
      case MAISCHEN_AMT: //Abmaischtemperatur festlegen
        break;
    } 
    
    ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Held ){
          returnToMenu();
    }
  }   
    
    addTemperature();
  }
}

void kochen(){}

void returnToMenu(){
  selectedMenuEntry = 0;
  lastSelectedMenuEntry = -1;
  state = STATE_MENU; 
}

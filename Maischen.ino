int einmaischTemp = 48;
int abmaischTemp = 78;
int anzahlRasten = 3;
int* rastTemp=0;
int* rastDauer=0;

int aktuelleRast = 0;
int subState = 0;

State stateReachEinmaischTemp = State(reachEinmaischTemp);
State stateEinmaischen = State(einmaischen);
State stateReachRastTemp = State(reachRastTemp);
State stateWaitRastDauer = State(prepareRastDauer,waitRastDauer,NULL);
State stateReachAbmaischTemp = State(reachAbmaischTemp);

FSM fsmMaischeProzess = FSM(stateReachEinmaischTemp);

void enterEinmaischTemp() {
  
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    einmaischTemp += encoderValue;  
    if(einmaischTemp < 0)
      einmaischTemp = 0;
 
    clrScr(false,false);
    lcd.print("Einmaisch-.",0,16);
    lcd.print("Temp.: ",0,24);
    lcd.invertText(true);
    lcd.printNumI(int(einmaischTemp),49,24);
    lcd.invertText(false);
    lcd.print("~ C",70,24);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      fsmMaischen.immediateTransitionTo(stateEnterAnzahlRasten);
    }
  } 
  
}


void enterAnzahlRasten() {
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    anzahlRasten += encoderValue;
    if(anzahlRasten < 0)
      anzahlRasten = 0;
    
    clrScr(false,false);
    lcd.print("Anzahl Rasten",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(anzahlRasten),15,24);
    lcd.invertText(false);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      if(anzahlRasten > 0){
        subState = 0;
        if (rastTemp != 0) {
          delete [] rastTemp;
        }
        if (rastDauer != 0) {
          delete [] rastDauer;
        }
        rastTemp = new int[anzahlRasten];
        rastDauer = new int[anzahlRasten];
        aktuelleRast = 0;
        rastTemp[aktuelleRast] = einmaischTemp;
        rastDauer[aktuelleRast] = 30;
        
        fsmMaischen.immediateTransitionTo(stateDefineRast);
      }
      else
        fsmMaischen.immediateTransitionTo(stateEnterAbmaischTemp);
    }
  } 
}



void defineRast() {
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    if(subState == 0 ){
      rastTemp[aktuelleRast] += encoderValue;
      if(rastTemp[aktuelleRast] < 0)
        rastTemp[aktuelleRast] = 0;
    } else {
      rastDauer[aktuelleRast] += encoderValue;
      if(rastDauer[aktuelleRast] < 0)
        rastDauer[aktuelleRast] = 0;
    }
      
    clrScr(false,false);
    lcd.printNumI(int(aktuelleRast+1),8,12);
    lcd.print(". Rast",15+(aktuelleRast>9?7:0),8);
    lcd.print("Temp:",15,20);
    lcd.invertText(subState==0);
    lcd.printNumI(int(rastTemp[aktuelleRast]),57,16);
    lcd.invertText(false);
    lcd.print("Dauer:",15,24);
    lcd.invertText(subState==1);
    lcd.printNumI(int(rastDauer[aktuelleRast]),57,24);
    lcd.invertText(false);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      if(subState == 0 ){
        subState = 1;
        first = true;
      } else {
        subState = 0;
        aktuelleRast += 1;
        first = true;
        if(aktuelleRast >= anzahlRasten){
          fsmMaischen.immediateTransitionTo(stateEnterAbmaischTemp);
        } else {
          rastTemp[aktuelleRast] = rastTemp[aktuelleRast-1];
          rastDauer[aktuelleRast] = 30;
        }
      }
    }
  } 
}

void enterAbmaischTemp() {
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    abmaischTemp += encoderValue;  
    if(abmaischTemp < 0)
      abmaischTemp = 0;
    
    clrScr(false,false);
    lcd.print("Abmaisch-",0,16);
    lcd.print("Temp: ",0,24);
    lcd.invertText(true);
    lcd.printNumI(int(abmaischTemp),49,24);
    lcd.invertText(false);
    lcd.print("~ C",70,24);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        fsmMaischen.immediateTransitionTo(stateDoMaischen);
    }
  } 
}

void enterDoMaischen() {
  fsmMaischeProzess.immediateTransitionTo(stateReachEinmaischTemp);
  sollTemp = MIN_TEMP;
}

void doMaischen() {
  fsmMaischeProzess.update();
}

void reachEinmaischTemp() {
  if(sollTemp == MIN_TEMP){
    sollTemp = einmaischTemp;
    
    clrScr(false,false);
    
    lcd.print("Warte auf", CENTER, 8);
    lcd.print("Einmaischtemp", CENTER, 16);
    String s = "";
    s += einmaischTemp;
    s += "~ C";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 24);
  }
  if(temp >= einmaischTemp){
    sollTemp = MIN_TEMP;
    if(anzahlRasten > 0){
      fsmMaischeProzess.immediateTransitionTo(stateEinmaischen);
    }
    else 
      fsmMaischeProzess.immediateTransitionTo(stateReachAbmaischTemp);
  }
  
}

void einmaischen() {
  if(sollTemp==MIN_TEMP){
    //Temperatur ändern um Display reload zu verhindern
    //Wert sollte so klein sein, dass nicht geheizt wird.
    sollTemp = MIN_TEMP+1;
    alertMillis = millis();
    
    clrScr(false,false);
    
    lcd.print("Einmaischen", CENTER, 8);
    lcd.print("Weiter mit", CENTER, 16);
    lcd.print("Klick", CENTER, 24);
  }
  
  //Akustisch warnen
  if(alertMillis > 0 && (millis()-alertMillis/500)%2==0){
    tone(PIN_BUZZER, 262, 250);
  } else {
    noTone(PIN_BUZZER);
  }
  
  //Visuell Warnen
  if(alertMillis > 0 && (millis()-alertMillis/100)%2==0){
    digitalWrite(PIN_BG_LIGHT,LOW);
  } else {
    digitalWrite(PIN_BG_LIGHT,HIGH);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      sollTemp = MIN_TEMP;
      aktuelleRast = 0;
      alertMillis = 0;
      noTone(PIN_BUZZER);
      digitalWrite(PIN_BG_LIGHT,HIGH);
      fsmMaischeProzess.immediateTransitionTo(stateReachRastTemp);
    }
  } 
  
  //Nach spät. 10 sec. den Alarm Abbrechen
  if((millis()-alertMillis >= 10000){
    alertMillis = 0;
    noTone(PIN_BUZZER);
    digitalWrite(PIN_BG_LIGHT,HIGH);
  }
}

void reachRastTemp() {
  if(sollTemp==MIN_TEMP){
    sollTemp = rastTemp[aktuelleRast];
    
    clrScr(false,false);
    
    lcd.print("Warte auf", CENTER, 8);
    String s = "";
    s += aktuelleRast+1;
    s += ". Rast Temp.";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 16);
    s = "";
    s += rastTemp[aktuelleRast];
    s += "~ C";
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 24);
  }
  
  if(temp >= rastTemp[aktuelleRast]){
    fsmMaischeProzess.immediateTransitionTo(stateWaitRastDauer);
  }
}

void prepareRastDauer() {
  lastRest = MAX_LONG;
  storedSystemMillis = millis();
  dauer = rastDauer[aktuelleRast]*60*1000L;
}

void waitRastDauer() {
  long rest = dauer - (millis()-storedSystemMillis);
  
  //Anzeige nur ändern, wenn eine Sekunde vergangen ist.
  if(lastRest-rest > 1000){
    lastRest = rest;
    long std = ((rest/1000) / 60) / 60;
    long min = ((rest/1000) / 60) % 60;
    long sec = (rest/1000) % 60;
  
    clrScr(false,false);
    String s = "";
    s += aktuelleRast+1;
    s += ". Rast:";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 8);
    
    s = "";
    s += rastTemp[aktuelleRast];
    s += "~ C / ";
    s += rastDauer[aktuelleRast];
    s += "min";
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 16);
    
    s = "noch ";
    if(rastDauer[aktuelleRast] > 60){
      s += std;
      s += sec%2==0?":":" ";
    }
    s += min < 10 ? "0":"";
    s += min;
    s += sec%2==0?":":" ";
    s += sec < 10 ? "0":"";
    s += sec;
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 24);
  }
  
  if(rest <= 0){
    sollTemp = MIN_TEMP;
    if(anzahlRasten > aktuelleRast+1){
      aktuelleRast += 1;
      lastRest = MAX_LONG;
      fsmMaischeProzess.immediateTransitionTo(stateReachRastTemp);
    } else {
      fsmMaischeProzess.immediateTransitionTo(stateReachAbmaischTemp);
    }
  }
}

void reachAbmaischTemp() {
  if(sollTemp == MIN_TEMP){
    sollTemp = abmaischTemp;
    
    clrScr(false,false);
    
    lcd.print("Warte auf", CENTER, 8);
    lcd.print("Abmaischtemp.", CENTER, 16);
    String s = "";
    s += abmaischTemp;
    s += "~ C";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 24);
  }
  if(temp >= abmaischTemp){
    fsmMain.immediateTransitionTo(stateMenu);
  }
  
}

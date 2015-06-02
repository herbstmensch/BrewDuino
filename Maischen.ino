int einmaischTemp = 48;
int abmaischTemp = 78;
int anzahlRasten = 3;
int* rastTemp=0;
int* rastDauer=0;

int aktuelleRast = 0;
int subState = 0;

State stateReachEinmaischTemp = State(reachEinmaischTemp);
State stateReachRastTemp = State(reachRastTemp);
State stateWaitRastDauer = State(prepareRastDauer,waitRastDauer,NULL);
State stateReachAbmaischTemp = State(reachAbmaischTemp);

FSM fsmMaischeProzess = FSM(stateReachEinmaischTemp);

void enterEinmaischTemp() {
  
  encoderValue += encoder->getValue();
  if(encoderValue != lastEncoderValue){
    lastEncoderValue = encoderValue;
    einmaischTemp += encoderValue;  
    if(einmaischTemp < 0)
      einmaischTemp = 0;
 
    clrScr(false,false);
    lcd.print("Einmaischtmp",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(einmaischTemp),15,24);
    lcd.invertText(false);
    lcd.print("~C",29,24);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      fsmMaischen.immediateTransitionTo(stateEnterAnzahlRasten);
    }
  } 
  
}


void enterAnzahlRasten() {
  encoderValue += encoder->getValue()-lastEncoderValue;
  if(encoderValue != lastEncoderValue){
    lastEncoderValue = encoderValue;
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
      if(anzahlRasten > 0)
        fsmMaischen.immediateTransitionTo(stateDefineRast);
      else
        fsmMaischen.immediateTransitionTo(stateEnterAbmaischTemp);
    }
  } 
}



void defineRast() {
  encoderValue += encoder->getValue()-lastEncoderValue;
  if(encoderValue != lastEncoderValue){
    lastEncoderValue = encoderValue; 
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
    lcd.print("Temp :",15,20);
    lcd.invertText(subState==0);
    lcd.printNumI(int(rastTemp[aktuelleRast]),57,16);
    lcd.invertText(false);
    lcd.print("Dauer:",15,28);
    lcd.invertText(subState==1);
    lcd.printNumI(int(rastDauer[aktuelleRast]),57,24);
    lcd.invertText(false);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      if(subState == 0 ){
        subState = 1;
        storeEncoderValue();
      } else {
        subState = 0;
        aktuelleRast += 1;
        storeEncoderValue();
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
  encoderValue += encoder->getValue()-lastEncoderValue;
  if(encoderValue != lastEncoderValue){
    lastEncoderValue = encoderValue;
    abmaischTemp += encoderValue;  
    if(abmaischTemp < 0)
      abmaischTemp = 0;
    
    clrScr(false,false);
    lcd.print("Abmaischtmp",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(abmaischTemp),15,24);
    lcd.invertText(false);
    lcd.print("~C",29,24);
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
    aktuelleRast = 0;
    sollTemp == MIN_TEMP;
    if(anzahlRasten > 0)
      fsmMaischeProzess.immediateTransitionTo(stateReachRastTemp);
    else 
      fsmMaischeProzess.immediateTransitionTo(stateReachAbmaischTemp);
  }
  
}

void reachRastTemp() {
  if(sollTemp==MIN_TEMP){
    sollTemp = rastTemp[aktuelleRast];
    
    clrScr(false,false);
    
    lcd.print("Warte auf", CENTER, 8);
    String s = "";
    s += aktuelleRast+1;
    s += ". Rast Temp";
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
    Serial.print("Rest: ");
    Serial.println(rest);
    lastRest = rest;
    long min = (rest/1000) / 60;
    long sec = (rest/1000) % 60;
    
    Serial.print("Min:");
    Serial.print(min);
    Serial.print(" Sec: ");
    Serial.println(sec);
  
    clrScr(false,false);
    String s = "";
    s += aktuelleRast+1;
    s += ". Rast:";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 8);
    
    s = "";
    s += rastTemp[aktuelleRast];
    s += "~C / ";
    s += rastDauer[aktuelleRast];
    s += "min";
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 16);
    
    s = "noch ";
    s += min < 10 ? "0":"";
    s += min;
    s += ":";
    s += sec < 10 ? "0":"";
    s += sec;
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 24);
    Serial.println(buf);
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
    lcd.print("Abmaischtemp", CENTER, 16);
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

int einmaischTemp = 48;
int abmaischTemp = 78;
int anzahlRasten = 3;
int rastTemp[10] = {0,0,0,0,0,0,0,0,0,0};
int rastDauer[10] = {0,0,0,0,0,0,0,0,0,0};

int aktuelleRast = 0;
int subState = 0;

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
  
  if(buttonClicked()){
    fsmMaischen.immediateTransitionTo(stateEnterAnzahlRasten);
  } 
  
}


void enterAnzahlRasten() {
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    anzahlRasten += encoderValue;
    if(anzahlRasten < 0)
      anzahlRasten = 0;
    if(anzahlRasten > 10)
      anzahlRasten = 10;
    
    clrScr(false,false);
    lcd.print("Anzahl Rasten",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(anzahlRasten),15,24);
    lcd.invertText(false);
  }
  
  if(buttonClicked()){
    if(anzahlRasten > 0){
      subState = 0;
      aktuelleRast = 0;
      rastTemp[aktuelleRast] = einmaischTemp;
      rastDauer[aktuelleRast] = 30;
      
      fsmMaischen.immediateTransitionTo(stateDefineRast);
    }
    else
      fsmMaischen.immediateTransitionTo(stateEnterAbmaischTemp);
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
  
  if(buttonClicked()){
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
  
  if(buttonClicked()){
    fsmMaischen.immediateTransitionTo(stateReachEinmaischTemp);
  } 
}

void prepareReachEinmaischTemp() {
  setSollTemp(MIN_TEMP);
}

void reachEinmaischTemp() {
  if(sollTemp == MIN_TEMP){
    setSollTemp(einmaischTemp);
    turnOnStiring();
    
    clrScr(false,false);
    
    lcd.print("Warte auf", CENTER, 8);
    lcd.print("Einmaischtemp", CENTER, 16);
    snprintf(lcdBuf,sizeof(lcdBuf),"%i~C\0",einmaischTemp);
    lcd.print(lcdBuf, CENTER, 24);
  }
  
  #if DEBUG == 1
  temp = einmaischTemp+1;
  #endif
  
  if(temp >= einmaischTemp){
    setSollTemp(MIN_TEMP);
    alarm();
    fsmMaischen.immediateTransitionTo(stateEinmaischen);
  }
  
}

void einmaischen() {
  if(sollTemp==MIN_TEMP){
    //Temperatur ändern um Display reload zu verhindern
    //Wert sollte so klein sein, dass nicht geheizt wird.
    setSollTemp(MIN_TEMP+1);
    
    clrScr(false,false);
    
    lcd.print("Einmaischen", CENTER, 8);
    lcd.print("Weiter mit", CENTER, 16);
    lcd.print("Klick", CENTER, 24);
  }
  
  if(buttonClicked()){
    setSollTemp(MIN_TEMP);
    aktuelleRast = 0;
    cancelAlarm();
    if(anzahlRasten > 0){
      fsmMaischen.immediateTransitionTo(stateReachRastTemp);
    }
    else 
      fsmMaischen.immediateTransitionTo(stateReachAbmaischTemp);
  } 
}

void reachRastTemp() {
  if(sollTemp==MIN_TEMP){
     setSollTemp(rastTemp[aktuelleRast]);
    clrScr(false,false);
    
    lcd.print("Warte auf", CENTER, 8);
    snprintf(lcdBuf,sizeof(lcdBuf),"%i. Rast Temp.\0",aktuelleRast+1);
    lcd.print(lcdBuf, CENTER, 16);
    snprintf(lcdBuf,sizeof(lcdBuf),"%i~ C\0",rastTemp[aktuelleRast]);
    lcd.print(lcdBuf, CENTER, 24);
  }

  #if DEBUG == 1
  temp = rastTemp[aktuelleRast]+1;
  #endif
  
  if(temp >= rastTemp[aktuelleRast]){
    fsmMaischen.immediateTransitionTo(stateWaitRastDauer);
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
  if(lastRest-rest >= 500){
    lastRest = rest;
    int std = ((rest/1000) / 60) / 60;
    int min = ((rest/1000) / 60) % 60;
    int sec = (rest/1000) % 60;
  
    clrScr(false,false);

    snprintf(lcdBuf,sizeof(lcdBuf),"%i. Rast:\0",aktuelleRast+1);
    lcd.print(lcdBuf, CENTER, 8);
    
    snprintf(lcdBuf,sizeof(lcdBuf),"%i~ C / %imin\0",rastTemp[aktuelleRast],rastDauer[aktuelleRast]);
    lcd.print(lcdBuf, CENTER, 16);
    
    if(std > 0){
     snprintf(lcdBuf, sizeof(lcdBuf), (rest/500)%2==0?"noch %.2i:%.2i:%.2i\0":"noch %.2i %.2i %.2i\0",std,min,sec);
    } else {
      snprintf(lcdBuf, sizeof(lcdBuf), (rest/500)%2==0?"noch %02d:%02d\0":"noch %02d %02d\0",min,sec);
    }
    lcd.print(lcdBuf, CENTER, 24);
  }
  
  if(rest <= 0){
    setSollTemp(MIN_TEMP);
    if(anzahlRasten > aktuelleRast+1){
      aktuelleRast += 1;
      lastRest = MAX_LONG;
      fsmMaischen.immediateTransitionTo(stateReachRastTemp);
    } else {
      fsmMaischen.immediateTransitionTo(stateReachAbmaischTemp);
    }
  }
}

void reachAbmaischTemp() {
  if(sollTemp == MIN_TEMP){
    setSollTemp(abmaischTemp);
    
    clrScr(false,false);
    
    lcd.print("Warte auf", CENTER, 8);
    lcd.print("Abmaischtemp.", CENTER, 16);
    snprintf(lcdBuf,sizeof(lcdBuf),"%i~ C\0",abmaischTemp);
    lcd.print(lcdBuf, CENTER, 24);
  }
  if(temp >= abmaischTemp){
    alarm();
    turnOffStiring();
    fsmMain.immediateTransitionTo(stateMenu);
  }
  
}

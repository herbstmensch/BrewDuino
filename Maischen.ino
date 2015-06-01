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
  
  einmaischTemp += encoder->getValue()-savedEncoderValue;
  clrScr(false,false);
  lcd.print("Einmaischtmp",0,16);
  lcd.invertText(true);
  lcd.printNumI(int(einmaischTemp),15,24);
  lcd.invertText(false);
  lcd.print("~C",29,24);
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        fsmMaischen.immediateTransitionTo(stateEnterAnzahlRasten);
    }
  } 
  
}


void enterAnzahlRasten() {
  anzahlRasten += encoder->getValue()-savedEncoderValue;
  clrScr(false,false);
  lcd.print("Anzahl Rasten",0,16);
  lcd.invertText(true);
  lcd.printNumI(int(anzahlRasten),15,24);
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      aktuelleRast = -1;
      subState = 0;
      if (rastTemp != 0) {
        delete [] rastTemp;
      }
      if (rastDauer != 0) {
        delete [] rastDauer;
      }
      rastTemp = new int[anzahlRasten];
      rastDauer = new int[anzahlRasten];
      fsmMaischen.immediateTransitionTo(stateDefineRast);
    }
  } 
}



void defineRast() {
  
  if(aktuelleRast < 0){
    aktuelleRast = 0;
    rastTemp[aktuelleRast] = einmaischTemp;
    rastDauer[aktuelleRast] = 30;
  }
  
  if(subState == 0 ){
    rastTemp[aktuelleRast] += encoder->getValue()-savedEncoderValue;
  } else {
    rastDauer[aktuelleRast] += (encoder->getValue()-savedEncoderValue);
  }
    
  clrScr(false,false);
  lcd.printNumI(int(aktuelleRast+1),8,12);
  lcd.print(". Rast",15+(aktuelleRast>9?7:0),12);
  lcd.print("Temp :",15,20);
  lcd.invertText(subState==0);
  lcd.printNumI(int(rastTemp[aktuelleRast]),57,20);
  lcd.invertText(false);
  lcd.print("Dauer:",15,28);
  lcd.invertText(subState==1);
  lcd.printNumI(int(rastDauer[aktuelleRast]),57,28);
  lcd.invertText(false);
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      if(subState == 0 ){
        subState = 1;
      } else {
        subState = 0;
        aktuelleRast += 1;
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
  
  abmaischTemp += encoder->getValue()-savedEncoderValue;
  clrScr(false,false);
  lcd.print("Abmaischtmp",0,16);
  lcd.printNumI(int(abmaischTemp),15,24);
  lcd.print("~C",29,24);
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        fsmMaischen.immediateTransitionTo(stateDoMaischen);
    }
  } 
}

void enterDoMaischen() {
  fsmMaischeProzess.immediateTransitionTo(stateReachEinmaischTemp);
}

void doMaischen() {
  fsmMaischeProzess.update();
}

void reachEinmaischTemp() {
  sollTemp = einmaischTemp;
  
  clrScr(false,false);
  
  lcd.print("Warte auf", CENTER, 16);
  lcd.print("Einmaischtemp", CENTER, 24);
  String s = "";
  s += einmaischTemp;
  s += "~ C";
  char buf[14];
  s.toCharArray(buf,14);
  lcd.print(buf, CENTER, 32);
  
  if(temp >= einmaischTemp){
    aktuelleRast = 1;
    fsmMaischeProzess.immediateTransitionTo(stateReachRastTemp);
  }
  
}

void reachRastTemp() {
  sollTemp = rastTemp[aktuelleRast];
  
  clrScr(false,false);
  
  lcd.print("Warte auf", CENTER, 16);
  String s = "";
  s += aktuelleRast;
  s += ". Rast Temp";
  char buf[14];
  s.toCharArray(buf,14);
  lcd.print(buf, CENTER, 24);
  s = "";
  s += rastTemp[aktuelleRast];
  s += "~ C";
  s.toCharArray(buf,14);
  lcd.print(buf, CENTER, 32);
  
  if(temp >= rastTemp[aktuelleRast]){
    fsmMaischeProzess.immediateTransitionTo(stateWaitRastDauer);
  }
}

void prepareRastDauer() {
  lastRest = -1;
  storedSystemMillies = millies();
  dauer = rastDauer[aktuelleRast]*60*1000;
}

void waitRastDauer() {
  int rest = dauer - (millies()-storedSystemMillies);
  
  //Anzeige nur ändern, wenn eine Sekunde vergangen ist.
  if(lastRest-rest > 1000){
    lastRest = rest;
    int min = (rest/1000) / 60;
    int sec = (rest/1000) % 60;
  
    String s = "";
    s += aktuelleRast;
    s += ". Rast:";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 16);
    s = "";
    s += rastTemp[aktuelleRast];
    s += "~ C / ";
    s += rastDauer[aktuelleRast];
    s += "min";
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 24);
    
    s = "noch ";
    s += min < 10 ? "0":"";
    s += min;
    s += ":";
    s += sec < 10 ? "0":"";
    s += sec;
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 24);
  }
  if(rest <= 0){
    if(anzahlRasten > aktuelleRast){
      aktuelleRast += 1;
      lastSec = -1;
      fsmMaischeProzess.immediateTransitionTo(stateWaitRastTemp);
    } else {
      fsmMaischeProzess.immediateTransitionTo(stateReachAbmaischTemp);
    }
  }
}

void reachAbmaischTemp {
  
  sollTemp = abmaischTemp;
  
  clrScr(false,false);
  
  lcd.print("Warte auf", CENTER, 16);
  lcd.print("Abmaischtemp", CENTER, 24);
  String s = "";
  s += abmaischTemp;
  s += "~ C";
  char buf[14];
  s.toCharArray(buf,14);
  lcd.print(buf, CENTER, 32);
  
  if(temp >= abmaischTemp){
    fsmMain.immediateTransitionTo(menu);
  }
  
}

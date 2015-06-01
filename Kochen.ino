int kochzeit = 90;
int anzahlHopfengaben = 3;
int* hopfengaben=0;
int aktuelleHopfengabe=1

State stateReachKochTemp = State(reachKochmaischTemp);
State stateWaitKochDauer = State(prepareKochDauer,waitKochDauer,NULL);
FSM fsmKochProzess = FSM(stateReachKochTemp);

void enterKochzeit() {
  kochzeit += encoder->getValue()-savedEncoderValue;
  clrScr(false,false);
  lcd.print("Kochzeit",0,16);
  lcd.invertText(true);
  lcd.printNumI(int(kochzeit),15,24);
  lcd.invertText(false);
  lcd.print(" min",29,24);
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        fsmMaischen.immediateTransitionTo(stateEnterAnzahlHopfengaben);
    }
  } 
}

void enterAnzahlHopfengaben() {
  anzahlHopfengaben += encoder->getValue()-savedEncoderValue;
  clrScr(false,false);
  lcd.print("Anz. Hopfeng.",0,16);
  lcd.invertText(true);
  lcd.printNumI(int(anzahlHopfengaben),15,24);
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      aktuelleHopfengabe= -1;
      if (hopfengaben != 0) {
        delete [] hopfengaben;
      }
      hopfengaben = new int[anzahlHopfengaben];
      fsmKochen.immediateTransitionTo(stateDefineHopfengaben);
    }
  } 
}

void defineHopfengaben() {
  
   if(aktuelleHopfengabe < 0){
    aktuelleHopfengabe = 0;
    rastTemp[aktuelleHopfengabe] = einmaischTemp;
    rastDauer[aktuelleHopfengabe] = 30;
  }
  
  if(subState == 0 ){
    rastTemp[aktuelleHopfengabe] += encoder->getValue()-savedEncoderValue;
  } else {
    rastDauer[aktuelleHopfengabe] += (encoder->getValue()-savedEncoderValue);
  }
    
  clrScr(false,false);
  lcd.printNumI(int(aktuelleHopfengabe+1),8,12);
  lcd.print(". Hopfeng.",15+(aktuelleHopfengabe>9?7:0),12);
  lcd.print("Nach :",15,20);
  lcd.invertText(true);
  lcd.printNumI(int(hopfengaben[aktuelleHopfengabe]),57,20);
  lcd.invertText(false);
  lcd.print("Minuten",15,28);
  
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        aktuelleHopfengabe += 1;
        if(aktuelleHopfengabe >= anzahlHopfengaben){
          fsmKochen.immediateTransitionTo(stateDoKochen);
        } else {
          hopfengaben[aktuelleHopfengabe] = hopfengaben[aktuelleHopfengabe-1];
        }
    }
  } 
}

void enterDoKochen() {
  fsmKochProzess.immediateTransitionTo(stateReachEinmaischTemp);
}

void doKochen() {
  fsmKochProzess.update();
}

void prepareKochDauer() {
  lastRest = -1;
  storedSystemMillies = millies();
  dauer = kochZeit*60*1000;
}

void waitKochDauer() {
  int rest = dauer - (millies()-storedSystemMillies);
  
  //Anzeige nur ändern, wenn eine Sekunde vergangen ist.
  if(lastRest-rest > 1000){
    lastRest = rest;
    int min = (rest/1000) / 60;
    int sec = (rest/1000) % 60;
  
    String s = "";
    s += aktuelleRast;
    s += "Kochen:";
    char buf[14];
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

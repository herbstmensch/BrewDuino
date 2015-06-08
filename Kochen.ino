int kochzeit = 90;
int anzahlHopfengaben = 3;
int* hopfengaben=0;
int aktuelleHopfengabe=1;

State stateReachKochTemp = State(reachKochTemp);
State stateWaitKochDauer = State(prepareKochDauer,waitKochDauer,NULL);
FSM fsmKochProzess = FSM(stateReachKochTemp);

void enterKochzeit() {
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    kochzeit += encoderValue;
    if(kochzeit < 1)
      kochzeit = 1;
  
    clrScr(false,false);
    lcd.print("Kochzeit:",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(kochzeit),15,24);
    lcd.invertText(false);
    lcd.print(" min",29,24);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        fsmKochen.immediateTransitionTo(stateEnterAnzahlHopfengaben);
    }
  } 
}

void enterAnzahlHopfengaben() {
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    anzahlHopfengaben += encoderValue;
    if(anzahlHopfengaben < 0)
      anzahlHopfengaben = 0;
      
    clrScr(false,false);
    lcd.print("Anz. Hopfeng.",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(anzahlHopfengaben),15,24);
    lcd.invertText(false);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
      first = true;
      if(anzahlHopfengaben > 0){
        if (hopfengaben != 0) {
          delete [] hopfengaben;
        }
        hopfengaben = new int[anzahlHopfengaben];
        aktuelleHopfengabe = 0;
        hopfengaben[aktuelleHopfengabe] = 20>kochzeit?kochzeit:20;
        fsmKochen.immediateTransitionTo(stateDefineHopfengaben);
      }
      else
        fsmKochen.immediateTransitionTo(stateDoKochen);
    }
  } 
}

void defineHopfengaben() {
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    hopfengaben[aktuelleHopfengabe] += encoderValue;  
    
    if(aktuelleHopfengabe > 0){
      //Hopfengabe muss nach der vorangegangenen liegen
      if(hopfengaben[aktuelleHopfengabe] <= hopfengaben[aktuelleHopfengabe-1])
        hopfengaben[aktuelleHopfengabe] = hopfengaben[aktuelleHopfengabe-1]+1;
    } 
    //Hopfenhabe darf nicht nach Kochende liegen
    if(hopfengaben[aktuelleHopfengabe] > kochzeit)
      hopfengaben[aktuelleHopfengabe] = kochzeit;
    
    clrScr(false,false);
    lcd.printNumI(int(aktuelleHopfengabe+1),8,12);
    lcd.print(". Hopfeng.",15+(aktuelleHopfengabe>9?7:0),12);
    lcd.print("Nach :",15,20);
    lcd.invertText(true);
    lcd.printNumI(int(hopfengaben[aktuelleHopfengabe]),57,20);
    lcd.invertText(false);
    lcd.print("Minuten",15,28);
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    if( b == ClickEncoder::Clicked ){
        aktuelleHopfengabe += 1;
        if(aktuelleHopfengabe+1 >= anzahlHopfengaben){
          fsmKochen.immediateTransitionTo(stateDoKochen);
        } else {
          hopfengaben[aktuelleHopfengabe] = hopfengaben[aktuelleHopfengabe-1]+1;
        }
    }
  } 
}

void enterDoKochen() {
  first = true;
  fsmKochProzess.immediateTransitionTo(stateReachKochTemp);
}

void doKochen() {
  fsmKochProzess.update();
}

void reachKochTemp() {
  if(first){
    first = false;
    clrScr(false,false);
    
    lcd.print("Warte auf", CENTER, 16);
    lcd.print("Kochtemp.", CENTER, 24);
    String s = "";
    s += sollTemp;
    s += "~ C";
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 32);
  }
  
  if(temp >= sollTemp){
    fsmKochProzess.immediateTransitionTo(stateWaitKochDauer);
  }
  
}

void prepareKochDauer() {
  lastRest = MAX_LONG;
  storedSystemMillis = millis();
  dauer = kochzeit*60*1000L;
}

void waitKochDauer() {
  long rest = dauer - (millis()-storedSystemMillis);
  
  //Anzeige nur ändern, wenn eine Sekunde vergangen ist.
  if(lastRest-rest > 1000){
    lastRest = rest;
    long std = ((rest/1000) / 60) / 60;
    long min = ((rest/1000) / 60) % 60;
    long sec = (rest/1000) % 60;
  
    clrScr(false,false);
    lcd.print("Kochen:", CENTER, 16);
    
    String s = "noch ";
    if(kochzeit > 60){
      s += std;
      s += sec%2==0?":":" ";
    }
    s += min < 10 ? "0":"";
    s += min;
    s += sec%2==0?":":" ";
    s += sec < 10 ? "0":"";
    s += sec;
    char buf[14];
    s.toCharArray(buf,14);
    lcd.print(buf, CENTER, 24);
  }
  if(rest <= 0){
    fsmMain.immediateTransitionTo(stateMenu);
  }
}

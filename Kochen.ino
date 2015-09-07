int kochzeit = 90;
int anzahlHopfengaben = 3;
int[10] hopfengaben={0,0,0,0,0,0,0,0,0,0};
int aktuelleHopfengabe=1;
int alertedHopfengaben = 0;

State stateReachKochTemp = State(NULL,reachKochTemp,prepareKochDauer);
State stateWaitKochDauer = State(waitKochDauer);
State stateAlertHopfengabe = State(alertHopfengabe);
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
  
  if(buttonClicked()){
    fsmKochen.immediateTransitionTo(stateEnterAnzahlHopfengaben);
  } 
}

void enterAnzahlHopfengaben() {
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    anzahlHopfengaben += encoderValue;
    if(anzahlHopfengaben < 0)
      anzahlHopfengaben = 0;
    if(anzahlHopfengaben > 10)
      anzahlHopfengaben = 10;
      
    clrScr(false,false);
    lcd.print("Anz. Hopfeng.",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(anzahlHopfengaben),15,24);
    lcd.invertText(false);
  }
  
  if(buttonClicked()){
    first = true;
    if(anzahlHopfengaben > 0){
      aktuelleHopfengabe = 0;
      hopfengaben[aktuelleHopfengabe] = 20>kochzeit?kochzeit:20;
      fsmKochen.immediateTransitionTo(stateDefineHopfengaben);
    }
    else {
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
  
  if(buttonClicked()){
    aktuelleHopfengabe += 1;
    if(aktuelleHopfengabe+1 > anzahlHopfengaben){
      fsmKochen.immediateTransitionTo(stateDoKochen);
    } else {
      hopfengaben[aktuelleHopfengabe] = hopfengaben[aktuelleHopfengabe-1]+1;
    }
  } 
}

void enterDoKochen() {
  first = true;
  alertedHopfengaben = -1;
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
    //String s = "";
    //s += sollTemp;
    //s += "~ C";
    //char buf[14];
    //s.toCharArray(buf,14);
    snprintf(lcdBuf,sizeof(lcdBuf),"%i~ C\0",sollTemp)
    lcd.print(lcdBuf, CENTER, 32);
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
    
    //String s = "noch ";
    if(kochzeit > 60){
      //s += std;
      //s += sec%2==0?":":" ";
      snprintf(lcdBuf,sizeof(lcdBuf),"noch %.2i%s%.2i%s%.2i\0",std,sec%2==0?":":" ",min,sec%2==0?":":" ",sec);
    } else {
      snprintf(lcdBuf,sizeof(lcdBuf),"noch %.2i%s%.2i\0",min,sec%2==0?":":" ",sec);
    }
    //s += min < 10 ? "0":"";
    //s += min;
    //s += sec%2==0?":":" ";
    //s += sec < 10 ? "0":"";
    //s += sec;
    //char buf[14];
    //s.toCharArray(buf,14);
    
    lcd.print(lcdBuf, CENTER, 24);
    
    //Hopfengabe?
    if(anzahlHopfengaben-1 > alertedHopfengaben){
      long hg = hopfengaben[alertedHopfengaben+1]*60*1000;
      long verstrichen = (millis()-storedSystemMillis);
      if(verstrichen > hg){
        alertedHopfengaben += 1;
        first = true;
         fsmKochProzess.immediateTransitionTo(stateAlertHopfengabe);
      }
    }
    
    
  }
  if(rest <= 0){
    alarm();
    fsmMain.immediateTransitionTo(stateMenu);
  }
}

unsigned long hgMillis;

void alertHopfengabe(){
  if(first){
    alarm();
    hgMillis = alertMillis;
    clrScr(false, false);
    //String s = "";
    //s += (alertedHopfengaben+1);
    //s += ". Hopfeng.";
    //char buf[14];
    //s.toCharArray(buf,14);
    snprintf(lcdBuf,sizeof(lcdBuf),"%i. Hopfeng.\0",(alertedHopfengaben+1));
    lcd.print(lcdBuf, CENTER, 16);
    //s = "";
    //s += hopfengaben[alertedHopfengaben];
    //s += "min. Kochz.";
    //s.toCharArray(buf,14);
    snprintf(lcdBuf,sizeof(lcdBuf),"%i min. Kochz.\0",hopfengaben[alertedHopfengaben]);
    lcd.print(lcdBuf, CENTER, 24);
  }
  
  if(buttonClicked()){
    fsmKochProzess.immediateTransitionTo(stateWaitKochDauer);
    cancelAlarm();
  } 
  
  //Nach spät. 1 min. die Hopfengabe ausblenden
  if(millis()-hgMillis >= 60000){
    fsmKochProzess.immediateTransitionTo(stateWaitKochDauer);
  }
  
}

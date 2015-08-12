int time = 45;

void enterTimerTime(){
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    time += encoderValue;
    if(time < 1)
      time = 1;
  
    clrScr(false,false);
    lcd.print("Countdown:",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(time),15,24);
    lcd.invertText(false);
    lcd.print(" min",29,24);
  }
  
  if(buttonClicked()){
    fsmTimer.immediateTransitionTo(stateDoTimer);
  } 
}

void prepareDoTimer(){
  lastRest = MAX_LONG;
  storedSystemMillis = millis();
  dauer = time*60*1000L;
  clrScr(false,false);
  lcd.print("Countdown:", CENTER, 16);
}

void doTimer(){
  long rest = dauer - (millis()-storedSystemMillis);
  
  //Anzeige nur ändern, wenn eine Sekunde vergangen ist.
  if(lastRest-rest > 500){
    lastRest = rest;
    long std = ((rest/1000) / 60) / 60;
    long min = ((rest/1000) / 60) % 60;
    long sec = (rest/1000) % 60;
    
    String s = "";
    if(rastDauer[aktuelleRast] > 60){
      s += std;
      s += (rest/500)%2==0?":":" ";
    }
    s += min < 10 ? "0":"";
    s += min;
    s += (rest/500)%2==0?":":" ";
    s += sec < 10 ? "0":"";
    s += sec;
    char buf[14];
    s.toCharArray(buf,14);
    printRow(buf, CENTER, 3);
  }
  
  if(rest <= 0){
      alarm();
      fsmMain.immediateTransitionTo(stateMenu);
  }
}

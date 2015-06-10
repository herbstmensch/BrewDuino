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
}

void doTimer(){
  long rest = dauer - (millis()-storedSystemMillis);
  
  //Anzeige nur ändern, wenn eine Sekunde vergangen ist.
  if(lastRest-rest > 1000){
    lastRest = rest;
    long std = ((rest/1000) / 60) / 60;
    long min = ((rest/1000) / 60) % 60;
    long sec = (rest/1000) % 60;
    
    clrScr(false,false);
    lcd.print("Countdown:", CENTER, 16);
    
    String s = "";
    if(rastDauer[aktuelleRast] > 60){
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
      alarm();
      fsmMain.immediateTransitionTo(stateMenu);
  }
}

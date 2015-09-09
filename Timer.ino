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
  if(lastRest-rest >= 500){
    lastRest = rest;
    int std = ((rest/1000) / 60) / 60;
    int min = ((rest/1000) / 60) % 60;
    int sec = (rest/1000) % 60;
    
    if(time > 60){
      snprintf(lcdBuf, sizeof(lcdBuf), (rest/500)%2==0?"%.2i:%.2i:%.2i\0":"%.2i %.2i %.2i\0",std,min,sec);
    } else {
      snprintf(lcdBuf, sizeof(lcdBuf), (rest/500)%2==0?"%02d:%02d\0":"%02d %02d\0",min,sec);
    }
   
    printRow(lcdBuf, CENTER, 3);
  }
  
  if(rest <= 0){
      alarm();
      fsmMain.immediateTransitionTo(stateMenu);
  }
}

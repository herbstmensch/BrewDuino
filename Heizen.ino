void doHeizen(){
  encoderValue = encoder->getValue();
  if(encoderValue != 0 || first){
    first = false;
    setSollTemp(sollTemp + encoderValue);
    if(sollTemp > 101)
      setSollTemp(101);
    if(sollTemp < 0)
      setSollTemp(0);
      
    
    checkHeatingStatus(true);
  
    clrScr(false,false);
    lcd.print("Heizen auf:",0,16);
    lcd.invertText(true);
    lcd.printNumI(int(sollTemp),15,24);
    lcd.invertText(false);
    lcd.print("~ C",29,24);
  }
}

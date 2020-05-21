//function to handle both retrieval of data from sensors, as well as recording it on the SD card
void updateSensors() {
  oledTime = millis();

  if(sensorHeatRelay.getState()==true){
    sensorHeat_Status = "ON";
  }
  else if(sensorHeatRelay.getState()==false){
    sensorHeat_Status = "OFF";
  }

//Pressure, Temp, and Altitude
  t1 = thermocoupleA.readThermocoupleTemperature();
  t2 = thermocoupleB.readThermocoupleTemperature();

  pressureSensor = analogRead(HONEYWELL_PRESSURE);                      //Read the analog pin
  pressureSensorVoltage = pressureSensor * (5.0/8196);                  //Convert the analog number to voltage
  PressurePSI = (pressureSensorVoltage - (0.1*5.0))/(4.0/15.0);         //Convert the voltage to PSI
  PressureATM = PressurePSI*PSI_TO_ATM;                                 //Convert PSI reading to ATM

  OPCdata = SpsA.logUpdate();
  OPCdata += ",=," + SpsB.logUpdate();

  data = "";
  data = flightTimeStr()+ "," + String(flightMinutes()) + "," + "," + String(GPS.getLat(), 4) + "," + String(GPS.getLon(), 4) + "," 
  + String(GPS.getAlt_feet(), 1) + ","
  + String(GPS.getMonth()) + "/" + String(GPS.getDay()) + "/" + String(GPS.getYear()) + ","
  + String(GPS.getHour()) + ":" + String(GPS.getMinute()) + ":" + String(GPS.getSecond()) + ","
  + String(GPS.getSats()) + ",";
  
  if(GPS.getFixAge() > 4000){                                           //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
    data += "No Fix,";
  }
  else{
    data += "Fix,";
  }

  data += (String(t1,4) + "," +String(t2,4) + ",");     //Data string population
  data += (String(PressurePSI,6) + "," + String(PressureATM,6) + ",");
  data += (sensorHeat_Status + ",");
  data += (",=," + OPCdata);
  
  openFlightlogA();
  delay(100); 
  FlogA.println(data);
  closeFlightlogA();

  openFlightlogB();
  delay(100); 
  FlogB.println(data);
  closeFlightlogB();

  printData();
}

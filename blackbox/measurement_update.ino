//function to handle both retrieval of data from sensors, as well as recording it on the SD card
void updateSensors() {
  
  oledTime = millis();


  if(sensorHeatRelay.getState()){
    sensorHeat_Status = "ON";
  }
  else if(!sensorHeatRelay.getState()){
    sensorHeat_Status = "OFF";
  }

  //////////////////////////////////////////////////////
  //////////// Raw calculation of variables ////////////
  //////////////////////////////////////////////////////
  // Temperature, Pressure, and OPc
  // First, temperature
  compassData.T1 = analogRead(THERMISTOR_A);                                   // All of these calculations have to do with the Steinhart-Hart equations and setting them up properly
  compassData.T2 = analogRead(THERMISTOR_B);
  compassData.T1 = log(((ADC_MAX/compassData.T1)-1)*CONST_R);
  compassData.T2 = log(((ADC_MAX/compassData.T2)-1)*CONST_R);
  compassData.T1 = CONST_A+CONST_B*compassData.T1+CONST_C*compassData.T1*compassData.T1*compassData.T1;
  compassData.T2 = CONST_A+CONST_B*compassData.T2+CONST_C*compassData.T2*compassData.T2*compassData.T2;
  compassData.T1 = 1/compassData.T1-C2K;                                                  // The final temperatures for both transistors in Celsius
  compassData.T2 = 1/compassData.T2-C2K;

  // Next, pressure
//  updatePressure();
//  compassData.PressurePSI = analogRead(HONEYWELL_PRESSURE);                      //Read the analog pin
//  compassData.PressurePSI = compassData.PressurePSI * (5.0/ADC_MAX);              //Convert the analog number to voltage
//  compassData.PressurePSI = (compassData.PressurePSI - (0.1*5.0))/(4.0/15.0);    //Convert the voltage to PSI
//  compassData.PressureATM = compassData.PressurePSI*PSI_TO_ATM;                  //Convert PSI reading to ATM
//  
  // Finally, OPC data
  OPCdata = (String(SpsA.getFanStatus()) + "," + SpsA.logUpdate());
  OPCdata += (",=," + String(SpsB.getFanStatus()) + SpsB.logUpdate());

  //////////////////////////////////////////////////////////////////////////////////
  ///////////// UPDATING THE SYSTEM DATA STRUCT (called "compassData") /////////////
  //////////////////////////////////////////////////////////////////////////////////
  //Using a struct means that update functions are only called once, so the data logged locally and sent externally is universally consistent.

  compassData.flightTime = millis();

  if (gpsConnected){                                                                  // The if statement is to ensure that if the gps isnt connected, the values for its data are all zero
    compassData.locationData.latitude = GPS.getLat();
    compassData.locationData.longitude = GPS.getLon();
    compassData.locationData.alt = GPS.getAlt_feet();
    compassData.locationData.mm = GPS.getMonth();
    compassData.locationData.dd = GPS.getDay();
    compassData.locationData.yyyy = GPS.getYear();
    compassData.locationData.hours = GPS.getHour();
    compassData.locationData.minutes = GPS.getMinute();
    compassData.locationData.seconds = GPS.getSecond();
    compassData.locationData.sats = GPS.getSats();
    compassData.locationData.fixAge = GPS.getFixAge();
  } else {
    compassData.locationData.latitude = 0.0;
    compassData.locationData.longitude = 0.0;
    compassData.locationData.alt = 0.0;
    compassData.locationData.mm = 00;
    compassData.locationData.dd = 00;
    compassData.locationData.yyyy = 0000;
    compassData.locationData.hours = 00;
    compassData.locationData.minutes = 00;
    compassData.locationData.seconds = 00;
    compassData.locationData.sats = 0;
    compassData.locationData.fixAge = 0;
  }
  compassData.sensorHeatStatus = sensorHeat_Status;
  compassData.spsA_data_abv.hits = SpsA.getTot();
  compassData.spsA_data_abv.numberCount[0] = SpsA.SPSdata.nums[0];
  compassData.spsA_data_abv.numberCount[1] = SpsA.SPSdata.nums[1];
  compassData.spsA_data_abv.numberCount[2] = SpsA.SPSdata.nums[2];
  compassData.spsA_data_abv.numberCount[3] = SpsA.SPSdata.nums[3];
  compassData.spsA_data_abv.numberCount[4] = SpsA.SPSdata.nums[4];
  compassData.spsB_data_abv.hits = SpsB.getTot();
  compassData.spsB_data_abv.numberCount[0] = SpsB.SPSdata.nums[0];
  compassData.spsB_data_abv.numberCount[1] = SpsB.SPSdata.nums[1];
  compassData.spsB_data_abv.numberCount[2] = SpsB.SPSdata.nums[2];
  compassData.spsB_data_abv.numberCount[3] = SpsB.SPSdata.nums[3];
  compassData.spsB_data_abv.numberCount[4] = SpsB.SPSdata.nums[4];
  compassData.PressureATM = baro.readPressure()*0.00000986923;
  compassData.PressurePSI = compassData.PressureATM*1/PSI_TO_ATM;
  Vout = analogRead(HONEYWELL_PRESSURE)/ADC_MAX*3.3;                      //Read the analog pin
  PPSI = ((Vout-0.1*VSUP)*(PMAX-PMIN))/(0.8*VSUP)+PMIN;
  PATM = PPSI*PSI_TO_ATM;
  compassData.PressureAnalogPSI = PPSI;
  compassData.PressureAnalogATM = PATM;
  
  //////////////////////////////////////////////////////////////////
  ////////// UPDATING THE DATA STRING (called "data") //////////////
  //////////////////////////////////////////////////////////////////
  
  dataLine = "";
  dataLine = flightTimeStr()+ "," + String(flightMinutes()) + "," + String(compassData.locationData.latitude, 4) + "," + String(compassData.locationData.longitude, 4) + "," 
  + String(compassData.locationData.alt, 1) + ","
  + String(compassData.locationData.mm) + "/" + String(compassData.locationData.dd) + "/" + String(compassData.locationData.yyyy) + ","
  + String(compassData.locationData.hours) + ":" + String(compassData.locationData.minutes) + ":" + String(compassData.locationData.seconds) + ","
  + String(compassData.locationData.sats) + ",";
  
  if(compassData.locationData.fixAge > 4000){                                    //GPS should update once per second, if data is more than 4 seconds old, fix was likely lost
    dataLine += "No Fix,";
  }
  else{
    dataLine += "Fix,";
  }
  
  dataLine += (String(compassData.T1,4) + "," +String(compassData.T2,4) + ",");     //Data string population
  dataLine += (String(compassData.PressurePSI,6) + "," + String(compassData.PressureATM,6) + ",");
  dataLine += (compassData.sensorHeatStatus + ",");
  dataLine += (",=," + OPCdata + ",");
  dataLine += (String(baroTemp,4) + "," + String(pressureAltitude,6));
  
  /////////////////////////////////////////////////////////////
  ///////////// Logging the data onto the SD card /////////////
  /////////////////////////////////////////////////////////////
  // All these functions are in the "SD.h" library, so for more information, go there

  openFlightlog();
  delay(100); 
  Flog.println(dataLine);                                                                   // Printing the data from A in the SD card
  closeFlightlog();
  
//  openFlightlogA();
//  delay(100); 
//  FlogA.println(data);                                                                    // Printing the data from A in the SD card
//  closeFlightlogA();
//
//  openFlightlogB();
//  delay(100); 
//  FlogB.println(data);                                                                    // Printing the data from B in the SD card
//  closeFlightlogB();

  ///////////////////////////////////////////////////////
  //////////// Printing the Data to Serial //////////////
  ///////////////////////////////////////////////////////
  // Go to "utility.ino" for more on this function
  
  printData();
}

void updatePressure(){

  // Read true temperature & Pressure
  baroTemp = baro.readTemperature();
  pressurePa = baro.readPressure();

  // Calculate altitude
  pressureAltitude = baro.getAltitude(pressurePa, seaLevelPressure)*FEET_PER_METER;
  pressureRelativeAltitude = baro.getAltitude(pressurePa, baroReferencePressure)*FEET_PER_METER; 
}

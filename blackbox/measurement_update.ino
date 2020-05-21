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
  // Temperature, Pressure, and Altitude (I'm pretty sure they mean OPC data instead of Altitude)
  // First, temperature
  
  adcVal1 = analogRead(THERMISTOR_A);                                   // All of these calculations have to do with the Steinhart-Hart equations and setting them up properly
  adcVal2 = analogRead(THERMISTOR_B);
  logR1 = log(((adcMax/adcVal1)-1)*R);
  logR2 = log(((adcMax/adcVal2)-1)*R);
  Tinv1 = A+B*logR2+C*logR1*logR1*logR1;
  Tinv2 = A+B*logR2+C*logR2*logR2*logR2;
  t1 = 1/Tinv1;                                                         // The final temperatures for both transistors
  t2 = 1/Tinv2;

  
  
  
  // Next, pressure
  pressureSensor = analogRead(HONEYWELL_PRESSURE);                      //Read the analog pin
  pressureSensorVoltage = pressureSensor * (5.0/8196);                  //Convert the analog number to voltage    //THESE NEED TO BE 3.3 INSTEAD OF 5.0!!!!!!!!!!
  pressurePSI = (pressureSensorVoltage - (0.1*5.0))/(4.0/15.0);         //Convert the voltage to PSI
  pressureATM = pressurePSI*PSI_TO_ATM;                                 //Convert PSI reading to ATM


  
  
  // Finally, OPC data
  OPCdata = SpsA.logUpdate();
  OPCdata += ",=," + SpsB.logUpdate();

  //////////////////////////////////////////////////////////////////////////////////
  ///////////// UPDATING THE SYSTEM DATA STRUCT (called "compassData") /////////////
  //////////////////////////////////////////////////////////////////////////////////
  // This is mainly for simplicity and means only calling these variables once in one data structure later on, which also saves some memory and logging time
  // The GPS update for the struct "locationData" comes from the main script "blackbox.ino" that's why it isn't shown above this

  compassData.flightTime = millis();
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
  compassData.sensorHeatStatus = sensorHeat_Status;
  compassData.T1 = t1;
  compassData.T2 = t2;
  compassData.PressureATM = pressureATM;
  compassData.PressurePSI = pressurePSI;
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
  
  //////////////////////////////////////////////////////////////////
  ////////// UPDATING THE DATA STRING (called "data") //////////////
  //////////////////////////////////////////////////////////////////
  
  data = "";
  data = flightTimeStr()+ "," + String(flightMinutes()) + "," + "," + String(compassData.locationData.latitude, 4) + "," + String(compassData.locationData.longitude, 4) + "," 
  + String(compassData.locationData.alt, 1) + ","
  + String(compassData.locationData.mm) + "/" + String(compassData.locationData.dd) + "/" + String(compassData.locationData.yyyy) + ","
  + String(compassData.locationData.hours) + ":" + String(compassData.locationData.minutes) + ":" + String(compassData.locationData.seconds) + ","
  + String(compassData.locationData.sats) + ",";
  
  if(compassData.locationData.fixAge > 4000){                                           //GPS should update once per second, if data is more than 2 seconds old, fix was likely lost
    data += "No Fix,";
  }
  else{
    data += "Fix,";
  }

  data += (String(compassData.T1,4) + "," +String(compassData.T2,4) + ",");     //Data string population
  data += (String(compassData.PressurePSI,6) + "," + String(compassData.PressureATM,6) + ",");
  data += (compassData.sensorHeatStatus + ",");
  data += (",=," + OPCdata);
  
  /////////////////////////////////////////////////////////////
  ///////////// Logging the data onto the SD card /////////////
  /////////////////////////////////////////////////////////////
  // All these functions are in the "SD.h" library, so for more information, go there
  
  openFlightlogA();
  delay(100); 
  FlogA.println(data);                                                                    // Printing the data from A in the SD card
  closeFlightlogA();

  openFlightlogB();
  delay(100); 
  FlogB.println(data);                                                                    // Printing the data from B in the SD card
  closeFlightlogB();

  ///////////////////////////////////////////////////////
  //////////// Printing the Data to Serial //////////////
  ///////////////////////////////////////////////////////
  // Go to "utility.ino" for more on this function
  
  printData();
}

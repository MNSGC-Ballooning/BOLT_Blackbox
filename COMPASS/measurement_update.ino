//function to handle both retrieval of data from sensors, as well as recording it on the SD card
void updateSensors() {

  updatePressure();

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



  // Finally, OPC data
  OPCdata = (String(SpsB.getFanStatus()) + "," + SpsB.logUpdate());

 

  //////////////////////////////////////////////////////////////////////////////////
  ///////////// UPDATING THE SYSTEM DATA STRUCT (called "compassData") /////////////
  //////////////////////////////////////////////////////////////////////////////////
  //Using a struct means that update functions are only called once, so the data logged locally and sent externally is universally consistent.

  compassData.flightTime = millis();

  compassData.sensorHeatStatus = sensorHeat_Status;
  compassData.spsB_data_abv.hits = SpsB.getTot();
  compassData.spsB_data_abv.fanStatus = SpsB.getFanStatus();
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
  dataLine = flightTimeStr()+ "," + String(flightMinutes()) + ",";
  
  dataLine += (String(compassData.T1,4) + "," +String(compassData.T2,4) + ",");     //Data string population
  dataLine += (String(compassData.PressurePSI,6) + "," + String(compassData.PressureATM,6) + ",");
  dataLine += (compassData.sensorHeatStatus + "," + String(baroTemp,4) + "," + String(pressureRelativeAltitude,6));
  dataLine += (",=," + OPCdata);

  
  /////////////////////////////////////////////////////////////
  ///////////// Logging the data onto the SD card /////////////
  /////////////////////////////////////////////////////////////
  // All these functions are in the "SD.h" library, so for more information, go there

  openFlightlog();
  delay(100); 
  Flog.println(dataLine);                                                                   // Printing the data from A in the SD card
  closeFlightlog();


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
  pressureRelativeAltitude = baro.getAltitude(pressurePa, baroReferencePressure)*FEET_PER_METER; 
}

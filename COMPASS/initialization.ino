////////////////////////////////////////////////////
////////// Initialize Data String //////////////////
////////////////////////////////////////////////////
void initData(){
//  Serial.begin(9600);
  DATA_SERIAL.begin(DATA_BAUD);                                         //Data Transfer
  

  String FHeader = "Flight Time, Minutes ,ATemp (C),BTemp (C),Pressure (PSI),Pressure (ATM),";
  FHeader += "Sensor Heater Status,baro Temp (C), baro Alt";
  FHeader += (",SPSB,B Fan," + SpsB.CSVHeader());

  pinMode(chipSelect, OUTPUT);                                         //initialize SD card
  
  if (!SD.begin(chipSelect)) {                                      //power LED will blink if no card is inserted
    Serial.println("No SD");
    SDcard = false;
  }
  SDcard = true;

  for (int i = 0; i < 100; i++) {                                      //Flight Log Name Cration
    Fname = String("FLog" + String(i / 10) + String(i % 10) + ".csv");
    if (!SD.exists(Fname.c_str())) {
      openFlightlog();
      break;
    }
  }
  
  Serial.println("Flight log created: " + Fname);
  
  Flog.println(FHeader);                                               //Set up Flight log format
  Serial.println("Flight log header added");                            

  closeFlightlog();
}

/////////////////////////////////////////////////
////////// Initialize Pressure //////////////////
/////////////////////////////////////////////////
void initPressure() {
  if(!baro.begin())
  {
    Serial.println(F("Could not find a valid MS5611 sensor, check wiring!"));
  }

  baroReferencePressure = baro.readPressure();                    // Get a reference pressure for relative altitude

  Serial.println(F("MS5611 barometer setup successful..."));

  updatePressure();
}

///////////////////////////////////////////////
////////// Initialize Relays //////////////////
///////////////////////////////////////////////
void initRelays(){
  sensorHeatRelay.init(false);                                          //Initialize relays
  
  sensorHeat_Status = "OFF";
}


////////////////////////////////////////////
////////// Initialize GPS //////////////////
////////////////////////////////////////////
void initOPCs() {                                                       //Sets up serial and initializes the OPCs
  SPSB_SERIAL.begin(SPS_BAUD);

  SpsB.initOPC();
  Serial.println("SPSB Initialized");
}

//Initializations
void initOLED(MicroOLED& named){                                        //Initializes the OLED Screen
  delay(100);
  Wire.begin();
  named.begin();                                                        //Initialize the OLED
  named.clear(ALL);                                                     //Clear the display's internal memory
  named.display();                                                      //Display what's in the buffer (splashscreen)
  delay(1000);                                                          //Delay 1000 ms
  named.clear(PAGE);                                                    //Clear the buffer.
  
  named.setFontType(1);
  named.clear(PAGE);
  named.setCursor(0,0);                                                 //Reset
  named.print("COMPASSSYSTEM ONLINE");
  named.display();
  delay(2000);
}

void initData(){
  Serial.begin(9600);
  DATA_SERIAL.begin(DATA_BAUD);                                         //Data Transfer
//  

  String FHeader = "Flight Time, Minutes, Lat, Long, Altitude (ft), Date, Hour:Min:Sec, Satellites,Fix,ATemp (C),BTemp (C),Pressure (PSI),Pressure (ATM),";
  FHeader += "Sensor Heater Status,";
  FHeader += "SPSA, " + SpsA.CSVHeader();
  FHeader += ",SPSB, " + SpsB.CSVHeader();

  pinMode(SD_A, OUTPUT);                                          //initialize SD card
  
  while (!sdA.begin(SD_A)) {                                       //power LED will blink if no card is inserted
    Serial.println("No SD A");
    SDcardA = false;
  }
  SDcardA = true;

  for (int i = 0; i < 100; i++) {                                       //Flight Log Name Cration
    FnameA = String("FLog" + String(i / 10) + String(i % 10) + ".csv");
    if (!sdA.exists(FnameA.c_str())) {
      openFlightlogA();
      break;
    }
  }
  
  Serial.println("Flight log A created: " + FnameA);
  FlogA.println(FHeader);                                                //Set up Flight log format
  Serial.println("Flight log header added");                            
  closeFlightlogA();

  pinMode(SD_B, OUTPUT);                                          //initialize SD card
  
  while (!sdB.begin(SD_B)) {                                       //power LED will blink if no card is inserted
    Serial.println("No SD B");
    SDcardB = false;
  }
  SDcardB = true;

  for (int i = 0; i < 100; i++) {                                       //Flight Log Name Cration
    FnameB = String("FLog" + String(i / 10) + String(i % 10) + ".csv");
    if (!sdB.exists(FnameB.c_str())) {
      openFlightlogB();
      break;
    }
  }
  
  Serial.println("Flight log B created: " + FnameB);
  FlogB.println(FHeader);                                                //Set up Flight log format
  Serial.println("Flight log header added");                            
  closeFlightlogB();


//  pinMode(chipSelect, OUTPUT);                                          //initialize SD card
//  
//  while (!SD.begin(chipSelect)) {                                       //power LED will blink if no card is inserted
//    Serial.println("No SD");
//    SDcard = false;
//  }
//  SDcard = true;
//
//  for (int i = 0; i < 100; i++) {                                       //Flight Log Name Cration
//    Fname = String("FLog" + String(i / 10) + String(i % 10) + ".csv");
//    if (!SD.exists(Fname.c_str())) {
//      openFlightlog();
//      break;
//    }
//  }
//  
//  Serial.println("Flight log created: " + Fname);
  
//  Flog.println(FHeader);                                                //Set up Flight log format
//  Serial.println("Flight log header added");                            
//
//  closeFlightlog();
}

void initGPS(){
  UBLOX_SERIAL.begin(UBLOX_BAUD);                                       //initiate GPS
  GPS.init();                                                           //Initiate GPS Data lines

  Serial.println("GPS begin");
  delay(50);
  if(GPS.setAirborne()){
    Serial.println("Airborne mode set!");
  }
  Serial.println("GPS configured");
}

void initTemp(){
  thermocoupleA.begin();
  thermocoupleB.begin();
  thermocoupleA.setThermocoupleType(MAX31856_TCTYPE_K);
  thermocoupleB.setThermocoupleType(MAX31856_TCTYPE_K);
}

void initRelays(){
  sensorHeatRelay.init(false);                                          //Initialize relays
  
  sensorHeat_Status = "OFF";
}

void initOPCs() {                                                       //Sets up serial and initializes the OPCs
  SPSB_SERIAL.begin(SPS_BAUD);
  SPSA_SERIAL.begin(SPS_BAUD);

  SpsA.initOPC();
  Serial.println("SPSA Initialized");
  SpsB.initOPC();
  Serial.println("SPSB Initialized");
}

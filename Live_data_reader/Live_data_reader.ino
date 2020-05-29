//Live Data Reader for the COMPASS Blackbox
//This system can be used in place of the main gondola to test the data stream.

#define DATA_SERIAL Serial3
#define DATA_BAUD 115200

struct spsData_abv{
  uint16_t hits;
  float numberCount[5];
};

struct inputPacket{
  uint8_t strt = 0, sysID = 0;
  uint16_t packetNum = 0;
  uint32_t relTime = millis();
  uint8_t hrs = 0, mins = 0, secs = 0;
  float lats, longs, alts, t1, t2, pressure;
  spsData_abv A,B;
  uint16_t checksum = 0;
  uint8_t stp = 0;
}inputData;

uint8_t counter = 0;
bool active = false;
uint8_t inputBytes[82] = {};
uint16_t checksumCalc = 0;

void setup() {
Serial.begin(DATA_BAUD);
DATA_SERIAL.begin(DATA_BAUD);

Serial.println("System initialized!");
}

void loop() {
  if (recieveData()) printData();
  //Serial.println(DATA_SERIAL.read(),HEX);
}


bool recieveData(){
  while (DATA_SERIAL.available()){
    inputBytes[counter] = DATA_SERIAL.read();
    Serial.println(inputBytes[counter], HEX);
    counter++;
  }
    if (inputBytes[0] == 0x42 && !active){
      Serial.println("SYSTEM ACTIVE");
      active = true;
      
    } else if (!active) {
      counter = 0;
      return false;
    }

    counter++;
    
    if (counter < 82){
      return false;
    }
//  } else {
//    counter = 0;
//    return false;
//  }

  checksumCalc = 0;
  
  for (unsigned short i = 0; i < 79; i++){
    checksumCalc += inputBytes[i];
  }

  if ((inputBytes[0] != 0x42)||(inputBytes[81] != 0x53)){
    Serial.println("Bad packet!");
    counter = 0;
    return false;
  }

  memcpy(&inputData, &inputBytes, 82);
  counter = 0;

  if (checksumCalc != inputData.checksum){
    Serial.println("Corrupt packet!");
    return false;
  }

  return true;
}

void printData(){
  Serial.println();
  Serial.println("Measurement Update");
  Serial.println("========================================================================================");
  Serial.println("         System Info");  
  Serial.print("system ID: ");
  Serial.println(inputData.sysID,HEX);
  Serial.print("Packet number: ");
  Serial.println(String(inputData.packetNum));
  Serial.println("             Time");
  Serial.print("Flight Time: ");
  Serial.println(inputData.relTime);
  Serial.print("Real Time: ");
  Serial.println((String(inputData.hrs) + ":" + String(inputData.mins) + ":" + String(inputData.secs)));
  Serial.println("------------------------------");
  Serial.println("             GPS");
  Serial.print("Latitude: ");
  Serial.println(String(inputData.lats));
  Serial.print("Longitude: ");
  Serial.println(String(inputData.longs));  
  Serial.print("Altitude: ");
  Serial.println(String(inputData.alts));
  Serial.println("------------------------------");
  Serial.println("          Temperature");
  Serial.println("   t1       t2"); 
  Serial.println((String(inputData.t1) + ", " +String(inputData.t2)));
  Serial.println("------------------------------");
  Serial.println("           Pressure");
  Serial.print("Pressure(PSI): ");
  Serial.println(String(inputData.pressure));
  Serial.println("------------------------------");
  Serial.println("             OPCs");
  Serial.println("SPS A: ");
  Serial.println(("hits: " + String(inputData.A.hits)));
  Serial.println(("number counts: " + String(inputData.A.numberCount[0]) + "," + String(inputData.A.numberCount[1]) + "," + String(inputData.A.numberCount[2]) + "," + String(inputData.A.numberCount[3]) + "," + String(inputData.A.numberCount[4])));
  Serial.println();
  Serial.println("SPS B: ");
  Serial.println(("hits: " + String(inputData.B.hits)));
  Serial.println(("number counts: " + String(inputData.B.numberCount[0]) + "," + String(inputData.B.numberCount[1]) + "," + String(inputData.B.numberCount[2]) + "," + String(inputData.B.numberCount[3]) + "," + String(inputData.B.numberCount[4])));
  Serial.println("========================================================================================");  
}

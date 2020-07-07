// GPS   
int getGPStime() 
{
  return (compassData.locationData.hours * 3600 + compassData.locationData.minutes * 60 + compassData.locationData.seconds);
}

void FixCheck(){                                                        //Check if gps fix is good
  if (compassData.locationData.fixAge < 4000) 
  {
    FixStatus = Fix;
  }
  else if(compassData.locationData.fixAge > 4000)
  {
    FixStatus = NoFix;
  }
  else
  {
    FixStatus = NoFix;
  }
}

String flightTimeStr() {                                                //Returns the flight time as a usable string for print statements  
  unsigned long t = compassData.flightTime / 1000;
  String fTime = "";
  fTime += (String(t / 3600) + ":");
  t %= 3600;
  fTime += String(t / 600);
  t %= 600;
  fTime += (String(t / 60) + ":");
  t %= 60;
  fTime += (String(t / 10) + String(t % 10));
  return fTime;
}

float flightMinutes() {                                                 //Return time in minutes
  float minutes = compassData.flightTime / 1000;
  minutes = minutes / 60;
  return minutes;
}

//void openFlightlogA() {                                                  //Open flight log
//  if (!FlightlogOpenA&&SDcardA) {
//    //add .c_str() next to Fname
//    FlogA = sdA.open(FnameA.c_str(), FILE_WRITE);
//    FlightlogOpenA = true;
//  }
//}
//void closeFlightlogA() {                                                 //Close flight log
//  if (FlightlogOpenA&&SDcardA) {
//    FlogA.close();
//    FlightlogOpenA = false;
//  }
//}
//
//void openFlightlogB() {                                                  //Open flight log
//  if (!FlightlogOpenB&&SDcardB) {
//    //add .c_str() next to Fname
//    FlogB = sdB.open(FnameB.c_str(), FILE_WRITE);
//    FlightlogOpenB = true;
//  }
//}
//void closeFlightlogB() {                                                 //Close flight log
//  if (FlightlogOpenB&&SDcardB) {
//    FlogB.close();
//    FlightlogOpenB = false;
//  }
//}

void openFlightlog() {                                                  //Open flight log
  if (!FlightlogOpen&&SDcard) {
    //add .c_str() next to Fname
    Flog = SD.open(Fname.c_str(), FILE_WRITE);
    FlightlogOpen = true;
  }
}
void closeFlightlog() {                                                 //Close flight log
  if (FlightlogOpen&&SDcard) {
    Flog.close();
    FlightlogOpen = false;
  }
}

void printData(){                                                       // Printing the data in a neat and orderly fashion. This is for debugging purposes. All of these will be logged onto the SD card
  Serial.println();
  Serial.println("Measurement Update");
  Serial.println("========================================================================================");
  Serial.println("             Time");
  Serial.print("Flight Time String: ");
  Serial.println(flightTimeStr());
  Serial.print("Flight Minutes: ");
  Serial.println(String(flightMinutes()));
  Serial.println("------------------------------");
  Serial.println("             GPS");
  Serial.print("Latitude: ");
  Serial.println(String(compassData.locationData.latitude, 4));
  Serial.print("Longitude: ");
  Serial.println(String(compassData.locationData.longitude, 4));  
  Serial.print("Altitude: ");
  Serial.println(String(compassData.locationData.alt, 1));
  Serial.print("Date and Time: ");
  Serial.println(String(compassData.locationData.mm) + "/" + String(compassData.locationData.dd) + "/" + String(compassData.locationData.yyyy) + " " + String(compassData.locationData.hours) + ":" + String(compassData.locationData.minutes) + ":" + String(compassData.locationData.seconds));
  Serial.print("Satellites and Fix Age: ");
  Serial.println((String(compassData.locationData.sats) + ", " + String(compassData.locationData.fixAge)));
  Serial.println("------------------------------");
  Serial.println("          Temperature");
  Serial.println("   t1       t2"); 
  Serial.println((String(compassData.T1,4) + ", " +String(compassData.T2,4)));
  Serial.println("------------------------------");
  Serial.println("           Pressure");
  Serial.print("Pressure(PSI): ");
  Serial.println(String(compassData.PressurePSI,6));
  Serial.print("Pressure(ATM): ");
  Serial.println(String(compassData.PressureATM,6));
  Serial.println("------------------------------");
  Serial.println("       System Statuses");
  Serial.print("Sensor Heater Relay: ");
  Serial.println(compassData.sensorHeatStatus);
  Serial.println("------------------------------");
  Serial.println("             OPCs");
  Serial.print("SPS A: ");
      if (SpsA.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      }
  Serial.print("SPS B: ");
      if (SpsB.getLogQuality()){                                       //OPC Statuses
        Serial.println("Good Log");
      } else {
        Serial.println("Bad Log");
      }      
  Serial.println("Raw data:");          
  Serial.println(OPCdata);
  Serial.println("========================================================================================");  
}

//Function to generate the output packet
void sendDataPacket(){
  byte outputBytes[86];                              //Define output byte arrays
  byte checksumBytes[2];

  outputData.checksum = 0;                           //Reset the checksum
  
  outputData.packetNum++;                            //Did someone say - "update the system with data points equivalent to what was just logged locally?" YES! I said that just now 6/12/1:52:27
  outputData.relTime = compassData.flightTime;
  outputData.hrs = compassData.locationData.hours;
  outputData.mins = compassData.locationData.minutes;
  outputData.secs = compassData.locationData.seconds;
  outputData.lats = compassData.locationData.latitude;
  outputData.longs = compassData.locationData.longitude;
  outputData.alts = compassData.locationData.alt;

//  outputData.hrs = 0;                             //Replace the actual GPS data with zeros if Florida requests no GPS data
//  outputData.mins = 0;
//  outputData.secs = 0;
//  outputData.lats = 0;
//  outputData.longs = 0;
//  outputData.alts = 0;
  
  outputData.t1 = compassData.T1;                   // Setting the output packet data structure. 
  outputData.t2 = compassData.T2;
  outputData.pressureMS = compassData.PressurePSI;
  outputData.pressureANA = compassData.PressureAnalogPSI;
  outputData.A.hits = compassData.spsA_data_abv.hits;
  outputData.A.numberCount[0] = compassData.spsA_data_abv.numberCount[0];
  outputData.A.numberCount[1] = compassData.spsA_data_abv.numberCount[1];
  outputData.A.numberCount[2] = compassData.spsA_data_abv.numberCount[2];
  outputData.A.numberCount[3] = compassData.spsA_data_abv.numberCount[3];
  outputData.A.numberCount[4] = compassData.spsA_data_abv.numberCount[4];
  outputData.B.hits = compassData.spsB_data_abv.hits;
  outputData.B.numberCount[0] = compassData.spsB_data_abv.numberCount[0];
  outputData.B.numberCount[1] = compassData.spsB_data_abv.numberCount[1];
  outputData.B.numberCount[2] = compassData.spsB_data_abv.numberCount[2];
  outputData.B.numberCount[3] = compassData.spsB_data_abv.numberCount[3];
  outputData.B.numberCount[4] = compassData.spsB_data_abv.numberCount[4];
    
  memcpy(&outputBytes, &outputData, 83);               //Pass the packet to the output array as bytes
  
  for (unsigned short i = 0; i < 83; i++){        //Calculate the checksum this literally adds up the numbers contained within the bytes and checks if they are the same
    outputData.checksum += outputBytes[i];
  }

  memcpy(&checksumBytes, &outputData.checksum, 2);    //Pass the checksum bytes to a staging array
  
  outputBytes[83] = checksumBytes[0];             //Add the checksum to the output
  outputBytes[84] = checksumBytes[1];
  
  outputBytes[85] = outputData.stp;                         //Add the stop byte
      
  Serial5.write(outputBytes,86);                     //Send the data
}

//Function to generate the output packet

void sendDataPacket(){
  byte outputBytes[82];
  byte checksumBytes[2];

  outputData.checksum = 0;
  
  outputData.packetNum++; 
  outputData.relTime = millis();
  outputData.hrs = compassData.locationData.hours;
  outputData.mins = compassData.locationData.minutes;
  outputData.secs = compassData.locationData.seconds;
  outputData.lats = compassData.locationData.latitude;
  outputData.longs = compassData.locationData.longitude;
  outputData.alts = compassData.locationData.alt;
  outputData.t1 = compassData.T1;
  outputData.t2 = compassData.T2;
  outputData.pressure = compassData.PressurePSI;
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
    
  memcpy(&outputBytes, &outputData, 79);               //Pass the packet to the output array as bytes
  
  for (unsigned short i = 0; i < 79; i++){        //Calculate the checksum
    outputData.checksum += outputBytes[i];
  }

   memcpy(&checksumBytes, &outputData.checksum, 2);    //Pass the checksum bytes to a staging array
  
  outputBytes[79] = checksumBytes[0];             //Add the checksum to the output
  outputBytes[80] = checksumBytes[1];
  
  outputBytes[81] = outputData.stp;                         //Add the stop byte
  
  for (unsigned short i = 0; i < 82; i++){        //Send the data
    DATA_SERIAL.write(outputBytes[i]);
  }
}

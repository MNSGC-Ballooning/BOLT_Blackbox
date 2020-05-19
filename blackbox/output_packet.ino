//Function to generate the output packet

void sendDataPacket(){
  byte outputBytes[82];
  byte checksumBytes[2];

  outputData.checksum = 0;
  
  outputData.packetNum++; 
  outputData.relTime = millis();
  outputData.hrs = GPS.getHour();
  outputData.mins = GPS.getMinute();
  outputData.secs = GPS.getSecond();
  outputData.lats = GPS.getLat();
  outputData.longs = GPS.getLon();
  outputData.alts = GPS.getAlt_feet();
  outputData.t1 = t1;
  outputData.t2 = t2;
  outputData.pressure = PressurePSI;
  outputData.A.hits = SpsA.getTot();
  outputData.A.numberCount[0] = SpsA.SPSdata.nums[0];
  outputData.A.numberCount[1] = SpsA.SPSdata.nums[1];
  outputData.A.numberCount[2] = SpsA.SPSdata.nums[2];
  outputData.A.numberCount[3] = SpsA.SPSdata.nums[3];
  outputData.A.numberCount[4] = SpsA.SPSdata.nums[4];
  outputData.B.hits = SpsB.getTot();
  outputData.B.numberCount[0] = SpsB.SPSdata.nums[0];
  outputData.B.numberCount[1] = SpsB.SPSdata.nums[1];
  outputData.B.numberCount[2] = SpsB.SPSdata.nums[2];
  outputData.B.numberCount[3] = SpsB.SPSdata.nums[3];
  outputData.B.numberCount[4] = SpsB.SPSdata.nums[4];
    
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

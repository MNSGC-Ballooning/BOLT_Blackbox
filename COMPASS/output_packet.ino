//Function to generate the output packet
void sendDataPacket(){
  byte outputBytes[PACKET];                              //Define output byte arrays

  outputData.checksum = 0;                           //Reset the checksum
  
  outputData.packetNum++;                            //Did someone say - "update the system with data points equivalent to what was just logged locally?" YES! I said that just now 6/12/1:52:27
  outputData.relTime = compassData.flightTime;
  
  outputData.t1 = compassData.T1;                   // Setting the output packet data structure. 
  outputData.t2 = compassData.T2;
  outputData.pressureMS = compassData.PressurePSI;
  outputData.B.hits = compassData.spsB_data_abv.hits;
  outputData.B.numberCount[0] = compassData.spsB_data_abv.numberCount[0];
  outputData.B.numberCount[1] = compassData.spsB_data_abv.numberCount[1];
  outputData.B.numberCount[2] = compassData.spsB_data_abv.numberCount[2];
  outputData.B.numberCount[3] = compassData.spsB_data_abv.numberCount[3];
  outputData.B.numberCount[4] = compassData.spsB_data_abv.numberCount[4];
    
  memcpy(&outputBytes, &outputData, 43);               //Pass the packet to the output array as bytes
  
  for (unsigned short i = 0; i < 43; i++){        //Calculate the checksum this literally adds up the numbers contained within the bytes and checks if they are the same
    outputData.checksum += outputBytes[i];
  }

  memcpy(&outputBytes[43], &outputData.checksum, 2);    //Pass the checksum bytes to a staging array
  
  
  outputBytes[45] = outputData.stp;                         //Add the stop byte
      
  Serial5.write(outputBytes,PACKET);                     //Send the data

  for (unsigned short i = 0; i < PACKET; i++){
    Serial.print(outputBytes[i],HEX);                     //Send the data
  }
}

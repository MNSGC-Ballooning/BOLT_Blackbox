//============================================================================================================================================
//MURI SPS Blackbox 
//Compact Optical Measurement of Particles via an Autonomous SPS System (COMPASS)
//Written by UMN MURI Spring/Summer 2020
//============================================================================================================================================
//
//Version Description: SPS standalone configuration. Takes in 5V and outputs a serial string as noted in the BOLT campaign documentation.
//
//=============================================================================================================================================
//=============================================================================================================================================
//           ____                                    
//          / ___|___  _ __ ___  _ __   __ _ ___ ___ 
//         | |   / _ \| '_ ` _ \| '_ \ / _` / __/ __|
//         | |__| (_) | | | | | | |_) | (_| \__ \__ \
//          \____\___/|_| |_| |_| .__/ \__,_|___/___/
//                              |_|                                                                                                                                                                                                                                                                                            
//=============================================================================================================================================
//=============================================================================================================================================

//System Preferences
#define SYSTEM_ID 0x01

/*  Teensy 3.5/3.6 pin connections:
     ------------------------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used         
                                  | Communication: (Pins used) (RX/TX on Teensy for UART)
     UBlox Neo m8n                | UART 1 (0,1)          
     SPS30 A                      | UART 2 (9,10)
     SPS30 B                      | UART 3 (7,8)
     Data Stream                  | UART 5 (34,33)
     Thermistor A                 | Analog (22/A8)
     Thermistor B                 | Analog (23/A9)
     SD A                         | SPI 0 (11,12,13,20)
     Pressure sensor              | Analog (21/A7)
     OLED                         | I2C 0 (18,19)
     OPC Heater                   | (35,36)
     LED                          | (5)
     
     -------------------------------------------------------------------------------------------------------------------------------------------
*/
/////////////////////////////
//////////Libraries//////////
/////////////////////////////
#include <SPI.h>                                                        //SPI Library for R1
#include <SD.h>                                                         //SD Library for logging
#include <UbloxGPS.h>                                                   //GPS Library
#include <Arduino.h>                                                    //Arduino kit
#include <SPS.h>                                                        //Library for SPS
#include <SFE_MicroOLED.h>                                              //Library for OLED
#include <LatchRelay.h>                                                 //Heater relay
#include <Wire.h>                                                       //I2C library if the I2C mode in SPS is disabled
#include <MS5611.h>

////////////////////////////////////
//////////Pin Definitions///////////
////////////////////////////////////
#define SENSOR_HEATER_ON 35                                             //Latching Relay pins for heaters
#define SENSOR_HEATER_OFF 36
#define HONEYWELL_PRESSURE A7                                          //Analog Honeywell Pressure Sensor
#define THERMISTOR_A A8                                                 //Analog pins for the thermistors
#define THERMISTOR_B A9
//#define SD_A 9
//#define SD_B 10
#define UBLOX_SERIAL Serial1                                            //Serial Pins
#define SPSA_SERIAL Serial2
#define SPSB_SERIAL Serial3                                           
#define DATA_SERIAL Serial5                                         

#define PIN_RESET 17                                                    //The library assumes a reset pin is necessary. The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is not being used

//////////////////////////////
//////////Constants///////////
//////////////////////////////
//Bauds
#define DATA_BAUD 115200                                                //Baud rate for the data string
#define SPS_BAUD 115200                                                 //Baud rate for the SPS30 sensors
#define GPS_BAUD 9600                                                   //Baud rate for the GPS

//Data Transfer
#define BEGIN 0x42                                                      //Start bit for the data transfer
#define STOP  0x53                                                      //Stop bit for the data transfer

//Values
#define MINUTES_TO_MILLIS 60000                                         //MATH TIME
#define PSI_TO_ATM  0.068046                                            //Pounds per square inch to atmospheres   
#define C2K 273.15                                                      //Degrees Celsius to Kelvin
#define DC_JUMPER 1                                                     //The DC_JUMPER is the I2C Address Select jumper. Set to 1 if the jumper is open (Default), or set to 0 if it's closed.
#define NoFix 0x00                                                      // If the GPS doesn't get a fix, it should return this data bit and that's what NoFix is for
#define Fix 0x01                                                        // Same thing as above, but this is if it gets a fix
#define ADC_MAX 8196.0                                                    // The maximum adc value given to the thermistor, should be 8196 for a teensy and 1024 for an Arduino
#define CONST_A 0.001125308852122                                       // A, B, and C are constants used for a 10k resistor and 10k thermistor for the steinhart-hart equation
#define CONST_B 0.000234711863267                                       // NOTE: These values change when the thermistor and/or resistor change value, so if that happens, more research needs to be done on those constants
#define CONST_C 0.000000085663516                                       
#define CONST_R 10000                                                   // 10k Ω resistor 
#define FEET_PER_METER 3.28084                                          // feet per meter
#define VSUP 3.3
#define PMAX 15.0
#define PMIN 0.0

//Control
#define HIGH_TEMP 16                                                    //Thermal control, this is the high temperature in celcius
#define LOW_TEMP 10                                                     // Low temperature in celcius
#define LOG_RATE 1400                                                   // The rate at which we send data over.This is 1.4 seconds, or 1,400 milliseconds
#define SCREEN_UPDATE_RATE 2000                                         // The update rate of the screen, which is 2 seconds, or 2,000 milliseconds

////////////////////////
//////////Data//////////
////////////////////////
struct gpsData{                                                         // Data structure for the data we get from the GPS. Many of these are self explainatory, but dd is day, mm is month, and yyyy is year
  uint16_t hours,minutes,seconds;                                       // fixAge is the time since last that the gps got a fix. sats is the number of satellites that the gps is picking up on.
  uint8_t dd, mm;
  uint16_t yyyy;
  unsigned int fixAge;
  uint8_t sats;
  float latitude, longitude, alt;
};

struct spsData_abv{                                                     // Data structure for the SPS data we receive. It contains the number of hits and a number count of 5
  uint16_t hits;
  float numberCount[5];
};

struct systemData{                                                      // Data structure fot the entire system. This includes a data structure for the SPS and the GPS, so they are nested structures
  unsigned long flightTime;                                             // This is done to make the code more efficient and easier to read. ALL of our data lies under the compassData data structure
  gpsData locationData;                                                 // So if we ever need any data, we know that it is all contained within this one structure and don't have to worry about where to go
  float T1 = -127.00, T2 = -127.00;
  float PressureATM, PressurePSI, PressureAnalogPSI, PressureAnalogATM;
  spsData_abv spsA_data_abv,spsB_data_abv;
  bool sensorHeatStatus;
}compassData;

struct outputPacket{                                                    // Data structure for the output packet of the system. This is the raw data being sent to the SD card. Everything from compassData
  uint8_t strt = BEGIN, sysID = SYSTEM_ID;                              // is copied over and transferred to output packet in a systematic way
  uint16_t packetNum = 0;
  uint32_t relTime = millis();
  uint8_t hrs = 0, mins = 0, secs = 0;
  float lats, longs, alts, t1, t2, pressureANA, pressureMS;
  spsData_abv A,B;
  uint16_t checksum = 0;
  uint8_t stp = STOP;
}outputData;



///////////////////////////////////
//////////Data Management//////////
///////////////////////////////////
//Data Log
unsigned long logCounter = 0;
String data;

//SDClass sdA;
//File FlogA;                                                              //Variables needed to establish the flight log
//String FnameA = "";
//boolean SDcardA = true;
//static boolean FlightlogOpenA = false;                                   //SD for Flight Computer
//
//SDClass sdB;
//File FlogB;                                                              //Variables needed to establish the flight log
//String FnameB = "";
//boolean SDcardB = true;
//static boolean FlightlogOpenB = false;                                   //SD for Flight Computer

SDClass SD;
File Flog;                                                                 //Variables needed to establish the flight log
static String dataLine;
String Fname = "";
boolean SDcard = true;
static boolean FlightlogOpen = false;                                      //SD for Flight Computer
const int chipSelect = BUILTIN_SDCARD; 

////////////////////////////////////////////////
//////////Environment Sensor Variables//////////
////////////////////////////////////////////////

// active heating variables
float sensTemp;
bool coldSensor = false;
LatchRelay sensorHeatRelay(SENSOR_HEATER_ON,SENSOR_HEATER_OFF);            //Declare latching relay objects and related logging variables
String sensorHeat_Status = "";

//Honeywell Pressure Sensor
//float pressureSensor;                                                    //Analog number given by sensor
//float pressureSensorVoltage;                                             //Voltage calculated from analog number
//float pressurePSI;                                                       //PSI calculated from voltage
//float pressureATM;                                                       //ATM calculated from PSI
float PPSI = 0;                                                     //PSI calculated from voltage
float PATM = 0;  
float Vout = 0;

// MS5611 Pressure Sensor Variables
MS5611 baro;
float seaLevelPressure;                                                    // in Pascals
float baroReferencePressure;                                               // some fun pressure/temperature readings below 
float baroTemp;                                                            // non-"raw" readings are in Pa for pressure, C for temp, meters for altitude
unsigned int pressurePa;
float pressureAltitude;
float pressureRelativeAltitude;
boolean baroCalibrated = false;                                            // inidicates if the barometer has been calibrated or not

//GPS
UbloxGPS GPS(&UBLOX_SERIAL);                                               // Setting the serial port for the GPS and initializing the values to a false no fix state
uint8_t FixStatus= NoFix;
bool gpsConnected = false;

////////////////////////
//////////OPCs//////////
////////////////////////
SPS SpsA(&SPSA_SERIAL);                                                    // Serial ports for the SPS A and B
SPS SpsB(&SPSB_SERIAL);   
String OPCdata = "";

////////////////////////////////////////////////
//////////MicroOLED Object Declaration//////////
////////////////////////////////////////////////
MicroOLED oled(PIN_RESET, DC_JUMPER);                                  //Object I2C declaration
bool finalMessage[2] = {false,false};
unsigned short screen = 0;
unsigned long oledTime = 0;
unsigned long screenUpdateTimer = 0;

void setup() {
  analogReadResolution(13);                                            // Since this is a Teensy we are using, the read bit resolution can be at a max of 13. So, we want the best resoliution possible. This is why the thermistors have a higher mac adc value than the Arduino
  SPI.begin();                                                         // Beginning the SPI and wire libraries and the serial monitor
  Wire.begin();
  Serial.begin(9600);
  Serial5.begin(115200);

  Serial.println("Beginning Initialization cycle!");
  initOLED(oled);                                                      //Initialize OLED Screen
  Serial.println("OLED init!");
  
  initData();                                                          //Initialize SD
  oledPrintNew(oled, "DatInit");
  Serial.println("Data init!");
  
  initGPS();                                                           //Initialize GPS
  oledPrintAdd(oled, "GPSInit");
  Serial.println("GPS init!");
  delay(1000);

  initPressure();
  oledPrintAdd(oled, "PrsInit");
  Serial.println("Pressure init!");

  initRelays();                                                        //Initialize Relays
  oledPrintNew(oled, "RlyInit");
  Serial.println("Actuator init!");

  initOPCs();                                                          //Initialize OPCs
  oledPrintAdd(oled, "OPCInit");
  Serial.println("OPC init!");
  delay(1000);
  
  Serial.println("Setup Complete");
  oledPrintNew(oled, " Setup Success");
}

void loop() {
  GPS.update();                                                        //Update GPS and plantower on private loops
  
  if (millis() - logCounter >= LOG_RATE) {
      logCounter = millis();
      
      updateSensors();                                                  //Updates and logs all sensor data
      sendDataPacket();                                                 //Output the data packet
      actHeat();                                                        //Controls active heating
      oledUpdate();                                                     //Update screen
  }   
}

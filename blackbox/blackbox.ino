//============================================================================================================================================
//MURI SPS Blackbox 
//Calculator of Optically Measured Particles with an Autonomous SPS System (COMPASS)
//Written by Nathan Pharis - phari009 Spring 2020
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

     UBlox Neo m8n                | UART 1 (0,1)          
     SPS30 A                      | UART 4 (31,32)
     SPS30 B                      | UART 3 (7,8)
     Data Stream                  | UART 5 (33,34)
     Thermistor A                 | A9
     Thermistor B                 | A10
     SD A                         | SPI 0 (11,12,13,9)
     SD B                         | SPI 0 (11,12,13,10)
     Pressure sensor              | I2C 0 (18,19)
     OLED                         | I2C 0 (18,19)
     OPC Heater                   | (35,36)
     
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
#include <Adafruit_MAX31856.h>                                          //Adafruit Library
#include <LatchRelay.h>                                                 //Heater relay
#include <Wire.h>                                                       //I2C library if the I2C mode in SPS is disabled

////////////////////////////////////
//////////Pin Definitions///////////
////////////////////////////////////
#define SENSOR_HEATER_ON 35                                             //Latching Relay pins for heaters
#define SENSOR_HEATER_OFF 36
#define HONEYWELL_PRESSURE A11                                          //Analog Honeywell Pressure Sensor
#define THERMISTOR_A A18                                                //Chip Select pin for SPI for the thermocouples
#define THERMISTOR_B A9
//#define SD_A 9
//#define SD_B 10
#define UBLOX_SERIAL Serial1                                            //Serial Pins
#define SPSA_SERIAL Serial4
#define SPSB_SERIAL Serial3                                           
#define DATA_SERIAL Serial5                                         
#define PIN_RESET 17                                                    //The library assumes a reset pin is necessary. The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is not being used

//////////////////////////////
//////////Constants///////////
//////////////////////////////
//Bauds
#define DATA_BAUD 115200
#define SPS_BAUD 115200
#define UBLOX_BAUD 9600

//Data Transfer
#define BEGIN 0x42
#define STOP  0x53

//Values
#define MINUTES_TO_MILLIS 60000                                         //MATH TIME
#define PSI_TO_ATM  0.068046                                            //Pounds per square inch to atmospheres   
#define C2K 273.15                                                      //Degrees Celsius to Kelvin
#define DC_JUMPER 1                                                     //The DC_JUMPER is the I2C Address Select jumper. Set to 1 if the jumper is open (Default), or set to 0 if it's closed.
#define NoFix 0x00
#define Fix 0x01
#define ADC_MAX 8196                                                    // The maximum adc value given to the thermistor, should be 8196 for a teensy and 1024 for an Arduino
#define CONST_A 0.001125308852122
#define CONST_B 0.000234711863267                                       // A, B, and C are constants used for a 10k resistor and 10k thermistor for the steinhart-hart equation
#define CONST_C 0.000000085663516                                       // NOTE: These values change when the thermistor and/or resistor change value, so if that happens, more research needs to be done on those constants
#define CONST_R 10000                                                   // 10k Ω resistor

//Control
#define HIGH_TEMP 16                                                    //Thermal control
#define LOW_TEMP 10
#define LOG_RATE 1000
#define SCREEN_UPDATE_RATE 2000

////////////////////////
//////////Data//////////
////////////////////////
struct gpsData{
  uint16_t hours,minutes,seconds;
  uint8_t dd, mm;
  uint16_t yyyy;
  unsigned int fixAge;
  uint8_t sats;
  float latitude, longitude, alt;
};

struct spsData_abv{
  uint16_t hits;
  float numberCount[5];
};

struct systemData{
  unsigned long flightTime;
  gpsData locationData;
  float T1 = -127.00, T2 = -127.00;
  float PressureATM, PressurePSI;
  spsData_abv spsA_data_abv,spsB_data_abv;
  bool sensorHeatStatus;
}compassData;

struct outputPacket{
  uint8_t strt = BEGIN, sysID = SYSTEM_ID;
  uint16_t packetNum = 0;
  uint32_t relTime = millis();
  uint8_t hrs = 0, mins = 0, secs = 0;
  float lats, longs, alts, t1, t2, pressure;
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
//File FlogA;                                                             //Variables needed to establish the flight log
//String FnameA = "";
//boolean SDcardA = true;
//static boolean FlightlogOpenA = false;                                   //SD for Flight Computer
//
//SDClass sdB;
//File FlogB;                                                             //Variables needed to establish the flight log
//String FnameB = "";
//boolean SDcardB = true;
//static boolean FlightlogOpenB = false;                                   //SD for Flight Computer

SDClass SD;
File Flog;                                                             //Variables needed to establish the flight log
static String dataLine;
String Fname = "";
boolean SDcard = true;
static boolean FlightlogOpen = false;                                   //SD for Flight Computer
const int chipSelect = BUILTIN_SDCARD; 

////////////////////////////////////////////////
//////////Environment Sensor Variables//////////
////////////////////////////////////////////////

//Thermistor measurements
float t1 = -127.00;                                                    //Temperature initialization values
float t2 = -127.00;
float Tinv1;                                                           // Intermediate temp values needed to calculate the actual tempurature
float Tinv2;
float adcVal1;
float adcVal2;
float logR1;
float logR2;

// active heating variables
float sensTemp;
bool coldSensor = false;
LatchRelay sensorHeatRelay(SENSOR_HEATER_ON,SENSOR_HEATER_OFF);        //Declare latching relay objects and related logging variables
String sensorHeat_Status = "";

//Honeywell Pressure Sensor
//float pressureSensor;                                                  //Analog number given by sensor
//float pressureSensorVoltage;                                           //Voltage calculated from analog number
//float pressurePSI;                                                     //PSI calculated from voltage
//float pressureATM;                                                     //ATM calculated from PSI

//GPS
UbloxGPS GPS(&UBLOX_SERIAL);
uint8_t FixStatus= NoFix;

////////////////////////
//////////OPCs//////////
////////////////////////
SPS SpsA(&SPSA_SERIAL);  
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
  analogReadResolution(13);
  SPI.begin();
  Wire.begin();
  Serial.begin(9600);

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

  initRelays();                                                        //Initialize Relays
  oledPrintAdd(oled, "RlyInit");
  Serial.println("Actuator init!");

  initOPCs();                                                          //Initialize OPCs
  oledPrintNew(oled, "OPCInit");
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

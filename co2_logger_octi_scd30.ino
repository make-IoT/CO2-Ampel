#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <SparkFun_SCD30_Arduino_Library.h>
#include <ESP8266WiFi.h>

// Set the pins used
#define cardSelect 15

File logfile;

SCD30 airSensorSCD30; // Objekt SDC30 Umweltsensor

// blink out an error code
void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}

// This line is not needed if you have Adafruit SAMD board package 1.6.2+
//   #define Serial SerialUSB

void setup() {
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  Serial.println("\r\nCO2-Sensor logger test");

  WiFi.forceSleepBegin(); // Wifi off
  Wire.begin(); // ---- Initialisiere den I2C-Bus 

  if (Wire.status() != I2C_OK) Serial.println("Something wrong with I2C");

  if (airSensorSCD30.begin() == false) {
    Serial.println("The SCD30 did not respond. Please check wiring."); 
    while(1) {
      yield(); 
      delay(1);
    } 
  }

  airSensorSCD30.setAutoSelfCalibration(false); // Sensirion no auto calibration
  airSensorSCD30.setMeasurementInterval(2);     // CO2-Messung alle 2 s
  Wire.setClock(100000L);            // 100 kHz SCD30 
  Wire.setClockStretchLimit(200000L);// CO2-SCD30

  // see if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
    error(2);
  }
  char filename[15];
  strcpy(filename, "/SENSOR00.TXT");
  for (uint8_t i = 0; i < 100; i++) {
    filename[7] = '0' + i/10;
    filename[8] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
    error(3);
  }
  Serial.print("Writing to "); 
  Serial.println(filename);

  pinMode(0, OUTPUT);
  Serial.println("Ready!");
}

uint8_t i=0;
void loop() {
  digitalWrite(0, HIGH);
  logfile.print("CO2 = "); logfile.println(airSensorSCD30.getCO2());
  Serial.print("CO2 = "); Serial.println(airSensorSCD30.getCO2());
  delay(500);
  digitalWrite(0, LOW);
  logfile.flush();  // 30mA instead of 10mA in avg ... 
  delay(5000);
}

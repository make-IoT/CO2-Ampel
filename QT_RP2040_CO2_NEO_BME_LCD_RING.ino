/* Wire - check pins_arduino.h for QT RP2040 /opt/arduino-1.8.13/portable/packages/rp2040/hardware/rp2040/1.2.1/variants/adafruitfeather
#define PIN_WIRE0_SDA  (24u) 
#define PIN_WIRE0_SCL  (25u) 
#define PIN_WIRE1_SDA  (22u)
#define PIN_WIRE1_SCL  (23u)
*/

#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <SerLCD.h> 

SensirionI2CScd4x scd4x;
Adafruit_BME680 bme; // I2C

//either LCD or RING not both!
//#define LCD 
//#define RING
#define SEALEVELPRESSURE_HPA (1013.25)

#ifdef LCD
SerLCD lcd; // Initialize the library with default I2C address 0x72
#endif

#ifdef RING
//Adafruit_NeoPixel WSpixels = Adafruit_NeoPixel((10<24)?10:24,23,NEO_GRB + NEO_KHZ800); //10 Bar
Adafruit_NeoPixel WSpixels = Adafruit_NeoPixel((20<24)?20:24,23,NEO_GRB + NEO_KHZ800); //20 Ring

//--------- Neopixel Messanzeige (Gauge)
void WSGauge(float val, float limit1, float limit2, float delta, int seg, int dir){
  int bright = 32;
  float current = 0;
  int i;
  val = round(val/delta)*delta; // Runden der Anzeige auf Delta-Schritte 
  for (int k=0;k<=(seg-1);k++) { // alle Pixel
    current = (k+1)*delta;
    if (dir==1) i=k;
    else {
      i=seg-1-k; 
      if (i == seg-1) i=0; 
      else i=i+1;
    } // clockwise or opposite
    if ((val>=current) && (val < limit1)) // gruen
      WSpixels.setPixelColor(i,0,bright,0);
    else if ((val>=current) && (val <= limit2)) // gelb
      WSpixels.setPixelColor(i,bright/2,bright/2,0);
    else if ((val >= current) && (val > limit2)) // rot
      WSpixels.setPixelColor(i,bright,0,0);
    else
      WSpixels.setPixelColor(i,0,0,0);
  }
  WSpixels.show(); // Anzeige
}
#else
Adafruit_NeoPixel pixels(1, 3, NEO_GRB + NEO_KHZ800);
#endif

void printUint16Hex(uint16_t value) {
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
    Serial.print("Serial: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Wire.begin();

    #ifdef LCD
      Wire1.begin();
      lcd.begin(Wire1); //Set up the LCD for I2C communicatiom
      //lcd.setBacklight(255, 255, 255); //Set backlight to bright white
      lcd.setContrast(5); //Set contrast. Lower to 0 for higher contrast.
      lcd.disableSystemMessages();
      lcd.clear(); //Clear the display - this moves the cursor to home position as well
      lcd.setCursor (0,0);
      lcd.print("www.co2ampel.org");
      lcd.setCursor (0,1);
      lcd.print("Version 1.0");
      delay(1000);
      lcd.setBacklight(0, 0, 0); //black is off
    #endif

    #ifdef RING
      WSpixels.begin();//-------------- Initialisierung Neopixel
      WSpixels.show();  
    #else
      pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
      // Test RGB
      pixels.setPixelColor(0,0,30,0,0);
      pixels.show();
      delay(500);
      pixels.setPixelColor(0,30,30,0,0);
      pixels.show();
      delay(500);
      pixels.setPixelColor(0,40,0,0,0);
      pixels.show();
      delay(500);
      pixels.setPixelColor(0,0,0,0,0); // alle aus
      pixels.show(); 
    #endif

    uint16_t error;
    char errorMessage[256];

    scd4x.begin(Wire);

    // stop potentially previously started measurement
    error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    uint16_t serial0;
    uint16_t serial1;
    uint16_t serial2;
    error = scd4x.getSerialNumber(serial0, serial1, serial2);
    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        printSerialNumber(serial0, serial1, serial2);
    }

    // Start Measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }
    
    if (!bme.begin(0x76)) {
      Serial.println("Could not find a valid BME680 sensor, check wiring!");
      delay(100);
    }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  Serial.println("Waiting for first measurement... (5 sec)");
  delay (5000);
}

void loop() {
    uint16_t error;
    char errorMessage[256];
    
    // Read Measurement
    uint16_t co2;
    float temperature;
    float humidity;
    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else if (co2 == 0) {
        Serial.println("Invalid sample detected, skipping.");
    } else {
        Serial.print("SCDx - Co2:");
        Serial.print(co2);
        Serial.print(" ppm, ");
        
        Serial.print("Temperature:");
        Serial.print(temperature);
        Serial.print(" *C, ");
        
        Serial.print("Humidity:");
        Serial.print(humidity);
        Serial.println(" %");
    }

    #ifdef LCD
      lcd.clear();
      lcd.setCursor (0,0);
      lcd.print("CO2:");
      lcd.setCursor(5,0);
      lcd.print(co2);
      lcd.setCursor (9,0);
      lcd.print("ppm");
    #endif

    #ifdef RING
       // WSGauge(co2,800,1000,100,10,true); //10 Bar
       WSGauge(co2,800,1000,100,20,true); //20 Ring
    #else
    
      if (( ( co2 ) < ( 800 ) ))
  {
    pixels.setPixelColor(0,0,30,0,0);
    pixels.show();
    #ifdef LCD
      lcd.setBacklight(0, 255, 0); //green
    #endif
  }
  else
  {
    if (( ( co2 ) < ( 1000 ) ))
    {
      pixels.setPixelColor(0,30,30,0,0);
      pixels.show();
      #ifdef LCD
        lcd.setBacklight(255, 255, 0); //yellow
      #endif
    }
    else
    {
      pixels.setPixelColor(0,40,0,0,0);
      pixels.show();
      #ifdef LCD
        lcd.setBacklight(255, 0, 0); //red
      #endif
    }
  }
  #endif

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("BME68x - Temperature: ");
  Serial.print(bme.temperature);
  Serial.print(" *C, ");

  Serial.print("Pressure: ");
  Serial.print(bme.pressure / 100.0);
  Serial.print(" hPa,");

  Serial.print("Humidity: ");
  Serial.print(bme.humidity);
  Serial.print(" %, ");

  Serial.print("Gas: ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.print(" KOhms, ");

  Serial.print("Altitude: ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");
  
  delay(5000);
}

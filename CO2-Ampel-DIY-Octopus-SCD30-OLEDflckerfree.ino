/* This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details. */
#include <SparkFun_SCD30_Arduino_Library.h>
#include <Wire.h>
#include <Ticker.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeMonoBoldOblique24pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

Adafruit_SH110X display = Adafruit_SH110X(64, 128, &Wire);
GFXcanvas1 canvas(64,128);

unsigned long myTimer = 0;
unsigned long myTimeout = 60000; // heart beat 60sec, LED build-in

// OLED FeatherWing buttons map to different pins depending on board:
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2

int CO2 = 0 ;
//Reading CO2, humidity and temperature from the SCD30 By: Nathan Seidle SparkFun Electronics 

//https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library

SCD30 airSensorSCD30; // Objekt SDC30 Umweltsensor
String matrixausgabe_text  = " "; // Ausgabetext als globale Variable

volatile int matrixausgabe_index = 0;// aktuelle Position in Matrix

volatile int matrixausgabe_wait  = 0;// warte bis Anzeige durchgelaufen ist

//--------------------------------------- Charlieplex Matrix
Adafruit_IS31FL3731_Wing matrix = Adafruit_IS31FL3731_Wing();

Ticker matrixausgabe;
void matrixUpdate(){ // Update Charlieplexmatrix über Ticker
  int anzahlPixel=(matrixausgabe_text.length())*6;
  if (anzahlPixel > 15) { // Scrollen 
    if (matrixausgabe_index >= -anzahlPixel) {
      matrix.clear();
      matrix.setCursor(matrixausgabe_index,0);
      matrix.print(matrixausgabe_text);
      matrixausgabe_index--;
    } 
    else {
      matrixausgabe_index = 12;
      matrixausgabe_wait=0;
    }
  } 
  else {// nur 3 Zeichen lang-> kein Scroll 
    matrix.setCursor(0,0);
    matrix.print(matrixausgabe_text);
    matrixausgabe_wait  = 0; // no wait
  }
}
void matrixAnzeige(String text, int wait) { // Setze Ausgabetext
  if (matrixausgabe_text  != text) { // update nur bei neuem Text 
    matrix.clear();
    matrixausgabe_index = 0;
    matrixausgabe_text  = text;
    matrixausgabe_wait  = wait;
    while (matrixausgabe_wait) delay(10); // warte bis Text einmal ausgegeben ist
  }
};

Adafruit_NeoPixel WSpixels = Adafruit_NeoPixel((1<24)?1:24,15,NEO_RGB + NEO_KHZ800);

void setup(){ // Einmalige Initialisierung
  WiFi.forceSleepBegin(); // Wifi off
  pinMode( 12 , OUTPUT);
  pinMode( 13 , OUTPUT);
  pinMode( 14 , OUTPUT);
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

  airSensorSCD30.setMeasurementInterval(2);     // CO2-Messung alle 5 s

  pinMode( 2 , INPUT);
      
  Serial.begin(115200);
  matrix.begin();// Matrix initialisieren 
  delay(10);
  matrix.clear(); 

  matrix.setTextColor(60); // Helligkeit begrenzen 
  matrixausgabe.attach(0.08, matrixUpdate); // zyklisch aktualisieren

  //Serial.println();
  WSpixels.begin();//-------------- Initialisierung Neopixel

  digitalWrite( 12 , HIGH ); //--- test LEDs

  digitalWrite( 13 , HIGH ); //--- test LEDs

  digitalWrite( 14 , HIGH ); //--- test LEDs

  delay(500);
 
  digitalWrite( 12 , LOW );

  digitalWrite( 13 , LOW );

  digitalWrite( 14 , LOW );


  //Serial.println("128x64 OLED FeatherWing test");
  display.begin(0x3C, true); // Address 0x3C default

  //Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  
  Wire.setClock(100000L);            // 100 kHz SCD30 
  Wire.setClockStretchLimit(200000L);// CO2-SCD30
  
   display.setRotation(1);
   display.setTextSize(1);
   display.setTextColor(SH110X_WHITE);
   display.setCursor(0,0);
   display.println("CO2 Ampel V21/1");
   display.println("starten ...");
   display.println("");
   display.display();
   
   delay(2000);

  if (!( digitalRead(2) ))
  {
    display.println("Kalibierung starten?");
    display.println("Button(2) 5sec halten");
    display.display();
    
    delay( 5000 );
    if (!( digitalRead(2) ))
    {

  digitalWrite( 12 , HIGH ); //--- test LEDs

  digitalWrite( 13 , HIGH ); //--- test LEDs

  digitalWrite( 14 , HIGH ); //--- test LEDs

  int i = 0;
  int sensor = 0;

        for (i= 1; i<= ( 24 ); i=i+1)
      {
        sensor = airSensorSCD30.getCO2() ;

  display.clearDisplay();
  display.setRotation(0);
  display.display();

   
  canvas.fillScreen(SH110X_BLACK);
  canvas.setRotation(1);

  canvas.setTextSize(1);
  canvas.setTextColor(SH110X_WHITE);
  canvas.setFont(&FreeMonoBold24pt7b);
  canvas.setCursor(0,45);
  if (CO2 <= 9999) {
    canvas.print(String(sensor));
  } else 
  {
    canvas.print("----");
  }
  canvas.setFont();
  canvas.setCursor(110,40);
  canvas.print("ppm");
  canvas.setCursor(2,55);
  canvas.print("Kalibrierung "+String((i)*5)+"/120s");
    
  display.drawBitmap (0,0, canvas.getBuffer(), 64, 128, SH110X_WHITE, SH110X_BLACK);
  display.display();

        
        delay( 5000 );
        yield();
      }
      
      // Forced Calibration Sensirion SCD 30
      //Serial.print("Start SCD 30 calibration, please wait 20 s ...");
      //delay(20000);
      airSensorSCD30.setAltitudeCompensation(0); // Altitude in m ü NN 
      airSensorSCD30.setForcedRecalibrationFactor(400); // fresh air 



  delay(1000);
 
  digitalWrite( 12 , LOW );

  digitalWrite( 13 , LOW );

  digitalWrite( 14 , LOW );

    }
  }

  display.clearDisplay();
  display.setRotation(0);
  display.display();

  
}

void loop() { // Kontinuierliche Wiederholung 
  CO2 = airSensorSCD30.getCO2();
  
  canvas.fillScreen(SH110X_BLACK);
  canvas.setRotation(1);

  canvas.setTextSize(1);
  canvas.setTextColor(SH110X_WHITE);
  canvas.setFont(&FreeMonoBold24pt7b);
  canvas.setCursor(0,45);
  if (CO2 <= 9999) {
    canvas.print(String(CO2));
  } else 
  {
    canvas.print("----");
  }
  canvas.setFont();
  canvas.setCursor(110,40);
  canvas.print("ppm");
  canvas.setCursor(2,55);
  //display.print("www.co2ampel.org - V1");
  canvas.print(String(airSensorSCD30.getTemperature()-4)+"C");
  canvas.setCursor(92,55);
  canvas.print(String(airSensorSCD30.getHumidity())+"%");
    
  display.drawBitmap (0,0, canvas.getBuffer(), 64, 128, SH110X_WHITE, SH110X_BLACK);
  display.display();

  // don't use this as long GPIO2 is set as INPUT to trigger sensor callibration!
  // heardbeat - on build in LED
  /*
  if (millis() > myTimeout + myTimer ) {
    myTimer = millis();

    digitalWrite( 2 , LOW );
    delay( 500 );
    digitalWrite( 2 , HIGH );
  }
  */
  
  matrixAnzeige(String(String(( airSensorSCD30.getCO2() / 10 ))),1);
  Serial.println("CO2:"+String(String(CO2)));
  //Serial.println();
  if (( ( CO2 ) < ( 1000 ) ))
  {
    digitalWrite( 12 , LOW );
    digitalWrite( 13 , LOW );
    digitalWrite( 14 , HIGH );
    WSpixels.setPixelColor(0,(0<32)?0:32,(64<32)?64:32,(0<48)?0:48);
    WSpixels.show();
    //Serial.print("green");
    //Serial.println();
  }
  else
  {
    if (( ( CO2 ) < ( 1400 ) ))
    {
      digitalWrite( 12 , LOW );
      digitalWrite( 13 , HIGH );
      digitalWrite( 14 , LOW );
      WSpixels.setPixelColor(0,(64<32)?64:32,(64<32)?64:32,(0<48)?0:48);
      WSpixels.show();
      //Serial.print("yellow");
      //Serial.println();
    }
    else
    {
      digitalWrite( 12 , HIGH );
      digitalWrite( 13 , LOW );
      digitalWrite( 14 , LOW );
      WSpixels.setPixelColor(0,(64<32)?64:32,(0<32)?0:32,(0<48)?0:48);
      WSpixels.show();
      //Serial.print("red");
      //Serial.println();
    }
  }
  delay( 2000 );
}

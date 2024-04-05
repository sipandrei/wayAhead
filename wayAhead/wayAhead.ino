
#include <SD.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define SCREEN_RESET -1
#define SCREEN_ADRESS 0x3C

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, SCREEN_RESET);

#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

static const int gpsRX = 4;
static const int gpsTX = 3;
static const uint32_t GPSBaud = 4800;

TinyGPSPlus gps;
SoftwareSerial ss(gpsRX, gpsTX);

#define SDCHIPSELECT 7
String name;

static const int selectButton = 6;
static const int cycleButton = 5;

void setup() {
  String prefix = "activity";
  int activityNumber = 0;
  String suffix = ".gpx";
  name = prefix + activityNumber + suffix;
  Serial.begin(9600);
  // put your setup code here, to run once:
  if(!oled.begin(SCREEN_ADRESS))
    Serial.print("Screen not found!");
  if(!SD.begin(SDCHIPSELECT))
      Serial.println("SD Initialization failed");
  while(SD.exists(name)){
    activityNumber++;
    name = prefix + activityNumber + suffix;
  }

  pinMode(selectButton, INPUT_PULLUP);
  pinMode(cycleButton, INPUT_PULLUP);

  ss.begin(GPSBaud);
}

void loop() {
  // put your main code here, to run repeatedly:

}

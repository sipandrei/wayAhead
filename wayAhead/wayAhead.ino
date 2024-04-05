
#include <SD.h>

File recording;

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define SCREEN_RESET -1
#define SCREEN_ADDRESS 0x3C

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
String buffer;

static const int selectButton = 6;
static const int cycleButton = 5;
int selectButtonState = LOW;
int cycleButonState = LOW;
int debounceDelay = 50;
int lastDebounce = 0;

int buttonCheck(int buttonReading, int oldState){
  int state = oldState;
  if(buttonReading != oldState)
    lastDebounce = millis();
  if(millis()-lastDebounce > debounceDelay){
    if(buttonReading != oldState)
      state = buttonReading;
  }
  return state;
}

void initializeRecording(String filename){
  recording = SD.open(filename, FILE_WRITE);
}

void setup() {
  Serial.begin(9600);
  buffer.reserve(1024);
  
  String prefix = "activity";
  int activityNumber = 0;
  String suffix = ".gpx";
  name = prefix + activityNumber + suffix;
  
  if(!oled.begin(SCREEN_ADDRESS))
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
  selectButtonState = buttonCheck(digitalRead(selectButton), selectButtonState);
  cycleButtonState = buttonCheck(digitalRead(cycleButton), cycleButtonState);
}

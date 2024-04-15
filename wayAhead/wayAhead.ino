
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

String type = "cycling";

#define SDCHIPSELECT 7
String name;
String buffer;
int lastMillis = 0;

static const int selectButton = 6;
static const int cycleButton = 5;
int selectButtonState = LOW;
int cycleButtonState = LOW;
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

void addLineToBuffer(String message){
//  unsigned long now = millis();
//  if((now - lastMillis) >= 10){
    buffer += message;
    buffer += "\n";
    //lastMillis = now;
  //}
}

String addTimeElement(){
  String timeElement = "\n<time>";
  timeElement +=gps.date.year();
  timeElement += "-";
  timeElement +=gps.date.month();
  timeElement += "-";
  timeElement +=gps.date.day();
  timeElement += "T";
  timeElement +=gps.time.hour();
  timeElement += ":";    
  timeElement +=gps.time.minute();
  timeElement += ":";
  timeElement +=gps.time.second();
  timeElement += "Z";
  timeElement += "</time>";
  return timeElement;
}

String addActivityType(){
  String activityType = "<name>Getto GPS Ride :]</name>";
  activityType +="\n<type>";
  activityType += type;
  activityType += "</type>";
  return activityType;
}

void initializeRecording(String filename){
  recording = SD.open(filename, FILE_WRITE);

  String header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<gpx creator=\"WayAheadGettoGPS\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\" version=\"1.1\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\" xmlns:gpxx=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3\">";
  header += "\n<metadata>";
  header += addTimeElement();
  header += "\n</metadata>";
  header +="\n<trk>";
  header += addActivityType();
  header += "\n<trkseg>";
  addLineToBuffer(header);
}

void writeIfAvailable(){
  
  unsigned int chunkSize = recording.availableForWrite();
  if(chunkSize && buffer.length() >= chunkSize){
    recording.write(buffer.c_str(),chunkSize);
    buffer.remove(0,chunkSize);
  }
}

void finishRecording(){
  String footer = "\n</trkseg>\n</trk>\n</gpx>";
  addLineToBuffer(footer);
  //afisare Saving...
  while(buffer.length() != 0){
    writeIfAvailable();
  }
}

String addTrackPoint(){
  String trackPoint = "<trkpt lat=\"";
  trackPoint += gps.location.lat();
  trackPoint += "\" lon=\"";
  trackPoint += gps.location.lng();
  trackPoint += "\">";
  trackPoint += "\n<ele>";
  trackPoint += gps.altitude.meters();
  trackPoint += "</ele>";
  trackPoint += addTimeElement();
  trackPoint += "\n</trkpt>";
  return trackPoint;
}

void setup() {
  Serial.begin(9600);
  buffer.reserve(512);  
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

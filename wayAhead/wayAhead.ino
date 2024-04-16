
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

bool cycling = true;
bool isRecording = false;
bool oldRecordingState = false;
int currentScreen = 1;
float oldLat,oldLon,oldAlt;
float lati,lon,alt;

#define SDCHIPSELECT 7
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
  if(gps.date.isValid()){
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
  activityType += cycling?"cycling":"running";
  activityType += "</type>";
  return activityType;
}

void initializeRecording(String filename){
  while(!gps.location.isValid()){}
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

String addTrackPoint(float latitude, float longitude, float alti){
  String trackPoint = "<trkpt lat=\"";
  trackPoint += latitude;
  trackPoint += "\" lon=\"";
  trackPoint += longitude;
  trackPoint += "\">";
  trackPoint += "\n<ele>";
  trackPoint += alti
  trackPoint += "</ele>";
  trackPoint += addTimeElement();
  trackPoint += "\n</trkpt>";
  return trackPoint;
}

void buttonHandling(){
  if(selectButtonState = 1){
    switch(currentScreen/10){
      case 0:
        isRecording = true;
        newRecording = true;
        currentScreen = 10; //waiting for sattelites
        break;
      case 1:
        isRecording = false;
        currentScreen = 0; //saving screen
        break;
    }
  }
  if(cycleButtonState = 1){
    switch(currentScreen/10){
      case 0:
        switch(currentScreen%10){
          case 1:
            cycling = !cycling;
            break;
        }
        while(cycleButtonState = 1){}
        break;
      case 1:
        if(currentScreen%10 < 3){
          currentScreen++;
        } else {
          currentScreen = currentScreen/10*10+1;
        }
        break;
    }
  }
}

String nameMaker(){
  String name;
   int activityNumber = 0;
   String prefix = "activity";
   String suffix = ".gpx";
   while(SD.exists(name)){
    activityNumber++;
    name = prefix + activityNumber + suffix;
  }
  return name;
}

void setup() {
  Serial.begin(9600);
  buffer.reserve(512);    
  if(!oled.begin(SCREEN_ADDRESS))
    Serial.print("Screen not found!");
  if(!SD.begin(SDCHIPSELECT))
      Serial.println("SD Initialization failed");
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(cycleButton, INPUT_PULLUP);

  ss.begin(GPSBaud);
}

void recordingHandling(float lati, float lon, float alt){
  if(oldRecordingState == false){
    initializeRecording(nameMaker())
  }
  int now = millis();
  if(now-lastMillis >= 2000){
    addTrackPoint(lati,lon,alt);
    lastMillis = now;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  selectButtonState = buttonCheck(digitalRead(selectButton), selectButtonState);
  cycleButtonState = buttonCheck(digitalRead(cycleButton), cycleButtonState);
  oldRecordingState = isRecording;
  buttonHandling();
  if(isRecording){
    if(gps.location.isUpdated() && millis()-lastMillis > 2000){
    oldLat = lati;
    oldLon = lon;
    oldAlt = alt;
    lati = gps.location.lat();
    lon = gps.location.lng();
    alt = gps.altitude.meters();}
  }
}

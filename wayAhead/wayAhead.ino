
#include <SD.h>

File recording;

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <math.h>

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
float distance, averageSpeed;
int startRecordTime = 0;

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
    }
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
  distance = 0;
  averageSpeed = gps.speed.kmph();
  recording = SD.open(filename, FILE_WRITE);
  startRecordTime = millis();  

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
  savingScreen();
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
  trackPoint += alti;
  trackPoint += "</ele>";
  trackPoint += addTimeElement();
  trackPoint += "\n</trkpt>";
  return trackPoint;
}

void buttonHandling(){
  if(selectButtonState = 1){
    switch(currentScreen/10){
      case 0:
        if(currentScreen == 1){
          isRecording = true;
          oldRecordingState = false;
          currentScreen = 10; //instant Infor screen
          }
        break;
      case 1:
        if(currentScreen == 12){
          isRecording = false;
          oldRecordingState = true;
          currentScreen = 0; //saving screen
        }
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
          currentScreen = currentScreen/10*10+0;
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


void updateDistance(){
  //float p2p = 2*6371*sqrt((1-cos(lati-oldLat)+cos(lati)*cos(oldLat)*(1-cos(lon-oldLon))/2)); //haversine equation
  float p2p = TinyGPSPlus::distanceBetween(lati,lon,oldLat,oldLon);
  distance += p2p;
}

void updateAverageSpeed(){
  averageSpeed += gps.speed.kmph();
  averageSpeed /= 2;
}

void recordingHandling(){
  if(oldRecordingState == false){
    initializeRecording(nameMaker());
  }
  int now = millis();
  if(now-lastMillis >= 2000 and gps.location.isUpdated()){
    oldLat = lati;
    oldLon = lon;
    oldAlt = alt;
    lati = gps.location.lat();
    lon = gps.location.lng();
    alt = gps.altitude.meters();
    updateDistance();
    updateAverageSpeed();
    addTrackPoint(lati,lon,alt);
    lastMillis = now;
  }
}

void savingScreen(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(display.width()/2-4,10);             // Start at top-left corner
  display.println(F("Salvare Activitate. . ."));
  display.display();
  currentScreen = 1;
}

void startRecordingScreen(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(display.width()/2-4,10);
  if(cycling){
    display.println(F("• bicicleta"));
    display.println(F("  alergare"));
  } else {
    display.println(F("  bicicleta"));
    display.println(F("• alergare"));
  }
  display.display();
}

void stopScreen(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(display.width()/3-4,10);
  display.println(F("Doresti sa salvezi?"));
  display.display();
}

void rideInfo(float viteza){
  int delta = (millis() - startRecordTime)*0.001;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE); 
  display.drawLine(display.width()-1, 10, display.width()+1, 10, SSD1306_WHITE);   
  display.drawLine(display.width()-1, 20, display.width()+1, 20, SSD1306_WHITE);
  display.drawLine(display.width()-1, 40, display.width()+1, 40, SSD1306_WHITE);  
  display.setCursor(display.width()/3-4,5);
  display.print(gps.satellites.value());
  display.print("sat");
  display.setCursor(display.width()/3-4,15);
  display.print(delta/60/60); delta -= delta/60/60*60*60;
  display.print(":");
  display.print(delta/60); delta -= delta/60*60;
  display.print(":");
  display.println(delta);
  display.setCursor(display.width()/2-4,35);
  display.print(viteza);
  display.println(F("km/h"));
  display.setCursor(display.width()/2-4,50);
  display.print(distance);
  display.println(F("km"));
  display.display();
}

void screenHandler(){
  switch(currentScreen){
    case 0:
      savingScreen();
    break;
    case 1:
      startRecordingScreen();
    break;
    case 11:
      rideInfo(averageSpeed);
    break;
    case 12:
      stopScreen();
    break;
    case 10:
      rideInfo(gps.speed.kmph());
    break; 
  }
}

void setup() {
  Serial.begin(9600);
  buffer.reserve(512);    
  if(!display.begin(SSD1306_SWITCHCAPVCC))
    Serial.print("Screen not found!");
  if(!SD.begin(SDCHIPSELECT))
      Serial.println("SD Initialization failed");
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(cycleButton, INPUT_PULLUP);
  display.clearDisplay();
  
  ss.begin(GPSBaud);
}

void loop(){
  while(ss.available() > 0)
    gps.encode(ss.read());
  // put your main code here, to run repeatedly:
  selectButtonState = buttonCheck(digitalRead(selectButton), selectButtonState);
  cycleButtonState = buttonCheck(digitalRead(cycleButton), cycleButtonState);
  oldRecordingState = isRecording;
  buttonHandling();
  if(isRecording){
   recordingHandling(); 
  } 
  else if(oldRecordingState == true) {
    finishRecording();
  }
  screenHandler();
}

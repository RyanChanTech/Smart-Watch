// TODO: run new JSON parser with old API link

/*
 * Code written by Ryan Chan
 * WiFi and OpenWeather portion based on code from Random Nerd Tutorials: https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/
 * 
 * To use this, you much install these libraries from the Arduino Library Manager:
 * Arduino_JSON
 * TimeLib
 * Adafruit_GFX
 * Adafruit BusIO
 * Adafruit Zero DMA
 * Adafruit ST7735 and ST7789
 * Adafruit SPIFlash
 */

//include Wifi libraries. to disable, go to places with "@"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//include TFT Display libraries
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <TimeLib.h>

//include fonts
#include <Fonts/FreeSansBoldOblique24pt7b.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h >
#include <Fonts/FreeSansOblique12pt7b.h >

const char* ssid = "yourWifiNameHere";
const char* password = "yourPasswordHere";

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "your API key here";

// Replace with your country code and city
String latitude = "your latitude here";
String longitude = "your longitude here";

int weatherUpdateInterval = 30; // the number of minutes before the weather data automatically updates

const int TFT_CS = 17;
const int TFT_RST = 5;
const int TFT_DC = 32;
const int TFT_BL = 25;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

unsigned long last_time;

String weekdays[] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};

int seconds = 0;
int minutes = 0;
int hours = 0;
int days = 0;

void calculate_times(){
  if(seconds == 60){
    minutes++;
    seconds = 0;
  }
  
  if( minutes == 60 ){
    hours++;
    minutes = 0;
  }

  if(hours == 24){
    days++;
    hours = 0;
  }
}

// prepending 0's needs... alot of work..
String getFormattedHHMM(){
  String formatted_time = "";
  
  if(hours < 10){
    formatted_time += "0";
  }
  formatted_time += hours;
  formatted_time += ":";

  if(minutes < 10){
    formatted_time += "0";
  }
  formatted_time += minutes;

  return formatted_time;
}

String getFormattedSS(){
  String formatted_time = ":";
  if(seconds < 10){
    formatted_time += "0";
  }
  formatted_time += seconds;
  return formatted_time;  
}

//@
void initWifi(){
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wifi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void setup() {
  // Initialize Serial
  Serial.begin(115200);

  // Initialize TFT display
  tft.init(240, 240);
  tft.fillScreen(ST77XX_BLACK);

  
  // Initialize Wifi@
  tft.setCursor(10, 75);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&FreeSans12pt7b);
  tft.print("Connecting to WiFi...");
  initWifi();
  tft.fillScreen(ST77XX_BLACK);

  last_time = millis();
}

boolean firstRun = true;

void loop() {
  // Keep track of updates
  boolean updateHHMM = false; // update HH:MM (hours:minutes)
  boolean updateSS = false; // update :SS (seconds) 
  boolean updateWeather = false;
  boolean updateDate = false;
  
  int delta = millis() - last_time;
  if(delta >= 999){
    seconds++;
    last_time = millis();
    calculate_times();

    updateSS = true;

    if(seconds == 0 || firstRun){
      updateHHMM = true;
    }

    if( ((hours == 0) && (seconds == 0)) || firstRun){
      updateDate = true;
    }

    if( ((minutes % weatherUpdateInterval == 0) && (seconds == 0)) || firstRun){
      updateWeather = true;
    }
    
    firstRun = false;
  }

  // Display Date
  if(updateDate){
    tft.setTextWrap(true);
    tft.fillRect(10,0,200,30,ST77XX_BLACK);
    tft.setCursor(10, 20);
    tft.setFont(&FreeSans12pt7b);
    tft.setTextColor(ST77XX_WHITE);
    tft.print(weekdays[days%7]);
  }

  // Display HH:MM (hours:minutes)
  if(updateHHMM){
    tft.fillRect(10,30,130,70,ST77XX_BLACK);
    tft.setCursor(10, 75);
    tft.setTextColor(ST77XX_WHITE);
    tft.setFont(&FreeSansBoldOblique24pt7b);
    tft.print(getFormattedHHMM());
    updateHHMM = false;
  }

  // Display :SS (:seconds)
  if(updateSS){
    tft.fillRect(135,30,50,70,ST77XX_BLACK);
    tft.setCursor(135, 75);
    tft.setTextColor(ST77XX_WHITE);
    tft.setFont(&FreeSansBoldOblique12pt7b);
    tft.print(getFormattedSS());
    updateSS = false;
  }

  
  // Get Weather Data@
  if(updateWeather && (WiFi.status() == WL_CONNECTED)){
      //String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + yourCity + "," + yourCountryCode + "&APPID=" + openWeatherMapApiKey;
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=Scranton,US&APPID=da7b0960d86381f11c5f300c2b546bb1";
      //String serverPath = "https://api.openweathermap.org/data/2.5/onecall?lat=" + latitude + "&lon=" + longitude + "&appid=" + openWeatherMapApiKey;

      String jsonBuffer = httpGETRequest(serverPath.c_str());
      StaticJsonDocument<21000> doc;
      deserializeJson(doc, jsonBuffer);
  
      // Display Humidity
      tft.setCursor(160,170);
      tft.setFont(&FreeSansOblique12pt7b);
      tft.setTextColor(ST77XX_WHITE);
      int currentWeather = doc["main"]["temp"];
      tft.print(currentWeather);
      tft.drawCircle(185, 165, 30, ST77XX_CYAN);
      tft.drawCircle(185, 165, 29, ST77XX_CYAN);
      tft.drawCircle(185, 165, 28, ST77XX_CYAN);

      unsigned long epochTime = doc["current"]["dt"];
      Serial.println(day(epochTime));
      Serial.println(month(epochTime));
      Serial.println(year(epochTime));
      Serial.println(hour(epochTime));
      Serial.println(minute(epochTime));
      Serial.println(second(epochTime));
      
  }
  
}

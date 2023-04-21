// Blynk Adaption Templates 

#define BLYNK_TEMPLATE_ID "TMPLkb-TrHKS"
#define BLYNK_TEMPLATE_NAME "accident"
#define BLYNK_AUTH_TOKEN "CmCNG56x5yDAkNzIKPSjZr-CQAMmqwd4"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
char auth[] = BLYNK_AUTH_TOKEN;

// Telegram setup start's from here 

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  //  Library install from github https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h> // librabry from library module v

// Initialize Telegram BOT

#define BOTtoken "5997540339:AAEAncDDV_BRqAiWHC8dC3jmysUD5e1htiU"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group

#define CHAT_ID "1769425929"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 0.03 second.

int botRequestDelay = 0.03;
unsigned long lastTimeBotRan;

// Define pin for telegram opertaion on ESP8266

const int valvePin = 14; // Relay pin for operating Soleniod Value 
bool valveState = LOW;

// Handle what happens when you receive new messages

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user, You're not vehicle owner", "");
      continue;
    }
    
// Print the received message
    
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/setup") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your Vehicle.\n\n";
      welcome += "/antitheft to turn GPIO ON \n";
      welcome += "/disable to turn GPIO OFF \n";
      welcome += "/update to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/antitheft") {
      bot.sendMessage(chat_id, "Anti-theft turned on for your vehicle, fuel supply will be cut-off shortly", "");
      valveState = HIGH;
      digitalWrite(valvePin, valveState);
    }
    
    if (text == "/disable") {
      bot.sendMessage(chat_id, "Anti-theft turned off for your vehicle, fuel supply will be restored shortly", "");
      valveState = LOW;
      digitalWrite(valvePin, valveState);
    }
    
    if (text == "/update") {
      if (digitalRead(valvePin)){
        bot.sendMessage(chat_id, "Anti-theft turned is on for your vehicle", "");
      }
      else{
        bot.sendMessage(chat_id, "Anti-theft turned off for your vehicle", "");
      }
    }
  }
}

// Sensor' Setup, Include all librarie's

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include<Wire.h>
#include <Adafruit_MPU6050.h> // Acclerometer Sensor library
#include <Adafruit_Sensor.h>  // Acclerometer Sensor supporting library

Adafruit_MPU6050 mpu;


#define rs 10 // GPIO10
#define en 12 // GPI4 d6
#define d4 0  // GPIO0 d3
#define d5 2  // GPIO5 d4
#define d6 3  // GPIO3 rx
#define d7 1  // GPIO1 tx


static const int RX= D7, TX= D8;
static const uint32_t GPSBaud = 9600;
const char* ssid     = "Iot";//Replace with Network SSID
const char* password = "Iot12345";//Replace with Network Password

TinyGPSPlus gps;
//WiFiClient  client;
WiFiServer server(80);
SoftwareSerial soft(RX, TX);
String latitude_data;
String longitude_data;


// intialising Void Setup Parameter's

void setup()
{
  // blynk one  time run
  
    Blynk.begin(auth, ssid, password);
   pinMode(16, INPUT);
  Wire.begin(D2, D1);

 // telegram one time run
 
   if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  
   #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(valvePin, OUTPUT);
  digitalWrite(valvePin, valveState);
  
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
 
 // Serial monitor for IDE Environment 
 
  Serial.begin(115200);
  soft.begin(GPSBaud);
  WiFi.begin(ssid, password);
  server.begin();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  
    Serial.println("WiFi connecting...          ");
  }
 
  
   Serial.print("WiFi connected ");
 
   Serial.println(WiFi.localIP());
 
if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // set accelerometer range to +-8G
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // set gyro range to +- 500 deg/s
  
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // set filter bandwidth to 21 Hz
  
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
  
// Connect to Wi-Fi
   WiFi.mode(WIFI_STA);
// WiFi.begin(ssid, password);
}

// Setting up of Void Loop Parameter's
void loop()
{
   Blynk.run();
  gyro();
  vibration();
  valve();
    Blynk.virtualWrite(V0,18.9891);   //Langi
        Blynk.virtualWrite(V1,73.9627); //lati
}

void getgps(){
   while (soft.available() > 0)
    if (gps.encode(soft.read()))
    {
      displaydata();
      displaywebpage();
    }
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("GPS Connection Error!!"));
    while (true);  }

}

void displaydata()
{
  if (gps.location.isValid())
  {
    double latitude = (gps.location.lat());
    double longitude = (gps.location.lng());
    latitude_data= (String(latitude, 6));
    longitude_data= (String(longitude, 6));
    delay(20000);
  }
  else
  {
    Serial.println(F("Data error!!!"));
  }
}

// Setting up of Web-page using Ip-address in-case predetermined value's are triggerred 

void displaywebpage()
{
    WiFiClient client = server.available();
    if (!client)
    {
      return;
    }
    String page = "<html><center><p><h1>Real Time Vehicle Tracking using IoT/ IOT-ER & SAFETY</h1><a style=""color:RED;font-size:125%;"" href=""http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
//    page += latitude_data;
page += 18.9891;
    page += "+";
//    page += longitude_data;
 page += 73.9627;
    page += ">Click here For Live Location</a> </p></center></html>";
    
    client.print(page);
    delay(100);
}

void gyro(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  float y=(a.acceleration.y);
  Serial.print(y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  Serial.println("");
  delay(500);
  Blynk.virtualWrite(V2,(a.acceleration.x));  
  Blynk.virtualWrite(V3,(a.acceleration.y)); 
  Blynk.virtualWrite(V4,(a.acceleration.z)); 

// Condition's for Accelerometer is to be set here !
 
  if(y>8 ||y<-8){
    getgps();
    Blynk.logEvent("accident_alert","Accident detection!!!"); 
  }
}

// void for Vibaration Sensor 

void vibration(){
    int data=digitalRead(16);     // d0 is the pin for vibaration sensor 
  delay(200);
  if(data==LOW) // Vibraion Sensor Condition
  {
    Serial.println("vibration detect");
    Blynk.virtualWrite(V5,"vibration detect");
     getgps();
     Blynk.logEvent("accident_alert","Accident detection!!!"); 
  }
  else
  {
    Blynk.virtualWrite(V5,"vibration is normal");
    Serial.println("vibration is normal");
  }
}                                                                                                                                                                                                                            

// Anti-theft Using telegram Bot (Valve)

void valve(){
    if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

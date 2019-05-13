#include <Time.h>
#include <TimeLib.h>
#include <SD.h>
#include <SPI.h>
#include <SD_t3.h>
#include <LiquidCrystal.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <SoftwareSerial.h>
#include <Snooze.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SoftwareSerial esp8266(0, 1);
//Wifi Setup
char ssid[] = "_";            // your network SSID (name)
char pass[] = "_";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//Thingspeak Setup
char server[] = "api.thingspeak.com";
char apiKey[] = "_";
int cloud_interval = 20000; //Time interval in seconds when the data is sent to the cloud
int cloud_cycler = cloud_interval; //Sends the first point that is recorded to the cloud

//Address Setup
char timeServer[] = "time.nist.gov";  // NTP server
unsigned int localPort = 2390;        // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;  // NTP timestamp is in the first 48 bytes of the message
const int UDP_TIMEOUT = 2000;    // timeout in miliseconds to wait for an UDP packet to arrive
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets
WiFiEspUDP Udp;// A UDP instance to let us send and receive packets over UDP
WiFiEspClient client;//Initiallize Ethernet client object

//Sensing Setup
//const float cal_intercept = 5.4106; //Y-intercept of the linear calibration model
const float cal_intercept = 2.5;
const float cal_lin = 118.66; //Linear component coeffcient
const float cal_sqr = 0; //Square compoenent coefficient
const float cal_cub = 0; //Cube component coefficient

const float sample_rate = 1; //Sampling rate of data acquired from the sensor (in Hz)
const int data_input_pin = 23; //Assigns the number of the pin which will receive the voltage from the current-to-voltage converter
const int card = BUILTIN_SDCARD; //Assigns location of the SD card
float sensor_cycle = 0;
String data_String = "";

//LCD Setup
const int rs = 33, en = 34, d4 = 35, d5 = 36, d6 = 37, d7 = 38; //Assigns the pins which control the LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //Activates the LCD
int time_set = 0;
long cycler = -60000;
int synch_interval = 1000;
int post_set_interval = 40000;

//Data Transfer Setup
long LED_14_TimeOn = 0;
long LED_15_TimeOn = 0;

//Sleep Setup
SnoozeTimer timer;
SnoozeBlock config(timer); //Connect the timer driver
long timer_int = (1000 / sample_rate);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  //pinMode(14, INPUT); //Wifi Toggle
  //  float wifi_connect = digitalRead(14);
  int wifi_con = 1; //1=connect to wifi
  //  if (wifi_connect ==HIGH) {
  //    wifi_con = 0; //0=don't connect to wifi
  //  }

  pinMode(data_input_pin, INPUT); //Sensor Input Pin Setup
  timer.setTimer(timer_int); //Sets the sleep timer
  lcd.begin(16, 2);  //Set up the LCD's number of columns and rows

  if (wifi_con == 1) {
    lcd.print("Connecting");
    lcd.setCursor(0, 1);
    lcd.print("to Network...");


    //Initiallize Serial Communication with ESP8266
    Serial.begin(115200);
    esp8266.begin(115200);

    //Initialize ESP8266 module
    WiFi.init(&Serial1);

    //Check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
      lcd.clear();
      lcd.print("Error: Reset");
      // don't continue
      while (true);
    }

    //Attempt to connect to WiFi network
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(ssid, pass);
    }

    //Indicate successful connection
    Serial.println("You're connected to the network");
    lcd.clear();
    lcd.print("Connected.");
    lcd.setCursor(0, 1);
    lcd.print("Synching Time...");
    Udp.begin(localPort);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  //SERIAL INFO TRANSFER INDICATOR
  //    // Send bytes from ESP8266 -> Teensy to Computer
  //    if ( esp8266.available() ) {
  //        Serial.write( Serial1.read() );
  //    }
  //
  //    // Send bytes from Computer -> Teensy back to ESP8266
  //    if ( Serial.available() ) {
  //        esp8266.write( Serial.read() );
  //    }
  //pinMode(14, INPUT); //Wifi Toggle
  ////  float wifi_connect = digitalRead(14);
  int wifi_con = 1; //1=connect to wifi
  //  if (wifi_connect ==HIGH) {
  //    wifi_con = 0; //0=don't connect to wifi
  //  }

  if (wifi_con == 1) {
    //TIME RETRIEVER
    if (millis() - cycler > synch_interval) {
      sendNTPpacket(timeServer); // send an NTP packet to a time server

      //Wait for a reply for UDP_TIMEOUT miliseconds
      unsigned long startMs = millis();
      while (!Udp.available() && (millis() - startMs) < UDP_TIMEOUT) {}

      Serial.println(Udp.parsePacket());
      if (Udp.parsePacket()) {
        Serial.println("packet received");
        // We've received a packet, read the data from it into the buffer
        Udp.read(packetBuffer, NTP_PACKET_SIZE);

        //The timestamp starts at byte 40 of the received packet and is four bytes,
        //or two words, long. First, extract the two words:

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        //Combine the four bytes (two words) into a long integer
        //This is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        Serial.print("Seconds since Jan 1 1900 = ");
        Serial.println(secsSince1900);

        //Convert NTP time into everyday time:
        Serial.print("Unix time = ");
        //Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        //Subtract seventy years:
        unsigned long epoch = secsSince1900 - seventyYears;

        epoch = epoch + 3UL; //Time Lag Correction
        epoch = epoch - 21600UL; //Time Zone Correction (Greenwich Mean→US Central)
        delay(1000);
        setTime(epoch);
        time_set = 1;


        Serial.println(epoch); //Print Unix time

        //    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
        //    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
        //    Serial.print(':');
        //    if (((epoch % 3600) / 60) < 10) {
        //      // In the first 10 minutes of each hour, we'll want a leading '0'
        //      Serial.print('0');
        //    }
        //    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
        //    Serial.print(':');
        //    if ((epoch % 60) < 10) {
        //      // In the first 10 seconds of each minute, we'll want a leading '0'
        //      Serial.print('0');
        //    }
        //    Serial.println(epoch % 60); // print the second
      }
      cycler = millis();
    }


    //SENSOR DATALOGGING AND DISPLAY
    if (millis() - sensor_cycle > (1000 / sample_rate)) {
      if (time_set == 1) {
        String data_String = "";
        float data_voltage = (3.3 / 1023) * analogRead(data_input_pin); //Receive voltage from sensor via current-voltage converter
        float o2_data_point = (cal_intercept) + (cal_lin) * (data_voltage) + (cal_sqr) * pow(data_voltage, 2) + (cal_cub) * pow(data_voltage, 3); //Convert voltage from sensor into datapoint
        int unixtime = now();
        data_String = String(o2_data_point) + "," + String(unixtime) + "\n";
        File data_File = SD.open("datalog.txt", FILE_WRITE);//Open the datalog file on the SD card
        data_File.println(data_String);//Send time data and O2 datapoint to the SD card
        data_File.close();

        if (millis()-cloud_cycler > cloud_interval) {
          sendDatatoCloud(String(o2_data_point)); //Send O2 datapoint to thingspeak server, which automatically time stamps it
          cloud_cycler=millis();
        }
        
        //data print
        sensor_cycle = millis();
        synch_interval = post_set_interval;
        lcd.clear();
        lcd.print("FiO2 and Time:");
        lcd.setCursor(0, 1);
        lcd.print(String(o2_data_point));
        lcd.print("%,");
        if (hour() < 10) {
          lcd.print('0');
        }
        lcd.print(hour());
        lcd.print(':');
        if (minute() < 10) {
          lcd.print('0');
        }
        lcd.print(minute());
        lcd.print(':');
        if (second() < 10) {
          lcd.print('0');
        }
        lcd.print(second());
        Snooze.sleep(config);
      }

    }
  }
  if (wifi_con == 0) {
    if (millis() - sensor_cycle > (1000 / sample_rate)) {
      String data_String = "";
      float data_voltage = (3.3 / 1024) * analogRead(data_input_pin); //Receive voltage from sensor via current-voltage converter
      float o2_data_point = (cal_intercept) + (cal_lin) * (data_voltage) + (cal_sqr) * pow(data_voltage, 2) + (cal_cub) * pow(data_voltage, 3); //Convert voltage from sensor into datapoint
      int unixtime = now();
      data_String = String(o2_data_point) + "\n";
      File data_File = SD.open("datalog.txt", FILE_WRITE);//Open the datalog file on the SD card
      data_File.println(data_String);//Send time data and O2 datapoint to the SD card
      data_File.close();

      //Data Print
      sensor_cycle = millis();
      lcd.clear();
      lcd.print("FiO2:");
      lcd.setCursor(0, 1);
      lcd.print(String(o2_data_point));
      lcd.print("%");
      Snooze.sleep(config);
    }
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Send an NTP request to the time server at the given address
void sendNTPpacket(char *ntpSrv) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE); //Set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;// Stratum, or type of clock
  packetBuffer[2] = 6; // Polling Interval
  packetBuffer[3] = 0xEC;// Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  Udp.beginPacket(ntpSrv, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void sendDatatoCloud(String data) {
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr += "&field1=";
  getStr += data;
  getStr += "\r\n\r\n";

  client.connect(server, 80);
  client.println(getStr);
  client.stop();
}

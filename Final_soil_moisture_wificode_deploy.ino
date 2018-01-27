/*
 Soil Moisture Sensor

 Connect moisture sensor to Arduino board
 */

/////////////////////
// Library Includes //
//////////////////////
// SoftwareSerial is required (even you don't intend on
// using it).
#include <SoftwareSerial.h> 
#include <SparkFunESP8266WiFi.h>
#include <LiquidCrystal.h> // add Liquid Crystal Library


//////////////////////////////
// WiFi Network Definitions //
//////////////////////////////
// Replace these two character strings with the name and
// password of your WiFi network.
const char mySSID[] = "Guest";
const char myPSK[] = "";

// Create a client and initiate a connectios
ESP8266Client client;
  
/////////////////
// HTTP Strings //
//////////////////
const String destServer = "bloom-sense-app.herokuapp.com";
const int destPort = 80; // default http port is 80

const String httpRequest = "POST /readings HTTP/1.1\r\n"
                           "Host: bloom-sense-app.herokuapp.com\r\n"
                           "Connection: keep-alive\r\n"
                           "Keep-Alive: timeout=30, max=1000\r\n"
                           "Content-Type: application/json;\r\n";


LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Declare variables 
// int redPin = 6;
//  int greenPin = 9;
//  int bluePin = 10;
const int led = 13;           // LED connected to digital pin 13
const int sensor = A2;       // soil moisture sensor connected to analog pin 2
const int threshold = 700;  // threshold value to decide when the LED blinks
int percent = 0;
int analogValue;
const int sensor_id = 1;
const int plant_id = 1;
unsigned long currentTime = 0;
unsigned long lastRequestTime = 0; 
const unsigned long delayTime = 10 * 1000; //10 seconds in ms

void setup() 
{
   // initialize the LED pin as an output:
    pinMode(led, OUTPUT);
    pinMode(sensor, INPUT);
    lcd.begin(16, 2);              // Initialize lcd, set up the LCD's number of columns and rows
    lcd.print("Moisture Sensor");  // Print a message to the LCD
    Serial.begin(9600);           // initialize serial communications:
    
    int status;
  //  Serial.begin(9600);
    
    // To turn the MG2639 shield on, and verify communication
    // always begin a sketch by calling cell.begin().
    status = esp8266.begin();
    if (status <= 0)
    {
      Serial.println("Unable to communicate with shield. Looping");
      while(1) ;
    }
    
    esp8266.setMode(ESP8266_MODE_STA); // Set WiFi mode to station
    if (esp8266.status() <= 0) // If we're not already connected
    {
      if (esp8266.connect(mySSID, myPSK) < 0)
      {
        Serial.println("Error connecting");
        while (1) ;
      }    
    }
    
    // Get our assigned IP address and print it:
    Serial.print(F("My IP address is: "));
    Serial.println(esp8266.localIP());
    
    connectToServer();

    delay(1000);

    // print your WiFi shield's IP address:
    //WiFi.setDNS(dns);
    //Serial.print("Dns configured.");
}

void loop()
{
  currentTime = millis(); // get from the cpu

  if(!client.connected())
  {
    Serial.println("Connection lost!");
      
    connectToServer();
    Serial.println("Connected.");
  }

  //read value of sensor and store it in the variable analogValue:
    analogValue = analogRead(sensor);
  
    // if the analog value is low, turn on the LED:
    //if the analog value is less than threshold LED blinks
     if (analogValue == LOW) {
        if (analogValue <= threshold) {
          digitalWrite(led, HIGH);
        } else {
          digitalWrite(led, LOW);
        }

     }

    percent = convertToPercent(analogValue); 
     // Turn on the cursor
    lcd.setCursor(0, 1);
    printValuesToSerial();
  
  if (currentTime - lastRequestTime >= delayTime) 
    
  {   
    // Post to database!
    postToDatabase();
    lastRequestTime = currentTime;
  }
  
}
 // Convert analog readings to percent
  int convertToPercent(int sensorValue) {
    int percentValue = 0;
    // Map the values
    percentValue = map(sensorValue, 1023, 465, 0, 100);
    if(percentValue > 100)
      percentValue = 100;
    
    return percentValue;
  }

// Print moisture level values 
void printValuesToSerial() {
  // print the analog value:
  Serial.print("\n\nAnalog Value: ");
  Serial.println(analogValue);
  Serial.print("\nPercent: ");
  Serial.println(percent);
  Serial.print("%");
  // display percentage on lcd
  lcd.print(percent);
  // display percent symbol at the end
  lcd.print("%");
  delay(1000);
  // clear extra characters (if % goes from 1% to 100% and back to1%,
  // lcd tends to read 1%%)
  lcd.print(" ");
  delay(1); 
}

void connectToServer()
{
  int connectedVar = client.connect(destServer, destPort);
//  if (client.connect(destServer, destPort) <= 0)
if (connectedVar <= 0)
  {
    Serial.println(connectedVar);
    Serial.println("Failed to connect to server.");
    return;
  }
}

void closeServerConnection()
{
    // connected() is a boolean return value - 1 if the 
  // connection is active, 0 if it's closed.
  if (client.connected())
    client.stop(); // stop() closes a TCP connection.
}

void postToDatabase()
{

 // Set up to post parameters:
 String JsonData = "{\"reading\": {\"sensor_id\": ";
  JsonData += String(sensor_id) + ", \"plant_id\": ";
  JsonData += String(plant_id) + ", \"value\": \"";
  JsonData += String(percent) + "\" }}";
//
  Serial.println(JsonData);  
  Serial.println("Posting to database!");

  client.print(httpRequest);
  client.print("Content-Length: "); 
  client.println(JsonData.length());
  //client.print("Content-Length: "); client.println(data.length());
  client.println();
  client.print(JsonData);
//  client.println();
//  client.print(data);

  // available() will return the number of characters
  // currently in the receive buffer.
  while (client.available())
    Serial.write(client.read()); // read() gets the FIFO char
 
}





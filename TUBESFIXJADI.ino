/**********************************************************************************
    TITLE:(IoT based Temperature Control System With ESP32 + DS18B20 Dallas Temperature Sensor + 0.96 inch OLED Display
    + Auto and Manual Modes + Temperature Set Point and Hysteresis +  EEPROM + Real time feedback)
 **********************************************************************************/

/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID "TMPL698Mjz12J"
#define BLYNK_TEMPLATE_NAME "ESP32 On and Off"
#define BLYNK_AUTH_TOKEN "8k9QqGsEYEhQ9sJ-QKolD3BfnjX9SaLR"
#define BLYNK_PRINT Serial
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "HUAWEI-TfH6";
char pass[] = "pm52FSNs";

//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>
Preferences pref;

#include <SPI.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Setpoint and hysteresis values (in degrees Celsius)
float setpoint = 30;
float hysteresis = 2;
float currentTemp = 0;

#define ONE_WIRE_BUS 17 // GPIO 34 of ESP32 for DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


// define the GPIO connected with Relays
#define HEATING_PIN 19
#define COOLING_PIN 23
#define wifiLed     16
#define I2C_SDA 21
#define I2C_SCL 22
#define LEDRED 18
#define LEDREDALERT 4
#define LEDGREEN 5
#define LEDBLUE 15



//Change the virtual pins according the rooms
#define VPIN_Text           V0
#define VPIN_Mode           V1
#define VPIN_currentTemp    V2
#define VPIN_setpoint       V3
#define VPIN_hysteresis     V4
#define VPIN_heaterbtn      V5
#define VPIN_coolerbtn      V6

// Relay and Mode State
bool heaterState = LOW; //Define integer to remember the toggle state for heater
bool coolerState = LOW; //Define integer to remember the toggle state for cooler
bool modeState = LOW; //Define integer to remember the mode
bool LEDREDSTATE = LOW;
bool LEDREDALERTSTATE = LOW;
bool LEDGREENSTATE = LOW;
bool LEDBLUESTATE = LOW;

int wifiFlag = 0;


char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;


// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_heaterbtn) {
  heaterState = param.asInt();
  digitalWrite(HEATING_PIN, !heaterState);
  pref.putBool("Heater", heaterState);
}

BLYNK_WRITE(VPIN_coolerbtn) {
  coolerState = param.asInt();
  digitalWrite(COOLING_PIN, !coolerState);
  pref.putBool("Cooler", coolerState);
}

BLYNK_WRITE(VPIN_Mode) {
  modeState = param.asInt();
  pref.putBool("Mode", modeState);
}

BLYNK_WRITE(VPIN_setpoint) {
  setpoint = param.asFloat();
  pref.putBool("setemp", setpoint);
}

BLYNK_WRITE(VPIN_hysteresis) {
  hysteresis = param.asFloat();
  pref.putBool("Hyst", hysteresis);
}

void checkBlynkStatus() { // called every 2 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, HIGH);
  }
  if (isconnected == true) {
    wifiFlag = 0;
    digitalWrite(wifiLed, LOW);
    Blynk.virtualWrite(VPIN_Text, "IoT Temperature Controller");
  }
}

BLYNK_CONNECTED() {
  // update the latest state to the server 
  Blynk.virtualWrite(VPIN_Text, "IoT Temperature Controller");
  Blynk.virtualWrite(VPIN_Mode, modeState);
  Blynk.syncVirtual(VPIN_currentTemp);
  Blynk.syncVirtual(VPIN_setpoint);
  Blynk.syncVirtual(VPIN_hysteresis);
  Blynk.virtualWrite(VPIN_heaterbtn, heaterState);
  Blynk.virtualWrite(VPIN_coolerbtn, coolerState);

}

void readSensor() {

  // Send the command to get temperatures
  sensors.requestTemperatures();
  currentTemp = sensors.getTempCByIndex(0);
}

void sendSensor()
{
  readSensor();
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(VPIN_currentTemp, currentTemp);
  Blynk.virtualWrite(VPIN_Text, "IOT");
}

void getRelayState()
{
  //Serial.println("reading data from NVS");
  modeState = pref.getBool("Mode", 0);
  Blynk.virtualWrite(VPIN_Mode, modeState);
  delay(200);
  heaterState = pref.getBool("Heater", 0);
  digitalWrite(HEATING_PIN, !heaterState);
  Blynk.virtualWrite(VPIN_heaterbtn, heaterState);
  delay(200);
  coolerState = pref.getBool("Cooler", 0);
  digitalWrite(COOLING_PIN, !coolerState);
  Blynk.virtualWrite(VPIN_coolerbtn, coolerState);
  delay(200);
  setpoint = pref.getBool("setemp", 0);
  Blynk.virtualWrite(VPIN_setpoint, setpoint);
  delay(200);
  hysteresis = pref.getBool("Hyst", 0);
  Blynk.virtualWrite(VPIN_hysteresis, hysteresis);
  delay(200);
}

void setup()
{
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  //Open namespace in read-write mode
  pref.begin("Relay_State", false);

  pinMode(HEATING_PIN, OUTPUT);
  pinMode(COOLING_PIN, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  pinMode(LEDRED,OUTPUT);
  pinMode(LEDREDALERT,OUTPUT);
  pinMode(LEDGREEN,OUTPUT);
  pinMode(LEDBLUE,OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(35, 0);
  display.println(" IoT ");
  display.setCursor(25, 20);
  display.println(" Temp. ");
  display.setCursor(0, 45);
  display.println("Controller");
  display.display();
  delay(2000); // Pause for 2 seconds
  
  //During Starting all Relays should TURN OFF
  digitalWrite(HEATING_PIN, !heaterState);
  digitalWrite(COOLING_PIN, !coolerState);

  sensors.begin();   // Enabling DS18B20 sensor
  digitalWrite(wifiLed, HIGH);

  //Blynk.begin(auth, ssid, pass);
  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  timer.setInterval(500L, sendSensor); // Sending Sensor Data to Blynk Cloud every 1 second
  Blynk.config(auth);
  delay(500);

  getRelayState(); // Get the last state of Relays and Set values of Temp & Hysteresis

  Blynk.virtualWrite(VPIN_heaterbtn, heaterState);
  Blynk.virtualWrite(VPIN_coolerbtn, coolerState);
  Blynk.virtualWrite(VPIN_setpoint, setpoint);
  Blynk.virtualWrite(VPIN_hysteresis, hysteresis);
}

void loop()
{
  Blynk.run();
  timer.run();

  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(" IoT Temp. Controller ");
  display.display();
  display.setTextSize(3); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 17);
  display.println(currentTemp);
  display.println(" ");
  display.drawRect(90, 17, 5, 5, WHITE);     // put degree symbol ( ° )
  display.setCursor(97, 17);
  display.println("C");
  display.display();

  Serial.println(hysteresis);
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 57);
  display.print("Err: ");
  display.print(hysteresis);
  display.display();
  Serial.println(setpoint);
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(67, 57);
  display.print("Temp: ");
  display.print(setpoint);
  display.display();

  // Check the mode and control the heater and cooler accordingly
  if (modeState == 1) {
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(32, 45);
    display.print("Auto Mode");
    display.display();
    // In auto mode, control the heater and cooler based on the current temperature and setpoint

    if (currentTemp > setpoint + hysteresis) {
      heaterState = 0;
      coolerState = 1;
      digitalWrite(LEDRED,HIGH);
      digitalWrite(LEDGREEN,LOW);
      digitalWrite(LEDBLUE,LOW);
      digitalWrite(HEATING_PIN, !heaterState);
      pref.putBool("Heater", heaterState);
      digitalWrite(COOLING_PIN, !coolerState);
      pref.putBool("Cooler", coolerState);
      Serial.println("Cooler ON");
      Blynk.virtualWrite(VPIN_heaterbtn, heaterState);
      Blynk.virtualWrite(VPIN_coolerbtn, coolerState);
      if (currentTemp > setpoint + 10){
        digitalWrite(LEDREDALERT,HIGH);
        delay(300);
        digitalWrite(LEDREDALERT,LOW);
      }

    } else if (currentTemp < setpoint - hysteresis) {
      digitalWrite(LEDRED,LOW);
      digitalWrite(LEDGREEN,LOW);
      digitalWrite(LEDBLUE,HIGH);
      heaterState = 1;
      coolerState = 0;
      digitalWrite(HEATING_PIN, !heaterState);
      pref.putBool("Heater", heaterState);
      Serial.println("Heater ON");
      digitalWrite(COOLING_PIN, !coolerState);
      pref.putBool("Cooler", coolerState);
      Blynk.virtualWrite(VPIN_heaterbtn, heaterState);
      Blynk.virtualWrite(VPIN_coolerbtn, coolerState);
    } else {
      heaterState = 0;
      coolerState = 0;
      digitalWrite(LEDRED,LOW);
      digitalWrite(LEDGREEN,HIGH);
      digitalWrite(LEDBLUE,LOW);
      digitalWrite(HEATING_PIN, !heaterState);
      pref.putBool("Heater", heaterState);
      Serial.println("Heater OFF");
      digitalWrite(COOLING_PIN, !coolerState);
      pref.putBool("Cooler", coolerState);
      Serial.println("Cooler OFF");
      Blynk.virtualWrite(VPIN_heaterbtn, heaterState);
      Blynk.virtualWrite(VPIN_coolerbtn, coolerState);
    }
  }
  if (modeState == 0)
  {
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(32, 45);
    display.print("Manual Mode");
    display.display();
  }

  delay(500);
}

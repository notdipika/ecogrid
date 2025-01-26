#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "zang";
const char* password = "gauravshah";

// Server URLs
const char* commandServer = "http://192.168.1.100:5000/get_command";
const char* sensorDataServer = "http://192.168.1.100:5000/sensor_data";
const char* alertServer = "http://192.168.1.100:5000/alert";

// Pins
#define ULTRASONIC_TRIG_PIN 19
#define ULTRASONIC_ECHO_PIN 21
#define MQ2_SENSOR_PIN 34
#define PIR_SENSOR_PIN 2
#define LED_PIN 13
#define DHT_PIN 4
#define LIGHT_PIN 23

// Thresholds
int gasThreshold = 35;
int waterLevelThreshold = 10; 

// DHT sensor setup
DHT dht(DHT_PIN, DHT11);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);

  digitalWrite(LIGHT_PIN, LOW); 
  digitalWrite(LED_PIN, LOW);

  dht.begin();
  lcd.begin(16, 2);
  lcd.backlight();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // fetchCommand();
    sendSensorData();
    checkMotion();
    checkWaterLevel();
    fetchCommand();
  } else {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.reconnect();
  }

  displaySensorData();
  delay(10000); // Main loop delay
}

void fetchCommand() {
  HTTPClient http;
  http.begin(commandServer);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String command = http.getString();
    command.trim();
    Serial.println("Received command: " + command);
    handleCommand(command);
  } else {
    Serial.println("Error fetching command");
  }

  http.end();
}

void handleCommand(String command) {
  command.toLowerCase();
  if (command == "turn on the light") {
    digitalWrite(LIGHT_PIN, HIGH);
    Serial.println("Light turned ON");
  } else if (command == "turn off the light") {
    digitalWrite(LIGHT_PIN, LOW);
    Serial.println("Light turned OFF");
  } else {
    Serial.println("Unknown command");
  }
}

void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int gasValue = analogRead(MQ2_SENSOR_PIN);

  if (gasValue > gasThreshold || temperature > 30) {
    sendAlert(gasValue, temperature);
  }

  HTTPClient http;
  http.begin(sensorDataServer);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String httpRequestData = "temperature=" + String(temperature) + "&humidity=" + String(humidity);
  int httpResponseCode = http.POST(httpRequestData);

  if (httpResponseCode > 0) {
    Serial.println("Sensor data sent successfully");
  } else {
    Serial.println("Error sending sensor data");
  }

  http.end();
}

void sendAlert(int gasLevel, float temp) {
  HTTPClient http;
  String alertMessage = "Alert! ";

  if (gasLevel > gasThreshold) {
    alertMessage += "Gas leak detected. ";
  }
  if (temp > 30) {
    alertMessage += "High temperature detected. ";
  }

  http.begin(alertServer);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String httpRequestData = "alert=" + alertMessage;
  int httpResponseCode = http.POST(httpRequestData);

  if (httpResponseCode > 0) {
    Serial.println("Alert sent successfully");
  } else {
    Serial.println("Error sending alert");
  }

  http.end();
}

void checkMotion() {
  int motionDetected = digitalRead(PIR_SENSOR_PIN);
  if (motionDetected == HIGH) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Motion detected: LED ON");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("No motion: LED OFF");
  }
}

void checkWaterLevel() {
  long duration, distance;

  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1;

  if (distance <= waterLevelThreshold) {
    sendWaterAlert();
  }
}

void sendWaterAlert() {
  HTTPClient http;
  String alertMessage = "Alert! Water level detected in the tank.";

  http.begin(alertServer);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String httpRequestData = "alert=" + alertMessage;
  int httpResponseCode = http.POST(httpRequestData);

  if (httpResponseCode > 0) {
    Serial.println("Water alert sent successfully");
  } else {
    Serial.println("Error sending water alert");
  }

  http.end();
}

void displaySensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");
}

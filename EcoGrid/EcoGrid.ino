#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>  // Include the library for the LCD

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* serverName = "http://<Flask_Server_IP>/sensor_data";  // Replace with your Flask server IP

DHT dht(4, DHT11);  // DHT11 sensor on pin 4
int ldrPin = 34;     // LDR sensor connected to pin 34

// MQ2 Smoke sensor setup
#define MQ2_SENSOR_PIN 34  // ADC pin for MQ2 (change as needed)

int gasThreshold = 35;  // Adjust the threshold for gas detection

// LCD setup: I2C address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();
  
  // Initialize LCD
  lcd.begin();
  lcd.backlight();  // Turn on the backlight of the LCD

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  int ldrValue = analogRead(ldrPin);  // Read the LDR value

  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read smoke (gas) level from MQ2 sensor
  int gasValue = analogRead(MQ2_SENSOR_PIN);  // Reading from the gas sensor pin

  // Check for gas and high temperature
  if (gasValue > gasThreshold || temperature > 30) {  // Assuming gas value above threshold or temperature over 30°C triggers an alert
    sendAlert(gasValue, temperature);
  }

  // Display temperature and humidity on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);  // Set cursor to the first line
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);  // Set cursor to the second line
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

  // Send data to Flask server
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "ldr=" + String(ldrValue) + "&temperature=" + String(temperature) + "&humidity=" + String(humidity);
    int httpResponseCode = http.POST(httpRequestData);

    if(httpResponseCode > 0) {
      Serial.println("Data Sent Successfully");
    } else {
      Serial.println("Error sending data");
    }

    http.end();
  }

  delay(10000);  // Delay between readings (10 seconds)
}

void sendAlert(int gasLevel, float temp) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String alertMessage = "Alert! ";

    // Check for gas alert
    if (gasLevel > gasThreshold) {
      alertMessage += "Gas leak detected. ";
    }

    // Check for temperature alert
    if (temp > 30) {
      alertMessage += "High temperature detected. ";
    }

    // Send the alert message via a POST request
    String serverAlertURL = "http://<Flask_Server_IP>/alert";  // Replace with your Flask server's alert endpoint
    http.begin(serverAlertURL);

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
}

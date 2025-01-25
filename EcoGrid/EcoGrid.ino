#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>  

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* serverName =  "http://127.0.0.1/sensor_data";

DHT dht(4, DHT11);    

#define MQ2_SENSOR_PIN 34  

int gasThreshold = 35;  

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();
  lcd.begin(16, 2);
  lcd.backlight();  

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int gasValue = analogRead(MQ2_SENSOR_PIN);  

  if (gasValue > gasThreshold || temperature > 30) {  
    sendAlert(gasValue, temperature);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

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

  delay(10000); 
}

void sendAlert(int gasLevel, float temp) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String alertMessage = "Alert! ";

    if (gasLevel > gasThreshold) {
      alertMessage += "Gas leak detected. ";
    }

    if (temp > 30) {
      alertMessage += "High temperature detected. ";
    }

    String serverAlertURL = "http://127.0.0.1/sensor_data/alert";  
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

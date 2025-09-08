#define BLYNK_TEMPLATE_ID "TMPL3VftnLToC"
#define BLYNK_TEMPLATE_NAME "irrigation"
#define BLYNK_AUTH_TOKEN "LEFjvbGQGa7cvMgh5DWRYEhtCZMLE0N0"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

char auth[] = BLYNK_AUTH_TOKEN;  // Blynk authentication token
char ssid[] = "OnePlus Nord CE 2";  // Your WiFi SSID
char pass[] = "56325632";  // Your WiFi password

BlynkTimer timer;

#define DHTPIN 4  // Pin connected to DHT sensor
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

#define RELAY_PIN D6  // Pin connected to the relay for motor control

// Define soil moisture thresholds for different plants
#define CLAY_SOIL_POTATO_START 7.5
#define CLAY_SOIL_POTATO_STOP 10.0
#define SANDY_SOIL_MILLET_START 4.5
#define SANDY_SOIL_MILLET_STOP 5.5
#define LOAMY_SOIL_WHEAT_START 6.0
#define LOAMY_SOIL_WHEAT_STOP 7.5

// Variables for soil and plant type (will be updated via Blynk app)
String soilType;
String plantType; 

// This function handles the soil type input from the Blynk app
BLYNK_WRITE(V3) {
  soilType = param.asStr();  // Get the soil type from virtual pin V3
  Serial.println("====================================");
  Serial.print("Soil Type set to: ");
  Serial.println(soilType);
  Serial.println("====================================");
}

// This function handles the plant type input from the Blynk app
BLYNK_WRITE(V4) {
  plantType = param.asStr();  // Get the plant type from virtual pin V4
  Serial.println("====================================");
  Serial.print("Plant Type set to: ");
  Serial.println(plantType);
  Serial.println("====================================");
}

void sendSensor() {
  int value = analogRead(A0);  // Read soil moisture sensor
  value = map(value, 400, 1023, 100, 0);  // Convert sensor value to percentage
  
  float h = dht.readHumidity();  // Read humidity from DHT sensor
  float t = dht.readTemperature();  // Read temperature from DHT sensor

  // Check if the DHT sensor is working properly
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Send sensor data to Blynk (app will display it on respective virtual pins)
  Blynk.virtualWrite(V0, value);  // Soil moisture
  Blynk.virtualWrite(V1, t);  // Temperature
  Blynk.virtualWrite(V2, h);  // Humidity
  
  // Print sensor data to the Serial Monitor
  Serial.print("Soil Moisture: ");
  Serial.print(value);
  Serial.print("   Temperature: ");
  Serial.print(t);
  Serial.print("   Humidity: ");
  Serial.println(h);
  
  // Determine the thresholds based on soil type and plant type
  float startThreshold = 0;
  float stopThreshold = 0;
  
  if (soilType == "clay" && plantType == "potato") {
    startThreshold = CLAY_SOIL_POTATO_START;
    stopThreshold = CLAY_SOIL_POTATO_STOP;
  } else if (soilType == "sandy" && plantType == "millet") {
    startThreshold = SANDY_SOIL_MILLET_START;
    stopThreshold = SANDY_SOIL_MILLET_STOP;
  } else if (soilType == "loamy" && plantType == "wheat") {
    startThreshold = LOAMY_SOIL_WHEAT_START;
    stopThreshold = LOAMY_SOIL_WHEAT_STOP;
  }
  
// Control the relay based on soil moisture level and send relay state to Blynk


  if (value < startThreshold) {
    digitalWrite(RELAY_PIN, LOW);  // Turn on the motor
    Blynk.virtualWrite(V5, 1);     // Send "ON" to Blynk
    Serial.println("Motor ON (watering the plants)");
  } else if (value > stopThreshold) {
    digitalWrite(RELAY_PIN, HIGH); // Turn off the motor
    Blynk.virtualWrite(V5, 0);     // Send "OFF" to Blynk
    Serial.println("Motor OFF (sufficient moisture)");
  } else {
    Serial.println("Motor remains unchanged");
  }
}
void setup() {   
  Serial.begin(115200);
  
  Blynk.begin(auth, ssid, pass);  // Connect to Blynk and Wi-Fi
  dht.begin();  // Initialize the DHT sensor
  
  pinMode(RELAY_PIN, OUTPUT);  // Set the relay pin as output
  digitalWrite(RELAY_PIN, HIGH);  // Initialize relay to off (HIGH) state
  
  timer.setInterval(1000L, sendSensor);  // Send sensor data every second
}

void loop() {
  Blynk.run();  // Run Blynk process (maintain connection)
  timer.run();  // Run the timer to send sensor data
}

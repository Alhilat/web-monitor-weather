#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h> // Fixed: Added for HTTPS support
#include <DHT.h>

// --- CONFIGURATION ---
const char* ssid = "YOUR_WIFI_NAME";         // Replace with your WiFi Name
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi Password

// Paste your Google Deployment URL here (keep the quotes)
const char* serverName = "YOUR_DEPLOYMENT_URL_HERE"; 

#define DHTPIN 4      // Pin where DHT11 Data leg is connected
#define DHTTYPE DHT11 // Sensor type
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastTime = 0;
unsigned long timerDelay = 10000; // Send data every 10 seconds

void setup() {
  Serial.begin(115200);
  
  // --- FIX FOR CRASH ---
  // Wait 2 seconds for power to stabilize before turning on WiFi
  delay(2000); 
  Serial.println("\nSystem Starting...");

  // Set WiFi mode explicitly to Station to improve stability
  WiFi.mode(WIFI_STA); 
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");
  
  dht.begin();
}

void loop() {
  // Send data only if time has passed (non-blocking delay)
  if ((millis() - lastTime) > timerDelay) {
    
    if(WiFi.status() == WL_CONNECTED){
      
      float h = dht.readHumidity();
      float t = dht.readTemperature();

      // Check if sensor read failed
      if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }

      // Initialize Secure Client
      WiFiClientSecure client;
      client.setInsecure(); // Essential: Ignore Google's SSL certificate verification
      
      HTTPClient http;
      
      Serial.print("Sending data... ");
      
      // Start connection
      if (http.begin(client, serverName)) {  
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        
        // Prepare data payload
        String httpRequestData = "temperature=" + String(t) + "&humidity=" + String(h);
        
        // Send POST request
        int httpResponseCode = http.POST(httpRequestData);

        // Check result
        if (httpResponseCode > 0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
        } else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        http.end(); // Free resources
      } else {
        Serial.println("Unable to connect to Google Servers");
      }
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

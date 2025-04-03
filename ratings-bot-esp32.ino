// Include required libraries
#include <WiFi.h>          // For WiFi connectivity
#include <HTTPClient.h>     // For making HTTP requests
#include <ArduinoJson.h>    // For parsing JSON responses
#include <FastLED.h>        // For controlling the NeoPixel LED
#include <ESP32Servo.h>     // For controlling the servo motor

// Network configuration
const char* ssid = "PROJECTHUB Coworking";
const char* password = "OctobeR!2024";
const char* apiEndpoint = "http://192.168.86.47:5000/api/latest_feedback_id";

// Pin definitions
#define NEOPIXEL_PIN 38    // Data pin for NeoPixel LED
#define NUM_PIXELS 1       // Number of NeoPixel LEDs (just one)
#define SERVO_PIN 8        // PWM pin for servo control
#define TRIG_PIN 15        // Ultrasonic sensor trigger pin
#define ECHO_PIN 18        // Ultrasonic sensor echo pin

// Global objects and variables
CRGB leds[NUM_PIXELS];     // Array to hold LED color data
Servo dispenseServo;       // Servo motor object
int angle = 10;            // Current servo angle
long previousId = -1;      // Tracks the last feedback ID to detect changes

// Measures distance using ultrasonic sensor
// Returns distance in centimeters
long getDistance() {
  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure the response
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;  // Convert time to distance in cm
}

// Sets the LED color based on distance
// Red when far (>8cm), darker pink when close
void setDefaultColor() {
  long distance = getDistance();
  leds[0] = distance > 8 ? CRGB::Red : CRGB(128, 20, 40);
  FastLED.show();
}

// Blinks the LED in specified color
// Parameters:
//   color: RGB color to blink
//   times: number of blinks (default: 1)
//   delayMs: duration of each blink state (default: 200ms)
void blinkLED(CRGB color, int times = 1, int delayMs = 200) {
  for(int i = 0; i < times; i++) {
    leds[0] = color;
    FastLED.show();
    delay(delayMs);
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(delayMs);
  }
  setDefaultColor();
}

// Triggers the servo to dispense by rotating full range
// Makes a full 180-degree rotation and back
void triggerDispense() {
  // Rotate from 180 to 0 degrees
  for (angle = 180; angle > 0; angle--) {
    dispenseServo.write(angle);
    delay(15);  // Small delay for smooth movement
  }
  // Rotate back from 0 to 180 degrees
  for (angle = 0; angle < 180; angle++) {
    dispenseServo.write(angle);
    delay(15);
  }
}

// Initial setup function - runs once at startup
void setup() {
  Serial.begin(115200);    // Initialize serial communication
  
  // Configure ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize NeoPixel LED
  FastLED.addLeds<WS2812, NEOPIXEL_PIN, GRB>(leds, NUM_PIXELS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(25);  // Set to 25% brightness
  setDefaultColor();
  
  // Initialize servo motor
  dispenseServo.setPeriodHertz(50);    // Standard 50Hz servo frequency
  dispenseServo.attach(SERVO_PIN);
  dispenseServo.write(10);             // Set to initial position
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Print connection success and IP address
  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
  blinkLED(CRGB::Green, 3, 100);       // Blink green to indicate successful connection
}

// Main loop function - runs repeatedly
void loop() {
  // Monitor distance and update serial output
  long distance = getDistance();
  Serial.printf("Distance: %ld cm\n", distance);
  setDefaultColor();
  
  // Check WiFi connection and handle API communication
  if (WiFi.status() == WL_CONNECTED) {
    blinkLED(CRGB::Blue, 1, 50);       // Quick blue blink to show active polling
    
    // Make HTTP request to API
    HTTPClient http;
    http.begin(apiEndpoint);
    int httpResponseCode = http.GET();
    
    // Handle successful HTTP response
    if (httpResponseCode == 200) {
      String payload = http.getString();
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      // Process JSON response
      if (!error) {
        long latestId = doc["latest_id"];
        Serial.printf("Latest ID: %ld\n", latestId);
        
        // If new feedback detected, trigger dispenser
        if (latestId != previousId) {
          blinkLED(CRGB::Green, 10, 100);  // Blink green to indicate new feedback
          triggerDispense();
          previousId = latestId;
        }
      } else {
        Serial.println("JSON parsing failed");
        blinkLED(CRGB::Red, 2, 100);     // Blink red to indicate error
      }
    } else {
      Serial.printf("HTTP Request failed: %d\n", httpResponseCode);
      blinkLED(CRGB::Red, 2, 100);       // Blink red to indicate error
    }
    
    http.end();
    delay(1000);                         // Wait 1 second before next poll
  } else {
    // Handle WiFi disconnection
    Serial.println("WiFi disconnected - reconnecting...");
    blinkLED(CRGB::Orange, 2, 100);      // Blink orange to indicate WiFi issue
    WiFi.begin(ssid, password);
    delay(1000);
  }
}

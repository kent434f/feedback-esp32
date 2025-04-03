# Ratings Bot ESP32

An ESP32-based feedback response system that dispenses rewards based on new feedback submissions. The system monitors reward container levels and handles dispensing automatically.

## Hardware Requirements

- ESP32-S3 DevKit
- WS2812B NeoPixel LED
- SG90 Servo Motor
- HC-SR04 Ultrasonic Distance Sensor
- Reward container with mounting for ultrasonic sensor

## Pin Connections (All GPIO pins)

### NeoPixel LED
- Data Pin: GPIO 38

### Servo Motor
- PWM Pin: GPIO 8

### Ultrasonic Sensor (Container Level Detection)
- Trigger Pin: GPIO 15
- Echo Pin: GPIO 18

## Container Level Monitoring

The ultrasonic sensor is mounted at the top of the reward container and measures the distance to the rewards:
- Distance > 8cm: Container is empty or near-empty (LED turns RED)
- Distance ≤ 8cm: Container has sufficient rewards (LED turns Dark Pink)

## LED Color Indicators

The NeoPixel LED provides visual feedback about the system's status:
- **Red**: Container empty/near-empty (distance > 8cm)
- **Dark Pink**: Container has sufficient rewards (distance ≤ 8cm)
- **Green (3 blinks)**: Successful WiFi connection
- **Blue (quick blink)**: API polling in progress
- **Green (10 blinks)**: New feedback detected, dispensing reward
- **Red (2 blinks)**: Error (HTTP request or JSON parsing)
- **Orange (2 blinks)**: WiFi connection issue

## Network Configuration

The device connects to WiFi and polls an API endpoint for new feedback:
- Network: PROJECTHUB Coworking
- API Endpoint: `http://192.168.86.47:5000/api/latest_feedback_id`

## API Response Format

```json
{
    "status": "success",
    "latest_id": 42,
    "timestamp": "2024-03-14T12:34:56.789123"
}
```

## Dependencies

Required Arduino libraries:
- WiFi.h - For WiFi connectivity
- HTTPClient.h - For making HTTP requests
- ArduinoJson.h - For parsing JSON responses
- FastLED.h - For controlling the NeoPixel LED
- ESP32Servo.h - For controlling the servo motor

## Operation

1. On startup, the device:
   - Initializes all components
   - Connects to WiFi
   - Sets the servo to initial position
   - Indicates successful connection with green LED blinks

2. During operation, the device:
   - Continuously monitors reward container level using ultrasonic sensor
   - Updates LED color based on container level
   - Polls the API endpoint every second
   - Triggers the servo when new feedback is detected
   - Provides visual feedback through LED colors

3. Error handling:
   - Automatically attempts to reconnect if WiFi connection is lost
   - Indicates various error states through LED color patterns
   - Logs errors to Serial monitor (115200 baud)

## Container Setup

The ultrasonic sensor should be mounted at the top of the reward container:
- Mount the sensor facing downward
- Ensure the sensor has a clear line of sight to the rewards
- The 8cm threshold can be adjusted in the code based on container size

## Reference

For ESP32-S3 DevKit pin layout, refer to:
[ESP32-S3 DevKit Pinout Diagram](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/_images/ESP32-S3_DevKitC-1_pinlayout_v1.1.jpg)

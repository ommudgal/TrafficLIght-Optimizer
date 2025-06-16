#include <LiquidCrystal.h>

// LCD pin definitions
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// LED pin definitions
const int redPin = A3;      // Red LED
const int yellowPin = A4;   // Yellow LED
const int greenPin = A5;    // Green LED

int vehicleCount = 0;       // To store the vehicle count received from ESP32
int lightTiming = 0;        // Timing for traffic lights
int countdown = 0;          // Countdown timer
String esp32IP = "";        // To store the ESP32's IP address

bool lastButtonState = LOW; // Last button state
unsigned long lastDebounceTime = 0; // For debounce logic
const unsigned long debounceDelay = 50; // Debounce delay

void setup() {
  // Initialize LCD
  lcd.begin(16, 2);  // 16x2 LCD
  lcd.print("Traffic System");

  // Initialize LED pins
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  // Start serial communication
  Serial.begin(115200);
}

void loop() {
  // Check for data from ESP32
  if (Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n');  // Read data

    // Parse vehicle count and IP address
    int commaIndex = receivedData.indexOf(',');
    if (commaIndex != -1) {
      String countStr = receivedData.substring(0, commaIndex);
      String ipStr = receivedData.substring(commaIndex + 1);

      vehicleCount = countStr.toInt();  // Convert to integer
      esp32IP = ipStr;  // Store IP address
      lightTiming = map(vehicleCount, 0, 100, 5, 20);  // Calculate timing in seconds
    }
  }

  // Traffic light logic
  static unsigned long lastUpdate = millis();
  static int currentLight = 0; // 0 = Red, 1 = Yellow, 2 = Green
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    countdown--; // Decrease countdown timer
    if (countdown <= 0) {
      currentLight = (currentLight + 1) % 3; // Cycle to the next light
      switch (currentLight) {
        case 0: countdown = lightTiming; break;  // Red light
        case 1: countdown = 2; break;            // Yellow light
        case 2: countdown = lightTiming; break;  // Green light
      }
    }
  }

  // Update LEDs
  switch (currentLight) {
    case 0: // Red light
      analogWrite(redPin, 255);
      analogWrite(yellowPin, 0);
      analogWrite(greenPin, 0);
      break;
    case 1: // Yellow light
      analogWrite(redPin, 0);
      analogWrite(yellowPin, 255);
      analogWrite(greenPin, 0);
      break;
    case 2: // Green light
      analogWrite(redPin, 0);
      analogWrite(yellowPin, 0);
      analogWrite(greenPin, 255);
      break;
  }

  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.print(esp32IP);
  
  lcd.setCursor(0, 1);
  lcd.print("Car:");
  lcd.print(vehicleCount);
  lcd.print(" Time:");
  lcd.print(countdown);

  delay(100); // Small delay for LCD update
}

#include <avr/io.h>

const int fanPin = 9;      // PWM output pin for the fan (Timer 1)
const int ledPin = 13;     // Onboard LED pin

// Watchdog and Startup tracking variables
const unsigned long WATCHDOG_TIMEOUT = 60000; // 60 seconds in milliseconds

unsigned long lastCommunicationTime = 0;

void setup() {
  pinMode(fanPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  // Turn LED ON at boot to signify the board is waiting for the ESP01
  digitalWrite(ledPin, HIGH); 
  
  // Configure Timer 1 for 25kHz Fast PWM on 8MHz Clock
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS10); // No prescaler
  
  ICR1 = 319; // Exact top value for 25kHz on an 8MHz clock
  
  // Initialize Hardware Serial Port at 9600 Baud, one way, so esp tx --> arduino rx
  Serial.begin(9600);
  
  // Set initial speed to 100%
  setFanDutyCycle(100);
  // Prime the communication timer so we do not go to watchdog right away at boot
  lastCommunicationTime = millis();
}

void setFanDutyCycle(int dutyPercent) {
  dutyPercent = constrain(dutyPercent, 0, 100);
  OCR1A = (dutyPercent * 319L) / 100; // 8MHz hardware math
}

void loop() {
  // 1. Check for incoming Serial data from ESP01
  if (Serial.available() > 0) {
    int receivedValue = Serial.read(); // Read the incoming raw byte
    
    if (receivedValue >= 0 && receivedValue <= 100) {
      // Reset safety timer
      lastCommunicationTime = millis();
      
      // Clear the startup LED state on successful message
      digitalWrite(ledPin, LOW); 
      
      setFanDutyCycle(receivedValue);
    }
  }

  // 2. Safety Watchdog Check
  // Only trigger if we have already received the first communication.
  // This prevents a slow ESP01 boot/Wi-Fi connection from accidentally triggering the fail-safe.
  if ((millis() - lastCommunicationTime) > WATCHDOG_TIMEOUT) {
    setFanDutyCycle(100); // Fail-safe: Blast the fan at full speed

    // Fail-safe Warning: Flash aggressively to indicate lost communication
    digitalWrite(ledPin, HIGH); delay(50);
    digitalWrite(ledPin, LOW);  delay(50);
  } else {
    // Normal operation: No delays, no LED power usage, highly responsive
    delay(10); 
  }
}
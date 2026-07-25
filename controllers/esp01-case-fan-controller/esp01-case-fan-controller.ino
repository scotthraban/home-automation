#include <ESP8266WiFi.h>

const int pwmPin = 5;    // Pin D1 - Connect to BOTH fan PWM wires
const int tachPin1 = 14; // Pin D5 - Connect to Fan 1 Tach wire
const int tachPin2 = 12; // Pin D6 - Connect to Fan 2 Tach wire

// Volatile variables modified inside the hardware interrupts
volatile unsigned long pulseCount1 = 0;
volatile unsigned long pulseCount2 = 0;

unsigned long lastMillis = 0;

// Interrupt Service Routines (ISRs) stored in RAM for maximum speed
void ICACHE_RAM_ATTR handleTach1() { pulseCount1++; }
void ICACHE_RAM_ATTR handleTach2() { pulseCount2++; }

void setup() {
  Serial.begin(115200);
  
  // Configure PWM Output
  pinMode(pwmPin, OUTPUT);
  analogWriteFreq(25000); // Set to 25kHz PC fan standard
  
  // Configure Tach Inputs with microcontoller internal pullups
  // Note: External 10k resistors to 3.3V are still highly recommended
  pinMode(tachPin1, INPUT_PULLUP);
  pinMode(tachPin2, INPUT_PULLUP);
  
  // Attach hardware interrupts to listen for pulses
  attachInterrupt(digitalPinToInterrupt(tachPin1), handleTach1, FALLING);
  attachInterrupt(digitalPinToInterrupt(tachPin2), handleTach2, FALLING);
}

void loop() {
  // Read and calculate RPM every 2 seconds
  if (millis() - lastMillis >= 2000) {
    noInterrupts(); // Temporarily pause interrupts to safely copy data
    unsigned long count1 = pulseCount1;
    unsigned long count2 = pulseCount2;
    pulseCount1 = 0; // Reset counters
    pulseCount2 = 0;
    interrupts();   // Turn interrupts back on
    
    unsigned long elapsedSeconds = (millis() - lastMillis) / 1000;
    lastMillis = millis();
    
    // PC fans generate 2 pulses per full 360-degree blade revolution
    // Formula: (Pulses / 2) * (60 seconds / elapsed time)
    float rpm1 = (count1 / 2.0) * (60.0 / elapsedSeconds);
    float rpm2 = (count2 / 2.0) * (60.0 / elapsedSeconds);
    
    Serial.print("Fan 1: "); Serial.print(rpm1); Serial.print(" RPM | ");
    Serial.print("Fan 2: "); Serial.print(rpm2); Serial.println(" RPM");
  }
  
  // Your Prometheus fetching and adjustFanSpeed() code goes here...
}
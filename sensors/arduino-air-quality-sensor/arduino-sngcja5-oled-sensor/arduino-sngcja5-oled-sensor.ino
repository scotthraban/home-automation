
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_Particle_Sensor_SN-GCJA5_Arduino_Library.h"

#define CURRENT_INTERVAL 30000

#define SAMPLE_COUNT 96
#define SAMPLE_INTERVAL 900000

#define DISPLAY_PERIOD 5000

Adafruit_SSD1306 display(128, 64, &Wire, -1);

SFE_PARTICLE_SENSOR airSensor;

int current = -1;
long currentTaken = 0;

int samples[SAMPLE_COUNT];
long sampleTaken = 0;
int currentSampleIndex = SAMPLE_COUNT - 1;

long displayStarted =  0;
bool displayStopped = true; 

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  bool airSensorStatus = airSensor.begin();
  
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    samples[i] = -1;
  }
}

void loop() {

  if (millis() > (currentTaken + CURRENT_INTERVAL)) {
    current = getPm25Aqi();
    samples[currentSampleIndex] = current;

    if (millis() > (sampleTaken + SAMPLE_INTERVAL)) {
      currentSampleIndex = (currentSampleIndex + 1) % SAMPLE_COUNT;
      sampleTaken = millis();
    }
  }

  doDisplay();
}

void doDisplay() {

  if (!displayStopped && millis() > (displayStarted + DISPLAY_PERIOD)) {
    stopDisplay();
  }

  // TODO: Replace with button check...
  if (millis() > (displayStarted + (DISPLAY_PERIOD * 2))) {
    startDisplay();
  }
}

void startDisplay() {

  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 24);
  display.cp437(true);

  display.print(current);

  display.display();

  display.clearDisplay();

  displayStarted = millis();
  displayStopped = false;
}

void stopDisplay() {
  display.clearDisplay();
  display.fillScreen(SSD1306_BLACK);
  display.display();
  display.clearDisplay();

  displayStopped = true;
}

//void displayDebug(String msg) {
//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setTextColor(SSD1306_WHITE);
//  display.setCursor(0, 0);
//  display.cp437(true);
//  display.write(msg.c_str());
//  display.display();
//  display.clearDisplay();
//
//  delay(1000);
//}

int getPm25Aqi() {
  int pm25 = airSensor.getPM2_5();
  currentTaken = millis();
  return pm25;
}

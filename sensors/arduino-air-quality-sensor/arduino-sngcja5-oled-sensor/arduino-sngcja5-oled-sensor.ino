
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_Particle_Sensor_SN-GCJA5_Arduino_Library.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define DISPLAY_PERIOD 5000

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SFE_PARTICLE_SENSOR airSensor;

long sampleTaken = 0;

int samplePm25[60];
int samplePm10[60];
int sampleIndex = 0;

int averagePm25[30];
int averagePm10[30];
int averageIndex;

long displayStarted =  0;
bool displayStopped = true; 

void setup() {
  
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  airSensor.begin();
  
  for (int i = 0; i < 60; i++) {
    samplePm25[i] = -1;
    samplePm10[i] = -1;
  }
  for (int i = 0; i < 30; i++) {
    averagePm25[i] = -1;
    averagePm10[i] = -1;
  }
}

void loop() {
  if (millis() > (sampleTaken + 1000)) {
    samplePm25[sampleIndex] = getPm25Aqi();
    samplePm10[sampleIndex] = getPm10Aqi();

    sampleIndex = (sampleIndex +1) % 60;

    if (sampleIndex == 0) {
      averagePm25[averageIndex] = arrayAverage(samplePm25, 60);
      averagePm10[averageIndex] = arrayAverage(samplePm10, 60);

      averageIndex = (averageIndex + 1) % 30;
    }

    sampleTaken = millis();
  }

  doDisplay();
}

int arrayAverage(int arr[], int len) {
  int sum = 0;
  int count = 0;
  for (int i = 0; i < len; i++) {
    if (arr[i] != -1) {
      sum += arr[i];
      count++;
    }
  }
  
  if (count > 0) {
    return sum / count;
  } else {
    return -1;
  }
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
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  display.setTextSize(2);
  display.setCursor(0, 0);

  // We get about ten characters per line
  
  printPadded(arrayAverage(samplePm25, 60));
  display.println(" 2.5/1");

  printPadded(arrayAverage(samplePm10, 60));
  display.println("  10/1");

  printPadded(arrayAverage(averagePm25, 30));
  display.println(" 2.5/30");

  display.display();

  display.clearDisplay();

  displayStarted = millis();
  displayStopped = false;
}

void printPadded(int aqi) {
  if (aqi < 10) {
    display.print("  ");
  } else if (aqi < 100) {
    display.print(" ");
  }

  display.print(aqi);
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
  return calculatePm25Aqi(airSensor.getPM2_5());
}

int calculatePm25Aqi(float pm2_5) {
  // https://forum.airnowtech.org/t/the-aqi-equation/169

  float truncated = trunc(pm2_5 * 10.0) / 10.0;

  if (truncated >= 250.5) {
    return calculateAqi(truncated, 250.5, 500.4, 301, 500);
  } else if (truncated >= 150.5) {
    return calculateAqi(truncated, 150.5, 250.4, 201, 300);
  } else if (truncated >= 55.5) {
    return calculateAqi(truncated, 55.5, 150.4, 151, 200);
  } else if (truncated >= 35.5) {
    return calculateAqi(truncated, 35.5, 55.4, 101, 150);
  } else if (truncated >= 12.1) {
    return calculateAqi(truncated, 12.1, 35.4, 51, 100);
  } else {
    return calculateAqi(truncated, 0.0, 12.0, 0, 50);
  }
}

int getPm10Aqi() {
  return calculatePm10Aqi(airSensor.getPM10());
}

int calculatePm10Aqi(float pm10) {
  // https://forum.airnowtech.org/t/the-aqi-equation/169

  float truncated = trunc(pm10);

  if (truncated >= 425.0) {
    return calculateAqi(truncated, 425.0, 604.0, 301, 500);
  } else if (truncated >= 355.0) {
    return calculateAqi(truncated, 355.0, 424.0, 201, 300);
  } else if (truncated >= 255.0) {
    return calculateAqi(truncated, 255.0, 354.0, 151, 200);
  } else if (truncated >= 155.0) {
    return calculateAqi(truncated, 155.0, 254.0, 101, 150);
  } else if (truncated >= 55.0) {
    return calculateAqi(truncated, 55.0, 154.0, 51, 100);
  } else {
    return calculateAqi(truncated, 0.0, 54.0, 0, 50);
  }
}

int calculateAqi(float concI, float concL, float concH, int aqiL, int aqiH) {
  return (((aqiH - aqiL) / (concH - concL)) * (concI - concL)) + aqiL;
}

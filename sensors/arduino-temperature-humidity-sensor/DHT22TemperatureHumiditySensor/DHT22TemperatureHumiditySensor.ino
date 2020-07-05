#include <DHT.h>
#include <MySensor.h>  

#define SDEBUG 0
#define SEND_ONLY_CHANGES 0 // Send temperature only if changed? 1 = Yes 0 = No
#define SEND_STATUS 1 // whether to send sensor debug status
#define SLEEP_TIME 30000 // Sleep time between reads (in milliseconds)

#define DHTPIN 7
#define DHTTYPE DHT22

#define TEMP_ID 0
#define HUM_ID 1
#define STATUS_ID 2

DHT dht(DHTPIN, DHTTYPE);
MySensor gw;
MyMessage msg(0, 0);

float lastTemperature = 0.0;
float lastHumidity = 0.0;

void setup()  
{
  #if SDEBUG == 1
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("DHT22 Debugging:");
  #endif

  // Initialize DHT22 sensor
  dht.begin();

  // Startup and initialize MySensors library. Set callback for incoming messages. 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("DHT22 Temp/Hum", "1.1");

  gw.present(TEMP_ID, S_TEMP);
  gw.present(HUM_ID, S_HUM);
}


void loop()     
{
  // Process incoming messages (like config from server)
  gw.wait(2000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(!gw.getConfig().isMetric);

  #if SDEBUG == 1
  Serial.print("N: ");
  Serial.print(gw.getNodeId());
  Serial.print(", T: ");
  Serial.print(temperature);
  Serial.print(", H: ");
  Serial.print(humidity);
  Serial.println(".");
  #endif

  boolean success = !isnan(temperature) && !isnan(humidity);
  if (!success) {
    temperature = lastTemperature;
    humidity = lastHumidity;
  }

  #if SEND_STATUS == 1
  gw.send(msg.setSensor(STATUS_ID).setType(40).set(success ? "OK" : "ERROR"));
  #endif

  // Only send data if temperature has changed and no error
  #if SEND_ONLY_CHANGES == 1
  if (lastTemperature != temperature || lastHumidity != humidity) {
  #else
  {
  #endif
 
    #if SDEBUG == 1
    Serial.println("Sending...");
    #endif

    // Send in the new readings
    gw.send(msg.setSensor(TEMP_ID).setType(V_TEMP).set(temperature, 1));
    gw.send(msg.setSensor(HUM_ID).setType(V_HUM).set(humidity, 1));

    // Save new readings for next compare
    lastTemperature = temperature;
    lastHumidity = humidity;
  }

  // TODO: Send battery

  gw.sleep(SLEEP_TIME);
}

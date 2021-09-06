import os
import sys
import traceback
import json
import time
import requests
import aqi
import paho.mqtt.client as mqtt

CONNECTED = False
DEBUG = False

def onConnect(client, userdata, flags, rc):
    global CONNECTED
    CONNECTED = True


def fetch(id) :
    r = requests.get("https://www.purpleair.com/json?show=" + id, timeout = 5)

    r.raise_for_status()

    data = json.loads(r.text)["results"][0]
    paName = data["Label"]
    paLabel = data["Label"].replace(" ", "_").replace("/", "_").replace("-", "_").replace("(", "").replace(")", "")
    pm25 = json.loads(data["Stats"])["v1"]
    paAqi = float(aqi.to_iaqi(aqi.POLLUTANT_PM25, pm25, algo=aqi.ALGO_EPA))
    paTemp = float(data["temp_f"])
    paHumidity = float(data["humidity"])
    return (paName, paLabel, paAqi, paTemp, paHumidity)


def publishMeasurement(client, id, name, label, unit, value):

    config_topic = "homeassistant/sensor/" + label + "/config"

    state_topic = "openhab/in/" + label + "/state"

    availability_topic = "openhab/in/" + label + "/status"

    config = { "name": name,
               "state_topic": state_topic,
               "availability_topic": availability_topic
             }
    if (unit):
        config["unit_of_measurement"] = unit

    if (DEBUG):
        print(id, label, "config", config_topic, json.dumps(config))
    else:
        info = client.publish(config_topic, json.dumps(config), retain = True)
        info.wait_for_publish()

    if (DEBUG):
        print(id, label, "status", availability_topic, "online")
    else:
        info = client.publish(availability_topic, "online")
        info.wait_for_publish()

    if (DEBUG):
        print(id, label, "state", state_topic, value)
    else:
        info = client.publish(state_topic, value)
        info.wait_for_publish()


def publish(client, id, data):
    paName, paLabel, paAqi, paTemp, paHumidity = data

    publishMeasurement(client, id, paName + " AQI", paLabel + "_Aqi", "PM2.5", paAqi)

    publishMeasurement(client, id, paName + " Temperature", paLabel + "_Temperature", "°F", paTemp)

    publishMeasurement(client, id, paName + " Humidity", paLabel + "_Humidity", None, paHumidity)


def fetchAndPublish(client, id):
    try:
        data = fetch(id)
        publish(client, id, data)
    except:
        print("Error for id " + id);


def fetchAndPublishAll(client):
    f = open("etc/ids.json", "r")
    ids = json.loads(f.read())
    f.close()

    for id in ids:
        fetchAndPublish(client, id)

try:

    if (os.getenv("DEBUG")):
        DEBUG = True

    client = mqtt.Client(client_id="purple-air-to-openhab")
    client.username_pw_set(os.getenv("PURPLE_AIR_MQTT_USERNAME"), os.getenv("PURPLE_AIR_MQTT_PASSWORD"))

    client.on_connect = onConnect

    client.connect("mosquitto")

    client.loop_start()

    start = time.mktime(time.localtime())

    while not CONNECTED and (time.mktime(time.localtime()) - start) < 10.0:
        time.sleep(0.250)

    fetchAndPublishAll(client)

    client.loop_stop()
    client.disconnect()
except:
    traceback.print_exc()
    sys.exit(-1)

sys.exit(0)
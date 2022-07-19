import os
import sys
import traceback
import json
import time
import datetime
import requests
import aqi
import paho.mqtt.client as mqtt

CONNECTED = False
DEBUG = False

def onConnect(client, userdata, flags, rc):
    global CONNECTED
    CONNECTED = True


def fetch(group_id):
    api_key = os.getenv("PURPLE_AIR_API_KEY")

    r = requests.get(
        "https://api.purpleair.com/v1/groups/{}/members".format(group_id),
        params = { "fields" : "name,pm2.5,temperature,humidity,last_seen" },
        headers = { "X-API-Key" : api_key },
        timeout = 5)

    r.raise_for_status()

    response = json.loads(r.text)
    fields = response["fields"]
    datas = response["data"]

    paData = []
    for data in datas:
        paId = data[fields.index("sensor_index")]
        paName = data[fields.index("name")]
        paLabel = data[fields.index("name")].replace(" ", "_").replace("/", "_").replace("-", "_").replace("(", "").replace(")", "")
        pm25 = data[fields.index("pm2.5")]
        paAqi = float(aqi.to_iaqi(aqi.POLLUTANT_PM25, pm25, algo=aqi.ALGO_EPA))
        paTemp = float(data[fields.index("temperature")])
        paHumidity = float(data[fields.index("humidity")])
        paData.append((paId, paName, paLabel, paAqi, paTemp, paHumidity))
        if DEBUG or datetime.datetime.timestamp(datetime.datetime.now()) - data[fields.index("last_seen")] > 86400:
            print("Sensor id {} last seen at {}"
                .format(paId, datetime.datetime.fromtimestamp(data[fields.index("last_seen")])))

    return paData


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


def publish(client, data):
    paId, paName, paLabel, paAqi, paTemp, paHumidity = data

    publishMeasurement(client, paId, paName + " AQI", paLabel + "_Aqi", "PM2.5", paAqi)

    publishMeasurement(client, paId, paName + " Temperature", paLabel + "_Temperature", "°F", paTemp)

    publishMeasurement(client, paId, paName + " Humidity", paLabel + "_Humidity", None, paHumidity)


def fetchAndPublish(client, group_id):
    try:
        datas = fetch(group_id)
        for data in datas:
            publish(client, data)
    except:
        traceback.print_exc()
        print("Error for group id " + group_id)


def fetchAndPublishAll(client):
    f = open("etc/ids.json", "r")
    group_ids = json.loads(f.read())
    f.close()

    for group_id in group_ids:
        fetchAndPublish(client, group_id)

try:

    if (os.getenv("DEBUG")):
        DEBUG = True

    client = None
    if not DEBUG:
        client = mqtt.Client(client_id="purple-air-to-openhab")
        client.username_pw_set(os.getenv("PURPLE_AIR_MQTT_USERNAME"), os.getenv("PURPLE_AIR_MQTT_PASSWORD"))

        client.on_connect = onConnect

        client.connect("mosquitto")

        client.loop_start()

        start = time.mktime(time.localtime())

        while not CONNECTED and (time.mktime(time.localtime()) - start) < 10.0:
            time.sleep(0.250)

    fetchAndPublishAll(client)

    if not DEBUG:
        client.loop_stop()
        client.disconnect()
except:
    traceback.print_exc()
    sys.exit(-1)

sys.exit(0)
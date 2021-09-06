# Purple Air to openHAB over MQTT

This script reads from the [PurpleAir](https://www.purpleair.com/) API, gathering data from a user defined set of nodes, and sends the data over MQTT, in my case to my openHAB instance.

The MQTT messaging is leveraging the HomeAssistant format, which openHAB supports, and allows these nodes to be auto detected and placed into the openHAB inbox for easy inclusion.

## Building

I use the build.sh file included, since I need to allow this to run on arm64 (Raspberry PI), and deploy this to my local Docker registry, so I run a multi-arch build using buildx, YMMV, there is nothing preventing this from being build using just the `docker build` command.

## Configuration

Since this script was designed to be run in Docker container, it is pretty opinionated about where it gets its configuration from.

The script expects a file containing the ids of the nodes to be provided in a JSON file located at `/purple-air/etc/ids.json`. The format of the file should be:

```json
[
    "12345",
    "23456",
    "34567"
]
```

Credentails to connect to MQTT should be provided using two environment variables:
 - `PURPLE_AIR_MQTT_USERNAME`
 - `PURPLE_AIR_MQTT_PASSWORD`

## Runtime

I personally run this as a CronJob K8s, running the container every 5 minutes (please be nice to the free PurpleAir service!). I provide the ids.json file as a ConfigMap.
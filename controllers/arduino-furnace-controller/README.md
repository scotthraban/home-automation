# Arduino MySensors Furnace Controller

This controller was the first version of a furnace controller that I created. It ran on an Arduino, leveraging the MySensors libraries to communicate over nRF24 series radios back to another Arduino that was running a serial gateway to talk to the openHAB server.

My biggest problem with MySensors was that every so often, the radios just would not talk reliably (probably my fault), and I never got around to getting the serial gateway off of the breadboard (definitely my fault). So I eventually switched to using ESP8266 board, which simplifies things quite a bit since on one board you have the microcontroller and the radio (Wifi). I have found this to be much more reliable for me.

One valuable thing that I learned from this project, that I carried forward to the ESP8266 version was that you should always fail to off. If no update is received in a reasonable amount of time, shut off the furnance. This prevents your house from getting heated up to 88 degrees in the middle of winter. True story.

## Software

[Arduino sketch for Arduino Nano](FurnaceController)

## Wiring Schematic

I did not create one, much to my shame. It was not too much more than what they describe on the MySensors site, the only custom bit was controlling a transistor to control an isolation relay to control the 24v alternating current for turning the furnace on/off.

## Circuit Board

N/A - rats nest on a prototype board. Nothing to be proud of,

## References
 - [MySensors](https://mysensors.org)

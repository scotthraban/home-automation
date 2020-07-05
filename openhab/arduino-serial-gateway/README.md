# Arduino MySensors Serial Gateway

Since switching to using ESP8266 microcontrollers, this gateway is no longer needed.

This gateway was used to communicate with openHAB using serial communication over USB, using the Serial Gateway code from MySensors. I honestly do not recall if I had to make any changes to this code, or if I was able to just copy the MySensors code.

I am sure it is no fault of MySensors, I never was really satisfied with the reliability of the communication channel, which could likely has something to do with my never having actually soldered together the gateway for real, it just ran on a breadboard.

But I have switched to ESP8266, one board for communication and code, simpler, and, for me, more reliable.

## Software

[Arduino sketch for serial gateway](SerialGateway)

# Wiring Schematic

Do yourself a favor and follow the MySensors instructions.

# Circuit Board

Like I said, I never got off a breadboard...

## References
 - [MySensors Serial Gateway](https://mysensors.org/build/serial_gateway)

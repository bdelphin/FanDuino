# FanDuino

FanDuino is a simple DC fan controller for Arduino.
Designed to regulate 3pin PC fans with PWM.

## Materiel needed

To create your own, you'll need :
- an Arduino board (obviously). Tested with a Nano clone, should work with Uno and similar boards.
- an N-channel MOSFET, I've used an IRLB3034. It's kind of an overkill choice, but since I append to have some chinese counterfeits ones laying around ...
- a 10kOhms and a 4.7kOhms resistors.
- a DS18B20 temperature sensor.

## Instructions 

Wiring diagram coming soon.
For the software part, simply burn the sketch to the Arduino board.

## Serial commands available

- GET_FAN : Get current fan speed.
- GET_TEMP : Get current temperature.
- GET_ALL : Get current fan & temperature.
- OVERRIDE(value) : Override temperature profile with supplied 'value', given in %.
- OVERRIDE_OFF : Disable temperature override.
- GET_PROFILE : Get current temperature profile.
- SET_PROFILE(profile) : Set new temperature profile (see below).
- DEFAULT_PROFILE : Revert to 'factory' default temperature profile.

### Temperature profile

Temperature profile is composed of five temperature threshold (in °C) / fan speed (in %) pairs, each element being comma separated.
Example (the default profile): 20,0,25,25,30,50,40,75,50,100
It means that for a temperature below 20°C, fans should be spinning at 0% of their maximum speed. For a temperature below 25°C, fan speed should be 25%. For a temperature below 30°C, fan speed should be 50%, and so on.

## Todo

- Wiring diagram
- Take some pictures
- Design a case
- 3D print the case

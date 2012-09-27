To set an rgb led on the string send the following bytes over serial port to the arduino:

irgb

with:

byte i := index on the string 
byte r := red
byte g := green
byte b := blue


Every 11 messages the arduino writes back an 'a' character.


For communicating with the LED strip:

Pin 2 is the data pin.
Pin 4 is the clock pin.
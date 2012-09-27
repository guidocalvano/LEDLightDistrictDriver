To set an rgb led on the string send the following bytes over serial port to the arduino:

irgb

with:

byte i := index on the string 
byte r := red
byte g := green
byte b := blue


Every 11 messages the arduino writes back an 'a' character.
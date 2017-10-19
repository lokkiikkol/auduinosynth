# auduinosynth
a synth from an arduino uno based on auduino and wavetables, with minimal part count and maximal sounds. presets, pgmchange, aftertouch, etc...
all parameters are controllable by cc-messages, see the #defines in the beginning of the arduino file.

expect some nasty sounds and 8bit heaven!!

a puredata editor to control the synth, save to internal eeprom (19 presets,whohooo!!!), save to harddisk recall from synth etc.

hardware-wise you only need: arduino uno, a jack connector, two 1k resistors and a 10nf capacitor.
connect digital out 3 and 11 (both pwm) via a 1k resistor to a (mono) jack connector tip, connect arduino gnd to jack sleeve.
connect the 10nf cap from gnd to jack tip. 
see here for a pic: http://wiki.audacityteam.org/wiki/Connecting_your_Equipment#Jack.2FTRS_Plugs_.26_Sockets

for midi communication you can flash the mocolufa firmware to your uno's atmega8u2, see here: http://morecatlab.akiba.coocan.jp/lab/index.php/aruino/midi-firmware-for-arduino-uno-moco/?lang=en

this basically makes your uno a class compliant midi device.

for din-midi input you will need additional parts, like an optocoupler and a din-socket, schematic from shruti is in the repository as png.
midi out (for recall from synth function) is only tested via usb-midi but should work fine via din-midi as well.





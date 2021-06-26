RF433send
=========

Uses a RF433Mhz transmitter plugged on an Arduino to send a signal encoded in
various ways.

Goal is to manage the following encodings:
- Tri-bit (normal or inverted)
- Manchester


Installation
------------

Download a zip of this repository, then include it from the Arduino IDE.


Schematic
---------

1. Arduino board. Tested with NANO and UNO.

2. Radio Frequence 433Mhz TRANSMITTER like XY-FST.

Data pin number of which the transmitter is plugged is to be specified when
creating object.


Usage
-----

See [examples/01_send/01_send.ino](examples/01_send/01_send.ino) for an example.


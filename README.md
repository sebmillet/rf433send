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


Link with RCSwitch library
--------------------------

RSwitch library is available in Arduino library manager. It is also available
on github, here:
[https://github.com/sui77/rc-switch/](https://github.com/sui77/rc-switch/)

The example
[examples/04_rcswitch_send/04_rcswitch_send.ino](examples/04_rcswitch_send/04_rcswitch_send.ino)
shows how to emit RCSwitch protocols.


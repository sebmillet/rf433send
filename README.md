RF433send - Send any protocol over RF433
========================================

Uses a RF433Mhz transmitter plugged on an Arduino to send a signal encoded in
various ways.

Goal is to manage the following encodings:
- Tri-bit (normal or inverted)
- Manchester

Schematic: Arduino plugged on a RF433 TX component, like XY-FST.
Data pin number of which the transmitter is plugged is to be specified when
creating object.


// 06_send_20230617100200.ino

// Simple example of sending codes with a Radio Frequencies device.
// Sends code 6 times every 5 seconds.

/*
  Copyright 2021 Sébastien Millet

  `RF433send' is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  `RF433send' is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program. If not, see
  <https://www.gnu.org/licenses>.
*/

// Schematic:
//   RF433 TRANSMITTER data pin plugged on Arduino D4

#include "RF433send.h"

#define PIN_RFOUT  4

RfSend *tx_whatever;

void setup() {
    pinMode(PIN_RFOUT, OUTPUT);

    Serial.begin(115200);

        // rfsend_builder performs some asserts that, it failed, write details
        // to Serial and then block execution.
        // If construction is done before setup() execution (as is the case with
        // global variables), no output is done and we loose interesting debug
        // information.
        // Therefore I prefer this style over using a global radio object.

    tx_whatever = rfsend_builder(
        RfSendEncoding::TRIBIT,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,  // Do we want to invert 0 and 1 bits? No.
        6,       // Number of sendings
        nullptr, // No callback to keep/stop sending (if you want to send
                 // SO LONG AS a button is pressed, the function reading the
                 // button state is to be put here).
        12000,   // initseq
        0,       // lo_prefix
        0,       // hi_prefix
        0,       // first_lo_ign
        431,     // lo_short
        1236,    // lo_long
        0,       // hi_short
        0,       // hi_long
        430,     // lo_last
        12000,   // sep
        24       // nb_bits
    );

}

byte data[] = { 0x1f, 0x03, 0x14 };
void loop() {
    static int count = 0;

    byte n = tx_whatever->send(sizeof(data), data);
    Serial.print("Envoi effectué ");
    Serial.print(n);
    Serial.print(" fois\n");

    delay(5000);
}

// vim: ts=4:sw=4:tw=80:et

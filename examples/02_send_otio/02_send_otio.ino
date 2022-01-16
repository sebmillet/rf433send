// 02_send_otio.ino

// Example to send OTIO telecommand codes, when button is pressed.
// Code is sent 7 times.

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
//   Button plugged to Arduino GND and Arduino D6
//   Led (+ resistor) plugged to Arduino GND and Arduino D5
//
//   See file schema.fzz (Fritzing format) or schema.png

#include "RF433send.h"

#define PIN_RFOUT  4
#define PIN_BUTTON 6
#define PIN_LED    5

//RfSend *tx_flo;
//RfSend *tx_adf;
//RfSend *tx_sonoff;
RfSend *tx_otio;

bool button_is_pressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}

void setup() {
    pinMode(PIN_RFOUT, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);

    digitalWrite(PIN_LED, LOW);

    Serial.begin(115200);

        // RfSend constructor performs some assert that write to Serial before
        // blocking execution. If construction is done before setup() execution
        // (as is the case with global variables), no output is done and we
        // loose interesting debug information.
        // Therefore I prefer this style over creating a global radio object.

    tx_otio = rfsend_builder(
        RfSendEncoding::TRIBIT,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        7,
        nullptr,
        6976,           // initseq
        0,              // lo_prefix
        0,              // hi_prefix
        0,              // first_lo_ign
        562,            // lo_short
        1258,           // lo_long
        0,              // hi_short
        0,              // hi_long
        528,            // lo_last
        6996,           // sep
        32              // nb_bits
    );

}

const byte mydata_otio_open[] =  {0x90, 0x91, 0x92, 0x93};
const byte mydata_otio_close[] = {0xA0, 0x91, 0x92, 0x93};

void loop() {
    static int count = 0;

    if (button_is_pressed()) {
        digitalWrite(PIN_LED, HIGH);
        int m = ++count % 2;
        byte n;
        if (m == 0) {
            n = tx_otio->send(sizeof(mydata_otio_open), mydata_otio_open);
        } else if (m == 1) {
            n = tx_otio->send(sizeof(mydata_otio_close), mydata_otio_close);
        }
        Serial.print("Envoi effectué ");
        Serial.print(n);
        Serial.print(" fois\n");
        digitalWrite(PIN_LED, LOW);

        while (button_is_pressed())
            ;
    }
}

// vim: ts=4:sw=4:tw=80:et

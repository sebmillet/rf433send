// 01_send.ino

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
RfSend *tx_adf;
//RfSend *tx_sonoff;

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

//    tx_flo = rfsend_builder(
//        RfSendEncoding::TRIBIT_INVERTED,
//        PIN_RFOUT,
//        RFSEND_DEFAULT_CONVENTION,
//        0,
//        button_is_pressed,
//        24000,          // initseq
//        0,              // lo_prefix
//        0,              // hi_prefix
//        650,            // first_lo_ign
//        650,            // lo_short
//        1300,           // lo_long
//        0,              // hi_short
//        0,              // hi_long
//        0,              // lo_last (not used with TRIBIT_INVERTED)
//        24000,          // sep
//        12              // nb_bits
//    );

    tx_adf = rfsend_builder(
        RfSendEncoding::MANCHESTER,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        8,
        nullptr,
        5500,           // initseq
        0,              // lo_prefix
        0,              // hi_prefix
        0,              // first_lo_ign
        1150,           // lo_short
        0,              // lo_long (not used with MANCHESTER)
        0,              // hi_short (not used with MANCHESTER)
        0,              // hi_long (not used with MANCHESTER)
        0,              // lo_last (not used with MANCHESTER)
        6900,           // sep
        32              // nb_bits
    );

//    tx_sonoff = rfsend_builder(
//        RfSendEncoding::TRIBIT,
//        PIN_RFOUT,
//        RFSEND_DEFAULT_CONVENTION,
//        0,
//        button_is_pressed,
//        10000,          // initseq
//        0,              // lo_prefix
//        0,              // hi_prefix
//        0,              // first_lo_ign
//        350,            // lo_short
//        1000,           // lo_long
//        0,              // hi_short
//        0,              // hi_long
//        350,            // lo_last
//        10000,          // sep
//        24              // nb_bits
//    );
}

#include "codes.h"

void loop() {
    static int count = 0;

    if (button_is_pressed()) {
        digitalWrite(PIN_LED, HIGH);
        int m = ++count % 2;
        byte n;
        if (m == 0) {
            n = tx_adf->send(sizeof(mydata_adf_1), mydata_adf_1);
        } else if (m == 1) {
            n = tx_adf->send(sizeof(mydata_adf_2), mydata_adf_2);
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

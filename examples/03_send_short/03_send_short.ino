// 03_send_short.ino

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

// FIXME

RfSend *tx_flo;
RfSend *tx_otio;
RfSend *tx_flo_mod;
RfSend *tx_adf;
RfSend *tx_sonoff;
RfSend *tx_flor;

bool button_is_pressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}

void setup() {
    pinMode(PIN_RFOUT, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);

    digitalWrite(PIN_LED, LOW);

    Serial.begin(115200);

    tx_flo = rfsend_builder(
        RfSendEncoding::TRIBIT_INVERTED,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        2,
        nullptr,
        24000,          // initseq
        0,              // lo_prefix
        0,              // hi_prefix
        650,            // first_lo_ign
        650,            // lo_short
        1300,           // lo_long
        0,              // hi_short
        0,              // hi_long
        0,              // lo_last (not used with TRIBIT_INVERTED)
        24000,          // sep
        12              // nb_bits
    );

    tx_otio = rfsend_builder(
        RfSendEncoding::TRIBIT,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        2,
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
        16              // nb_bits
    );

    tx_flo_mod = rfsend_builder(
        RfSendEncoding::TRIBIT_INVERTED,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        2,
        nullptr,
        15000,          // initseq
        0,              // lo_prefix
        0,              // hi_prefix
        2000,           // first_lo_ign
        400,            // lo_short
        1000,           // lo_long
        900,            // hi_short
        1500,           // hi_long
        0,              // lo_last (not used with TRIBIT_INVERTED)
        24000,          // sep
        16              // nb_bits
    );

    tx_adf = rfsend_builder(
        RfSendEncoding::MANCHESTER,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        2,
        nullptr,
        10000,          // initseq
        0,              // lo_prefix
        0,              // hi_prefix
        0,              // first_lo_ign
        1150,           // lo_short
        0,              // lo_long (not used with MANCHESTER)
        0,              // hi_short (not used with MANCHESTER)
        0,              // hi_long (not used with MANCHESTER)
        0,              // lo_last (not used with MANCHESTER)
        10000,          // sep
        16              // nb_bits
    );

    tx_sonoff = rfsend_builder(
        RfSendEncoding::TRIBIT,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        2,
        nullptr,
        10000,          // initseq
        0,              // lo_prefix
        0,              // hi_prefix
        0,              // first_lo_ign
        350,            // lo_short
        1000,           // lo_long
        0,              // hi_short
        0,              // hi_long
        350,            // lo_last
        10000,          // sep
        12              // nb_bits
    );

    tx_flor = rfsend_builder(
        RfSendEncoding::TRIBIT,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        2,
        nullptr,
        17888, // initseq
         1432, // lo_prefix
         1424, // hi_prefix
            0, // first_lo_ign
          474, // lo_short
          952, // lo_long
            0, // hi_short
            0, // hi_long
         1400, // lo_last
        19324, // sep
           12  // nb_bits
    );

}

const byte mydata_flo[] =  {0x07, 0x51};
const byte mydata_otio[] = {0xAD, 0x15};
const byte mydata_flo_mod[] =  {0xD5, 0x62};
const byte mydata_adf[] = {0xD3, 0xE5};
const byte mydata_sonoff[] = {0x05, 0x91};
const byte mydata_flor[] = {0x03, 0x40};

struct send_code_t {
    RfSend **ptx;
    byte sz;
    const byte *code;
};

send_code_t const codes[] = {
    {&tx_flo, sizeof(mydata_flo), mydata_flo},
    {&tx_otio, sizeof(mydata_otio), mydata_otio},
    {&tx_flo_mod, sizeof(mydata_flo_mod), mydata_flo_mod},
    {&tx_adf, sizeof(mydata_adf), mydata_adf},
    {&tx_sonoff, sizeof(mydata_sonoff), mydata_sonoff},
    {&tx_flor, sizeof(mydata_flor), mydata_flor}
};

void loop() {
    static int count = 3;

    if (button_is_pressed()) {
        digitalWrite(PIN_LED, HIGH);

        int m = count % (sizeof(codes) / sizeof(*codes));

        byte n = (*codes[m].ptx)->send(codes[m].sz, codes[m].code);

        Serial.print("Envoi effectué ");
        Serial.print(n);
        Serial.print(" fois (code: ");
        Serial.print(m);
        Serial.print(")\n");
        digitalWrite(PIN_LED, LOW);

        while (button_is_pressed())
            ;
    }
}

// vim: ts=4:sw=4:tw=80:et
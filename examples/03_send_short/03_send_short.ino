// 03_send_short.ino

// Sends codes using various different code timings/encodings, in turn.
// Uses a button press to trigger sending. Every time the button is pressed,
// will use the next timings/encoding.
// Also sends code so long as the button is pressed (there is no pre-defined
// fixed number of sendings).

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

RfSend *tx_flo;
RfSend *tx_otio;
RfSend *tx_flo_mod;
RfSend *tx_adf;
RfSend *tx_sonoff;
RfSend *tx_flor;
RfSend *tx_adf32;
RfSend *tx_adf8;
RfSend *tx_adfalt;

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
        0,
        button_is_pressed,
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
        0,
        button_is_pressed,
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

    tx_flo_mod = rfsend_builder(
        RfSendEncoding::TRIBIT_INVERTED,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        0,
        button_is_pressed,
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
        0,
        button_is_pressed,
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
        0,
        button_is_pressed,
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
        0,
        button_is_pressed,
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

    tx_adf32 = rfsend_builder(
        RfSendEncoding::MANCHESTER,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        0,
        button_is_pressed,
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
        32              // nb_bits
    );

    tx_adf8 = rfsend_builder(
        RfSendEncoding::MANCHESTER,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        0,
        button_is_pressed,
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
        8               // nb_bits
    );

    tx_adfalt = rfsend_builder(
        RfSendEncoding::MANCHESTER,
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        0,
        button_is_pressed,
        4000,           // initseq
        0,              // lo_prefix
        0,              // hi_prefix
        0,              // first_lo_ign
        400,            // lo_short
        0,              // lo_long (not used with MANCHESTER)
        0,              // hi_short (not used with MANCHESTER)
        0,              // hi_long (not used with MANCHESTER)
        0,              // lo_last (not used with MANCHESTER)
        4000,           // sep
        16              // nb_bits
    );
}

const byte mydata_flo[] =  {0x07, 0x51};
const byte mydata_otio[] = {0x8A, 0x34, 0xE6, 0xBF};
const byte mydata_flo_mod[] =  {0xD5, 0x62};
const byte mydata_adf[] = {0x03, 0xE0};
const byte mydata_sonoff[] = {0x05, 0x91};
const byte mydata_flor[] = {0x03, 0x40};
const byte mydata_adf_32bit[] = {0xF0, 0x55, 0xAA, 0x00};
const byte mydata_adf_8bit[] = {0x55};
const byte mydata_adf_8bit_2[] = {0x44};
const byte mydata_adf_alt1[] = {0x03, 0xE0};
const byte mydata_adf_alt2[] = {0xF3, 0x0F};

struct send_code_t {
    RfSend **ptx;
    byte sz;
    const byte *code;
};

send_code_t const codes[] = {
    {&tx_flo, sizeof(mydata_flo), mydata_flo},                //  0
    {&tx_otio, sizeof(mydata_otio), mydata_otio},             //  1
    {&tx_flo_mod, sizeof(mydata_flo_mod), mydata_flo_mod},    //  2
    {&tx_adf, sizeof(mydata_adf), mydata_adf},                //  3
    {&tx_sonoff, sizeof(mydata_sonoff), mydata_sonoff},       //  4
    {&tx_flor, sizeof(mydata_flor), mydata_flor},             //  5
    {&tx_adf32, sizeof(mydata_adf_32bit), mydata_adf_32bit},  //  6
    {&tx_adf8, sizeof(mydata_adf_8bit), mydata_adf_8bit},     //  7
    {&tx_adf8, sizeof(mydata_adf_8bit_2), mydata_adf_8bit_2}, //  8
    {&tx_adfalt, sizeof(mydata_adf_alt1), mydata_adf_alt1},   //  9
    {&tx_adfalt, sizeof(mydata_adf_alt2), mydata_adf_alt2}    // 10
};

void loop() {
    static int count = 0;

    if (button_is_pressed()) {
        digitalWrite(PIN_LED, HIGH);

        int m = count++ % (sizeof(codes) / sizeof(*codes));

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

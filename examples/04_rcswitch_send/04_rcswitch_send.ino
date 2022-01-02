// 04_rcswitch_send.ino

// Sends signals following RCSwitch lib specifications

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

struct txinfo {
    RfSend *rfsender;
    const char *protocol_name;
};

txinfo tx[12];

bool button_is_pressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}

void setup() {
    pinMode(PIN_RFOUT, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);

    digitalWrite(PIN_LED, LOW);

    Serial.begin(115200);

    Serial.print(F("NOTE (*)\n"
            "    About RCSwitch lib protocol 4: it works only if you update \n"
            "    RCSwitch::nSeparationLimit to a lower value.\n"
            "    The actual value in the library is too big.\n"
            "    This is in the file RCSwitch.cpp, line:\n"
            "        const unsigned int RCSwitch::nSeparationLimit = 4300;\n"
            "    The author of RF433send replaced it with the below:\n"
            "        const unsigned int RCSwitch::nSeparationLimit = 2300;\n"
            "\n"
            "NOTE (**)\n"
            "    About RCSwitch lib procotols 8 and 9: do not seem to work.\n"
            "    The author of RF433send could never see it work, even when\n"
            "    using RCSwitch lib SendDemo.ino code and\n"
            "    ReceiveDemo_Simple.ino in conjunction.\n"
            "    RF433send and RF433recv make it work though, but the author\n"
            "    could not say if the protocol is properly followed.\n"
            "    Help welcome!\n\n"
            ));

    tx[0] = { rfsend_builder(RfSendEncoding::TRIBIT, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 10850, 0, 0,
                  0, 350, 1050, 0, 0, 350, 10850, 32),
              "protocol 1" };
    tx[1] = { rfsend_builder(RfSendEncoding::TRIBIT, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 6500, 0, 0,
                  0, 650, 1300, 0, 0, 650, 6500, 32),
              "protocol 2" };
    tx[2] = { rfsend_builder(RfSendEncoding::TRIBIT, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 7100, 0, 0,
                  0, 400, 900, 600, 1100, 400, 7100, 32),
              "protocol 3" },
    tx[3] = { rfsend_builder(RfSendEncoding::TRIBIT, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 2280, 0, 0,
                  0, 380, 1140, 0, 0, 380, 2280, 32),
              "protocol 4, see NOTE (*)" };
    tx[4] = { rfsend_builder(RfSendEncoding::TRIBIT, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 7000, 0, 0,
                  0, 500, 1000, 0, 0, 500, 7000, 32),
              "protocol 5" };
    tx[5] = { rfsend_builder(RfSendEncoding::TRIBIT_INVERTED, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 10350, 0, 0,
                  450, 450, 900, 0, 0, 0, 10350, 32),
              "protocol 6" };
    tx[6] = { rfsend_builder(RfSendEncoding::TRIBIT, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 9300, 0, 0,
                  0, 150, 900, 0, 0, 150, 9300, 32),
              "protocol 7" };
    tx[7] = { rfsend_builder(RfSendEncoding::TRIBIT, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 26000, 0, 0,
                  0, 1400, 600, 3200, 3200, 600, 26000, 32),
              "protocol 8 (**)" };
    tx[8] = { rfsend_builder(RfSendEncoding::TRIBIT_INVERTED, PIN_RFOUT,
                  RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 26000, 0, 0,
                  1400, 1400, 600, 3200, 3200, 0, 26000, 32),
              "protocol 9 (**)" };
    tx[9] = { rfsend_builder(RfSendEncoding::TRIBIT_INVERTED, PIN_RFOUT,
                  RFSEND_CONVENTION_1, 0, button_is_pressed, 6570, 0, 0, 365,
                  365, 1095, 0, 0, 0, 6570, 32),
              "protocol 10" };
    tx[10] = { rfsend_builder(RfSendEncoding::TRIBIT_INVERTED, PIN_RFOUT,
                   RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 9720, 0, 0,
                   270, 270, 540, 0, 0, 0, 9720, 32),
               "protocol 11" };
    tx[11] = { rfsend_builder(RfSendEncoding::TRIBIT_INVERTED, PIN_RFOUT,
                   RFSEND_DEFAULT_CONVENTION, 0, button_is_pressed, 11520, 0, 0,
                   320, 320, 640, 0, 0, 0, 11520, 32),
               "protocol 12" };
//#define ENFORCE_CODE_SENT 11

        // Defensive programming: make sure tx was declared correctly
        // Note that to be 100% clean, check should be done *before* assignments
        // above. BUT, I prefer to check it here, as the last index used in tx
        // shows up just above.
    if (sizeof(tx) / sizeof(*tx) != 12) {
        Serial.print(F("\nFATAL: bad tx array size, check this code!\n\n"));
        while (1)
            ;
    }
}

byte mydata[4];

void loop() {
    static uint16_t count = 0;
    if (button_is_pressed()) {
        digitalWrite(PIN_LED, HIGH);

        byte code = (count++ % (sizeof(tx) / sizeof(*tx)));
#ifdef ENFORCE_CODE_SENT
        code = ENFORCE_CODE_SENT;
#endif

            // Why 3000000000 at the beginning? -> RCSwitch output will be
            // aligned correctly even when count reaches 10 or 100.
            // And why 3? Why not 1000000000, 2000000000 or 4000000000?
            // Because.
            //
            // In the end (all the below, applies to decimal representation):
            // - The 4 rightmost digits show protocol number
            // - As of position 5 (starting from rightmost digit that has
            //   position 1), the 5 digits show the sending count number.
            // - At the left there is a leading 3, that's just decorating :D
        uint32_t v = 3000000000UL + 10000UL * count + code + 1;
            // Switch little/big endian to fit with RCSwitch library mood.
        mydata[0] = v >> 24;
        mydata[1] = v >> 16;
        mydata[2] = v >> 8;
        mydata[3] = v;
        byte n = tx[code].rfsender->send(sizeof(mydata) / sizeof(*mydata),
                mydata);

        Serial.print(F("sent: "));
        Serial.print(n);
        Serial.print(F(" time(s), RCSwitch protocol: '"));
        Serial.print(tx[code].protocol_name);
        Serial.print(F("'\n"));

        digitalWrite(PIN_LED, LOW);

        while (button_is_pressed())
            ;
    }
}

// vim: ts=4:sw=4:tw=80:et

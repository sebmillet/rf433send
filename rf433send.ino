// rf433send.ino

/*
  Copyright 2021 Sébastien Millet

  `rf433send' is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  `rf433send' is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses>.
*/

//
// Schematic:
//   RF433 TRANSMITTER data pin plugged on Arduino D4
//   See file schema.fzz (Fritzing format) or schema.png
//

#include <Arduino.h>

#define assert(cond) { \
    if (!(cond)) { \
        assert_failed(__LINE__); \
    } \
}
static void assert_failed(int line) {
    Serial.print("rf433send.ino");
    Serial.print(":");
    Serial.print(line);
    Serial.print(":");
    Serial.println(" assertion failed, aborted.");
    while (1)
        ;
}

enum class Repeat {
    FIXED,
    CALLBACK
};

class ProtocolTimings {
    private:
        Repeat repeat;
        byte nb_repeats;
        bool (*repeat_callback)();
        uint16_t initseq;
        uint16_t lo_short;
        uint16_t lo_long;
        uint16_t hi_short;
        uint16_t hi_long;
        uint16_t lo_last;
        uint16_t sep;
        byte nb_bits;
        byte nb_bytes;

    public:
        ProtocolTimings(Repeat arg_repeat,
            byte arg_nb_repeats, bool (*arg_repeat_callback)(),
            uint16_t arg_initseq, uint16_t arg_lo_short, uint16_t arg_lo_long,
            uint16_t arg_hi_short, uint16_t arg_hi_long, uint16_t arg_lo_last,
            uint16_t arg_sep, byte arg_nb_bits);
};

ProtocolTimings::ProtocolTimings(Repeat arg_repeat,
        byte arg_nb_repeats, bool (*arg_repeat_callback)(),
        uint16_t arg_initseq, uint16_t arg_lo_short, uint16_t arg_lo_long,
        uint16_t arg_hi_short, uint16_t arg_hi_long, uint16_t arg_lo_last,
        uint16_t arg_sep, byte arg_nb_bits):

        repeat(arg_repeat),
        nb_repeats(arg_nb_repeats),
        repeat_callback(arg_repeat_callback),
        initseq(arg_initseq),
        lo_short(arg_lo_short),
        lo_long(arg_lo_long),
        hi_short(arg_hi_short),
        hi_long(arg_hi_long),
        lo_last(arg_lo_last),
        sep(arg_sep),
        nb_bits(arg_nb_bits),
        nb_bytes((arg_nb_bits + 7) >> 3) {
    assert(nb_bits);
        // Defensive programming
        // If nb_bits is non-zero, nb_bytes is for sure non-zero, too.
    assert(nb_bytes);
}

class RfSend {
    private:
        byte pin_rfout;
        const ProtocolTimings *ppt;

        void mydelay_us(unsigned long d);
        virtual void rf_emit_signal(const byte val, unsigned long d);

    public:
        RfSend(byte arg_pin_rfout, const ProtocolTimings* arg_ppt);
        virtual ~RfSend();
        virtual void init();
        virtual void send(byte *data);
};

RfSend::RfSend(byte arg_pin_rfout, const ProtocolTimings* arg_ppt):
        pin_rfout(arg_pin_rfout),
        ppt(arg_ppt) {
// WRITE NOTHING HERE
        // Notes
        //   It is very important to start by this call before anything else.
        //   Why? Because init() puts RF transmitter at rest.
    init();

    assert(ppt);
}

RfSend::~RfSend() {
        // Notes
        //   In case object would be destroyed in the middle of a transmission,
        //   we call init() to put RF transmitter at rest.
    init();
}

void RfSend::init() {
    pinMode(pin_rfout, OUTPUT);
      // Put RF transmitter at rest
    digitalWrite(pin_rfout, LOW);
}

void RfSend::send(byte *data) {
    
}

void RfSend::rf_emit_signal(const byte val, unsigned long d) {
    digitalWrite(pin_rfout, val ? LOW : HIGH);
    mydelay_us(d);
}

#define MYDELAY_STEP        10000

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || \
    defined(__AVR_ATtiny85__)
  // Attiny case
#if F_CPU == 16000000UL
#define MYDELAY_TIMER_SHIFT 4
#elif F_CPU == 8000000UL
#define MYDELAY_TIMER_SHIFT 3
#else
#define MYDELAY_TIMER_SHIFT 0
#endif

#else
  // non-Attiny case
#define MYDELAY_TIMER_SHIFT 0

#endif

void RfSend::mydelay_us(unsigned long d) {
    d >>= MYDELAY_TIMER_SHIFT;

    while (d >= MYDELAY_STEP) {
        d -= MYDELAY_STEP;
        delayMicroseconds(MYDELAY_STEP);
    }
    delayMicroseconds(d);
}

#define PIN_RFOUT  4
#define PIN_BUTTON 6
#define PIN_LED    5

const ProtocolTimings pt (
    Repeat::FIXED,  // repeat
    4,              // nb_repeats
    nullptr,        // repeat_callback
    24000,          // initseq
    650,            // lo_short
    1300,           // lo_long
    0,              // hi_short
    0,              // hi_long
    650,            // lo_last
    24000,          // sep
    12              // nb_bits
);
RfSend rfsend(PIN_RFOUT, &pt);

void setup() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);

    digitalWrite(PIN_LED, LOW);

    Serial.begin(115200);
}

void loop() {
    static int prev_val = HIGH;
    static unsigned long prev_t = 0;

    unsigned long t = micros();

        // De-bouncing
    if (t - prev_t < 10000)
        return;

    int val = digitalRead(PIN_BUTTON);

    if (val != prev_val) {
        prev_val = val;
        prev_t = t;

        if (val == HIGH) {
            digitalWrite(PIN_LED, LOW);
            Serial.print("Bouton relâché\n");
        } else {
            digitalWrite(PIN_LED, HIGH);
            Serial.print("Bouton pressé\n");
        }
    }
}

// vim: ts=4:sw=4:tw=80:et

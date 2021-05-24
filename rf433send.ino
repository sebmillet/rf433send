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
    Serial.print(" assertion failed, aborted.\n");
    while (1)
        ;
}

class SignalShape {
    friend class RfSend;
    friend class RfSendTribit;

    private:
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
        SignalShape(uint16_t arg_initseq,
                uint16_t arg_lo_short, uint16_t arg_lo_long,
                uint16_t arg_hi_short, uint16_t arg_hi_long,
                uint16_t arg_lo_last, uint16_t arg_sep, byte arg_nb_bits);
};

SignalShape::SignalShape(uint16_t arg_initseq,
            uint16_t arg_lo_short, uint16_t arg_lo_long,
            uint16_t arg_hi_short, uint16_t arg_hi_long,
            uint16_t arg_lo_last, uint16_t arg_sep, byte arg_nb_bits):

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

            //
            // If nb_repeats is equal to zero, then ignore it (that is, only
            // rely on repeat_callback() return value to stop sending).
            // If repeat_callback is nullptr, then ignore it (that is, only rely
            // on nb_repeats to work out how many repeats are to be done).
            //
            // - If none is specified (nb_repeats = 0, repeat_callback =
            //   nullptr) then it is a fatal error (failed assert).
            //
            // - If both are specified, then rely on the two criteria, that is,
            // repeat maximum nb_repeats, but less if repeat_callback() returns
            // false before nb_repeats has been reached. So it is a 'OR' logical
            // test between the two criteria.
            //
        byte nb_repeats;           // Repeat that number of times
        bool (*repeat_callback)(); // Repeat until repeat_callback() returns
                                   // false.

        virtual void put_tx_at_rest() const;
        virtual void mydelay_us(unsigned long d) const;
        virtual void tx_data_once(const byte *data) const = 0;

    protected:
        const SignalShape *psignal;

        virtual byte get_nth_bit(const byte *data, int n) const;
        virtual void tx_signal_atom(byte bitval, unsigned long d) const;

    public:
        RfSend(byte arg_pin_rfout, byte arg_nb_repeats,
               bool (*arg_repeat_callback)(), const SignalShape* arg_psignal);
        virtual ~RfSend();

            // About send() method being 'final': I wanted to highlight the fact
            // that it MUST end with execution of put_tx_at_rest(), to be sure
            // TX is not left active.
            // Of course you can remove 'final' and override the method happily.
            // But, you've been warned... ;-)
        virtual int send(const byte *data) const final;
};

RfSend::RfSend(byte arg_pin_rfout, byte arg_nb_repeats,
               bool (*arg_repeat_callback)(), const SignalShape* arg_psignal):
        pin_rfout(arg_pin_rfout),
        nb_repeats(arg_nb_repeats),
        repeat_callback(arg_repeat_callback),
        psignal(arg_psignal) {
// *WRITE NOTHING HERE* - THE 2 INSTRUCTIONS BELOW MUST BE CALLED FIRST THING.
// Would something bad happen (like a failed assert), we don't want to end up
// with a TX left active.
    pinMode(pin_rfout, OUTPUT); // Must remain 1st instruction in constructor
    put_tx_at_rest();           // Must remain 2nd instruction in constructor

    assert(psignal);
    assert(nb_repeats || repeat_callback);
}

RfSend::~RfSend() {
    put_tx_at_rest(); // Must remain 1st instruction in destructor
}

void RfSend::put_tx_at_rest() const {
    digitalWrite(pin_rfout, LOW);
}

    // Bit numbering starts at 0 (least significant) until 'nb_bits - 1' (most
    // significant).
byte RfSend::get_nth_bit(const byte *data, int n) const {
    assert(n >= 0);
    assert(n < psignal->nb_bits);
    int index = (n >> 3);
        // Defensive programming: test on nb_bits is sufficient.
    assert(index < psignal->nb_bytes);
    byte bitread = (1 << (n & 0x07));
    return !!(data[index] & bitread);
}

void RfSend::tx_signal_atom(byte bitval, unsigned long d) const {
    digitalWrite(pin_rfout, bitval ? LOW : HIGH);
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

void RfSend::mydelay_us(unsigned long d) const {
    d >>= MYDELAY_TIMER_SHIFT;

    while (d >= MYDELAY_STEP) {
        d -= MYDELAY_STEP;
        delayMicroseconds(MYDELAY_STEP);
    }
    delayMicroseconds(d);
}

int RfSend::send(const byte *data) const {

    tx_signal_atom(1, psignal->initseq);

    int count = 0;
    bool repeat = true;
    while (repeat) {
        tx_data_once(data);

        ++count;
        if (nb_repeats && count == nb_repeats)
            repeat = false;
        if (repeat_callback && !(*repeat_callback)())
            repeat = false;
    }

    put_tx_at_rest();
// *WRITE NOTHING HERE* - put_tx_at_rest() must be the last instruction before
// return.
    return count;
}

class RfSendTribit: public RfSend {
    private:
        virtual void tx_data_once(const byte *data) const override;

    public:
        RfSendTribit(byte arg_pin_rfout, byte arg_nb_repeats,
                bool (*arg_repeat_callback)(), const SignalShape* arg_psignal):
            RfSend(arg_pin_rfout,
                    arg_nb_repeats,
                    arg_repeat_callback,
                    arg_psignal) { }
        virtual ~RfSendTribit();
};

RfSendTribit::~RfSendTribit() { }

void RfSendTribit::tx_data_once(const byte *data) const {
    for (int i = psignal->nb_bits - 1; i >= 0; --i) {
        byte bitval = get_nth_bit(data, i);
        tx_signal_atom(0, bitval ? psignal->lo_short : psignal->lo_long);
        tx_signal_atom(1, bitval ? psignal->hi_long : psignal->hi_short);
    }
    tx_signal_atom(0, psignal->lo_last);
    tx_signal_atom(1, psignal->sep);
}

#define PIN_RFOUT  4
#define PIN_BUTTON 6
#define PIN_LED    5

const SignalShape signal(
    24000,          // initseq
    650,            // lo_short
    1300,           // lo_long
    0,              // hi_short
    0,              // hi_long
    650,            // lo_last
    24000,          // sep
    12              // nb_bits
);
RfSendTribit *pradio;

bool button_is_pressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}

void setup() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);

    digitalWrite(PIN_LED, LOW);

    Serial.begin(115200);

    pradio = new RfSendTribit(PIN_RFOUT, 0, button_is_pressed, &signal);
}

const byte mydata[] = { 0x05, 0x55};

void loop() {
    if (button_is_pressed()) {
        digitalWrite(PIN_LED, HIGH);
        int n = pradio->send(mydata);
        Serial.print("Envoi effectué ");
        Serial.print(n);
        Serial.print(" fois\n");
        digitalWrite(PIN_LED, LOW);

        while (button_is_pressed())
            ;
    }
}

// vim: ts=4:sw=4:tw=80:et

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

#define ASSERT_OUTPUT_TO_SERIAL

#define assert(cond) { \
    if (!(cond)) { \
        assert_failed(__LINE__); \
    } \
}
static void assert_failed(int line) {
#ifdef ASSERT_OUTPUT_TO_SERIAL
    Serial.print("rf433send.ino");
    Serial.print(":");
    Serial.print(line);
    Serial.print(":");
    Serial.print(" assertion failed, aborted.\n");
#endif
    while (1)
        ;
}


// * *********** **************************************************************
// * SignalShape **************************************************************
// * *********** **************************************************************

class SignalShape {
    friend class RfSend;
    friend class RfSendTribit;
    friend class RfSendTribitInv;
    friend class RfSendManchester;

    private:
        uint16_t initseq;
        uint16_t first_lo_ign;
        uint16_t lo_short;
        uint16_t lo_long;
        uint16_t hi_short;
        uint16_t hi_long;
        uint16_t lo_last;
        uint16_t sep;
        byte nb_bits;
        byte nb_bytes;

    public:
        SignalShape(uint16_t arg_initseq, uint16_t arg_first_lo_ign,
                uint16_t arg_lo_short, uint16_t arg_lo_long,
                uint16_t arg_hi_short, uint16_t arg_hi_long,
                uint16_t arg_lo_last, uint16_t arg_sep, byte arg_nb_bits);
};

SignalShape::SignalShape(uint16_t arg_initseq, uint16_t arg_first_lo_ign,
            uint16_t arg_lo_short, uint16_t arg_lo_long,
            uint16_t arg_hi_short, uint16_t arg_hi_long,
            uint16_t arg_lo_last, uint16_t arg_sep, byte arg_nb_bits):

        initseq(arg_initseq),
        first_lo_ign(arg_first_lo_ign),
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

    assert((!hi_short && !hi_long) || (hi_short && hi_long));
    if (!hi_short) {
        hi_short = lo_short;
        hi_long = lo_long;
    }
}


// * ****** *******************************************************************
// * RfSend *******************************************************************
// * ****** *******************************************************************

#define RFSEND_CONVENTION_0               0
#define RFSEND_CONVENTION_1               1
#define RFSEND_DEFAULT_CONVENTION         RFSEND_CONVENTION_0

#define RFSEND_HARDCODED_MAX_NB_REPEATS   20

class RfSend {
    private:
        byte pin_rfout;
        const byte convention;

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
        RfSend(byte arg_pin_rfout, byte arg_convention, byte arg_nb_repeats,
               bool (*arg_repeat_callback)(), const SignalShape* arg_psignal);
        virtual ~RfSend();

            // About send() method being 'final': I wanted to highlight the fact
            // that it MUST end with execution of put_tx_at_rest(), to be sure
            // TX is not left active, no matter what a child is doing.
            // Of course you can remove 'final' and override the method happily.
            // But, you've been warned... ;-)
        virtual int send(const byte *data) const final;
};

RfSend::RfSend(byte arg_pin_rfout, byte arg_convention, byte arg_nb_repeats,
               bool (*arg_repeat_callback)(), const SignalShape*
               arg_psignal):
        pin_rfout(arg_pin_rfout),
        convention(arg_convention),
        nb_repeats(arg_nb_repeats),
        repeat_callback(arg_repeat_callback),
        psignal(arg_psignal) {
// *WRITE NOTHING HERE* - THE 2 INSTRUCTIONS BELOW MUST BE CALLED FIRST THING.
// Would something bad happen (like a failed assert), we don't want to end up
// with a TX left active.
    pinMode(pin_rfout, OUTPUT); // Must remain 1st instruction in constructor
    put_tx_at_rest();           // Must remain 2nd instruction in constructor

    assert(psignal);
    assert(convention == RFSEND_CONVENTION_0 || convention == RFSEND_CONVENTION_1);

    assert(nb_repeats || repeat_callback);
#ifdef RFSEND_HARDCODED_MAX_NB_REPEATS
    if (!nb_repeats || nb_repeats > RFSEND_HARDCODED_MAX_NB_REPEATS)
        nb_repeats = RFSEND_HARDCODED_MAX_NB_REPEATS;
#endif
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

    int index = (int)psignal->nb_bytes - 1 - (n >> 3);
        // Defensive programming: test on nb_bits is sufficient.
    assert(index >= 0 && index < psignal->nb_bytes);

    byte bitread = (1 << (n & 0x07));
    if (convention == RFSEND_CONVENTION_0)
        return !!(data[index] & bitread);
    else
        return !(data[index] & bitread);
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


// * ************ *************************************************************
// * RfSendTribit *************************************************************
// * ************ *************************************************************

class RfSendTribit: public RfSend {
    private:
        virtual void tx_data_once(const byte *data) const override;

    public:
        RfSendTribit(byte arg_pin_rfout, byte arg_convention,
                byte arg_nb_repeats, bool (*arg_repeat_callback)(),
                const SignalShape* arg_psignal):
            RfSend(arg_pin_rfout, arg_convention,
                    arg_nb_repeats, arg_repeat_callback, arg_psignal) { }
        virtual ~RfSendTribit();
};

RfSendTribit::~RfSendTribit() { }

void RfSendTribit::tx_data_once(const byte *data) const {
    for (int i = psignal->nb_bits - 1; i >= 0; --i) {
        byte bitval = get_nth_bit(data, i);
        tx_signal_atom(0, bitval ? psignal->lo_long : psignal->lo_short);
        tx_signal_atom(1, bitval ? psignal->hi_short : psignal->hi_long);
    }
    tx_signal_atom(0, psignal->lo_last);
    tx_signal_atom(1, psignal->sep);
}


// * *************** **********************************************************
// * RfSendTribitInv **********************************************************
// * *************** **********************************************************

class RfSendTribitInv: public RfSend {
    private:
        virtual void tx_data_once(const byte *data) const override;

    public:
        RfSendTribitInv(byte arg_pin_rfout, byte arg_convention,
                byte arg_nb_repeats, bool (*arg_repeat_callback)(),
                const SignalShape* arg_psignal):
            RfSend(arg_pin_rfout, arg_convention,
                    arg_nb_repeats, arg_repeat_callback, arg_psignal) { }
        virtual ~RfSendTribitInv();
};

RfSendTribitInv::~RfSendTribitInv() { }

void RfSendTribitInv::tx_data_once(const byte *data) const {
    tx_signal_atom(0, psignal->first_lo_ign);
    for (int i = psignal->nb_bits - 1; i >= 0; --i) {
        byte bitval = get_nth_bit(data, i);
        tx_signal_atom(1, bitval ? psignal->lo_long : psignal->lo_short);
        tx_signal_atom(0, bitval ? psignal->hi_short : psignal->hi_long);
    }
    tx_signal_atom(1, psignal->sep);
}


// * **************** *********************************************************
// * RfSendManchester *********************************************************
// * **************** *********************************************************

class RfSendManchester: public RfSend {
    private:
        virtual void tx_data_once(const byte *data) const override;

    public:
        RfSendManchester(byte arg_pin_rfout, byte arg_convention,
                byte arg_nb_repeats, bool (*arg_repeat_callback)(),
                const SignalShape* arg_psignal):
            RfSend(arg_pin_rfout, arg_convention,
                    arg_nb_repeats, arg_repeat_callback, arg_psignal) { }
        virtual ~RfSendManchester();
};

RfSendManchester::~RfSendManchester() { }

void RfSendManchester::tx_data_once(const byte *data) const {
        // Like always with Manchester, we must start with a leading zero (not
        // part of user data).
    for (int i = psignal->nb_bits; i >= 0; --i) {
        byte bitval = (i == psignal->nb_bits ? 0 : get_nth_bit(data, i));
            // Not a typo. Manchester uses one timing by definition. We
            // arbitrarily decide that it is lo_short.
        tx_signal_atom(bitval, psignal->lo_short);
        tx_signal_atom(1 - bitval, psignal->lo_short);
    }
    tx_signal_atom(1, psignal->sep);
}


// * ********* ****************************************************************
// * Execution *************************************************************
// * ********* ****************************************************************

#define PIN_RFOUT  4
#define PIN_BUTTON 6
#define PIN_LED    5

const SignalShape signal_flo(
    24000,          // initseq
    650,            // first_lo_ign
    650,            // lo_short
    1300,           // lo_long
    0,              // hi_short
    0,              // hi_long
    650,            // lo_last
    24000,          // sep
    12              // nb_bits
);

const SignalShape signal_adf(
    5500,           // initseq
    0,              // first_lo_ign
    1150,           // lo_short
    0,              // lo_long
    0,              // hi_short
    0,              // hi_long
    0,              // lo_last
    6900,           // sep
    32              // nb_bits
);

RfSend *rf_flo;
RfSend *rf_adf;

bool button_is_pressed() {
    return digitalRead(PIN_BUTTON) == LOW;
}

void setup() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);

    digitalWrite(PIN_LED, LOW);

    Serial.begin(115200);

        // RfSend constructor performs some assert that write to Serial before
        // blocking execution. If construction is done before setup() execution
        // (as is the case with global variables), no output is done and we
        // loose interesting debug information.
        // Therefore I prefer this style over creating a global radio object.

    rf_flo = new RfSendTribitInv(
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        0,
        button_is_pressed,
        &signal_flo
    );

    rf_adf = new RfSendManchester(
        PIN_RFOUT,
        RFSEND_DEFAULT_CONVENTION,
        0,
        button_is_pressed,
        &signal_adf
    );
}

const byte mydata_flo[] = { 0x05, 0x55};
const byte mydata_adf[] = { 0xee, 0xdc, 0x56, 0x78 };

void loop() {
    if (button_is_pressed()) {
        digitalWrite(PIN_LED, HIGH);
        int n = rf_adf->send(mydata_adf);
        Serial.print("Envoi effectué ");
        Serial.print(n);
        Serial.print(" fois\n");
        digitalWrite(PIN_LED, LOW);

        while (button_is_pressed())
            ;
    }
}

// vim: ts=4:sw=4:tw=80:et

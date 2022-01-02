// RF433send.cpp

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

#include <Arduino.h>

#include "RF433send.h"

#define RFSEND_ASSERT_OUTPUT_TO_SERIAL

#define rfsend_assert(cond) { \
    if (!(cond)) { \
        assert_failed(__LINE__); \
    } \
}
static void assert_failed(int line) {
#ifdef RFSEND_ASSERT_OUTPUT_TO_SERIAL
    Serial.print("\nrf433send.cpp");
    Serial.print(":");
    Serial.print(line);
    Serial.print(":");
    Serial.print(" assertion failed, aborted.\n");
    delay(10);
#endif
    while (1)
        ;
}


// * ****** *******************************************************************
// * RfSend *******************************************************************
// * ****** *******************************************************************

RfSend::RfSend(byte arg_pin_rfout, byte arg_convention, byte arg_nb_repeats,
                bool (*arg_repeat_callback)(), uint16_t arg_initseq,
                uint16_t arg_lo_prefix, uint16_t arg_hi_prefix,
                uint16_t arg_first_lo_ign, uint16_t arg_lo_short,
                uint16_t arg_lo_long, uint16_t arg_hi_short,
                uint16_t arg_hi_long, uint16_t arg_lo_last, uint16_t arg_sep,
                byte arg_nb_bits):
        pin_rfout(arg_pin_rfout),
        convention(arg_convention),
        nb_repeats(arg_nb_repeats),
        repeat_callback(arg_repeat_callback),
        initseq(arg_initseq),
        lo_prefix(arg_lo_prefix),
        hi_prefix(arg_hi_prefix),
        first_lo_ign(arg_first_lo_ign),
        lo_short(arg_lo_short),
        lo_long(arg_lo_long),
        hi_short(arg_hi_short),
        hi_long(arg_hi_long),
        lo_last(arg_lo_last),
        sep(arg_sep),
        nb_bits(arg_nb_bits),
        nb_bytes((arg_nb_bits + 7) >> 3)
{
// *WRITE NOTHING HERE* - THE 2 INSTRUCTIONS BELOW MUST BE CALLED FIRST THING.
// Would something bad happen (like a failed assert), we don't want to end up
// with a TX left active.
    pinMode(pin_rfout, OUTPUT); // Must remain 1st instruction in constructor
    put_tx_at_rest();           // Must remain 2nd instruction in constructor

    rfsend_assert(convention == RFSEND_CONVENTION_0
                 || convention == RFSEND_CONVENTION_1);

    rfsend_assert(nb_repeats || repeat_callback);
#ifdef RFSEND_HARDCODED_MAX_NB_REPEATS
    if (!nb_repeats || nb_repeats > RFSEND_HARDCODED_MAX_NB_REPEATS)
        nb_repeats = RFSEND_HARDCODED_MAX_NB_REPEATS;
#endif

    rfsend_assert(nb_bits);
        // Defensive programming
        // If nb_bits is non-zero, nb_bytes is for sure non-zero, too.
    rfsend_assert(nb_bytes);

        // lo_prefix and hi_prefix work always together, you can have none or
        // both, but not one without the other.
    rfsend_assert((!lo_prefix && !hi_prefix) || (lo_prefix && hi_prefix));

    rfsend_assert((!hi_short && !hi_long) || (hi_short && hi_long));
    if (!hi_short) {
        hi_short = lo_short;
        hi_long = lo_long;
    }
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

    rfsend_assert(n >= 0);
    rfsend_assert(n < nb_bits);

    int index = (int)nb_bytes - 1 - (n >> 3);
        // Defensive programming: test on nb_bits is sufficient.
    rfsend_assert(index >= 0 && index < nb_bytes);

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

#define RFSEND_MYDELAY_STEP        10000

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || \
    defined(__AVR_ATtiny85__)
  // Attiny case
#if F_CPU == 16000000UL
#define RFSEND_MYDELAY_TIMER_SHIFT 4
#elif F_CPU == 8000000UL
#define RFSEND_MYDELAY_TIMER_SHIFT 3
#else
#define RFSEND_MYDELAY_TIMER_SHIFT 0
#endif

#else
  // non-Attiny case
#define RFSEND_MYDELAY_TIMER_SHIFT 0

#endif

void RfSend::mydelay_us(unsigned long d) const {
    d >>= RFSEND_MYDELAY_TIMER_SHIFT;

    while (d >= RFSEND_MYDELAY_STEP) {
        d -= RFSEND_MYDELAY_STEP;
        delayMicroseconds(RFSEND_MYDELAY_STEP);
    }
    delayMicroseconds(d);
}

int RfSend::send(byte len, const byte *data) const {
    rfsend_assert(len == nb_bytes);

    tx_signal_atom(1, initseq);

    int count = 0;
    bool repeat = true;
    while (repeat) {
        if (lo_prefix) {
            tx_signal_atom(0, lo_prefix);
            tx_signal_atom(1, hi_prefix);
        }

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

RfSendTribit::~RfSendTribit() { }

void RfSendTribit::tx_data_once(const byte *data) const {
    for (int i = nb_bits - 1; i >= 0; --i) {
        byte bitval = get_nth_bit(data, i);
        tx_signal_atom(0, bitval ? lo_long : lo_short);
        tx_signal_atom(1, bitval ? hi_short : hi_long);
    }
    tx_signal_atom(0, lo_last);
    tx_signal_atom(1, sep);
}


// * *************** **********************************************************
// * RfSendTribitInv **********************************************************
// * *************** **********************************************************

RfSendTribitInv::~RfSendTribitInv() { }

void RfSendTribitInv::tx_data_once(const byte *data) const {
    tx_signal_atom(0, first_lo_ign);
    for (int i = nb_bits - 1; i >= 0; --i) {
        byte bitval = get_nth_bit(data, i);
        tx_signal_atom(1, bitval ? hi_long : hi_short);
        tx_signal_atom(0, bitval ? lo_short : lo_long);
    }
    tx_signal_atom(1, sep);
}


// * **************** *********************************************************
// * RfSendManchester *********************************************************
// * **************** *********************************************************

RfSendManchester::~RfSendManchester() { }

void RfSendManchester::tx_data_once(const byte *data) const {
        // Like always with Manchester, we must start with a leading zero (not
        // part of user data).
    for (int i = nb_bits; i >= 0; --i) {
        byte bitval = (i == nb_bits ? 0 : get_nth_bit(data, i));
            // Not a typo. Manchester uses one timing by definition. We
            // arbitrarily decide that it is lo_short.
        tx_signal_atom(bitval, lo_short);
        tx_signal_atom(1 - bitval, lo_short);
    }
    tx_signal_atom(1, sep);
}


// * ************** ***********************************************************
// * rfsend_builder ***********************************************************
// * ************** ***********************************************************

    // Yes, code below is uggly...
    // TODO (?)
    //   Use a struct to put together all these parameters?
RfSend* rfsend_builder(RfSendEncoding enc, byte pin_rfout, byte convention,
        byte nb_repeats, bool (*repeat_callback)(), uint16_t initseq,
        uint16_t lo_prefix, uint16_t hi_prefix, uint16_t first_lo_ign,
        uint16_t lo_short, uint16_t lo_long, uint16_t hi_short,
        uint16_t hi_long, uint16_t lo_last, uint16_t sep, byte nb_bits) {
    RfSend *ret;
    if (enc == RfSendEncoding::TRIBIT) {
        ret = new RfSendTribit(pin_rfout, convention, nb_repeats,
                repeat_callback, initseq, lo_prefix, hi_prefix, first_lo_ign,
                lo_short, lo_long, hi_short, hi_long, lo_last, sep, nb_bits);
    } else if (enc == RfSendEncoding::TRIBIT_INVERTED) {
        ret = new RfSendTribitInv(pin_rfout, convention, nb_repeats,
                repeat_callback, initseq, lo_prefix, hi_prefix, first_lo_ign,
                lo_short, lo_long, hi_short, hi_long, lo_last, sep, nb_bits);
    } else if (enc == RfSendEncoding::MANCHESTER) {
        ret = new RfSendManchester(pin_rfout, convention, nb_repeats,
                repeat_callback, initseq, lo_prefix, hi_prefix, first_lo_ign,
                lo_short, lo_long, hi_short, hi_long, lo_last, sep, nb_bits);
    } else {
        rfsend_assert(false);
    }

    return ret;
}

// vim: ts=4:sw=4:tw=80:et

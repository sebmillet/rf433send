// RF433send.h

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

//
// Schematic:
//   RF433 TRANSMITTER data pin plugged on Arduino D4
//   Button plugged to Arduino GND and Arduino D6
//   Led (+ resistor) plugged to Arduino GND and Arduino D5
//
//   See file schema.fzz (Fritzing format) or schema.png
//

#ifndef _RF433SEND_H
#define _RF433SEND_H

#include <Arduino.h>

#define RFSEND_CONVENTION_0               0
#define RFSEND_CONVENTION_1               1
#define RFSEND_DEFAULT_CONVENTION         RFSEND_CONVENTION_0

#define RFSEND_HARDCODED_MAX_NB_REPEATS   200

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
            // false before nb_repeats has been reached.
            //
        byte nb_repeats;           // Repeat that number of times
        bool (*repeat_callback)(); // Repeat until repeat_callback() returns
                                   // false.

        virtual void put_tx_at_rest() const;
        virtual void mydelay_us(unsigned long d) const;
        virtual void tx_data_once(const byte *data) const = 0;

    protected:
        uint16_t initseq;
        uint16_t lo_prefix;
        uint16_t hi_prefix;
        uint16_t first_lo_ign;
        uint16_t lo_short;
        uint16_t lo_long;
        uint16_t hi_short;
        uint16_t hi_long;
        uint16_t lo_last;
        uint16_t sep;
        byte nb_bits;

        byte nb_bytes;

        virtual byte get_nth_bit(const byte *data, int n) const;
        virtual void tx_signal_atom(byte bitval, unsigned long d) const;

    public:
        RfSend(byte arg_pin_rfout, byte arg_convention, byte arg_nb_repeats,
                bool (*arg_repeat_callback)(), uint16_t arg_initseq,
                uint16_t arg_lo_prefix, uint16_t arg_hi_prefix,
                uint16_t arg_first_lo_ign, uint16_t arg_lo_short,
                uint16_t arg_lo_long, uint16_t arg_hi_short,
                uint16_t arg_hi_long, uint16_t arg_lo_last, uint16_t arg_sep,
                byte arg_nb_bits);
        virtual ~RfSend();

            // About send() method being 'final': I wanted to highlight the fact
            // that it MUST end with execution of put_tx_at_rest(), to be sure
            // TX is not left active, no matter what a child is doing. Of course
            // you can remove 'final' and override the method happily.
            // But, you've been warned... ;-)
            //
            // About 'len' parameter: yes, it is not useful.
            // nb_bits is given to the constructor and this enforces nb_bytes.
            // When send() is called, len MUST BE equal to nb_bytes as
            // calculated from nb_bits at construction time.
            // 'len' is requested as a safeguard, to avoid mistakes when using
            // different protocols.
            // If 'len' does not have expected value, this causes an assert to
            // fail.
        virtual int send(byte len, const byte *data) const final;
};


// * ************ *************************************************************
// * RfSendTribit *************************************************************
// * ************ *************************************************************

class RfSendTribit: public RfSend {
    private:
        virtual void tx_data_once(const byte *data) const override;

    public:
        RfSendTribit(byte arg_pin_rfout, byte arg_convention,
                byte arg_nb_repeats, bool (*arg_repeat_callback)(),
                uint16_t arg_initseq, uint16_t arg_lo_prefix,
                uint16_t arg_hi_prefix, uint16_t arg_first_lo_ign,
                uint16_t arg_lo_short, uint16_t arg_lo_long,
                uint16_t arg_hi_short, uint16_t arg_hi_long,
                uint16_t arg_lo_last, uint16_t arg_sep, byte arg_nb_bits):
            RfSend(arg_pin_rfout, arg_convention, arg_nb_repeats,
                    arg_repeat_callback, arg_initseq, arg_lo_prefix,
                    arg_hi_prefix, arg_first_lo_ign, arg_lo_short, arg_lo_long,
                    arg_hi_short, arg_hi_long, arg_lo_last, arg_sep,
                    arg_nb_bits) { }
        virtual ~RfSendTribit();
};


// * *************** **********************************************************
// * RfSendTribitInv **********************************************************
// * *************** **********************************************************

class RfSendTribitInv: public RfSend {
    private:
        virtual void tx_data_once(const byte *data) const override;

    public:
        RfSendTribitInv(byte arg_pin_rfout, byte arg_convention,
                byte arg_nb_repeats, bool (*arg_repeat_callback)(),
                uint16_t arg_initseq, uint16_t arg_lo_prefix,
                uint16_t arg_hi_prefix, uint16_t arg_first_lo_ign,
                uint16_t arg_lo_short, uint16_t arg_lo_long,
                uint16_t arg_hi_short, uint16_t arg_hi_long,
                uint16_t arg_lo_last, uint16_t arg_sep, byte arg_nb_bits):
            RfSend(arg_pin_rfout, arg_convention, arg_nb_repeats,
                    arg_repeat_callback, arg_initseq, arg_lo_prefix,
                    arg_hi_prefix, arg_first_lo_ign, arg_lo_short, arg_lo_long,
                    arg_hi_short, arg_hi_long, arg_lo_last, arg_sep,
                    arg_nb_bits) { }
        virtual ~RfSendTribitInv();
};


// * **************** *********************************************************
// * RfSendManchester *********************************************************
// * **************** *********************************************************

class RfSendManchester: public RfSend {
    private:
        virtual void tx_data_once(const byte *data) const override;

    public:
        RfSendManchester(byte arg_pin_rfout, byte arg_convention,
                byte arg_nb_repeats, bool (*arg_repeat_callback)(),
                uint16_t arg_initseq, uint16_t arg_lo_prefix,
                uint16_t arg_hi_prefix, uint16_t arg_first_lo_ign,
                uint16_t arg_lo_short, uint16_t arg_lo_long,
                uint16_t arg_hi_short, uint16_t arg_hi_long,
                uint16_t arg_lo_last, uint16_t arg_sep, byte arg_nb_bits):
            RfSend(arg_pin_rfout, arg_convention, arg_nb_repeats,
                    arg_repeat_callback, arg_initseq, arg_lo_prefix,
                    arg_hi_prefix, arg_first_lo_ign, arg_lo_short, arg_lo_long,
                    arg_hi_short, arg_hi_long, arg_lo_last, arg_sep,
                    arg_nb_bits) { }
        virtual ~RfSendManchester();
};


// * ************** ***********************************************************
// * rfsend_builder ***********************************************************
// * ************** ***********************************************************

enum class RfSendEncoding {
    TRIBIT,
    TRIBIT_INVERTED,
    MANCHESTER
};

RfSend* rfsend_builder(RfSendEncoding enc, byte pin_rfout, byte convention,
        byte nb_repeats, bool (*repeat_callback)(), uint16_t initseq,
        uint16_t lo_prefix, uint16_t hi_prefix, uint16_t first_lo_ign,
        uint16_t lo_short, uint16_t lo_long, uint16_t hi_short,
        uint16_t hi_long, uint16_t lo_last, uint16_t sep, byte nb_bits);

#endif // _RF433SEND_H

// vim: ts=4:sw=4:tw=80:et

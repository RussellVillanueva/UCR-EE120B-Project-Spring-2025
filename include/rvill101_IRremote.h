#ifndef IRREMOTE_H
#define IRREMOTE_H

/* NEC IR decoder using Timer1 Input Capture on your recvpin (e.g. PB0 / D8) */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define MAX_EDGES         100
#define HEADER_MARK_MIN  12000  // ≈6 ms (0.5µs ticks)
#define HEADER_MARK_MAX  20000  // ≈10 ms
#define HEADER_SPACE_MIN  4000  // ≈2 ms
#define HEADER_SPACE_MAX 10000  // ≈5 ms
#define BIT_SPACE_THRESH  2000  // ≈1 ms

// scratch buffer while looking for header
static volatile uint16_t _tmp[MAX_EDGES];
static volatile uint8_t  _tmp_cnt;

// once header found, real capture buffer
static volatile uint16_t _buf[MAX_EDGES];
static volatile uint8_t  _buf_cnt, _pulse_len;

// state flags
static volatile uint8_t  _hdr_ok, _got_frame, _first;

// last capture timestamp
static volatile uint16_t _last;

// ISR: Input-Capture on ICP1 (PB0 / D8)
ISR(TIMER1_CAPT_vect) {
    uint16_t now = ICR1;
    if (!_first) {
        _first = 1;
        _last  = now;
        TCCR1B |= (1<<ICES1);  // switch to rising
        return;
    }
    uint16_t dt = now - _last;
    _last = now;
    TCCR1B ^= (1<<ICES1);     // toggle edge

    if (!_hdr_ok) {
        if (_tmp_cnt < MAX_EDGES) _tmp[_tmp_cnt++] = dt;
        else       { _tmp[0] = _tmp[_tmp_cnt-1]; _tmp_cnt = 1; }
        if (_tmp_cnt >= 2) {
            uint16_t m = _tmp[_tmp_cnt-2], s = _tmp[_tmp_cnt-1];
            if (m >= HEADER_MARK_MIN && m <= HEADER_MARK_MAX
             && s >= HEADER_SPACE_MIN && s <= HEADER_SPACE_MAX) {
                _hdr_ok = 1;
                _buf_cnt = 0;
                _buf[_buf_cnt++] = m;
                _buf[_buf_cnt++] = s;
            }
        }
    } else {
        if (_buf_cnt < MAX_EDGES) _buf[_buf_cnt++] = dt;
        if (_buf_cnt >= 66) {
            _pulse_len = _buf_cnt;
            _got_frame = 1;
            _hdr_ok    = 0;
            _tmp_cnt   = 0;
            _first     = 0;
        }
    }
}

static void _timer1_init(void) {
    TCCR1A = 0;
    TCCR1B = (1<<CS11);     // prescaler=8, start on falling (ICES1=0)
    TCNT1  = 0;
    _first   = 0;
    _tmp_cnt = 0;
    _buf_cnt = 0;
    _got_frame = 0;
    _hdr_ok  = 0;
    _last    = 0;
    TIMSK1 |= (1<<ICIE1);
}

// pull 32 bits out LSB-first
static uint32_t _decode_nec(void) {
    int st = -1;
    for (int i = 0; i+1 < _pulse_len; ++i) {
        if (_buf[i] >= HEADER_MARK_MIN && _buf[i] <= HEADER_MARK_MAX
         && _buf[i+1] >= HEADER_SPACE_MIN && _buf[i+1] <= HEADER_SPACE_MAX) {
            st = i; break;
        }
    }
    if (st < 0) return 0;
    if (_pulse_len < 10) return 0xFFFFFFFF;
    uint32_t code = 0;
    for (uint8_t b = 0; b < 32; ++b) {
        uint16_t space = _buf[st + 2 + b*2 + 1];
        if (space > BIT_SPACE_THRESH) code |= (1UL << b);
    }
    return code;
}

// public API
static inline void IRinit(void) {
    _timer1_init();
    sei();
}

static inline void IRresume(void) {
    _got_frame = 0;
}

// returns 1 if a code (or repeat) is ready, and writes to *p
static inline int ir_read(uint32_t *p) {
    if (!_got_frame) return 0;
    _got_frame = 0;
    *p = _decode_nec();
    return 1;
}

#endif // IRREMOTE_H

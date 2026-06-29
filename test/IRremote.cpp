// // #include <avr/io.h>
// // #include <avr/interrupt.h>
// // #include "IRremote.h"

// // #define MAX_EDGES          100
// // #define HEADER_MARK_MIN  12000  // ≈6 ms (ticks @0.5µs)
// // #define HEADER_MARK_MAX  20000  // ≈10 ms
// // #define HEADER_SPACE_MIN  4000  // ≈2 ms
// // #define HEADER_SPACE_MAX 10000  // ≈5 ms
// // #define BIT_SPACE_THRESH  2000  // ≈1 ms

// // // scratch buffer while looking for header
// // static volatile uint16_t _tmp[MAX_EDGES];
// // static volatile uint8_t  _tmp_cnt;

// // // once header found, real capture buffer
// // static volatile uint16_t _buf[MAX_EDGES];
// // static volatile uint8_t  _buf_cnt, _pulse_len;

// // // state flags
// // static volatile uint8_t  _hdr_ok, _got_frame, _first;

// // // last capture timestamp
// // static volatile uint16_t _last;

// // // ISR: Input-Capture on D8 (ICP1)
// // ISR(TIMER1_CAPT_vect) {
// //     uint16_t now = ICR1;
// //     if (!_first) {
// //         // drop very first edge; align on falling
// //         _first = 1;
// //         _last  = now;
// //         // switch to rising
// //         TCCR1B |= (1<<ICES1);
// //         return;
// //     }
// //     // measure interval
// //     uint16_t dt = now - _last;
// //     _last = now;
// //     // toggle edge
// //     TCCR1B ^= (1<<ICES1);

// //     if (!_hdr_ok) {
// //         // buffer until we detect header mark+space
// //         if (_tmp_cnt < MAX_EDGES) {
// //             _tmp[_tmp_cnt++] = dt;
// //         } else {
// //             // overflow: keep only newest
// //             _tmp[0]   = _tmp[_tmp_cnt-1];
// //             _tmp_cnt  = 1;
// //         }
// //         if (_tmp_cnt >= 2) {
// //             uint16_t m = _tmp[_tmp_cnt-2], s = _tmp[_tmp_cnt-1];
// //             if (m >= HEADER_MARK_MIN && m <= HEADER_MARK_MAX
// //              && s >= HEADER_SPACE_MIN && s <= HEADER_SPACE_MAX) {
// //                 // header found: seed real buffer
// //                 _hdr_ok   = 1;
// //                 _buf_cnt  = 0;
// //                 _buf[_buf_cnt++] = m;
// //                 _buf[_buf_cnt++] = s;
// //             }
// //         }
// //     } else {
// //         // record real bits (mark+space pairs) up to 2 + 32*2 = 66 edges
// //         if (_buf_cnt < MAX_EDGES) {
// //             _buf[_buf_cnt++] = dt;
// //         }
// //         if (_buf_cnt >= 66) {
// //             _pulse_len  = _buf_cnt;
// //             _got_frame  = 1;
// //             _hdr_ok     = 0;
// //             _tmp_cnt    = 0;
// //             _first      = 0;
// //         }
// //     }
// // }

// // static void _timer1_init(void) {
// //     TCCR1A = 0;
// //     TCCR1B = (1<<CS11);   // prescaler=8, ICES1=0 (start on falling)
// //     TCNT1  = 0;
// //     _first      = 0;
// //     _tmp_cnt    = 0;
// //     _buf_cnt    = 0;
// //     _got_frame  = 0;
// //     _hdr_ok     = 0;
// //     _last       = 0;
// //     TIMSK1    |= (1<<ICIE1);
// // }

// // // scan for the header in _buf[]
// // static int _find_header(void) {
// //     for (uint8_t i = 0; i+1 < _pulse_len; i++) {
// //         uint16_t m = _buf[i], s = _buf[i+1];
// //         if (m >= HEADER_MARK_MIN && m <= HEADER_MARK_MAX
// //          && s >= HEADER_SPACE_MIN && s <= HEADER_SPACE_MAX) {
// //             return i;
// //         }
// //     }
// //     return -1;
// // }

// // // pull 32 bits out LSB-first
// // static uint32_t _decode_nec(void) {
// //     int st = _find_header();
// //     if (st < 0) return 0;
// //     // repeat‐frame check: if only header and no data, NEC uses repeat code
// //     if (_pulse_len < 10) return 0xFFFFFFFF;
// //     uint32_t code = 0;
// //     for (uint8_t b = 0; b < 32; b++) {
// //         uint16_t space = _buf[st + 2 + b*2 + 1];
// //         if (space > BIT_SPACE_THRESH)
// //             code |= (1UL << b);
// //     }
// //     return code;
// // }

// // void ir_init(void) {
// //     _timer1_init();
// //     sei();
// // }

// // uint32_t ir_read(void) {
// //     if (!_got_frame) return 0;
// //     _got_frame = 0;
// //     return _decode_nec();
// // }



// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include "IRremote.h"

// // NEC timing thresholds @ 0.5µs ticks (Timer0 prescaler = 8 @16MHz)
// #define MAX_EDGES           100
// #define HEADER_MARK_MIN   12000  //  6 ms / 0.5µs
// #define HEADER_MARK_MAX   20000  // 10 ms / 0.5µs
// #define HEADER_SPACE_MIN  4000   //  2 ms
// #define HEADER_SPACE_MAX 10000   //  5 ms
// #define BIT_SPACE_THRESH  2000   //  1 ms

// static volatile uint16_t _buf[MAX_EDGES];
// static volatile uint8_t  _buf_cnt, _pulse_len, _got_frame, _header_seen;
// static volatile uint32_t _ovf_cnt, _last_time, _last_dt;
// volatile uint8_t header_flag = 0;


// // — micros() via Timer0 overflow + TCNT0
// static inline uint32_t _micros(void) {
//     return (_ovf_cnt << 8) + TCNT0;
// }

// // Timer0 overflow → extend to 32-bit microsecond counter
// ISR(TIMER0_OVF_vect) {
//     _ovf_cnt++;
// }

// // Pin-change on PCINT0 (PB0 / D8) → timestamp edges
// ISR(PCINT0_vect) {
//     uint32_t now = _micros();
//     uint32_t dt  = now - _last_time;
//     // detect NEC header (9 ms mark + 4.5 ms space)
//     if (!_header_seen) {
//         if (_last_dt >= HEADER_MARK_MIN && _last_dt <= HEADER_MARK_MAX
//          && dt      >= HEADER_SPACE_MIN && dt      <= HEADER_SPACE_MAX) 
//         {
//             _header_seen = 1;
//             header_flag = 1;
//             _buf_cnt     = 0;
//             _buf[_buf_cnt++] = (uint16_t)_last_dt;
//             _buf[_buf_cnt++] = (uint16_t)dt;
//         }
//     }
//     else {
//         if (_buf_cnt < MAX_EDGES) {
//             _buf[_buf_cnt++] = (uint16_t)dt;
//         }
//         if (_buf_cnt >= 66) {
//             _pulse_len   = _buf_cnt;
//             _buf_cnt     = 0;
//             _got_frame   = 1;
//             _header_seen = 0;
//         }
//     }
//     _last_dt   = dt;
//     _last_time = now;
// }

// // Decode the 32 data bits (LSB first) after header
// static uint32_t _decode_nec(void) {
//     if (_pulse_len < 66) return 0;
//     if (!(_buf[0] >= HEADER_MARK_MIN && _buf[0] <= HEADER_MARK_MAX
//        && _buf[1] >= HEADER_SPACE_MIN && _buf[1] <= HEADER_SPACE_MAX)) {
//         return 0;
//     }
//     uint32_t code = 0;
//     for (uint8_t b = 0; b < 32; b++) {
//         if (_buf[2 + b*2 + 1] > BIT_SPACE_THRESH) {
//             code |= (1UL << b);
//         }
//     }
//     return code;
// }

// void ir_init(void) {
//     // Timer0 as free-running @ 0.5µs ticks
//     TCCR0A  = 0;
//     TCCR0B  = (1 << CS01);   // prescaler /8
//     TCNT0   = 0;
//     _ovf_cnt = 0;
//     TIMSK0 |= (1 << TOIE0);  // overflow interrupt

//     // PCINT0 on PB0 (D8)
//     DDRB   &= ~(1 << PB0);
//     PORTB  |=  (1 << PB0);
//     PCICR  |=  (1 << PCIE0);
//     PCMSK0 |=  (1 << PCINT0);

//     _buf_cnt    = 0;
//     _got_frame  = 0;
//     _header_seen= 0;
//     _last_dt    = 0;
//     _last_time  = _micros();

//     sei();  // global interrupts on
// }

// uint32_t ir_read(void) {
//     if (!_got_frame) return 0;
//     _got_frame = 0;
//     return _decode_nec();
// }

// uint32_t ir_micros(void) {
//     return _micros();
// }




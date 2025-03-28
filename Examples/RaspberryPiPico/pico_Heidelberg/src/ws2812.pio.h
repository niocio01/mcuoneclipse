// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ------ //
// ws2812 //
// ------ //

#define ws2812_wrap_target 0
#define ws2812_wrap 3
#define ws2812_pio_version 0

#define ws2812_T1 4
#define ws2812_T2 3
#define ws2812_T3 3

static const uint16_t ws2812_program_instructions[] = {
            //     .wrap_target
    0x6221, //  0: out    x, 1            side 0 [2] 
    0x1223, //  1: jmp    !x, 3           side 1 [2] 
    0x1300, //  2: jmp    0               side 1 [3] 
    0xa342, //  3: nop                    side 0 [3] 
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program ws2812_program = {
    .instructions = ws2812_program_instructions,
    .length = 4,
    .origin = -1,
    .pio_version = 0,
#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0x0
#endif
};

static inline pio_sm_config ws2812_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + ws2812_wrap_target, offset + ws2812_wrap);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}

#include "hardware/clocks.h"
static inline void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, float freq, bool rgbw) {
  pio_gpio_init(pio, pin);
  pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true); /* set pin direction. 1: number of pins. true: as output pin */
  pio_sm_config c = ws2812_program_get_default_config(offset);
  sm_config_set_sideset_pins(&c, pin); /* use pin as side set */
  sm_config_set_out_shift(&c, false, true, rgbw ? 32 : 24); /* false: shift left. true: auto-pull. Number of bits based on rgb or rgbw */
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX); /* combine both FIFOs as TX_FIFO, so we have a bigger FIFO */
  /* calculate state machine clocking based on protocol needs */
  int cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
  float div = clock_get_hz(clk_sys) / (freq * cycles_per_bit);
  sm_config_set_clkdiv(&c, div);
  /* initialize PIO and start it */
  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true); /* true: enable pio */
}

#endif

// --------------- //
// ws2812_parallel //
// --------------- //

#define ws2812_parallel_wrap_target 0
#define ws2812_parallel_wrap 4
#define ws2812_parallel_pio_version 0

#define ws2812_parallel_T1 4
#define ws2812_parallel_T2 3
#define ws2812_parallel_T3 3

static const uint16_t ws2812_parallel_program_instructions[] = {
            //     .wrap_target
    0xa023, //  0: mov    x, null                    
    0x6028, //  1: out    x, 8                       
    0xa20b, //  2: mov    pins, !null            [2] 
    0xa301, //  3: mov    pins, x                [3] 
    0xa003, //  4: mov    pins, null                 
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program ws2812_parallel_program = {
    .instructions = ws2812_parallel_program_instructions,
    .length = 5,
    .origin = -1,
    .pio_version = 0,
#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0x0
#endif
};

static inline pio_sm_config ws2812_parallel_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + ws2812_parallel_wrap_target, offset + ws2812_parallel_wrap);
    return c;
}

#include "hardware/clocks.h"
static inline void ws2812_parallel_program_init(PIO pio, uint sm, uint offset, uint pin_base, uint pin_count, float freq) {
  for(uint i=pin_base; i<pin_base+pin_count; i++) {
    pio_gpio_init(pio, i);
  }
  pio_sm_set_consecutive_pindirs(pio, sm, pin_base, pin_count, true); /* configure number of pins. true: as output pin */
  pio_sm_config c = ws2812_parallel_program_get_default_config(offset);
  sm_config_set_out_shift(&c, true, true, 32); /* true: shift left. true: auto-pull. 32: number of bits */
  sm_config_set_out_pins(&c, pin_base, pin_count); /* set the 'out' pins */
  sm_config_set_set_pins(&c, pin_base, pin_count); /* set the 'set' pins */
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX); /* combine both FIFOs as TX_FIFO, so we have a bigger FIFO */
  /* calculate state machine clocking based on protocol needs */
  int cycles_per_bit = ws2812_parallel_T1 + ws2812_parallel_T2 + ws2812_parallel_T3;
  float div = clock_get_hz(clk_sys) / (freq * cycles_per_bit);
  sm_config_set_clkdiv(&c, div);
  /* initialize PIO and start it */
  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true); /* true: enable pio */
}

#endif


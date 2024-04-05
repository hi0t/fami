#include "delay.h"
#include "si5351.h"
#include "stm8s.h"
#include <stdint.h>

#define SW_PIN 3
#define UNUSED_PIN 6

static void change_freq(uint8_t sw)
{
    // si5351_setup_multisynth_int(0, SI5351_PLL_B, 26); // PAL (26.601712 MHz)
    // si5351_setup_multisynth_int(0, SI5351_PLL_A, 30); // NTSC (21.477270 MHz)
    // si5351_setup_multisynth_int(0, SI5351_PLL_A, 24); // PAL 5/4 (26.8465875 MHz)

    si5351_enable_outputs(0);
    if (sw) {
        si5351_setup_multisynth_int(0, SI5351_PLL_B, 26);
        si5351_setup_multisynth_int(1, SI5351_PLL_B, 26);
    } else {
        si5351_setup_multisynth_int(0, SI5351_PLL_A, 30);
        si5351_setup_multisynth_int(1, SI5351_PLL_A, 30);
    }
    si5351_enable_outputs(1);
}

void main(void)
{
    PD_DDR |= (1 << UNUSED_PIN);
    PD_CR1 |= (1 << UNUSED_PIN);

    si5351_init(SI5351_CRYSTAL_LOAD_10PF);
    si5351_setup_pll(SI5351_PLL_A, 25, 193181, 250000); // NTSC (21.477270 MHz), PAL 5/4 (26.8465875 MHz)
    si5351_setup_pll(SI5351_PLL_B, 27, 520141, 781250); // PAL (26.601712 MHz)

    uint8_t sw = PC_IDR & (1 << SW_PIN);
    uint8_t prev = sw;
    change_freq(sw);
    while (1) {
        sw = PC_IDR & (1 << SW_PIN);
        if (sw != prev) {
            delay_ms(250);
            if (sw != prev) {
                prev = sw;
                change_freq(sw);
            }
        }
        nop();
    }
}

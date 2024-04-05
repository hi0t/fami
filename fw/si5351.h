#pragma once

#include <stdint.h>

typedef enum {
    SI5351_CRYSTAL_LOAD_6PF = (1 << 6),
    SI5351_CRYSTAL_LOAD_8PF = (2 << 6),
    SI5351_CRYSTAL_LOAD_10PF = (3 << 6)
} si5351_crystal_load_t;

typedef enum {
    SI5351_PLL_A,
    SI5351_PLL_B,
} si5351_pll_t;

// The app http://www.adafruit.com/downloads/ClockBuilderDesktopSwInstallSi5351.zip
// is used to calculate pll and multisynth. R divider not implemented

void si5351_init(si5351_crystal_load_t load_cap);
void si5351_enable_outputs(uint8_t enabled);
// PLL "Feedback divider"
void si5351_setup_pll_int(si5351_pll_t pll, uint8_t mult);
void si5351_setup_pll(si5351_pll_t pll, uint8_t mult, uint32_t num, uint32_t denom);
// Output "Multisynth divider"
void si5351_setup_multisynth_int(uint8_t output, si5351_pll_t src, uint32_t div);
void si5351_setup_multisynth(uint8_t output, si5351_pll_t src, uint32_t div, uint32_t num, uint32_t denom);

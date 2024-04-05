#include "si5351.h"
#include "i2c.h"
#include <math.h>

#define SI5351_ADDRESS 0x60

#define SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL 3
#define SI5351_REGISTER_16_CLK0_CONTROL 16
#define SI5351_REGISTER_17_CLK1_CONTROL 17
#define SI5351_REGISTER_18_CLK2_CONTROL 18
#define SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1 42
#define SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1 50
#define SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1 58
#define SI5351_REGISTER_177_PLL_RESET 177
#define SI5351_REGISTER_183_CRYSTAL_INTERNAL_LOAD_CAPACITANCE 183

static void si5351_write(uint8_t addr, uint8_t val)
{
    i2c_start();
    i2c_write_addr((uint8_t)((SI5351_ADDRESS << 1) + I2C_WRITE));
    i2c_write(addr);
    i2c_write(val);
    i2c_stop();
}

void si5351_init(si5351_crystal_load_t load_cap)
{
    i2c_init();

    si5351_enable_outputs(0);
    si5351_write(SI5351_REGISTER_16_CLK0_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_17_CLK1_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_18_CLK2_CONTROL, 0x80);
    si5351_write(SI5351_REGISTER_183_CRYSTAL_INTERNAL_LOAD_CAPACITANCE, load_cap);
}

void si5351_enable_outputs(uint8_t enabled)
{
    si5351_write(SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, enabled ? 0x00 : 0xFF);
}

void si5351_setup_pll_int(si5351_pll_t pll, uint8_t mult)
{
    si5351_setup_pll(pll, mult, 0, 1);
}

void si5351_setup_pll(si5351_pll_t pll, uint8_t mult, uint32_t num, uint32_t denom)
{
    uint32_t P1;
    uint32_t P2;
    uint32_t P3;

    if (num == 0) {
        // Integer mode
        P1 = 128 * mult - 512;
        P2 = num;
        P3 = denom;
    } else {
        // Fractional mode
        P1 = (uint32_t)(128 * mult + floorf(128 * ((float)num / (float)denom)) - 512);
        P2 = (uint32_t)(128 * num - denom * floorf(128 * ((float)num / (float)denom)));
        P3 = denom;
    }

    uint8_t baseaddr = (pll == SI5351_PLL_A ? 26 : 34);
    si5351_write(baseaddr, (P3 & 0x0000FF00) >> 8);
    si5351_write(baseaddr + 1, (P3 & 0x000000FF));
    si5351_write(baseaddr + 2, (P1 & 0x00030000) >> 16);
    si5351_write(baseaddr + 3, (P1 & 0x0000FF00) >> 8);
    si5351_write(baseaddr + 4, (P1 & 0x000000FF));
    si5351_write(baseaddr + 5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
    si5351_write(baseaddr + 6, (P2 & 0x0000FF00) >> 8);
    si5351_write(baseaddr + 7, (P2 & 0x000000FF));

    si5351_write(SI5351_REGISTER_177_PLL_RESET, (pll == SI5351_PLL_A ? (1 << 5) : (1 << 7)));
}

void si5351_setup_multisynth_int(uint8_t output, si5351_pll_t src, uint32_t div)
{
    si5351_setup_multisynth(output, src, div, 0, 1);
}

void si5351_setup_multisynth(uint8_t output, si5351_pll_t src, uint32_t div, uint32_t num, uint32_t denom)
{
    uint32_t P1;
    uint32_t P2;
    uint32_t P3;

    if (num == 0) {
        // Integer mode
        P1 = 128 * div - 512;
        P2 = 0;
        P3 = denom;
    } else if (denom == 1) {
        // Fractional mode, simplified calculations
        P1 = 128 * div + 128 * num - 512;
        P2 = 128 * num - 128;
        P3 = 1;
    } else {
        // Fractional mode
        P1 = (uint32_t)(128 * div + floorf(128 * ((float)num / (float)denom)) - 512);
        P2 = (uint32_t)(128 * num - denom * floorf(128 * ((float)num / (float)denom)));
        P3 = denom;
    }

    uint8_t baseaddr = 0;
    switch (output) {
    case 0:
        baseaddr = SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1;
        break;
    case 1:
        baseaddr = SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1;
        break;
    case 2:
        baseaddr = SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1;
        break;
    }

    si5351_write(baseaddr, (P3 & 0x0000FF00) >> 8);
    si5351_write(baseaddr + 1, (P3 & 0x000000FF));
    si5351_write(baseaddr + 2, (P1 & 0x00030000) >> 16);
    si5351_write(baseaddr + 3, (P1 & 0x0000FF00) >> 8);
    si5351_write(baseaddr + 4, (P1 & 0x000000FF));
    si5351_write(baseaddr + 5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
    si5351_write(baseaddr + 6, (P2 & 0x0000FF00) >> 8);
    si5351_write(baseaddr + 7, (P2 & 0x000000FF));

    uint8_t clk = 0x0F; // 8mA drive strength
    if (src == SI5351_PLL_B)
        clk |= (1 << 5); // Uses PLLB
    if (num == 0)
        clk |= (1 << 6); // Integer mode
    switch (output) {
    case 0:
        si5351_write(SI5351_REGISTER_16_CLK0_CONTROL, clk);
        break;
    case 1:
        si5351_write(SI5351_REGISTER_17_CLK1_CONTROL, clk);
        break;
    case 2:
        si5351_write(SI5351_REGISTER_18_CLK2_CONTROL, clk);
        break;
    }
}

#pragma once
#include "esphome.h"
#include <Wire.h>

class TLV320ADC3101 : public Component {
 public:
  TLV320ADC3101(uint8_t i2c_address) : i2c_address_(i2c_address) {}

  void setup() override {
    Wire.begin();
    sw_reset();
    delay(100);

    set_pll_config(false, 1, 1);
    set_NADC_clock_div(false, 1);
    set_MADC_clock_div(false, 1);
    write_register(0x30, 0x01); // BCLK_N_DIV

    configure_clockgen("BCLK", "PLL_CLK");
    set_AOSR(128);

    set_pll_config(true, 1, 1);
    set_PLL_mult_J(32);
    set_PLL_frac_mult_D(0);
    set_NADC_clock_div(false, 8);
    set_MADC_clock_div(true, 2);

    select_processing_block("PRB_R2");
    config_adc_audio_interface("I2S", true);
    config_dout("disabled");
    config_adc_audio_interface("I2S", 32, true);
    configure_adc_digital(true, true, "disabled");

    write_register(0x34, 0x3F); // LEFT_PGA_SEL_1
    write_register(0x36, 0x3F); // LEFT_PGA_SEL_2
    write_register(0x37, 0x3F); // RIGHT_PGA_SEL_1
    write_register(0x39, 0x3F); // RIGHT_PGA_SEL_2
    write_register(0x3B, 0x98); // LEFT_APGA_CTRL
    write_register(0x3C, 0x98); // RIGHT_APGA_CTRL
    write_register(0x40, 0x00); // DITHER_CTRL
    write_register(0x42, 0x00); // ADC_FINE_VOL_CTRL
    write_register(0x3B, 0x18); // Repeated for LEFT_APGA_CTRL
    write_register(0x3C, 0x18); // Repeated for RIGHT_APGA_CTRL

    write_register(0x51, 0x50); // MICBIAS_CTRL

    set_NADC_clock_div(true, 8);
    write_register(0x56, 0x02); // I2S_TDM_CTRL
    // TDM registration should be added dynamically as needed
  }

  void config_tdm(uint8_t tdm, bool enable) {
    write_register(0x3A, tdm); // CH_OFFSET_1
    if (enable) {
      config_dout("primary");
    }
  }

 private:
  uint8_t i2c_address_;

  void sw_reset() {
    write_register(0x00, 0x01); // Reset register
  }

  void set_pll_config(bool power_up, int pll_div_P, int pll_multiplier_R) {
    uint8_t config = (power_up << 7) | ((pll_div_P & 0x07) << 4) | (pll_multiplier_R & 0x0F);
    write_register(0x05, config); // PLL_PROG_PR
  }

  void set_NADC_clock_div(bool power_up, int div) {
    uint8_t config = (power_up << 7) | (div & 0x7F);
    write_register(0x12, config); // NADC clock divisor
  }

  void set_MADC_clock_div(bool power_up, int div) {
    uint8_t config = (power_up << 7) | (div & 0x7F);
    write_register(0x13, config); // MADC clock divisor
  }

  void configure_clockgen(const char* pll_clk_src, const char* codec_clk_src) {
    uint8_t pll_source = 0b10;  // Assume PLL source BCLK
    uint8_t codec_source = 0b11; // Assume CODEC_CLK as PLL_CLK

    uint8_t config = (pll_source << 2) | (codec_source);
    write_register(0x04, config); // CLKGEN configuration
  }

  void set_AOSR(uint8_t aosr) {
    write_register(0x14, aosr);
  }

  void set_PLL_mult_J(uint8_t j) {
    write_register(0x06, j);
  }

  void set_PLL_frac_mult_D(uint16_t d) {
    write_register(0x07, (d >> 8) & 0xFF); // D_MSB
    write_register(0x08, d & 0xFF);        // D_LSB
  }

  void select_processing_block(const char* prb) {
    uint8_t prb_code = strcmp(prb, "PRB_R2") == 0 ? 0x08 : 0x00;
    write_register(0x3D, prb_code); // PRB_SELECT
  }

  void config_adc_audio_interface(const char* interface, bool DOUT_3_stating) {
    uint8_t interface_sel = strcmp(interface, "I2S") == 0 ? 0x00 : 0x00;
    uint8_t config = (interface_sel << 6) | (DOUT_3_stating);
    write_register(0x1B, config); // INTERFACE_CTRL_1
  }

  void config_dout(const char* dout_output_source) {
    uint8_t source = strcmp(dout_output_source, "primary") == 0 ? 0x01 : 0x00;
    write_register(0x35, source); // DOUT_CTRL configuration
  }

  void configure_adc_digital(bool left_chan, bool right_chan, const char* mode) {
    uint8_t config = (left_chan << 7) | (right_chan << 6);
    write_register(0x52, config); // ADC_DIGITAL configuration
  }

  void write_register(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(i2c_address_);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
  }
};

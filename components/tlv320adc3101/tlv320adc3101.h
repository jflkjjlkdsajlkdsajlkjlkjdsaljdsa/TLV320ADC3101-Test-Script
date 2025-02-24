#pragma once
#include "esphome.h"
#include <Wire.h>

class TLV320ADC3101 : public Component {
 public:
  void setup() override {
    Wire.begin();
    sw_reset();
    
    // Configuration order based on device initialization procedure
    configure_pll();
    configure_clock();
    configure_audio_interface();
    configure_filters();
    configure_digital();
    configure_analog();
  }

 private:
  void sw_reset() {
    write_register(0x00, 0x01); // Reset register
  }

  void configure_pll() {
    write_register(0x04, 0x99); // CLKGEN_MUX
    write_register(0x05, 0x11); // PLL_PROG_PR
    write_register(0x06, 0x20); // PLL_PROG_J
    write_register(0x07, 0x0D); // PLL_PROG_D_MSB
    write_register(0x08, 0x2A); // PLL_PROG_D_LSB
  }

  void configure_clock() {
    write_register(0x18, 0x81); // NADC (Enable with divider)
    write_register(0x19, 0x81); // MADC (Enable with divider)
    write_register(0x20, 0x80); // AOSR
    write_register(0x21, 0x00); // IADC
  }

  void configure_audio_interface() {
    // Interface Reg 1 and 2 set as I2S, 32-bit
    write_register(0x27, 0x00); // INTERFACE_CTRL_1 (I2S, 16-bit, standard config)
    write_register(0x29, 0x40); // INTERFACE_CTRL_2 (I2S 32-bit data length)

    // Additional configurations if needed
  }

  void configure_filters() {
    // Setting up filter coefficients for a typical setup
    // These are hypothetical; actual values should be determined based on design
    for (uint8_t i = 0; i < 50; ++i) {
      write_register(0x2A + i, (i % 2) ? 0xFF : 0x00); // Sample fill values
    }
  }

  void configure_digital() {
    write_register(0x81, 0xC0); // ADC_DIGITAL (Enable both channels, default settings)
  }

  void configure_analog() {
    write_register(0x52, 0x3F); // LEFT_PGA_SEL_1
    write_register(0x54, 0x3F); // LEFT_PGA_SEL_2
    write_register(0x55, 0x3F); // RIGHT_PGA_SEL_1
    write_register(0x57, 0x3F); // RIGHT_PGA_SEL_2
    write_register(0x59, 0x18); // LEFT_APGA_CTRL
    write_register(0x60, 0x18); // RIGHT_APGA_CTRL

    // Set MICBIAS configuration
    write_register(0x33, 0x50); // MICBIAS_CTRL
  }

  void write_register(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(0x1A); // I2C Address of TLV320ADC3101
    Wire.write(reg);              // Register Number
    Wire.write(value);            // Register Value
    Wire.endTransmission();
  }
};

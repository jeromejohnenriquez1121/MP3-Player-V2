#pragma once

#include "gpio.h"

// SPI pins
gpio_s sclk_pin;
gpio_s miso_pin;
gpio_s mosi_pin;

// GPIO pins
gpio_s xdcs_pin;
gpio_s cs_pin;
gpio_s reset_pin;
gpio_s dreq_pin;

void decoder__initialize(void);

void decoder__send_to_sci(uint8_t address, uint8_t high_byte, uint8_t low_byte);

uint16_t decoder__read_from_sci(uint8_t address);

void decoder__send_to_sdi(uint8_t byte_to_send);
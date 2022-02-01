#include "decoder.h"
#include "ssp0_mp3.h"

#include "delay.h"
#include "gpio.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum { write_opcode = 0x2, read_opcode = 0x3 } op_code_e;

typedef enum {
  mode_reg = 0x0,
  status_reg = 0x1,
  clock_freq_reg = 0x3
} sci_reg_e;

static uint32_t delay_time = 100;

static void enable_pins(void);
static void reset_decoder(void);

static void enable_xdcs(void);
static void disable_xdcs(void);
static void enable_cs(void);
static void disable_cs(void);

static bool data_is_ready(void);

/* ------------------------- Public Functions ------------------------- */

void decoder__initialize(void) {
  reset_decoder();
  enable_pins();
  ssp0_mp3__initialize(1);
}

void decoder__send_to_sci(uint8_t address, uint8_t high_byte,
                          uint8_t low_byte) {
  enable_cs();
  ssp0_mp3__send_byte(write_opcode);
  ssp0_mp3__send_byte(address);
  ssp0_mp3__send_byte(high_byte);
  ssp0_mp3__send_byte(low_byte);

  while (!data_is_ready) {
    ;
  }

  disable_cs();
}

uint16_t decoder__read_from_sci(uint8_t address) {
  const uint8_t dummy_byte = 0xFF;

  enable_cs();
  ssp0_mp3__send_byte(read_opcode);
  ssp0_mp3__send_byte(address);
  uint8_t high_byte_result = ssp0_mp3__send_byte(dummy_byte);
  uint8_t low_byte_result = ssp0_mp3__send_byte(dummy_byte);

  while (!data_is_ready) {
    ;
  }

  disable_cs();

  return (uint16_t)(high_byte_result << 8) | (low_byte_result << 0);
}

void decoder__send_to_sdi(uint8_t byte_to_send) {
  enable_xdcs();
  ssp0_mp3__send_byte(byte_to_send);

  while (!data_is_ready) {
    ;
  }

  disable_xdcs();
}

/* ------------------------- Private Functions ------------------------- */
static void enable_pins(void) {
  // Set SPI pins
  sclk_pin = gpio__construct_with_function(0, 15, GPIO__FUNCTION_2);
  miso_pin = gpio__construct_with_function(0, 17, GPIO__FUNCTION_2);
  mosi_pin = gpio__construct_with_function(0, 18, GPIO__FUNCTION_2);

  // Set GPIO pins
  xdcs_pin = gpio__construct_with_function(1, 4, GPIO__FUNCITON_0_IO_PIN);
  cs_pin = gpio__construct_with_function(1, 1, GPIO__FUNCITON_0_IO_PIN);
  reset_pin = gpio__construct_with_function(4, 28, GPIO__FUNCITON_0_IO_PIN);
  dreq_pin = gpio__construct_with_function(1, 0, GPIO__FUNCITON_0_IO_PIN);

  // Set GPIO direction
  gpio__set_as_output(xdcs_pin);
  gpio__set_as_output(cs_pin);
  gpio__set_as_output(reset_pin);
  gpio__set_as_input(dreq_pin);

  // Initialize GPIO values
  gpio__set(xdcs_pin);
  gpio__set(cs_pin);
  gpio__set(reset_pin);
}

static void reset_decoder(void) {
  gpio__reset(reset_pin);
  delay__ms(delay_time);

  gpio__set(reset_pin);
}

static void enable_xdcs(void) { gpio__reset(xdcs_pin); }

static void disable_xdcs(void) { gpio__set(xdcs_pin); }

static void enable_cs(void) { gpio__reset(cs_pin); }

static void disable_cs(void) { gpio__set(cs_pin); }

static bool data_is_ready(void) { return gpio__get(dreq_pin); }

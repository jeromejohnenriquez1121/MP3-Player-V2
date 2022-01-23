#include "decoder.h"
#include "ssp0_mp3.h"

#include "delay.h"
#include "gpio.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define DEBUG_ENABLE 1

typedef enum { write_opcode = 0x2, read_opcode = 0x3 } opcode_e;

typedef enum {
  sci_mode = 0x0,
  sci_status = 0x1,
  sci_clock_freq = 0x3,
  sci_volume = 0xB
} sci_register_address_e;

static uint32_t delay_time = 100;

static void reset_decoder(void);
static void initialize_ssp0(uint32_t max_clock_in_mhz);
static void set_up_pins(void);

static void enable_xdcs(void);
static void enable_cs(void);

static void disable_xdcs(void);
static void disable_cs(void);

static bool is_data_ready(void);

static void get_status(void);
/* -------------------- Public Functions -------------------- */
void decoder__initialize(void) {
  reset_decoder();
  set_up_pins();
  initialize_ssp0(1);

  decoder__send_to_sci(sci_mode, 0x48, 0x00);
  decoder__send_to_sci(sci_clock_freq, 0x60, 0x00);

  initialize_ssp0(12);

  get_status();
}

void decoder__send_to_sci(uint8_t address, uint8_t high_byte,
                          uint8_t low_byte) {
  enable_cs();
  ssp0_mp3__send_byte(write_opcode);
  ssp0_mp3__send_byte(address);
  ssp0_mp3__send_byte(high_byte);
  ssp0_mp3__send_byte(low_byte);

  while (!is_data_ready()) {
    ;
  }

  disable_cs();

#if DEBUG_ENABLE
  printf("Sent 0x%02x%02x to 0x%04x in VS1053.\n", high_byte, low_byte,
         address);
#endif
}

uint16_t decoder__read_from_sci(uint8_t address) {
  const uint8_t dummy_byte = 0xFF;
  uint16_t result = 0x0;

  enable_cs();
  ssp0_mp3__send_byte(read_opcode);
  ssp0_mp3__send_byte(address);
  uint8_t high_byte = ssp0_mp3__send_byte(dummy_byte);
  uint8_t low_byte = ssp0_mp3__send_byte(dummy_byte);

  while (!is_data_ready()) {
    ;
  }

  disable_cs();

  result |= (low_byte << 0) | (high_byte << 8);

  return result;
}

void decoder__send_to_sdi(uint8_t byte_to_send) {
  enable_xdcs();
  ssp0_mp3__send_byte(byte_to_send);

  while (!is_data_ready()) {
    ;
  }

  disable_xdcs();
}

/* -------------------- Private Functions -------------------- */
static void reset_decoder(void) {
  gpio__reset(reset_pin);
  delay__ms(delay_time);

  gpio__set(reset_pin);
  delay__ms(delay_time);

#if DEBUG_ENABLE
  printf("Reset VS1053.\n");
#endif
}

static void initialize_ssp0(uint32_t max_clock_in_mhz) {
  ssp0_mp3__initialize(max_clock_in_mhz);

#if DEBUG_ENABLE
  printf("Initialize ssp0 at %ld MHz.\n", max_clock_in_mhz);
#endif
}

static void set_up_pins(void) {
  // SPI pins:
  sclk_pin = gpio__construct_with_function(0, 15, GPIO__FUNCTION_2);
  miso_pin = gpio__construct_with_function(0, 17, GPIO__FUNCTION_2);
  mosi_pin = gpio__construct_with_function(0, 18, GPIO__FUNCTION_2);

  // GPIO pins:
  xdcs_pin = gpio__construct_with_function(1, 4, GPIO__FUNCITON_0_IO_PIN);
  cs_pin = gpio__construct_with_function(1, 1, GPIO__FUNCITON_0_IO_PIN);
  reset_pin = gpio__construct_with_function(4, 28, GPIO__FUNCITON_0_IO_PIN);
  dreq_pin = gpio__construct_with_function(1, 0, GPIO__FUNCITON_0_IO_PIN);

  // Set GPIO direction:
  gpio__set_as_output(xdcs_pin);
  gpio__set_as_output(cs_pin);
  gpio__set_as_output(reset_pin);
  gpio__set_as_input(dreq_pin);

  gpio__set(xdcs_pin);
  gpio__set(cs_pin);
  gpio__set(reset_pin);

#if DEBUG_ENABLE
  printf("VS1053 pins initialized.\n");
#endif
}

static void enable_xdcs(void) { gpio__reset(xdcs_pin); }

static void enable_cs(void) { gpio__reset(cs_pin); }

static void disable_xdcs(void) { gpio__set(xdcs_pin); }

static void disable_cs(void) { gpio__set(cs_pin); }

static bool is_data_ready(void) { return gpio__get(dreq_pin); }

static void get_status(void) {
  uint16_t mode_reading = decoder__read_from_sci(sci_mode);
  uint16_t status_reading = decoder__read_from_sci(sci_status);
  uint16_t clock_freq_reading = decoder__read_from_sci(sci_clock_freq);

#if DEBUG_ENABLE
  printf("Mode reading: 0x%04x.\n", mode_reading);
  printf("Status reading: 0x%04x.\n", status_reading);
  printf("Clock frequency reading: 0x%04x.\n", clock_freq_reading);
#endif
}

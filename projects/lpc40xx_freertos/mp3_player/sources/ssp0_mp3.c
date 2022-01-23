#include "ssp0_mp3.h"

#include "clock.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

#include <stdint.h>

static void turn_on_peripheral(void);
static void set_control_registers(void);
static void set_prescalar_register(uint32_t max_clock_in_mhz);

/* -------------------- Public Functions -------------------- */

void ssp0_mp3__initialize(uint32_t max_clock_as_mhz) {
  turn_on_peripheral();
  set_control_registers();
  set_prescalar_register(max_clock_as_mhz);
}

uint8_t ssp0_mp3__send_byte(uint8_t byte_to_send) {
  const uint32_t sr__is_busy = (1 << 4);

  LPC_SSP0->DR = byte_to_send & 0xFF;

  while (LPC_SSP0->SR & sr__is_busy) {
    ;
  }

  return (uint8_t)LPC_SSP0->DR & 0xFF;
}

/* -------------------- Private Functions -------------------- */
static void turn_on_peripheral(void) {
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);
}
static void set_control_registers(void) {
  const uint32_t cr0__dss_8_bit_transfer = (7 << 0);
  const uint32_t cr1__sse_ssp_enable = (1 << 1);

  LPC_SSP0->CR0 |= cr0__dss_8_bit_transfer;
  LPC_SSP0->CR1 |= cr1__sse_ssp_enable;
}
static void set_prescalar_register(uint32_t max_clock_in_mhz) {
  uint32_t peripheral_clock_in_mhz =
      clock__get_core_clock_hz() / (1000UL * 1000UL);
  uint8_t divider = 2;

  while ((max_clock_in_mhz < (peripheral_clock_in_mhz / divider)) &&
         (divider <= 254)) {
    divider += 2;
  }

  LPC_SSP0->CPSR |= divider & 0xFF;
}
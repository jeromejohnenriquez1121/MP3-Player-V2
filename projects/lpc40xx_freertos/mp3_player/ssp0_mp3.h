#pragma once

#include <stdint.h>

void ssp0_mp3__initialize(uint32_t max_clock_in_mhz);

uint8_t ssp0_mp3__send_byte(uint8_t byte_to_send);
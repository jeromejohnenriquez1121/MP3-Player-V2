#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"

#include "mp3_reader.h"
#include "mp3_player.h"

QueueHandle_t name_queue;
QueueHandle_t data_queue;

static void initialize_queues(void);

int main(void) {
  sj2_cli__init();
  initialize_queues();

  xTaskCreate(mp3_reader__reader_task, "Read mp3 file", 4096 / sizeof(void *),
              NULL, PRIORITY_LOW, NULL);
  xTaskCreate(mp3_player__player_task, "Read mp3 file", 4096 / sizeof(void *),
              NULL, PRIORITY_HIGH, NULL);

  vTaskStartScheduler();

  return 1; //Should never return
}

static void initialize_queues(void) {
  name_queue = xQueueCreate(1, sizeof(char[32]));
  data_queue = xQueueCreate(1, sizeof(char[512]));
}

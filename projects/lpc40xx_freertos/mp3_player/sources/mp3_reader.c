#include "mp3_reader.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"

QueueHandle_t name_queue;
QueueHandle_t data_queue;

void mp3_reader__reader_task(void *parameter) {
  char song_name[32];
  char song_data[512];

  while (1) {
    xQueueReceive(name_queue, &song_name[0], portMAX_DELAY);
    printf("Received %s from queue.\n", &song_name[0]);
  }
}

#include "mp3_player.h"

#include "FreeRTOS.h"
#include "queue.h"

#include <stdio.h>

QueueHandle_t data_queue;

void mp3_player__player_task(void *parameter) {
  char song_data[512];

  while (1) {
    xQueueReceive(data_queue, &song_data[0], portMAX_DELAY);
    for (int i = 0; i < sizeof(song_data); i++) {
      printf("Sent %s\n", &song_data[i]);
    }
  }
}

/*
 * main.cc
 *
 *  Created on: 21/10/2023
 *      Author: nico3
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "main_functions.h"

extern "C" void app_main(void) {
  setup();
  while (true) {
	  printf("Hello from app_main!\n");
    // loop();

    // trigger one inference every 500ms
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}


//#include <stdio.h>
//#include <stdbool.h>
//#include <unistd.h>
//
////#include "buffer.h"
//
//void app_main(void)
//{
//    while (true) {
//        printf("Hello from app_main!\n");
//        sleep(1);
//    }
//}


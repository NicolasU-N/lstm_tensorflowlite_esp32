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

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_heap_caps.h"

void tf_main(void) {
	setup();

	while (true) {
		loop();

        // Obtener la información del uso de la RAM
        size_t free_heap_size = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        size_t minimum_free_heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);

        // Imprimir la información del uso de la RAM
        printf("Free Heap Size: %d bytes\n", free_heap_size);
        printf("Minimum Free Heap Size: %d bytes\n", minimum_free_heap_size);

		// trigger one inference every 500ms
		vTaskDelay(pdMS_TO_TICKS(500));
	}

}

extern "C" void app_main() {
	xTaskCreate((TaskFunction_t) &tf_main, "tf_main", 4 * 1024, NULL, 8, NULL);
	vTaskDelete(NULL);
}


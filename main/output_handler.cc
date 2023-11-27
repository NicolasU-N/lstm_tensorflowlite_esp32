/*
 * output_handler.cc
 *
 *  Created on: 22/10/2023
 *      Author: nico3
 */

//#include "output_handler.h"
//#include "tensorflow/lite/micro/micro_log.h"
//
//void HandleOutput(const float *output_vector, int output_size) {
//	// Log the output values
//	MicroPrintf("Output vector: [");
//	for (int i = 0; i < output_size; i++) {
//		MicroPrintf("%f", static_cast<double>(output_vector[i]));
//		if (i < output_size - 1) {
//			MicroPrintf(", ");
//		}
//	}
//	MicroPrintf("]");
//}

#include <cstdio>
#include "output_handler.h"
#include "tensorflow/lite/micro/micro_log.h"

void HandleOutput(const float *output_vector, int output_size) {
    // Definimos un buffer lo suficientemente grande para guardar los resultados.
    // Asumiendo que cada número flotante se convierte a un máximo de 20 caracteres.
    char buffer[output_size * 20 + 50]; // +50 para el texto adicional y las comas.
    int position = 0;

    position += snprintf(&buffer[position], sizeof(buffer) - position, "Output vector: [");

    for (int i = 0; i < output_size; i++) {
        position += snprintf(&buffer[position], sizeof(buffer) - position, "%f", output_vector[i]);
        if (i < output_size - 1) {
            position += snprintf(&buffer[position], sizeof(buffer) - position, ", ");
        }
    }

    snprintf(&buffer[position], sizeof(buffer) - position, "]");

    MicroPrintf("%s", buffer);
}

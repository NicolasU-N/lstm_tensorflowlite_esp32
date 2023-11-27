/*
 * output_handler.h
 *
 *  Created on: 22/10/2023
 *      Author: nico3
 */

#ifndef MAIN_OUTPUT_HANDLER_H_
#define MAIN_OUTPUT_HANDLER_H_

#include "tensorflow/lite/c/common.h"

// Called by the main loop to produce some output based on the x and y values
void HandleOutput(const float *output_vector, int output_size);

#endif /* MAIN_OUTPUT_HANDLER_H_ */

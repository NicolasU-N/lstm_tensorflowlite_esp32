/*
 * main_functions.h
 *
 *  Created on: 21/10/2023
 *      Author: nico3
 */

#ifndef MAIN_MAIN_FUNCTIONS_H_
#define MAIN_MAIN_FUNCTIONS_H_

// Expose a C friendly interface for main functions.
#ifdef __cplusplus
extern "C" {
#endif

// Initializes all data needed for the example. The name is important, and needs
// to be setup() for Arduino compatibility.
void setup();

// Runs one iteration of data gathering and inference. This should be called
// repeatedly from the application code. The name needs to be loop() for Arduino
// compatibility.
void loop();

#ifdef __cplusplus
}
#endif

#endif /* MAIN_MAIN_FUNCTIONS_H_ */

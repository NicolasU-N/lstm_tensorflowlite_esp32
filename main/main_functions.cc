/*
 * main_functions.cc
 *
 *  Created on: 21/10/2023
 *      Author: nico3
 */

/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ==============================================================================*/

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_log.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "main_functions.h"
#include "model_data.h"

#include "constants.h"
#include "output_handler.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_log.h>

// Globals, used for compatibility with Arduino-style sketches.
namespace {
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;
//int inference_count = 0;

// In order to use optimized tensorflow lite kernels, a signed int8_t quantized
// model is preferred over the legacy unsigned model format. This means that
// throughout this project, input images must be converted from unisgned to
// signed format. The easiest and quickest way to convert from unsigned to
// signed 8-bit integers is to subtract 128 from the unsigned value to get a
// signed value.

#ifdef CONFIG_IDF_TARGET_ESP32S3
constexpr int scratchBufSize = 39 * 1024;
#else
constexpr int scratchBufSize = 0;
#endif
// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 80 * 1024 + scratchBufSize; //Estaba en 81
static uint8_t *tensor_arena; //[kTensorArenaSize]; // Maybe we should move this to external
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
	// Map the model into a usable data structure. This doesn't involve any
	// copying or parsing, it's a very lightweight operation.
	model = tflite::GetModel(g_model);
	if (model->version() != TFLITE_SCHEMA_VERSION) {
		MicroPrintf(
				"Model provided is schema version %d not equal to supported "
						"version %d.", model->version(), TFLITE_SCHEMA_VERSION);
		return;
	}

	if (tensor_arena == NULL) {
		tensor_arena = (uint8_t*) heap_caps_malloc(kTensorArenaSize,
		MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
	}
	if (tensor_arena == NULL) {
		printf("Couldn't allocate memory of %d bytes\n", kTensorArenaSize);
		return;
	}

	// Pull in only the operation implementations we need.
	static tflite::MicroMutableOpResolver<16> resolver;
	if (resolver.AddFill() != kTfLiteOk) {
		return;
	}
	if (resolver.AddPack() != kTfLiteOk) {
		return;
	}
	if (resolver.AddShape() != kTfLiteOk) {
		return;
	}
	if (resolver.AddQuantize() != kTfLiteOk) {
		return;
	}
	if (resolver.AddMul() != kTfLiteOk) {
		return;
	}
	if (resolver.AddTanh() != kTfLiteOk) {
		return;
	}
	if (resolver.AddLogistic() != kTfLiteOk) {
		return;
	}
	if (resolver.AddSplit() != kTfLiteOk) {
		return;
	}
	if (resolver.AddGather() != kTfLiteOk) {
		return;
	}
	if (resolver.AddAdd() != kTfLiteOk) {
		return;
	}
	if (resolver.AddLess() != kTfLiteOk) {
		return;
	}
	if (resolver.AddStridedSlice() != kTfLiteOk) {
		return;
	}
	if (resolver.AddWhile() != kTfLiteOk) {
		return;
	}
	if (resolver.AddReshape() != kTfLiteOk) {
		return;
	}
	if (resolver.AddFullyConnected() != kTfLiteOk) {
		return;
	}
	if (resolver.AddUnidirectionalSequenceLSTM() != kTfLiteOk) {
		return;
	}

	// Build an interpreter to run the model with.
	static tflite::MicroInterpreter static_interpreter(model, resolver,
			tensor_arena, kTensorArenaSize);
	interpreter = &static_interpreter;

	// Allocate memory from the tensor_arena for the model's tensors.
	TfLiteStatus allocate_status = interpreter->AllocateTensors();
	if (allocate_status != kTfLiteOk) {
		MicroPrintf("AllocateTensors() failed");
		return;
	}

	// Obtain pointers to the model's input and output tensors.
	input = interpreter->input(0);
	output = interpreter->output(0);
//
//	// Obtener la forma del tensor de entrada
//	int input_tensor_dims = input->dims->size;
//	std::vector<int> input_shape(input_tensor_dims);
//
//	for (int i = 0; i < input_tensor_dims; i++) {
//		input_shape[i] = input->dims->data[i];
//	}
//
//	// Imprimir la forma
//	MicroPrintf("Forma del tensor de entrada: [");
//	for (int i = 0; i < input_shape.size(); i++) {
//		MicroPrintf(" %d", input_shape[i]);
//		if (i < input_shape.size() - 1) {
//			MicroPrintf(", ");
//		}
//	}
}

long long total_time = 0;
long long start_time = 0;
extern long long softmax_total_time;
extern long long dc_total_time;
extern long long conv_total_time;
extern long long fc_total_time;
extern long long pooling_total_time;
extern long long add_total_time;
extern long long mul_total_time;

// The name of this function is important for Arduino compatibility.
void loop() {
	// Define tu entrada como un vector de números de punto flotante
	float input_vector[168] = { 7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 45.0, 24.0, 22.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 21.0, 63.0, 22.0, 17.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			25.0, 23.0, 8.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 74.0,
			13.0, 22.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 92.0, 53.0,
			80.0, 103.0, 55.0, 55.0, 56.0, 81.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 8.0, 74.0, 13.0, 7.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 44.0, 30.0, 12.0, 16.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 31.0, 52.0, 32.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 9.0, 39.0, 12.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0 };

	//	// Cuantizar la entrada
//	int8_t input_quantized[168]; // Tamaño esperado de la entrada
//	for (int i = 0; i < 168; i++) {
//		input_quantized[i] = (int8_t)(
//				(input_vector[i] - input->params.zero_point)
//						/ input->params.scale);
//	}

//	for (int i = 0; i < 18 * 9; i++) {
//		input->data.f[i] = input_vector[i];
//	}
//	for (int i = 0; i < 168; i++) {
//		input->data.f[i] = input_vector[i];
//	}

//Copia los datos del arreglo "test2" al tensor de entrada
	memcpy(input->data.f, input_vector, 168 * sizeof(float));

	long long start_time = esp_timer_get_time();

// Run inference, and report any error
	TfLiteStatus invoke_status = interpreter->Invoke();
	if (invoke_status != kTfLiteOk) {
		MicroPrintf("Invoke failed on x");
		return;
	}

	long long total_time = (esp_timer_get_time() - start_time);
	printf("Total time = %lld\n", total_time / 1000);
//printf("Softmax time = %lld\n", softmax_total_time / 1000);
//	printf("FC time = %lld\n", fc_total_time / 1000);
//	printf("DC time = %lld\n", dc_total_time / 1000);
//	printf("conv time = %lld\n", conv_total_time / 1000);
//	printf("Pooling time = %lld\n", pooling_total_time / 1000);
//	printf("add time = %lld\n", add_total_time / 1000);
//	printf("mul time = %lld\n", mul_total_time / 1000);

	/* Reset times */
	total_time = 0;
//softmax_total_time = 0;
	dc_total_time = 0;
	conv_total_time = 0;
	fc_total_time = 0;
	pooling_total_time = 0;
	add_total_time = 0;
	mul_total_time = 0;

// Obtener el resultado del modelo
	float output_quantized[24];
	for (int i = 0; i < 24; i++) {
		output_quantized[i] = output->data.int8[i];
	}

// Output the results. A custom HandleOutput function can be implemented
// for each supported hardware target.
	HandleOutput(output_quantized, 24); //TODO Uncomment

//	int8_t y_quantized = output->data.f[0];
// Dequantize the output from integer to floating-point
//	float y = (y_quantized - output->params.zero_point) * output->params.scale;

// Output the results. A custom HandleOutput function can be implemented
// for each supported hardware target.
//	MicroPrintf("y_value: %f", static_cast<double>(y));
}


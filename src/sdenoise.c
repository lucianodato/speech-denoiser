/*
speech-denoise -- Speech noise reduction LV2

Copyright 2017 Luciano Dato <lucianodato@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/
*/

/**
* \file sdenoise.c
* \author Luciano Dato
* \brief The main file for host interaction
*/

#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

#include <rnnoise.h>
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define SDENOISE_URI "https://github.com/lucianodato/speech-denoiser"

#define FRAME_SIZE 480 //Frame default size (For 48 kHz sampling rate)

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

///---------------------------------------------------------------------

/**
* Enumeration of LV2 ports.
*/
typedef enum
{
	NREPEL_ENABLE = 0,
	NREPEL_LATENCY = 1,
	NREPEL_INPUT = 2,
	NREPEL_OUTPUT = 3,
} PortIndex;

/**
* Struct for speech-denoiser instance, the host is going to use.
*/
typedef struct
{
	const float* input; //input of samples from host (changing size)
	float* output; //output of samples to host (changing size)
	float samp_rate; //Sample rate received from the host

	//Parameters for the algorithm (user input)
	float* enable; //For soft bypass (click free bypass)
	float* report_latency; //Latency necessary

	//Parameters values for RNNoise libray
	int frame_size; //RNNOISE frame input size
	DenoiseState *st;//RNNoise instance

	//Algorithm exta variables
	float tau; //time constant for soft bypass
	float wet_dry_target; //softbypass target for softbypass
	float wet_dry; //softbypass coeff

	//Buffers for processing and outputting
	int input_latency;
	float* in_fifo; //internal input buffer
	float* out_fifo; //internal output buffer
	float* rnnoise_input_frame;
	float* rnnoise_output_frame;
	int read_ptr; //buffers read pointer

} SDenoise;

/**
* Instantiates the plugin.
*/
static LV2_Handle
instantiate(const LV2_Descriptor* descriptor, double rate, const char* bundle_path,
						const LV2_Feature* const* features)
{
	//Actual struct declaration
	SDenoise* self = (SDenoise*)calloc(1,sizeof(SDenoise));

	//Sampling related
	self->samp_rate = (float)rate;

	//RNNoise related
	self->frame_size = FRAME_SIZE;
	self->st = rnnoise_create();

	//processing buffers
	self->in_fifo = (float*)calloc(self->frame_size, sizeof(float));
	self->out_fifo = (float*)calloc(self->frame_size, sizeof(float));
	self->rnnoise_input_frame = (float*)calloc(self->frame_size, sizeof(float));
	self->rnnoise_output_frame = (float*)calloc(self->frame_size, sizeof(float));
	self->input_latency = self->frame_size;
	self->read_ptr = 0; //the initial position because we are that many samples ahead

	//soft bypass
	self->tau = (1.f - expf(-2.f * M_PI * 25.f * 64.f  / self->samp_rate));
	self->wet_dry = 0.f;

	return (LV2_Handle)self;
}

/**
* Used by the host to connect the ports of this plugin.
*/
static void
connect_port(LV2_Handle instance, uint32_t port, void* data)
{
	SDenoise* self = (SDenoise*)instance;

	switch ((PortIndex)port)
	{
		case NREPEL_ENABLE:
		self->enable = (float*)data;
		break;
		case NREPEL_LATENCY:
		self->report_latency = (float*)data;
		break;
		case NREPEL_INPUT:
		self->input = (const float*)data;
		break;
		case NREPEL_OUTPUT:
		self->output = (float*)data;
		break;
	}
}

/**
* Main process function of the plugin.
*/
static void
run(LV2_Handle instance, uint32_t n_samples)
{
	SDenoise* self = (SDenoise*)instance;

	//handy variables
	int k;
	unsigned int pos;

	//Inform latency at run call
	*(self->report_latency) = (float) self->input_latency;

	//Softbypass targets in case of disabled or enabled
	if(*(self->enable) == 0.f)
	{ //if disabled
		self->wet_dry_target = 0.f;
	}
	else
	{ //if enabled
		self->wet_dry_target = 1.f;
	}
	//Interpolate parameters over time softly to bypass without clicks or pops
	self->wet_dry += self->tau * (self->wet_dry_target - self->wet_dry) + FLT_MIN;

	//main loop for processing
	for (pos = 0; pos < n_samples; pos++)
	{
		//Copy samples in the input buffer
		self->in_fifo[self->read_ptr] = self->input[pos];
		//Output samples in the output buffer
		self->output[pos] = self->out_fifo[self->read_ptr];
		//Now move the read pointer
		self->read_ptr++;

		//Once the buffer is full we can do stuff
		if (self->read_ptr >= self->frame_size)
		{
			//Reset the ptr position
			self->read_ptr = 0;

			//Copy samples to RNNoise input array and scale to short limits
			for (k = 0; k < self->frame_size; k++)
			{
				self->rnnoise_input_frame[k] = self->in_fifo[k];
			}

			//------------PROCESSING-------------

			//Only call rrnoise if enabled
			if (*(self->enable) != 0.f)
			{
				//Scaling up to short values
				for (k = 0; k < self->frame_size; k++)
				{
					self->rnnoise_input_frame[k] *= SHRT_MAX;
				}

				//Process input_frame
				rnnoise_process_frame(self->st, self->rnnoise_output_frame, self->rnnoise_input_frame);

				//Scaling down to float values
				for (k = 0; k < self->frame_size; k++)
				{
					self->rnnoise_input_frame[k] = self->rnnoise_output_frame[k] / SHRT_MAX;
				}
			}

			//-----------------------------------

			//Output processed samples from RNNoise to output fifo considering soft bypass
			for (k = 0; k < self->frame_size; k++)
			{
				self->out_fifo[k] = (1.f - self->wet_dry) * self->in_fifo[k] + self->wet_dry * self->rnnoise_input_frame[k];
			}

			//-------------------------------
		}//if
	}//main loop
}


/**
* Cleanup and freeing memory.
*/
static void
cleanup(LV2_Handle instance)
{
	free(instance);
}

/**
* extension for additional interfaces.
*/
static const void*
extension_data(const char* uri)
{
	return NULL;
}

/**
* Descriptor for linking methods.
*/
static const
LV2_Descriptor descriptor =
{
	SDENOISE_URI,
	instantiate,
	connect_port,
	NULL,
	run,
	NULL,
	cleanup,
	extension_data
};

/**
* Symbol export using the descriptor above.
*/
LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index)
	{
		case 0:
		return &descriptor;
		default:
		return NULL;
	}
}

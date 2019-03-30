#include "MK64F12.h"

void convolve(uint16_t *input, float *output, int length, float *h, int h_length)
{
	for (int i = 0; i <= length - h_length; i++)
	{
		output[i] = 0.0;
		for (int j = 0; j < h_length; j++)
		{
			output[i] += input[i + j] * h[h_length - j - 1];
		}
	}
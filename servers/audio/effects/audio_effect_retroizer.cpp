/*************************************************************************/
/*  audio_effect_retroizer.cpp                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "audio_effect_retroizer.h"
#include "servers/audio_server.h"

static void smbFft(float *fftBuffer, long fftFrameSize, long sign)
/*
	FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
	Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
	time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
	and returns the cosine and sine parts in an interleaved manner, ie.
	fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
	must be a power of 2. It expects a complex input signal (see footnote 2),
	ie. when working with 'common' audio signals our input signal has to be
	passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
	of the frequencies of interest is in fftBuffer[0...fftFrameSize].
*/
{
	float wr, wi, arg, *p1, *p2, temp;
	float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
	long i, bitm, j, le, le2, k;

	for (i = 2; i < 2 * fftFrameSize - 2; i += 2) {
		for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
			if (i & bitm) j++;
			j <<= 1;
		}
		if (i < j) {
			p1 = fftBuffer + i;
			p2 = fftBuffer + j;
			temp = *p1;
			*(p1++) = *p2;
			*(p2++) = temp;
			temp = *p1;
			*p1 = *p2;
			*p2 = temp;
		}
	}
	for (k = 0, le = 2; k < (long)(log((double)fftFrameSize) / log(2.) + .5); k++) {
		le <<= 1;
		le2 = le >> 1;
		ur = 1.0;
		ui = 0.0;
		arg = Math_PI / (le2 >> 1);
		wr = cos(arg);
		wi = sign * sin(arg);
		for (j = 0; j < le2; j += 2) {
			p1r = fftBuffer + j;
			p1i = p1r + 1;
			p2r = p1r + le2;
			p2i = p2r + 1;
			for (i = j; i < 2 * fftFrameSize; i += le) {
				tr = *p2r * ur - *p2i * ui;
				ti = *p2r * ui + *p2i * ur;
				*p2r = *p1r - tr;
				*p2i = *p1i - ti;
				*p1r += tr;
				*p1i += ti;
				p1r += le;
				p1i += le;
				p2r += le;
				p2i += le;
			}
			tr = ur * wr - ui * wi;
			ui = ur * wi + ui * wr;
			ur = tr;
		}
	}
}
void AudioEffectRetroizerInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {

	while (p_frame_count) {
		int to_process = MIN(p_frame_count, fft_size - process_pos);

		{
			const AudioFrame *out_read_ptr = output.ptr() + process_pos;
			for (int i = 0; i < to_process; i++) {
				*p_dst_frames++ = *out_read_ptr++;
			}

			float *in_write_ptr = fft_input.ptrw() + process_pos * 2;
			for (int i = to_process; i > 0; i--) {
				float window = -0.5 * Math::cos(2.0 * Math_PI * (double)(process_pos + i) / (double)fft_size) + 0.5;
				*in_write_ptr++ = window * (p_src_frames->l + p_src_frames->r) * 0.5f;
				*in_write_ptr++ = 0.0f;
				++p_src_frames;
			}
			process_pos += to_process;
		}

		p_frame_count -= to_process;

		if (process_pos == fft_size) {
			const float MIN_ENERGY = 0.5f;

			smbFft(fft_input.ptrw(), fft_size, -1);

			int low_bin = -1;
			int mid_bin = -1;
			int high_bin = -1;
			float e_low = MIN_ENERGY;
			float e_mid = MIN_ENERGY;
			float e_high = MIN_ENERGY;
			{
				int min_bin = min * fft_size;
				int mid_start_bin = mid_cut * fft_size;
				int high_start_bin = high_cut * fft_size;
				int max_bin = max * fft_size;
				const float *in_read_ptr = fft_input.ptr();
				{
					// Low
					for (int j = min_bin; j < mid_start_bin; j++) {
						float ce = *in_read_ptr++;
						float se = *in_read_ptr++;
						float e = sqrtf(ce * ce + se * se);
						if (e > e_low) {
							e_low = e;
							low_bin = j;
						}
					}
				}
				{
					// Mid
					for (int j = mid_start_bin; j < high_start_bin; j++) {
						float ce = *in_read_ptr++;
						float se = *in_read_ptr++;
						float e = sqrtf(ce * ce + se * se);
						if (e > e_mid) {
							e_mid = e;
							mid_bin = j;
						}
					}
				}
				{
					// High
					for (int j = high_start_bin; j < max_bin; j++) {
						float ce = *in_read_ptr++;
						float se = *in_read_ptr++;
						float e = sqrtf(ce * ce + se * se);
						if (e > e_high) {
							e_high = e;
							high_bin = j;
						}
					}
				}
			}

			e_low = 1.0f;
			e_mid = 1.0f;
			e_high = 1.0f;
			if (high_bin >= 0) {
				e_low *= 0.5f;
				e_mid *= 0.5f;
			}
			if (mid_bin >= 0) {
				e_low *= 0.5f;
				e_high *= 0.5f;
			}
			if (low_bin >= 0) {
				e_mid *= 0.5f;
				e_high *= 0.5f;
			}

			AudioFrame *__restrict out_write_ptr = output.ptrw();
			float f_low = low_bin + 0.5f;
			float f_mid = mid_bin + 0.5f;
			float f_high = high_bin + 0.5f;
			float inv_fft_size = 1.0f / fft_size;
			for (int i = 0; i < fft_size; i++) {
				float t = i * inv_fft_size;
				float out = 0.0f;
				// Low => Triangle
				if (f_low > 0.0f) {
					out += e_low * 2.0f * fabsf(2.0f * (f_low * t - floorf(f_low * t + 0.5f))) - 1.0f;
				}
				// Mid => Sine
				if (f_mid > 0.0f) {
					out += e_mid * sin(t * 2.0f * Math_PI * f_mid);
				}
				// High => Square
				if (f_high > 0.0f) {
					out += e_high * 2.0f * (2.0f * floorf(f_high * t) - floorf(2.0f * f_high * t)) + 1.0f;
				}

				out_write_ptr->l = out_write_ptr->r = out;
				++out_write_ptr;
			}

			memcpy(new_prev_output.ptrw(), output.ptr(), sizeof(AudioFrame) * fft_size);

			{
				const AudioFrame *__restrict prev_out_read_ptr = prev_output.ptr() + fft_size;
				out_write_ptr = output.ptrw();
				for (int i = 0; i < fft_size / 2; i++) {
					float a = (i * 2) * inv_fft_size;
					*out_write_ptr *= (1.0f - a) * (*(--prev_out_read_ptr)) * a;
					out_write_ptr++;
				}
			}

			memcpy(prev_output.ptrw(), new_prev_output.ptr(), sizeof(AudioFrame) * fft_size);

			process_pos = 0;
		}
	}
}

Ref<AudioEffectInstance> AudioEffectRetroizer::instance() {

	Ref<AudioEffectRetroizerInstance> ins;
	ins.instance();
	ins->base = Ref<AudioEffectRetroizer>(this);
	ins->min = min;
	ins->mid_cut = mid_cut;
	ins->high_cut = high_cut;
	ins->max = max;
	ins->fft_size = 512;
	ins->output.resize(ins->fft_size);
	memset(ins->output.ptrw(), 0, sizeof(AudioFrame) * ins->fft_size);
	ins->prev_output.resize(ins->fft_size);
	memset(ins->prev_output.ptrw(), 0, sizeof(AudioFrame) * ins->fft_size);
	ins->new_prev_output.resize(ins->fft_size * 2);
	memset(ins->new_prev_output.ptrw(), 0, sizeof(AudioFrame) * ins->fft_size);
	ins->fft_input.resize(ins->fft_size * 2);
	ins->process_pos = 0;
	return ins;
}

AudioEffectRetroizer::AudioEffectRetroizer() :
		min(0.0f),
		mid_cut(0.1f),
		high_cut(0.4f),
		max(0.7f) {}

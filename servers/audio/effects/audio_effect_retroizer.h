/*************************************************************************/
/*  audio_effect_retroizer.h                                             */
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

#ifndef AUDIO_EFFECT_RETROIZER_H
#define AUDIO_EFFECT_RETROIZER_H

#include "servers/audio/audio_effect.h"

class AudioEffectRetroizer;

class AudioEffectRetroizerInstance : public AudioEffectInstance {
	GDCLASS(AudioEffectRetroizerInstance, AudioEffectInstance);

	friend class AudioEffectRetroizer;
	Ref<AudioEffectRetroizer> base;

	float min;
	float mid_cut;
	float high_cut;
	float max;

	int fft_size;
	Vector<AudioFrame> output;
	Vector<AudioFrame> prev_output;
	Vector<AudioFrame> new_prev_output;
	Vector<float> fft_input;
	int process_pos;

public:
	virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count);
};

class AudioEffectRetroizer : public AudioEffect {
	GDCLASS(AudioEffectRetroizer, AudioEffect);

	friend class AudioEffectRetroizerInstance;

	float min;
	float mid_cut;
	float high_cut;
	float max;

public:
	Ref<AudioEffectInstance> instance();

	AudioEffectRetroizer();
};

#endif // AUDIO_EFFECT_RETROIZER_H

/*! \file emscripten.h
 *  \brief Headers of main file of SuperDerpy engine.
 *
 *   Contains basic functions shared by all views.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBSUPERDERPY_EMSCRIPTEN_H
#define LIBSUPERDERPY_EMSCRIPTEN_H

#include <allegro5/allegro_audio.h>

typedef struct {
	ALLEGRO_SAMPLE* sample;
	ALLEGRO_SAMPLE_INSTANCE* instance;
} EMSCRIPTEN_AUDIO_STREAM;

ALLEGRO_AUDIO_STREAM* emscripten_load_audio_stream(const char* filename, size_t buffer_count, unsigned int samples);
ALLEGRO_AUDIO_STREAM* emscripten_load_audio_stream_f(ALLEGRO_FILE* file, const char* ident, size_t buffer_count, unsigned int samples);
bool emscripten_set_audio_stream_gain(ALLEGRO_AUDIO_STREAM* stream, float val);
bool emscripten_set_audio_stream_playing(ALLEGRO_AUDIO_STREAM* stream, bool val);
bool emscripten_set_audio_stream_playmode(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_PLAYMODE mode);
bool emscripten_get_audio_stream_playing(ALLEGRO_AUDIO_STREAM* stream);
bool emscripten_get_audio_stream_attached(ALLEGRO_AUDIO_STREAM* stream);
ALLEGRO_PLAYMODE emscripten_get_audio_stream_playmode(ALLEGRO_AUDIO_STREAM* stream);
bool emscripten_rewind_audio_stream(ALLEGRO_AUDIO_STREAM* stream);
bool emscripten_attach_audio_stream_to_mixer(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_MIXER* mixer);
bool emscripten_attach_audio_stream_to_voice(ALLEGRO_AUDIO_STREAM* stream, ALLEGRO_VOICE* voice);
bool emscripten_detach_audio_stream(ALLEGRO_AUDIO_STREAM* stream);
unsigned int emscripten_get_audio_stream_frequency(ALLEGRO_AUDIO_STREAM* stream);
ALLEGRO_CHANNEL_CONF emscripten_get_audio_stream_channels(ALLEGRO_AUDIO_STREAM* stream);
ALLEGRO_AUDIO_DEPTH emscripten_get_audio_stream_depth(ALLEGRO_AUDIO_STREAM* stream);
unsigned int emscripten_get_audio_stream_length(ALLEGRO_AUDIO_STREAM* stream);
float emscripten_get_audio_stream_speed(ALLEGRO_AUDIO_STREAM* stream);
float emscripten_get_audio_stream_gain(ALLEGRO_AUDIO_STREAM* stream);
float emscripten_get_audio_stream_pan(ALLEGRO_AUDIO_STREAM* stream);
bool emscripten_set_audio_stream_speed(ALLEGRO_AUDIO_STREAM* stream, float val);
bool emscripten_set_audio_stream_pan(ALLEGRO_AUDIO_STREAM* stream, float val);
double emscripten_get_audio_stream_length_sec(ALLEGRO_AUDIO_STREAM* stream);
bool emscripten_seek_audio_stream_secs(ALLEGRO_AUDIO_STREAM* stream, double val);
double emscripten_get_audio_stream_position_secs(ALLEGRO_AUDIO_STREAM* stream);
void emscripten_destroy_audio_stream(ALLEGRO_AUDIO_STREAM* stream);
#ifdef ALLEGRO_UNSTABLE
bool emscripten_set_audio_stream_channel_matrix(ALLEGRO_AUDIO_STREAM* stream, const float* val);
#endif

uint64_t emscripten_get_audio_stream_played_samples(ALLEGRO_AUDIO_STREAM* stream) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));
void* emscripten_get_audio_stream_fragment(ALLEGRO_AUDIO_STREAM* stream) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));
bool emscripten_set_audio_stream_fragment(ALLEGRO_AUDIO_STREAM* stream, void* val) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));
bool emscripten_set_audio_stream_loop_secs(ALLEGRO_AUDIO_STREAM* stream, double start, double end) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));
unsigned int emscripten_get_audio_stream_fragments(ALLEGRO_AUDIO_STREAM* stream) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));
unsigned int emscripten_get_available_audio_stream_fragments(ALLEGRO_AUDIO_STREAM* stream) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));
ALLEGRO_EVENT_SOURCE* emscripten_get_audio_stream_event_source(ALLEGRO_AUDIO_STREAM* stream) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));
void emscripten_drain_audio_stream(ALLEGRO_AUDIO_STREAM* stream) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));
ALLEGRO_AUDIO_STREAM* emscripten_create_audio_stream(size_t fragment_count, unsigned int frag_samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf) __attribute__((unavailable("won't work in Emscripten until proper audio stream support is fixed!")));

#define al_create_audio_stream emscripten_create_audio_stream
#define al_load_audio_stream emscripten_load_audio_stream
#define al_load_audio_stream_f emscripten_load_audio_stream_f
#define al_destroy_audio_stream emscripten_destroy_audio_stream
#define al_rewind_audio_stream emscripten_rewind_audio_stream
#define al_drain_audio_stream emscripten_drain_audio_stream
#define al_attach_audio_stream_to_mixer emscripten_attach_audio_stream_to_mixer
#define al_attach_audio_stream_to_voice emscripten_attach_audio_stream_to_voice
#define al_detach_audio_stream emscripten_detach_audio_stream
#define al_seek_audio_stream_secs emscripten_seek_audio_stream_secs
#define al_get_audio_stream_playing emscripten_get_audio_stream_playing
#define al_get_audio_stream_playmode emscripten_get_audio_stream_playmode
#define al_get_audio_stream_attached emscripten_get_audio_stream_attached
#define al_get_audio_stream_frequency emscripten_get_audio_stream_frequency
#define al_get_audio_stream_channels emscripten_get_audio_stream_channels
#define al_get_audio_stream_depth emscripten_get_audio_stream_depth
#define al_get_audio_stream_length emscripten_get_audio_stream_length
#define al_get_audio_stream_speed emscripten_get_audio_stream_speed
#define al_get_audio_stream_gain emscripten_get_audio_stream_gain
#define al_get_audio_stream_pan emscripten_get_audio_stream_pan
#define al_get_audio_stream_length_sec emscripten_get_audio_stream_length_sec
#define al_get_audio_stream_position_secs emscripten_get_audio_stream_position_secs
#define al_get_audio_stream_played_samples emscripten_get_audio_stream_played_samples
#define al_get_audio_stream_fragment emscripten_get_audio_stream_fragment
#define al_get_audio_stream_fragments emscripten_get_audio_stream_fragments
#define al_get_available_audio_stream_fragments emscripten_get_available_audio_stream_fragmenst
#define al_get_audio_stream_event_source emscripten_get_audio_stream_event_source
#define al_set_audio_stream_fragment emscripten_set_audio_stream_fragment
#define al_set_audio_stream_loop_secs emscripten_set_audio_stream_loop_secs
#define al_set_audio_stream_gain emscripten_set_audio_stream_gain
#define al_set_audio_stream_playing emscripten_set_audio_stream_playing
#define al_set_audio_stream_playmode emscripten_set_audio_stream_playmode
#define al_set_audio_stream_speed emscripten_set_audio_stream_speed
#define al_set_audio_stream_pan emscripten_set_audio_stream_pan
#ifdef ALLEGRO_UNSTABLE
#define al_set_audio_stream_channel_matrix emscripten_set_audio_stream_channel_matrix
#endif

#endif

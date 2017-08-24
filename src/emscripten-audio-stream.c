/*! \file emscripten.c
 *  \brief Emscripten stub implementations.
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

#include "internal.h"
#include "emscripten.h"

#undef al_create_audio_stream
#undef al_load_audio_stream
#undef al_load_audio_stream_f
#undef al_destroy_audio_stream
#undef al_rewind_audio_stream
#undef al_drain_audio_stream
#undef al_attach_audio_stream_to_mixer
#undef al_attach_audio_stream_to_voice
#undef al_detach_audio_stream
#undef al_seek_audio_stream_secs
#undef al_get_audio_stream_frequency
#undef al_get_audio_stream_channels
#undef al_get_audio_stream_depth
#undef al_get_audio_stream_length
#undef al_get_audio_stream_speed
#undef al_get_audio_stream_gain
#undef al_get_audio_stream_pan
#undef al_get_audio_stream_playing
#undef al_get_audio_stream_playmode
#undef al_get_audio_stream_attached
#undef al_get_audio_stream_length_sec
#undef al_get_audio_stream_position_secs
#undef al_get_audio_stream_played_samples
#undef al_get_audio_stream_fragment
#undef al_get_audio_stream_fragments
#undef al_get_available_audio_stream_fragments
#undef al_get_audio_stream_event_source
#undef al_set_audio_stream_fragment
#undef al_set_audio_stream_loop_secs
#undef al_set_audio_stream_gain
#undef al_set_audio_stream_playing
#undef al_set_audio_stream_playmode
#undef al_set_audio_stream_speed
#undef al_set_audio_stream_pan
#ifdef ALLEGRO_UNSTABLE
#undef al_set_audio_stream_channel_matrix
#endif

SYMBOL_EXPORT ALLEGRO_AUDIO_STREAM *emscripten_load_audio_stream(const char* filename, size_t buffer_count, unsigned int samples) {
	EMSCRIPTEN_AUDIO_STREAM *stream = calloc(1, sizeof(EMSCRIPTEN_AUDIO_STREAM));
	stream->sample = al_load_sample(filename);
	stream->instance = al_create_sample_instance(stream->sample);
	al_set_sample_instance_playing(stream->instance, true);
	return (ALLEGRO_AUDIO_STREAM*)stream;
}

SYMBOL_EXPORT ALLEGRO_AUDIO_STREAM *emscripten_load_audio_stream_f(ALLEGRO_FILE *file, const char *ident, size_t buffer_count, unsigned int samples) {
	EMSCRIPTEN_AUDIO_STREAM *stream = calloc(1, sizeof(EMSCRIPTEN_AUDIO_STREAM));
	stream->sample = al_load_sample_f(file, ident);
	stream->instance = al_create_sample_instance(stream->sample);
	al_set_sample_instance_playing(stream->instance, true);
	return (ALLEGRO_AUDIO_STREAM*)stream;
}

SYMBOL_EXPORT bool emscripten_set_audio_stream_gain(ALLEGRO_AUDIO_STREAM *stream, float val) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_set_sample_instance_gain(s->instance, val);
}

SYMBOL_EXPORT bool emscripten_set_audio_stream_playing(ALLEGRO_AUDIO_STREAM *stream, bool val) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_set_sample_instance_playing(s->instance, val);
}

SYMBOL_EXPORT bool emscripten_set_audio_stream_playmode(ALLEGRO_AUDIO_STREAM *stream, ALLEGRO_PLAYMODE mode) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_set_sample_instance_playmode(s->instance, mode);
}

SYMBOL_EXPORT ALLEGRO_PLAYMODE emscripten_get_audio_stream_playmode(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_playmode(s->instance);
}


SYMBOL_EXPORT bool emscripten_get_audio_stream_playing(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_playing(s->instance);
}

SYMBOL_EXPORT bool emscripten_rewind_audio_stream(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_set_sample_instance_position(s->instance, 0);
}

SYMBOL_EXPORT bool emscripten_attach_audio_stream_to_mixer(ALLEGRO_AUDIO_STREAM *stream, ALLEGRO_MIXER *mixer) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_attach_sample_instance_to_mixer(s->instance, mixer);
}

SYMBOL_EXPORT bool emscripten_attach_audio_stream_to_voice(ALLEGRO_AUDIO_STREAM *stream, ALLEGRO_VOICE *voice) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_attach_sample_instance_to_voice(s->instance, voice);
}

SYMBOL_EXPORT bool emscripten_detach_audio_stream(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_detach_sample_instance(s->instance);
}

SYMBOL_EXPORT ALLEGRO_EVENT_SOURCE *emscripten_get_audio_stream_event_source(ALLEGRO_AUDIO_STREAM *stream) {
	return NULL;
}

SYMBOL_EXPORT void emscripten_drain_audio_stream(ALLEGRO_AUDIO_STREAM *stream) {}

SYMBOL_EXPORT unsigned int emscripten_get_audio_stream_frequency(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_frequency(s->instance);
}

SYMBOL_EXPORT ALLEGRO_CHANNEL_CONF emscripten_get_audio_stream_channels(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_channels(s->instance);
}

SYMBOL_EXPORT ALLEGRO_AUDIO_DEPTH emscripten_get_audio_stream_depth(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_depth(s->instance);
}

SYMBOL_EXPORT unsigned int emscripten_get_audio_stream_length(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_length(s->instance);
}

SYMBOL_EXPORT float emscripten_get_audio_stream_speed(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_speed(s->instance);
}

SYMBOL_EXPORT float emscripten_get_audio_stream_gain(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_gain(s->instance);
}

SYMBOL_EXPORT float emscripten_get_audio_stream_pan(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_pan(s->instance);
}

SYMBOL_EXPORT bool emscripten_get_audio_stream_attached(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_attached(s->instance);
}

SYMBOL_EXPORT bool emscripten_set_audio_stream_speed(ALLEGRO_AUDIO_STREAM *stream, float val) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_set_sample_instance_speed(s->instance, val);
}

SYMBOL_EXPORT bool emscripten_set_audio_stream_pan(ALLEGRO_AUDIO_STREAM *stream, float val) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_set_sample_instance_pan(s->instance, val);
}

#ifdef ALLEGRO_UNSTABLE
SYMBOL_EXPORT bool emscripten_set_audio_stream_channel_matrix(ALLEGRO_AUDIO_STREAM *stream, const float *val) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_set_sample_instance_channel_matrix(s->instance, val);
}
#endif

SYMBOL_EXPORT uint64_t emscripten_get_audio_stream_played_samples(ALLEGRO_AUDIO_STREAM *stream) {
	return 0;
}

SYMBOL_EXPORT void *emscripten_get_audio_stream_fragment(ALLEGRO_AUDIO_STREAM *stream) {
	return NULL;
}

SYMBOL_EXPORT bool emscripten_set_audio_stream_fragment(ALLEGRO_AUDIO_STREAM *stream, void *val) {
	return false;
}

SYMBOL_EXPORT bool emscripten_set_audio_stream_loop_secs(ALLEGRO_AUDIO_STREAM *stream, double start, double end) {
	return false;
}


SYMBOL_EXPORT unsigned int emscripten_get_audio_stream_fragments(ALLEGRO_AUDIO_STREAM *stream) {
	return 0;
}

SYMBOL_EXPORT unsigned int emscripten_get_available_audio_stream_fragments(ALLEGRO_AUDIO_STREAM *stream) {
	return 0;
}

SYMBOL_EXPORT double emscripten_get_audio_stream_length_sec(ALLEGRO_AUDIO_STREAM *stream) {
	// FIXME
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_length(s->instance);
}

SYMBOL_EXPORT bool emscripten_seek_audio_stream_secs(ALLEGRO_AUDIO_STREAM *stream, double val) {
	// FIXME
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_set_sample_instance_position(s->instance, val);
}

SYMBOL_EXPORT double emscripten_get_audio_stream_position_secs(ALLEGRO_AUDIO_STREAM *stream) {
	// FIXME
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	return al_get_sample_instance_position(s->instance);
}

SYMBOL_EXPORT void emscripten_destroy_audio_stream(ALLEGRO_AUDIO_STREAM *stream) {
	EMSCRIPTEN_AUDIO_STREAM *s = (EMSCRIPTEN_AUDIO_STREAM*)stream;
	al_destroy_sample_instance(s->instance);
	al_destroy_sample(s->sample);
}

ALLEGRO_AUDIO_STREAM* emscripten_create_audio_stream(size_t fragment_count, unsigned int frag_samples, unsigned int freq, ALLEGRO_AUDIO_DEPTH depth, ALLEGRO_CHANNEL_CONF chan_conf) {
	return NULL;
}

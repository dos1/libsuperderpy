/*! \file libsuperderpy.h
 *  \brief Headers of main file of libsuperderpy engine.
 *
 *   Include this to use the engine functions.
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
#ifndef LIBSUPERDERPY_MAIN_H
#define LIBSUPERDERPY_MAIN_H

struct Game;
struct GamestateResources;

#ifndef LIBSUPERDERPY_DATA_TYPE
#define LIBSUPERDERPY_DATA_TYPE void
#endif

#include <allegro5/allegro.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_video.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#if defined(ALLEGRO_ANDROID)
#define ALLEGRO_UNSTABLE
#include <allegro5/allegro_android.h>
#elif defined(ALLEGRO_WINDOWS)
#include <allegro5/allegro_windows.h>
#elif defined(ALLEGRO_WITH_XWINDOWS)
#include <allegro5/allegro_x.h>
#elif defined(ALLEGRO_MACOS)
#include <allegro5/allegro_osx.h>
#elif defined(ALLEGRO_IPHONE)
#include <allegro5/allegro_iphone.h>
#elif defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#include "character.h"
#include "config.h"
#include "gamestate.h"
#include "mainloop.h"
#include "maths.h"
#include "particle.h"
#include "shader.h"
#include "timeline.h"
#include "tween.h"
#include "utils.h"
#ifdef __EMSCRIPTEN__
#include "emscripten-audio-stream.h"
#endif

#define LIBSUPERDERPY_BITMAP_HASHMAP_BUCKETS 16

struct Viewport {
	int width; /*!< Width of the drawing canvas. */
	int height; /*!< Height of the drawing canvas. */
	float aspect; /*!< When set instead of width/height pair, makes the viewport side fluid; when non-zero, locks its aspect ratio. */
	bool integer_scaling; /*!< Ensure that the viewport is zoomed only with integer factors. */
	bool pixel_perfect; /*!< Ensure that the resulting image is really viewport-sized and (potentially) rescaled afterwards, as opposed to default transformation-based scaling. */
};

/*! \brief Main struct of the game. */
struct Game {
	ALLEGRO_DISPLAY* display; /*!< Main Allegro display. */

	ALLEGRO_TRANSFORM projection; /*!< Projection of the game canvas into the actual game window. */

	struct Viewport viewport, viewport_config;

	double time; /*!< In-game total passed time in seconds. */

	struct {
		int fx; /*!< Effects volume. */
		int music; /*!< Music volume. */
		int voice; /*!< Voice volume. */
		bool mute; /*!< Whether audio should be muted globally. */
		bool fullscreen; /*!< Fullscreen toggle. */
		bool debug; /*!< Toggles debug mode. */
		int width; /*!< Width of window as being set in configuration. */
		int height; /*!< Height of window as being set in configuration. */
	} config;

	struct {
		ALLEGRO_VOICE* v; /*!< Main voice used by the game. */
		ALLEGRO_MIXER* mixer; /*!< Main mixer of the game. */
		ALLEGRO_MIXER* music; /*!< Music mixer. */
		ALLEGRO_MIXER* voice; /*!< Voice mixer. */
		ALLEGRO_MIXER* fx; /*!< Effects mixer. */
	} audio; /*!< Audio resources. */

	struct {
		struct Gamestate* gamestates; /*!< List of known gamestates. */
		ALLEGRO_FONT* font_console; /*!< Font used in game console. */
		ALLEGRO_FONT* font_bsod; /*!< Font used in Blue Screens of Derp. */
		char console[5][1024];
		unsigned int console_pos;
		ALLEGRO_EVENT_QUEUE* event_queue; /*!< Main event queue. */
		ALLEGRO_TIMER* timer; /*!< Main LPS (logic) timer. */
		bool showconsole; /*!< If true, game console is rendered on screen. */
		bool showtimeline;

		struct {
			double old_time, fps, time;
			int frames_done;
		} fps_count; /*!< Used for counting the effective FPS. */

		ALLEGRO_CONFIG* config; /*!< Configuration file interface. */

		int argc;
		char** argv;

		struct {
			struct Gamestate* gamestate;
			struct Gamestate* current;
			int progress;
			int loaded, toLoad;
			volatile bool inProgress;
			double time;
		} loading;

		struct Gamestate* current_gamestate;

		struct List *garbage, *timelines, *shaders, *bitmaps[LIBSUPERDERPY_BITMAP_HASHMAP_BUCKETS];

		double timestamp;

		struct {
			int x, y;
			int w, h;
		} clip_rect;

		bool paused;

		volatile bool texture_sync;
		ALLEGRO_MUTEX* texture_sync_mutex;
		ALLEGRO_COND* texture_sync_cond;

		struct {
			bool verbose, livereload, autopause;
		} debug;

#ifdef ALLEGRO_MACOSX
		char cwd[MAXPATHLEN];
#endif

	} _priv; /*!< Private resources. Do not use in gamestates! */

	bool shutting_down; /*!< If true then shut down of the game is pending. */
	bool restart; /*!< If true then restart of the game is pending. */
	bool touch;
	bool joystick;
	bool mouse;

	bool show_loading_on_launch;

	const char* name;

	ALLEGRO_EVENT_SOURCE event_source;

	float loading_progress;
	ALLEGRO_BITMAP* loading_fb;

	struct {
		bool (*event)(struct Game* game, ALLEGRO_EVENT* ev);
		void (*destroy)(struct Game* game);
		void (*compositor)(struct Game* game, struct Gamestate* gamestates);
		void (*prelogic)(struct Game* game, double delta);
		void (*postlogic)(struct Game* game, double delta);
		void (*predraw)(struct Game* game);
		void (*postdraw)(struct Game* game);
	} handlers;

	LIBSUPERDERPY_DATA_TYPE* data;
};

struct Game* libsuperderpy_init(int argc, char** argv, const char* name, struct Viewport viewport);
int libsuperderpy_start(struct Game* game);
int libsuperderpy_run(struct Game* game);
void libsuperderpy_destroy(struct Game* game);

#endif

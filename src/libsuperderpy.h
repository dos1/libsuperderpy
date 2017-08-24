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

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_ttf.h>
#ifdef ALLEGRO_ANDROID
#include <allegro5/allegro_android.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "emscripten-audio-stream.h"
#endif
#include <sys/param.h>
#include "gamestate.h"
#include "config.h"
#include "timeline.h"
#include "utils.h"
#include "character.h"

#ifndef LIBSUPERDERPY_DATA_TYPE
#define LIBSUPERDERPY_DATA_TYPE void
#endif

struct Gamestate;

struct Viewport {
		int width; /*!< Actual available width of the drawing canvas. */
		int height; /*!< Actual available height of the drawing canvas. */
		float aspect;
		bool integer_scaling;
};

/*! \brief Main struct of the game. */
struct Game {
		ALLEGRO_DISPLAY *display; /*!< Main Allegro display. */

		ALLEGRO_TRANSFORM projection; /*!< Projection of the game canvas into the actual game window. */

		struct Viewport viewport, viewport_config;

		struct {
				int fx; /*!< Effects volume. */
				int music; /*!< Music volume. */
				int voice; /*!< Voice volume. */
				bool fullscreen; /*!< Fullscreen toggle. */
				bool debug; /*!< Toggles debug mode. */
				int width; /*!< Width of window as being set in configuration. */
				int height; /*!< Height of window as being set in configuration. */
		} config;

		struct {
				ALLEGRO_VOICE *v; /*!< Main voice used by the game. */
				ALLEGRO_MIXER *mixer; /*!< Main mixer of the game. */
				ALLEGRO_MIXER *music; /*!< Music mixer. */
				ALLEGRO_MIXER *voice; /*!< Voice mixer. */
				ALLEGRO_MIXER *fx; /*!< Effects mixer. */
		} audio; /*!< Audio resources. */

		struct {
				struct Gamestate *gamestates; /*!< List of known gamestates. */
				bool gamestate_scheduled; /*!< Whether there's some gamestate lifecycle management work to do. */
				ALLEGRO_FONT *font_console; /*!< Font used in game console. */
				ALLEGRO_FONT *font_bsod; /*!< Font used in Blue Screens of Derp. */
				ALLEGRO_BITMAP *console; /*!< Bitmap with game console. */
				ALLEGRO_BITMAP *console_tmp; /*!< Bitmap used for drawing game console. */
				ALLEGRO_EVENT_QUEUE *event_queue; /*!< Main event queue. */
				ALLEGRO_TIMER *timer; /*!< Main LPS (logic) timer. */
				bool showconsole; /*!< If true, game console is rendered on screen. */
				bool showtimeline;

				struct {
						double old_time, fps;
						int frames_done;
				} fps_count; /*!< Used for counting the effective FPS. */

				ALLEGRO_CONFIG *config; /*!< Configuration file interface. */

				struct {
						void (*Draw)(struct Game *game, void* data, float p);
						void* (*Load)(struct Game *game);
						void (*Start)(struct Game *game, void* data);
						void (*Stop)(struct Game *game, void* data);
						void (*Unload)(struct Game *game, void* data);

						void* data;
				} loading; /*!< Interface for accessing loading screen functions. */

				int argc;
				char** argv;

				struct {
						int p;
						struct Gamestate *tmp;
						double t;
						int loaded, toLoad;
				} tmp_gamestate;

				struct Gamestate *current_gamestate;

				struct libsuperderpy_list *garbage, *timelines;

				bool draw;

#ifdef ALLEGRO_MACOSX
				char cwd[MAXPATHLEN];
#endif

		} _priv; /*!< Private resources. Do not use in gamestates! */

		bool shuttingdown; /*!< If true then shut down of the game is pending. */
		bool restart; /*!< If true then restart of the game is pending. */
		bool touch;

		bool show_loading_on_launch;

		const char* name;

		ALLEGRO_EVENT_SOURCE event_source;

		bool (*eventHandler)(struct Game *game, ALLEGRO_EVENT *ev);
		void (*destroyHandler)(struct Game *game);

		LIBSUPERDERPY_DATA_TYPE *data;

};

struct Game* libsuperderpy_init(int argc, char **argv, const char* name, struct Viewport viewport);
int libsuperderpy_run(struct Game* game);
void libsuperderpy_destroy(struct Game* game);

#endif

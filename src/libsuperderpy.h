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

#ifdef _WIN32
#define UNICODE
#define _UNICODE
#include <tchar.h>
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
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
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

#ifdef LIBSUPERDERPY_IMGUI
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS 1
#include "3rdparty/cimgui/cimgui.h"
#endif

#define LIBSUPERDERPY_BITMAP_HASHMAP_BUCKETS 16

#if !defined(LIBSUPERDERPY_PRIV_ACCESS) && defined(__GNUC__)
#define LIBSUPERDERPY_DEPRECATED_PRIV __attribute__((deprecated))
#else
#define LIBSUPERDERPY_DEPRECATED_PRIV
#endif

#define STRINGIFY(a) #a
#if defined(__clang__) || defined(__codemodel__)
#define SUPPRESS_WARNING(x) _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wpragmas\"") _Pragma(STRINGIFY(clang diagnostic ignored x))
#define SUPPRESS_END _Pragma("clang diagnostic pop")
#elif defined(__GNUC__) && !defined(MAEMO5)
#define SUPPRESS_WARNING(x) _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wpragmas\"") _Pragma(STRINGIFY(GCC diagnostic ignored x))
#define SUPPRESS_END _Pragma("GCC diagnostic pop")
#else
#define SUPPRESS_WARNING(x)
#define SUPPRESS_END
#endif

#if defined(ALLEGRO_WINDOWS) && !defined(LIBSUPERDERPY_NO_MAIN_MANGLING)
int _libsuperderpy_main(int argc, char** argv);
#define main(a, b)                                                                      \
	wmain(int argc, wchar_t** wargv) {                                                    \
		char* argv[argc];                                                                   \
		for (int i = 0; i < argc; i++) {                                                    \
			size_t size = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL); \
			argv[i] = alloca(sizeof(char) * size);                                            \
			WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], size, NULL, NULL);         \
		}                                                                                   \
		return _libsuperderpy_main(argc, argv);                                             \
	}                                                                                     \
	int _libsuperderpy_main(a, b)
#endif

/*! \brief A list of user callbacks to register. */
struct Handlers {
	bool (*event)(struct Game* game, ALLEGRO_EVENT* ev);
	void (*destroy)(struct Game* game);
	void (*compositor)(struct Game* game);
	void (*prelogic)(struct Game* game, double delta);
	void (*postlogic)(struct Game* game, double delta);
	void (*predraw)(struct Game* game);
	void (*postdraw)(struct Game* game);
};

/*! \brief Parameters for engine initialization. All values default to 0/false/NULL. */
struct Params {
	int width; /*!< Width of the drawing canvas. */
	int height; /*!< Height of the drawing canvas. */
	float aspect; /*!< When set instead of width&height pair, makes the viewport resizeable with locked aspect ratio. */
	bool integer_scaling; /*!< Ensure that the viewport is zoomed only by integer factors. */
	bool depth_buffer; /*!< Request a depth buffer for the framebuffer's render target. */
	bool show_loading_on_launch; /*!< Whether the loading screen should be shown when loading the initial set of gamestates. */
	bool fixed_size; /*!< If set to true, the game's window will not be resizable. */
	int samples; /*!< How many samples should be used for multisampling; 0 to disable. */
	int sample_rate; /*!< Default sample rate of audio output; 0 to use engine default. */
	char* window_title; /*!< A title of the game's window. When NULL, al_get_app_name() is used. */
	struct Handlers handlers; /*!< A list of user callbacks to register. */
};

/*! \brief Main struct of the game. */
struct Game {
	ALLEGRO_DISPLAY* display; /*!< Main Allegro display. */
	ALLEGRO_EVENT_SOURCE event_source; /*!< Event source for user events. */
	struct {
		ALLEGRO_VOICE* v; /*!< Main voice used by the game. */
		ALLEGRO_MIXER* mixer; /*!< Main mixer of the game. */
		ALLEGRO_MIXER* music; /*!< Music mixer. */
		ALLEGRO_MIXER* voice; /*!< Voice mixer. */
		ALLEGRO_MIXER* fx; /*!< Effects mixer. */
	} audio; /*!< Audio resources. */

	LIBSUPERDERPY_DATA_TYPE* data; /*!< User defined structure. */

	struct {
		int width;
		int height;
	} viewport; /*!< Canvas size. */

	double time; /*!< In-game total passed time in seconds. */

	struct {
		int fx; /*!< Effects volume. */
		int music; /*!< Music volume. */
		int voice; /*!< Voice volume. */
		bool mute; /*!< Whether audio should be muted globally. */
		int samplerate; /*!< Sample rate of audio output. */
		bool fullscreen; /*!< Fullscreen toggle. */
		int width; /*!< Width of window as being set in configuration. */
		int height; /*!< Height of window as being set in configuration. */
		bool autopause; /*!< Pauses/resumes the game when the window loses/gains focus. */
		struct {
			bool enabled; /*!< Toggles debug mode. */
			bool verbose; /*!< Prints file names and line numbers with every message. */
			bool livereload; /*!< Automatically reloads gamestates on window focus. */
		} debug; /*!< Debug mode settings. */
	} config; /*!< Configuration values from the config file. */

	bool show_console; /*!< If true, game console is rendered on screen. */

	struct {
		int x, y;
		int w, h;
	} clip_rect; /*!< Clipping rectangle of the display's backbuffer. */

	struct {
		float progress;
		bool shown;
	} loading; /*!< Data about gamestate loading process. */

	struct {
		struct {
			bool touch;
			bool joystick;
			bool mouse;
		} available;
	} input;

	/// \private
	struct {
		struct Params params;

		struct Gamestate* gamestates; /*!< List of known gamestates. */
		ALLEGRO_FONT* font_console; /*!< Font used in game console. */
		ALLEGRO_FONT* font_bsod; /*!< Font used in Blue Screens of Derp. */
		char console[5][1024];
		unsigned int console_pos;
		ALLEGRO_EVENT_QUEUE* event_queue; /*!< Main event queue. */
		bool show_timeline;

		double speed; /*!< Speed of the game */

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
			int loaded, to_load;
			volatile bool in_progress;
			bool lock;
			double time;
		} loading;

		struct Gamestate* current_gamestate;

		struct List *garbage, *timelines, *shaders, *bitmaps[LIBSUPERDERPY_BITMAP_HASHMAP_BUCKETS];

		double timestamp;

		bool paused;
		bool started;

		volatile bool texture_sync;
		ALLEGRO_MUTEX* texture_sync_mutex;
		ALLEGRO_COND* texture_sync_cond;

		volatile bool in_bsod;
		volatile bool bsod_sync;
		ALLEGRO_MUTEX* bsod_mutex;
		ALLEGRO_COND* bsod_cond;

		ALLEGRO_MUTEX* mutex;

		char* name;

		bool shutting_down; /*!< If true then shut down of the game is pending. */
		bool restart; /*!< If true then restart of the game is pending. */

		ALLEGRO_TRANSFORM projection; /*!< Projection of the game canvas into the actual game window. */

		ALLEGRO_TRANSFORM* transforms;
		int transforms_no, transforms_alloc;

		int window_width, window_height;

		int samplerate;

#ifdef ALLEGRO_MACOSX
		char cwd[MAXPATHLEN];
#endif

	} _priv LIBSUPERDERPY_DEPRECATED_PRIV; /*!< Private resources. Do not use in gamestates! */
};

struct Game* libsuperderpy_init(int argc, char** argv, const char* name, struct Params params);
int libsuperderpy_start(struct Game* game);
int libsuperderpy_run(struct Game* game);
void libsuperderpy_destroy(struct Game* game);

#ifdef LIBSUPERDERPY_STATIC_GAMESTATES
void __libsuperderpy_register_gamestate(const char* name, struct GamestateAPI* api, struct Game* game);
#endif

#endif /* LIBSUPERDERPY_MAIN_H */

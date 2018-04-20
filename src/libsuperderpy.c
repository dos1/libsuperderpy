/*! \file main.c
 *  \brief Main file of SuperDerpy engine.
 *
 *  Contains basic functions shared by all views.
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
 *
 * Also, ponies.
 */
#ifdef LIBSUPERDERPY_MOUSE_EMULATION
#define ALLEGRO_UNSTABLE
#endif

#include "libsuperderpy.h"
#include "internal.h"
#include <dlfcn.h>
#include <libgen.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <sys/param.h>
#include <unistd.h>
#ifdef ALLEGRO_MACOSX
#include <mach-o/dyld.h>
#endif

SYMBOL_EXPORT struct Game* libsuperderpy_init(int argc, char** argv, const char* name, struct Viewport viewport) {
	struct Game* game = calloc(1, sizeof(struct Game));

	game->name = name;
	game->viewport_config = viewport;

#ifdef ALLEGRO_MACOSX
	getcwd(game->_priv.cwd, MAXPATHLEN);
	char exe_path[MAXPATHLEN];
	uint32_t size = sizeof(exe_path);
	_NSGetExecutablePath(exe_path, &size);
	chdir(dirname(exe_path));
#endif

	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		free(game);
		return NULL;
	}

	InitConfig(game);

	game->_priv.fps_count.frames_done = 0;
	game->_priv.fps_count.fps = 0;
	game->_priv.fps_count.old_time = 0;

	game->_priv.font_console = NULL;
	game->_priv.font_bsod = NULL;
	game->_priv.console_pos = 0;
	for (unsigned int i = 0; i < (sizeof(game->_priv.console) / sizeof(game->_priv.console[0])); i++) {
		game->_priv.console[i][0] = '\0';
	}

	game->_priv.garbage = NULL;
	game->_priv.timelines = NULL;

	game->_priv.paused = false;

	game->handlers.event = NULL;
	game->handlers.destroy = NULL;
	game->handlers.compositor = NULL;
	game->handlers.prelogic = NULL;
	game->handlers.postlogic = NULL;
	game->handlers.predraw = NULL;
	game->handlers.postdraw = NULL;

	game->config.fullscreen = strtol(GetConfigOptionDefault(game, "SuperDerpy", "fullscreen", "1"), NULL, 10);
	game->config.music = strtol(GetConfigOptionDefault(game, "SuperDerpy", "music", "10"), NULL, 10);
	game->config.voice = strtol(GetConfigOptionDefault(game, "SuperDerpy", "voice", "10"), NULL, 10);
	game->config.fx = strtol(GetConfigOptionDefault(game, "SuperDerpy", "fx", "10"), NULL, 10);
	game->config.debug = strtol(GetConfigOptionDefault(game, "SuperDerpy", "debug", "0"), NULL, 10);
	game->config.width = strtol(GetConfigOptionDefault(game, "SuperDerpy", "width", "1280"), NULL, 10);
	if (game->config.width < 320) { game->config.width = 320; }
	game->config.height = strtol(GetConfigOptionDefault(game, "SuperDerpy", "height", "720"), NULL, 10);
	if (game->config.height < 180) { game->config.height = 180; }

	game->_priv.showconsole = game->config.debug;
	game->_priv.showtimeline = false;

	if (!al_init_image_addon()) {
		fprintf(stderr, "failed to initialize image addon!\n");
		/*al_show_native_message_box(display, "Error", "Error", "Failed to initialize al_init_image_addon!",
																														 NULL, ALLEGRO_MESSAGEBOX_ERROR);*/
		return NULL;
	}

	if (!al_install_audio()) {
		fprintf(stderr, "failed to initialize audio!\n");
		return NULL;
	}

	if (!al_init_acodec_addon()) {
		fprintf(stderr, "failed to initialize audio codecs!\n");
		return NULL;
	}

	if (!al_install_keyboard()) {
		fprintf(stderr, "failed to initialize keyboard!\n");
		return NULL;
	}

	if (!al_init_primitives_addon()) {
		fprintf(stderr, "failed to initialize primitives!\n");
		return NULL;
	}

	if (!al_install_mouse()) {
		fprintf(stderr, "failed to initialize the mouse!\n");
		return NULL;
	}

	if (!al_init_video_addon()) {
		fprintf(stderr, "failed to initialize the video addon!\n");
		return NULL;
	}

	al_init_font_addon();

	if (!al_init_ttf_addon()) {
		fprintf(stderr, "failed to initialize fonts!\n");
		return NULL;
	}

	game->touch = false;

	if (!strtol(GetConfigOptionDefault(game, "SuperDerpy", "disableTouch", "0"), NULL, 10)) {
		game->touch = al_install_touch_input();
	}

#ifdef LIBSUPERDERPY_MOUSE_EMULATION
	if (game->touch) {
		al_set_mouse_emulation_mode(ALLEGRO_MOUSE_EMULATION_TRANSPARENT);
	}
#endif

	al_install_joystick();

	al_set_new_display_flags((game->config.fullscreen ? (ALLEGRO_FULLSCREEN_WINDOW | ALLEGRO_FRAMELESS) : ALLEGRO_WINDOWED) | ALLEGRO_RESIZABLE | ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);
#ifdef __EMSCRIPTEN__
	al_set_new_display_flags((al_get_new_display_flags() | ALLEGRO_WINDOWED) ^ ALLEGRO_FULLSCREEN_WINDOW);
#endif
	al_set_new_display_option(ALLEGRO_VSYNC, 2 - strtol(GetConfigOptionDefault(game, "SuperDerpy", "vsync", "1"), NULL, 10), ALLEGRO_SUGGEST);

#ifdef LIBSUPERDERPY_ORIENTATION_LANDSCAPE
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);
#elif defined(LIBSUPERDERPY_ORIENTATION_PORTRAIT)
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_PORTRAIT, ALLEGRO_SUGGEST);
#endif

	al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 24, ALLEGRO_SUGGEST);

#ifdef ALLEGRO_WINDOWS
	al_set_new_window_position(20, 40); // workaround nasty Windows bug with window being created off-screen
#endif

	al_set_new_window_title(game->name);
	game->display = al_create_display(game->config.width, game->config.height);
	if (!game->display) {
		fprintf(stderr, "Failed to create display!\n");
		return NULL;
	}

#ifdef ALLEGRO_ANDROID
	al_android_set_apk_file_interface();
	al_android_set_apk_fs_interface();
#endif

	SetupViewport(game, viewport);

	PrintConsole(game, "libsuperderpy");
	PrintConsole(game, "OpenGL%s (%08X)", al_get_opengl_variant() == ALLEGRO_OPENGL_ES ? " ES" : "", al_get_opengl_version());

	PrintConsole(game, "Max bitmap size: %d", al_get_display_option(game->display, ALLEGRO_MAX_BITMAP_SIZE));
	PrintConsole(game, "Color buffer bits: %d", al_get_display_option(game->display, ALLEGRO_COLOR_SIZE));
	PrintConsole(game, "Depth buffer bits: %d", al_get_display_option(game->display, ALLEGRO_DEPTH_SIZE));
	PrintConsole(game, "Stencil buffer bits: %d", al_get_display_option(game->display, ALLEGRO_STENCIL_SIZE));
	PrintConsole(game, "NPOT bitmaps: %d", al_get_display_option(game->display, ALLEGRO_SUPPORT_NPOT_BITMAP));
	PrintConsole(game, "Separate alpha blender: %d", al_get_display_option(game->display, ALLEGRO_SUPPORT_SEPARATE_ALPHA));

	if (!al_get_display_option(game->display, ALLEGRO_COMPATIBLE_DISPLAY)) {
		al_destroy_display(game->display);
		fprintf(stderr, "Created display is Allegro incompatible!\n");
		return NULL;
	}

	if (!al_get_display_option(game->display, ALLEGRO_CAN_DRAW_INTO_BITMAP)) {
		FatalError(game, true, "The created display does not support drawing into bitmaps.");
		al_destroy_display(game->display);
		return NULL;
	}

	PrintConsole(game, "Viewport %dx%d", game->viewport.width, game->viewport.height);

	ALLEGRO_BITMAP* icon = al_load_bitmap(GetDataFilePath(game, GetGameName(game, "icons/%s.png")));
	al_set_display_icon(game->display, icon);
	al_destroy_bitmap(icon);

	if (game->config.fullscreen) { al_hide_mouse_cursor(game->display); }
	al_inhibit_screensaver(true);

	al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

	game->_priv.gamestates = NULL;
	game->_priv.gamestate_scheduled = false;

	al_init_user_event_source(&(game->event_source));

	game->_priv.event_queue = al_create_event_queue();
	if (!game->_priv.event_queue) {
		FatalError(game, true, "Failed to create event queue.");
		al_destroy_display(game->display);
		return NULL;
	}

	ALLEGRO_AUDIO_DEPTH depth = ALLEGRO_AUDIO_DEPTH_FLOAT32;
#ifdef ALLEGRO_ANDROID
	depth = ALLEGRO_AUDIO_DEPTH_INT16;
#endif
	game->audio.v = al_create_voice(44100, depth, ALLEGRO_CHANNEL_CONF_2);
	if (!game->audio.v) {
		// fallback
		depth = (depth == ALLEGRO_AUDIO_DEPTH_FLOAT32) ? ALLEGRO_AUDIO_DEPTH_INT16 : ALLEGRO_AUDIO_DEPTH_FLOAT32;
		game->audio.v = al_create_voice(44100, depth, ALLEGRO_CHANNEL_CONF_2);
	}
	game->audio.mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.fx = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.music = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.voice = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	al_attach_mixer_to_voice(game->audio.mixer, game->audio.v);
	al_attach_mixer_to_mixer(game->audio.fx, game->audio.mixer);
	al_attach_mixer_to_mixer(game->audio.music, game->audio.mixer);
	al_attach_mixer_to_mixer(game->audio.voice, game->audio.mixer);
	al_set_mixer_gain(game->audio.fx, game->config.fx / 10.0);
	al_set_mixer_gain(game->audio.music, game->config.music / 10.0);
	al_set_mixer_gain(game->audio.voice, game->config.voice / 10.0);

	setlocale(LC_NUMERIC, "C");

	game->_priv.argc = argc;
	game->_priv.argv = argv;

	game->data = NULL;

	game->shutting_down = false;
	game->restart = false;

	game->show_loading_on_launch = false;

	game->loading_progress = 0;

	return game;
}

SYMBOL_EXPORT int libsuperderpy_run(struct Game* game) {
	al_register_event_source(game->_priv.event_queue, al_get_display_event_source(game->display));
	al_register_event_source(game->_priv.event_queue, al_get_mouse_event_source());
	al_register_event_source(game->_priv.event_queue, al_get_keyboard_event_source());
	al_register_event_source(game->_priv.event_queue, al_get_joystick_event_source());
	if (game->touch) {
		al_register_event_source(game->_priv.event_queue, al_get_touch_input_event_source());
#ifdef LIBSUPERDERPY_MOUSE_EMULATION
		al_register_event_source(game->_priv.event_queue, al_get_touch_input_mouse_emulation_event_source());
#endif
	}
	al_register_event_source(game->_priv.event_queue, &(game->event_source));

	al_clear_to_color(al_map_rgb(0, 0, 0));
	game->_priv.timer = al_create_timer(ALLEGRO_BPS_TO_SECS(60)); // logic timer
	if (!game->_priv.timer) {
		FatalError(game, true, "Failed to create logic timer.");
		return 1;
	}
	al_register_event_source(game->_priv.event_queue, al_get_timer_event_source(game->_priv.timer));

	al_flip_display();
	al_start_timer(game->_priv.timer);

	{
		struct Gamestate* tmp = game->_priv.gamestates;
		while (tmp) {
			// don't show loading screen on init if requested
			tmp->showLoading = game->show_loading_on_launch;
			tmp = tmp->next;
		}
	}

	game->_priv.loading.gamestate = AllocateGamestate(game, "loading");
	if (!OpenGamestate(game, game->_priv.loading.gamestate) || !LinkGamestate(game, game->_priv.loading.gamestate)) {
		// TODO: support loading-less scenario
		return 2;
	}
	game->_priv.loading.gamestate->data = (*game->_priv.loading.gamestate->api->Gamestate_Load)(game, NULL);
	PrintConsole(game, "Loading screen registered.");

	game->_priv.timestamp = al_get_time();
	game->_priv.draw = true;
#ifdef __EMSCRIPTEN__
	void libsuperderpy_mainloop(void* game);
	emscripten_set_main_loop_arg(libsuperderpy_mainloop, game, 0, true);
	return 0;
}

bool redraw = false;

SYMBOL_INTERNAL void libsuperderpy_mainloop_exit(struct Game* game) {
	libsuperderpy_destroy(game);
	free(game);
	printf("Halted.\n");
	emscripten_cancel_main_loop();
}

SYMBOL_INTERNAL void libsuperderpy_mainloop(void* g) {
	struct Game* game = (struct Game*)g;
	redraw = true;
	while (!al_is_event_queue_empty(game->_priv.event_queue) || redraw) {
#else
	bool redraw = false;
	while (1) {
#endif
		ClearGarbage(game);

		// TODO: split mainloop to functions to make it readable
		ALLEGRO_EVENT ev;
		if (game->_priv.draw && (((redraw || true) && al_is_event_queue_empty(game->_priv.event_queue)) || (game->_priv.gamestate_scheduled))) {
			game->_priv.gamestate_scheduled = false;
			struct Gamestate* tmp = game->_priv.gamestates;

			game->_priv.loading.toLoad = 0;
			game->_priv.loading.loaded = 0;
			game->loading_progress = 0;

			// TODO: support gamestate dependences/ordering
			while (tmp) {
				if (tmp->pending_stop) {
					PrintConsole(game, "Stopping gamestate \"%s\"...", tmp->name);
					game->_priv.current_gamestate = tmp;
					(*tmp->api->Gamestate_Stop)(game, tmp->data);
					tmp->started = false;
					tmp->pending_stop = false;
				}

				if (tmp->pending_load) { game->_priv.loading.toLoad++; }
				tmp = tmp->next;
			}

			tmp = game->_priv.gamestates;

			while (tmp) {
				if (tmp->pending_unload) {
					PrintConsole(game, "Unloading gamestate \"%s\"...", tmp->name);
					al_stop_timer(game->_priv.timer);
					tmp->loaded = false;
					tmp->pending_unload = false;
					game->_priv.current_gamestate = tmp;
					(*tmp->api->Gamestate_Unload)(game, tmp->data);
					al_resume_timer(game->_priv.timer);
				}
				if (tmp->pending_load) {
					al_stop_timer(game->_priv.timer);
					if (tmp->showLoading) {
						(*game->_priv.loading.gamestate->api->Gamestate_Start)(game, game->_priv.loading.gamestate->data);
					}

					if (!tmp->api) {
						if (!OpenGamestate(game, tmp) || !LinkGamestate(game, tmp)) {
							tmp->pending_load = false;
							tmp->pending_start = false;
							tmp->next = tmp;
							continue;
						}
					}
					if (tmp->api) {
						PrintConsole(game, "Loading gamestate \"%s\"...", tmp->name);
						game->_priv.loading.progress = -1;

						game->_priv.loading.current = tmp;
						game->_priv.current_gamestate = tmp;

						struct GamestateLoadingThreadData data = {.game = game, .gamestate = tmp, .bitmap_flags = al_get_new_bitmap_flags()};
						game->_priv.loading.inProgress = true;

#ifndef LIBSUPERDERPY_SINGLE_THREAD
						al_run_detached_thread(GamestateLoadingThread, &data);
						double time = al_get_time();
						while (game->_priv.loading.inProgress) {
							DrawGamestates(game);
							al_set_target_backbuffer(game->display);
							double delta = al_get_time() - time;
							if (tmp->showLoading) {
								(*game->_priv.loading.gamestate->api->Gamestate_Logic)(game, game->_priv.loading.gamestate->data, delta);
								(*game->_priv.loading.gamestate->api->Gamestate_Draw)(game, game->_priv.loading.gamestate->data);
							}
							time += delta;
							DrawConsole(game);
							al_flip_display();
						}
#else
						GamestateLoadingThread(&data);
#endif

						al_set_new_bitmap_flags(data.bitmap_flags);
						// TODO: compile shaders
						al_convert_memory_bitmaps();
						game->_priv.loading.loaded++;

						tmp->loaded = true;
						tmp->pending_load = false;
					}
					if (tmp->showLoading) {
						(*game->_priv.loading.gamestate->api->Gamestate_Stop)(game, game->_priv.loading.gamestate->data);
					}
					al_resume_timer(game->_priv.timer);
					game->_priv.timestamp = al_get_time();
				}

				tmp = tmp->next;
			}

			bool gameActive = false;
			tmp = game->_priv.gamestates;

			while (tmp) {
				if ((tmp->pending_start) && (tmp->loaded)) {
					PrintConsole(game, "Starting gamestate \"%s\"...", tmp->name);
					al_stop_timer(game->_priv.timer);
					game->_priv.current_gamestate = tmp;
					tmp->started = true;
					tmp->pending_start = false;
					(*tmp->api->Gamestate_Start)(game, tmp->data);
					al_resume_timer(game->_priv.timer);
				}

				if ((tmp->started) || (tmp->pending_start) || (tmp->pending_load)) {
					gameActive = true;
				}
				tmp = tmp->next;
			}

			if (!gameActive) {
				PrintConsole(game, "No gamestates left, exiting...");
#ifdef __EMSCRIPTEN__
				libsuperderpy_mainloop_exit(game);
#endif
				break;
			}

			al_convert_memory_bitmaps();

			double delta = al_get_time() - game->_priv.timestamp;
			game->_priv.timestamp += delta;
			delta *= ALLEGRO_BPS_TO_SECS(al_get_timer_speed(game->_priv.timer) / (1 / 60.f));
			if (!game->_priv.paused) {
				LogicGamestates(game, delta);
				DrawGamestates(game);
			}
			//redraw = true;

			DrawConsole(game);
			//al_wait_for_vsync();
			al_flip_display();
			redraw = false;

		} else {
#ifdef __EMSCRIPTEN__
			if (al_is_event_queue_empty(game->_priv.event_queue)) {
				return;
			}
#endif

			al_wait_for_event(game->_priv.event_queue, &ev);

			if (game->handlers.event) {
				if ((*game->handlers.event)(game, &ev)) {
					continue;
				}
			}

			if ((ev.type == ALLEGRO_EVENT_TIMER) && (ev.timer.source == game->_priv.timer)) {
				/*double delta = al_get_time() - game->_priv.timestamp;
				game->_priv.timestamp += delta;
				LogicGamestates(game, delta);
				redraw = true;*/
			} else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
#ifdef __EMSCRIPTEN__
				libsuperderpy_mainloop_exit(game);
#endif
				break;
			} else if (ev.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING) {
				PrintConsole(game, "Engine halted.");
				game->_priv.draw = false;
				al_stop_timer(game->_priv.timer);
				al_detach_voice(game->audio.v);
				FreezeGamestates(game);
				al_acknowledge_drawing_halt(game->display);
			} else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) {
				game->_priv.draw = true;
				al_acknowledge_drawing_resume(game->display);
				PrintConsole(game, "Engine resumed.");
				ReloadGamestates(game);
				UnfreezeGamestates(game);
				al_attach_mixer_to_voice(game->audio.mixer, game->audio.v);
				al_resume_timer(game->_priv.timer);
			} else if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(game->display);
				SetupViewport(game, game->viewport_config);
				ResizeGamestates(game);
			} else if ((game->config.debug) && (ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT)) {
				PauseExecution(game);
			} else if ((game->config.debug) && (ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN)) {
				ResumeExecution(game);
			}
#ifdef ALLEGRO_ANDROID
			else if ((ev.type == ALLEGRO_EVENT_KEY_CHAR) && ((ev.keyboard.keycode == ALLEGRO_KEY_MENU) || (ev.keyboard.keycode == ALLEGRO_KEY_TILDE) || (ev.keyboard.keycode == ALLEGRO_KEY_BACKQUOTE))) {
#else
			else if ((ev.type == ALLEGRO_EVENT_KEY_CHAR) && ((ev.keyboard.keycode == ALLEGRO_KEY_TILDE) || (ev.keyboard.keycode == ALLEGRO_KEY_BACKQUOTE))) {
#endif
				game->_priv.showconsole = !game->_priv.showconsole;
				if (ev.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) {
					game->_priv.showtimeline = game->_priv.showconsole;
				}
			} else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (game->config.debug) && (ev.keyboard.keycode == ALLEGRO_KEY_F1)) {
				int i;
				for (i = 0; i < 512; i++) {
					LogicGamestates(game, 1.0 / 60.0);
				}
				game->_priv.showconsole = true;
				PrintConsole(game, "DEBUG: 512 frames skipped...");
			} else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (game->config.debug) && (ev.keyboard.keycode == ALLEGRO_KEY_F9)) {
				if (game->_priv.paused) {
					PauseExecution(game);
				} else {
					ResumeExecution(game);
				}
			} else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (game->config.debug) && (ev.keyboard.keycode == ALLEGRO_KEY_F10)) {
				double speed = ALLEGRO_BPS_TO_SECS(al_get_timer_speed(game->_priv.timer)); // inverting
				speed -= 10;
				if (speed < 10) { speed = 10; }
				al_set_timer_speed(game->_priv.timer, ALLEGRO_BPS_TO_SECS(speed));
				game->_priv.showconsole = true;
				PrintConsole(game, "DEBUG: Gameplay speed: %.2fx", speed / 60.0);
			} else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (game->config.debug) && (ev.keyboard.keycode == ALLEGRO_KEY_F11)) {
				double speed = ALLEGRO_BPS_TO_SECS(al_get_timer_speed(game->_priv.timer)); // inverting
				speed += 10;
				if (speed > 600) { speed = 600; }
				al_set_timer_speed(game->_priv.timer, ALLEGRO_BPS_TO_SECS(speed));
				game->_priv.showconsole = true;
				PrintConsole(game, "DEBUG: Gameplay speed: %.2fx", speed / 60.0);
			} else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (ev.keyboard.keycode == ALLEGRO_KEY_F12)) {
				DrawGamestates(game);
				int flags = al_get_new_bitmap_flags();
				al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
				ALLEGRO_BITMAP* bitmap = al_create_bitmap(al_get_display_width(game->display), al_get_display_height(game->display));
				al_set_new_bitmap_flags(flags);
				ALLEGRO_BITMAP* target = al_get_target_bitmap();
				al_set_target_bitmap(bitmap);
				al_draw_bitmap(al_get_backbuffer(game->display), 0, 0, 0);
				al_set_target_bitmap(target);
				PrintConsole(game, "Screenshot made! Storing...");

				struct ScreenshotThreadData* data = malloc(sizeof(struct ScreenshotThreadData));
				data->game = game;
				data->bitmap = bitmap;
#ifndef LIBSUPERDERPY_SINGLE_THREAD
				al_run_detached_thread(ScreenshotThread, data);
#else
				ScreenshotThread(data);
#endif
			}
			EventGamestates(game, &ev);
		}
	}

#ifndef __EMSCRIPTEN__
	if (game->handlers.destroy) {
		libsuperderpy_destroy(game);
	}
	return 0;
#endif
}

SYMBOL_EXPORT void libsuperderpy_destroy(struct Game* game) {
	game->shutting_down = true;

	ClearGarbage(game);

	struct Gamestate *tmp = game->_priv.gamestates, *pom;
	while (tmp) {
		if (tmp->started) {
			PrintConsole(game, "Stopping gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Stop)(game, tmp->data);
			tmp->started = false;
		}
		if (tmp->loaded) {
			PrintConsole(game, "Unloading gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Unload)(game, tmp->data);
			tmp->loaded = false;
		}
		CloseGamestate(game, tmp);
		pom = tmp->next;
		free(tmp);
		tmp = pom;
	}

	(*game->_priv.loading.gamestate->api->Gamestate_Unload)(game, game->_priv.loading.gamestate->data);
	CloseGamestate(game, game->_priv.loading.gamestate);
	free(game->_priv.loading.gamestate);

	if (game->handlers.destroy) {
		(*game->handlers.destroy)(game);
	}

	ClearScreen(game);
#ifdef __EMSCRIPTEN__
	{
		ALLEGRO_BITMAP* bmp = al_create_bitmap(320, 180);
		al_set_target_bitmap(bmp);
		al_clear_to_color(al_map_rgb(0, 0, 0));
		ALLEGRO_FONT* font = al_create_builtin_font();
		al_draw_text(font, al_map_rgb(228, 127, 59), 320 / 2, 180 / 2 - 8 - 6, ALLEGRO_ALIGN_CENTER, "It's now safe to turn off");
		al_draw_text(font, al_map_rgb(228, 127, 59), 320 / 2, 180 / 2 - 8 + 6, ALLEGRO_ALIGN_CENTER, "your browser.");
		al_set_target_backbuffer(game->display);
		al_draw_scaled_bitmap(bmp, 0, 0, 320, 180, 0, -game->viewport.height * 0.2, game->viewport.width, game->viewport.height * 1.4, 0);
		al_flip_display();
		al_destroy_bitmap(bmp);
		al_destroy_font(font);
	}
#endif

	PrintConsole(game, "Shutting down...");
	DrawConsole(game);
	al_flip_display();
	while (game->_priv.garbage) {
		free(game->_priv.garbage->data);
		game->_priv.garbage = game->_priv.garbage->next;
	}
	al_destroy_timer(game->_priv.timer);
	Console_Unload(game);
	al_destroy_display(game->display);
	al_destroy_user_event_source(&(game->event_source));
	al_destroy_event_queue(game->_priv.event_queue);
	al_destroy_mixer(game->audio.fx);
	al_destroy_mixer(game->audio.music);
	al_destroy_mixer(game->audio.mixer);
	al_destroy_voice(game->audio.v);
	al_uninstall_audio();
	DeinitConfig(game);
#ifndef __EMSCRIPTEN__
	al_uninstall_system();
	char** argv = game->_priv.argv;
	bool restart = game->restart;
	free(game);
	if (restart) {
#ifdef ALLEGRO_MACOSX
		chdir(game->_priv.cwd);
#endif
		execv(argv[0], argv);
	}
#endif
}

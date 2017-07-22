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

#include <stdio.h>
#include <math.h>
#include <locale.h>
#include <dlfcn.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/param.h>
#include "internal.h"
#include "libsuperderpy.h"
#include "3rdparty/valgrind.h"
#ifdef ALLEGRO_MACOSX
#include <mach-o/dyld.h>
#endif

SYMBOL_EXPORT struct Game* libsuperderpy_init(int argc, char** argv, const char* name, struct Viewport viewport) {

	struct Game *game = calloc(1, sizeof(struct Game));

	game->name = name;
	game->viewport_config = viewport;

#ifdef ALLEGRO_MACOSX
	getcwd(game->_priv.cwd, MAXPATHLEN);
	char exe_path[MAXPATHLEN];
	uint32_t size = sizeof(exe_path);
	_NSGetExecutablePath(exe_path, &size);
	chdir(dirname(exe_path));
#endif


	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return NULL;
	}

	InitConfig(game);

	game->_priv.fps_count.frames_done = 0;
	game->_priv.fps_count.fps = 0;
	game->_priv.fps_count.old_time = 0;

	game->_priv.font_bsod = NULL;
	game->_priv.console = NULL;
	game->_priv.console_tmp = NULL;

	game->_priv.garbage = NULL;

	game->eventHandler = NULL;

	game->config.fullscreen = atoi(GetConfigOptionDefault(game, "SuperDerpy", "fullscreen", "1"));
	game->config.music = atoi(GetConfigOptionDefault(game, "SuperDerpy", "music", "10"));
	game->config.voice = atoi(GetConfigOptionDefault(game, "SuperDerpy", "voice", "10"));
	game->config.fx = atoi(GetConfigOptionDefault(game, "SuperDerpy", "fx", "10"));
	game->config.debug = atoi(GetConfigOptionDefault(game, "SuperDerpy", "debug", "0"));
	game->config.width = atoi(GetConfigOptionDefault(game, "SuperDerpy", "width", "1280"));
	if (game->config.width<320) game->config.width=320;
	game->config.height = atoi(GetConfigOptionDefault(game, "SuperDerpy", "height", "720"));
	if (game->config.height<180) game->config.height=180;

	game->_priv.showconsole = game->config.debug;

	if(!al_init_image_addon()) {
		fprintf(stderr, "failed to initialize image addon!\n");
		/*al_show_native_message_box(display, "Error", "Error", "Failed to initialize al_init_image_addon!",
																														 NULL, ALLEGRO_MESSAGEBOX_ERROR);*/
		return NULL;
	}

	if(!al_init_acodec_addon()){
		fprintf(stderr, "failed to initialize audio codecs!\n");
		return NULL;
	}

	if(!al_install_audio()){
		fprintf(stderr, "failed to initialize audio!\n");
		return NULL;
	}

	if(!al_install_keyboard()){
		fprintf(stderr, "failed to initialize keyboard!\n");
		return NULL;
	}

	if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
		return NULL;
	}

	if(!al_install_mouse()) {
		fprintf(stderr, "failed to initialize the mouse!\n");
		return NULL;
	}

	al_init_font_addon();

	if(!al_init_ttf_addon()){
		fprintf(stderr, "failed to initialize fonts!\n");
		return NULL;
	}

	game->touch = false;

	if (!atoi(GetConfigOptionDefault(game, "SuperDerpy", "disableTouch", "0"))) {
		game->touch = al_install_touch_input();
	}

#ifdef LIBSUPERDERPY_MOUSE_EMULATION
	if (game->touch) {
		al_set_mouse_emulation_mode(ALLEGRO_MOUSE_EMULATION_TRANSPARENT);
	}
#endif

	al_install_joystick();

	al_set_new_display_flags(ALLEGRO_PROGRAMMABLE_PIPELINE | (game->config.fullscreen ? ALLEGRO_FULLSCREEN_WINDOW : ALLEGRO_WINDOWED) | ALLEGRO_RESIZABLE | ALLEGRO_OPENGL ); // TODO: make ALLEGRO_PROGRAMMABLE_PIPELINE game-optional
	al_set_new_display_option(ALLEGRO_VSYNC, 2-atoi(GetConfigOptionDefault(game, "SuperDerpy", "vsync", "1")), ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_OPENGL, true, ALLEGRO_REQUIRE);

#ifdef LIBSUPERDERPY_ORIENTATION_LANDSCAPE
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);
#elif defined(LIBSUPERDERPY_ORIENTATION_PORTRAIT)
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_PORTRAIT, ALLEGRO_SUGGEST);
#endif

#ifdef ALLEGRO_WINDOWS
	al_set_new_window_position(20, 40); // workaround nasty Windows bug with window being created off-screen
#endif

	al_set_new_window_title(game->name);
	game->display = al_create_display(game->config.width, game->config.height);
	if(!game->display) {
		fprintf(stderr, "failed to create display!\n");
		return NULL;
	}

#ifdef ALLEGRO_ANDROID
	al_android_set_apk_file_interface();
	al_android_set_apk_fs_interface();
#endif

	SetupViewport(game, viewport);

	PrintConsole(game, "Viewport %dx%d", game->viewport.width, game->viewport.height);

	ALLEGRO_BITMAP *icon = al_load_bitmap(GetDataFilePath(game, GetGameName(game, "icons/%s.png")));
	al_set_display_icon(game->display, icon);
	al_destroy_bitmap(icon);

	if (game->config.fullscreen) al_hide_mouse_cursor(game->display);
	al_inhibit_screensaver(true);

	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR);

	game->_priv.gamestates = NULL;
	game->_priv.gamestate_scheduled = false;

	al_init_user_event_source(&(game->event_source));

	game->_priv.event_queue = al_create_event_queue();
	if(!game->_priv.event_queue) {
		FatalError(game, true, "Failed to create event queue.");
		al_destroy_display(game->display);
		return NULL;
	}

	game->audio.v = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
	game->audio.mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.fx = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.music = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.voice = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	al_attach_mixer_to_voice(game->audio.mixer, game->audio.v);
	al_attach_mixer_to_mixer(game->audio.fx, game->audio.mixer);
	al_attach_mixer_to_mixer(game->audio.music, game->audio.mixer);
	al_attach_mixer_to_mixer(game->audio.voice, game->audio.mixer);
	al_set_mixer_gain(game->audio.fx, game->config.fx/10.0);
	al_set_mixer_gain(game->audio.music, game->config.music/10.0);
	al_set_mixer_gain(game->audio.voice, game->config.voice/10.0);

	setlocale(LC_NUMERIC, "C");

	game->_priv.argc = argc;
	game->_priv.argv = argv;

	game->data = NULL;

	game->shuttingdown = false;
	game->restart = false;

	game->show_loading_on_launch = false;

	return game;
}

SYMBOL_EXPORT int libsuperderpy_run(struct Game *game) {

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

	al_clear_to_color(al_map_rgb(0,0,0));
	game->_priv.timer = al_create_timer(ALLEGRO_BPS_TO_SECS(60)); // logic timer
	if(!game->_priv.timer) {
		FatalError(game, true, "Failed to create logic timer.");
		return 1;
	}
	al_register_event_source(game->_priv.event_queue, al_get_timer_event_source(game->_priv.timer));

	al_flip_display();
	al_start_timer(game->_priv.timer);

	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		// don't show loading screen on init if requested
		tmp->showLoading = game->show_loading_on_launch;
		tmp = tmp->next;
	}

	char libname[1024] = {};
	snprintf(libname, 1024, "libsuperderpy-%s-loading" LIBRARY_EXTENSION, game->name);
	void *handle = dlopen(libname, RTLD_NOW);
	if (!handle) {
		FatalError(game, true, "Error while initializing loading screen: %s", dlerror());
		return 2;
	} else {

#define GS_LOADINGERROR FatalError(game, true, "Error on resolving loading symbol: %s", dlerror()); return 3;

		if (!(game->_priv.loading.Draw = dlsym(handle, "Draw"))) { GS_LOADINGERROR; }
		if (!(game->_priv.loading.Load = dlsym(handle, "Load"))) { GS_LOADINGERROR; }
		if (!(game->_priv.loading.Start = dlsym(handle, "Start"))) { GS_LOADINGERROR; }
		if (!(game->_priv.loading.Stop = dlsym(handle, "Stop"))) { GS_LOADINGERROR; }
		if (!(game->_priv.loading.Unload = dlsym(handle, "Unload"))) { GS_LOADINGERROR; }
	}

	game->_priv.loading.data = (*game->_priv.loading.Load)(game);

	bool redraw = false;
	game->_priv.draw = true;

	while(1) {
		ClearGarbage(game);

		// TODO: split mainloop to functions to make it readable
		ALLEGRO_EVENT ev;
		if (game->_priv.draw && ((redraw && al_is_event_queue_empty(game->_priv.event_queue)) || (game->_priv.gamestate_scheduled))) {

			game->_priv.gamestate_scheduled = false;
			struct Gamestate *tmp = game->_priv.gamestates;

			game->_priv.tmp_gamestate.toLoad = 0;
			game->_priv.tmp_gamestate.loaded = 0;

			// TODO: support gamestate dependences
			while (tmp) {
				if (tmp->pending_stop) {
					PrintConsole(game, "Stopping gamestate \"%s\"...", tmp->name);
					game->_priv.current_gamestate = tmp;
					(*tmp->api->Gamestate_Stop)(game, tmp->data);
					tmp->started = false;
					tmp->pending_stop = false;
				}

				if (tmp->pending_load) game->_priv.tmp_gamestate.toLoad++;
				tmp=tmp->next;
			}

			tmp = game->_priv.gamestates;

			game->_priv.tmp_gamestate.t = -1;

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

					if (!tmp->api) {
						PrintConsole(game, "Opening gamestate \"%s\"...", tmp->name);
						char libname[1024];
						snprintf(libname, 1024, "libsuperderpy-%s-%s" LIBRARY_EXTENSION, game->name, tmp->name);
						tmp->handle = dlopen(libname,RTLD_NOW);
						if (!tmp->handle) {
							FatalError(game, false, "Error while opening gamestate \"%s\": %s", tmp->name, dlerror());

							tmp->pending_load = false;
							tmp->pending_start = false;
						} else {

							tmp->api = malloc(sizeof(struct Gamestate_API));

#define GS_ERROR FatalError(game, false, "Error on resolving gamestate symbol: %s", dlerror()); tmp->pending_load = false; tmp->pending_start = false; tmp=tmp->next; continue;

							if (!(tmp->api->Gamestate_Draw = dlsym(tmp->handle, "Gamestate_Draw"))) { GS_ERROR; }
							if (!(tmp->api->Gamestate_Logic = dlsym(tmp->handle, "Gamestate_Logic"))) { GS_ERROR; }

							if (!(tmp->api->Gamestate_Load = dlsym(tmp->handle, "Gamestate_Load"))) { GS_ERROR; }
							if (!(tmp->api->Gamestate_Start = dlsym(tmp->handle, "Gamestate_Start"))) { GS_ERROR; }
							if (!(tmp->api->Gamestate_Pause = dlsym(tmp->handle, "Gamestate_Pause"))) { GS_ERROR; }
							if (!(tmp->api->Gamestate_Resume = dlsym(tmp->handle, "Gamestate_Resume"))) { GS_ERROR; }
							if (!(tmp->api->Gamestate_Stop = dlsym(tmp->handle, "Gamestate_Stop"))) { GS_ERROR; }
							if (!(tmp->api->Gamestate_Unload = dlsym(tmp->handle, "Gamestate_Unload"))) { GS_ERROR; }

							if (!(tmp->api->Gamestate_ProcessEvent = dlsym(tmp->handle, "Gamestate_ProcessEvent"))) { GS_ERROR; }
							if (!(tmp->api->Gamestate_Reload = dlsym(tmp->handle, "Gamestate_Reload"))) { GS_ERROR; }

							if (!(tmp->api->Gamestate_ProgressCount = dlsym(tmp->handle, "Gamestate_ProgressCount"))) { GS_ERROR; }
						}
					}
					if (tmp->api) {
						PrintConsole(game, "Loading gamestate \"%s\"...", tmp->name);
						game->_priv.tmp_gamestate.p = 0;

						DrawGamestates(game);
						if (tmp->showLoading) {
							(*game->_priv.loading.Draw)(game, game->_priv.loading.data, game->_priv.tmp_gamestate.loaded/(float)game->_priv.tmp_gamestate.toLoad);
						}
						DrawConsole(game);
						if (al_get_time() - game->_priv.tmp_gamestate.t >= 1/60.0) {
							al_flip_display();
							game->_priv.tmp_gamestate.t = al_get_time();
						}
						game->_priv.tmp_gamestate.tmp = tmp;
						game->_priv.current_gamestate = tmp;
						tmp->data = (*tmp->api->Gamestate_Load)(game, &GamestateProgress);
						game->_priv.tmp_gamestate.loaded++;

						tmp->loaded = true;
						tmp->pending_load = false;
					}
					al_resume_timer(game->_priv.timer);
				}

				tmp=tmp->next;
			}

			bool gameActive = false;
			tmp=game->_priv.gamestates;

			while (tmp) {
				if ((tmp->pending_start)  && (tmp->loaded)) {
					PrintConsole(game, "Starting gamestate \"%s\"...", tmp->name);
					al_stop_timer(game->_priv.timer);
					game->_priv.current_gamestate = tmp;
					tmp->started = true;
					tmp->pending_start = false;
					(*tmp->api->Gamestate_Start)(game, tmp->data);
					al_resume_timer(game->_priv.timer);
				}

				if ((tmp->started) || (tmp->pending_start) || (tmp->pending_load)) gameActive = true;
				tmp=tmp->next;
			}

			if (!gameActive) {
				PrintConsole(game, "No gamestates left, exiting...");
				break;
			}

			DrawGamestates(game);
			DrawConsole(game);
			al_flip_display();
			redraw = false;

		} else {

			al_wait_for_event(game->_priv.event_queue, &ev);

			if (game->eventHandler) {
				if ((*game->eventHandler)(game, &ev)) {
					continue;
				}
			}

			if ((ev.type == ALLEGRO_EVENT_TIMER) && (ev.timer.source == game->_priv.timer)) {
				LogicGamestates(game);
				redraw = true;
			}
			else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				break;
			}
			else if(ev.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING) {
				PrintConsole(game, "Engine halted.");
				game->_priv.draw = false;
				al_stop_timer(game->_priv.timer);
				al_detach_voice(game->audio.v);
				FreezeGamestates(game);
				if (game->_priv.console) Console_Unload(game);
				al_acknowledge_drawing_halt(game->display);
			}
			else if(ev.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) {
				game->_priv.draw = true;
				al_acknowledge_drawing_resume(game->display);
				Console_Load(game);
				PrintConsole(game, "Engine resumed.");
				ReloadGamestates(game);
				UnfreezeGamestates(game);
				al_attach_mixer_to_voice(game->audio.mixer, game->audio.v);
				al_resume_timer(game->_priv.timer);
			}
			else if(ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(game->display);
				SetupViewport(game, game->viewport_config);
			}
#ifdef ALLEGRO_ANDROID
			else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (ev.keyboard.keycode == ALLEGRO_KEY_MENU)) {
#else
			else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && ((ev.keyboard.keycode == ALLEGRO_KEY_TILDE) || (ev.keyboard.keycode == ALLEGRO_KEY_BACKQUOTE))) {
#endif
				game->_priv.showconsole = !game->_priv.showconsole;
			}
			else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (game->config.debug) && (ev.keyboard.keycode == ALLEGRO_KEY_F1)) {
				int i;
				for (i=0; i<512; i++) {
					LogicGamestates(game);
				}
				game->_priv.showconsole = true;
				PrintConsole(game, "DEBUG: 512 frames skipped...");
			}	else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (game->config.debug) && (ev.keyboard.keycode == ALLEGRO_KEY_F10)) {
				double speed = ALLEGRO_BPS_TO_SECS(al_get_timer_speed(game->_priv.timer)); // inverting
				speed -= 10;
				if (speed<10) speed = 10;
				al_set_timer_speed(game->_priv.timer, ALLEGRO_BPS_TO_SECS(speed));
				game->_priv.showconsole = true;
				PrintConsole(game, "DEBUG: Gameplay speed: %.2fx", speed/60.0);
			}	else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (game->config.debug) && (ev.keyboard.keycode == ALLEGRO_KEY_F11)) {
				double speed = ALLEGRO_BPS_TO_SECS(al_get_timer_speed(game->_priv.timer)); // inverting
				speed += 10;
				if (speed>600) speed = 600;
				al_set_timer_speed(game->_priv.timer, ALLEGRO_BPS_TO_SECS(speed));
				game->_priv.showconsole = true;
				PrintConsole(game, "DEBUG: Gameplay speed: %.2fx", speed/60.0);
			} else if ((ev.type == ALLEGRO_EVENT_KEY_DOWN) && (ev.keyboard.keycode == ALLEGRO_KEY_F12)) {
				ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_USER_DOCUMENTS_PATH);
				char filename[255] = { };
				snprintf(filename, 255, "%s_%lld_%ld.png", game->name, (long long)time(NULL), clock());
				al_set_path_filename(path, filename);
				al_save_bitmap(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP), al_get_backbuffer(game->display));
				PrintConsole(game, "Screenshot stored in %s", al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
				al_destroy_path(path);
			}
			EventGamestates(game, &ev);
		}
	}

	return 0;
}

SYMBOL_EXPORT void libsuperderpy_destroy(struct Game *game) {
	game->shuttingdown = true;

	ClearGarbage(game);

	// in case of restart
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
		if (tmp->handle && !RUNNING_ON_VALGRIND) {
			PrintConsole(game, "Closing gamestate \"%s\"...", tmp->name);
			dlclose(tmp->handle);
		}
		free(tmp->name);
		if (tmp->api) {
			free(tmp->api);
		}
		pom = tmp->next;
		free(tmp);
		tmp=pom;
	}

	ClearScreen(game);
	PrintConsole(game, "Shutting down...");
	DrawConsole(game);
	al_flip_display();
	while (game->_priv.garbage) {
		free(game->_priv.garbage->data);
		game->_priv.garbage = game->_priv.garbage->next;
	}
	(*game->_priv.loading.Unload)(game, game->_priv.loading.data);
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
	al_uninstall_system();
	al_shutdown_ttf_addon();
	al_shutdown_font_addon();
	char** argv = game->_priv.argv;
	bool restart = game->restart;
	free(game);
	if (restart) {
#ifdef ALLEGRO_MACOSX
		chdir(game->_priv.cwd);
#endif
		execv(argv[0], argv); // FIXME: on OSX there's chdir called which might break it
	}
}

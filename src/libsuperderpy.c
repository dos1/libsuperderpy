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

#include "internal.h"
#include <dlfcn.h>
#include <libgen.h>
#include <locale.h>
#include <unistd.h>
#ifdef ALLEGRO_MACOSX
#include <mach-o/dyld.h>
#endif

static char* GetDefaultWindowWidth(struct Game* game) {
	char* buf = malloc(sizeof(char) * 255);
	double aspect = game->viewport_config.aspect ? game->viewport_config.aspect : (game->viewport_config.width / (double)game->viewport_config.height);
	if (aspect < 1.0) {
		aspect = 1.0;
	}
	snprintf(buf, 255, "%d", (int)(720 * aspect));
	return AddGarbage(game, buf);
}

static char* GetDefaultWindowHeight(struct Game* game) {
	char* buf = malloc(sizeof(char) * 255);
	double aspect = game->viewport_config.aspect ? game->viewport_config.aspect : (game->viewport_config.width / (double)game->viewport_config.height);
	if (aspect > 1.0) {
		aspect = 1.0;
	}
	snprintf(buf, 255, "%d", (int)(720 / aspect));
	return AddGarbage(game, buf);
}

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
	game->_priv.shaders = NULL;

	game->_priv.paused = false;

	game->handlers.event = NULL;
	game->handlers.destroy = NULL;
	game->handlers.compositor = NULL;
	game->handlers.prelogic = NULL;
	game->handlers.postlogic = NULL;
	game->handlers.predraw = NULL;
	game->handlers.postdraw = NULL;

	game->_priv.texture_sync = false;
	game->_priv.texture_sync_cond = al_create_cond();
	game->_priv.texture_sync_mutex = al_create_mutex();

	game->config.fullscreen = strtol(GetConfigOptionDefault(game, "SuperDerpy", "fullscreen", "1"), NULL, 10);
	game->config.music = strtol(GetConfigOptionDefault(game, "SuperDerpy", "music", "10"), NULL, 10);
	game->config.voice = strtol(GetConfigOptionDefault(game, "SuperDerpy", "voice", "10"), NULL, 10);
	game->config.fx = strtol(GetConfigOptionDefault(game, "SuperDerpy", "fx", "10"), NULL, 10);
	game->config.mute = strtol(GetConfigOptionDefault(game, "SuperDerpy", "mute", "0"), NULL, 10);
	game->config.debug = strtol(GetConfigOptionDefault(game, "SuperDerpy", "debug", "0"), NULL, 10);
	game->config.width = strtol(GetConfigOptionDefault(game, "SuperDerpy", "width", GetDefaultWindowWidth(game)), NULL, 10);
	if (game->config.width < 100) { game->config.width = 100; }
	game->config.height = strtol(GetConfigOptionDefault(game, "SuperDerpy", "height", GetDefaultWindowHeight(game)), NULL, 10);
	if (game->config.height < 100) { game->config.height = 100; }

	game->_priv.debug.verbose = strtol(GetConfigOptionDefault(game, "debug", "verbose", "0"), NULL, 10);
	game->_priv.debug.livereload = strtol(GetConfigOptionDefault(game, "debug", "livereload", "1"), NULL, 10);
	game->_priv.debug.autopause = strtol(GetConfigOptionDefault(game, "debug", "autopause", "1"), NULL, 10);

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

	game->mouse = al_install_mouse();
	if (!game->mouse) {
		fprintf(stderr, "failed to initialize the mouse!\n");
	}

	if (!al_init_video_addon()) {
		fprintf(stderr, "failed to initialize the video addon!\n");
		return NULL;
	}

	if (!al_init_font_addon() || !al_init_ttf_addon()) {
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

	game->joystick = false;

	if (!strtol(GetConfigOptionDefault(game, "SuperDerpy", "disableJoystick", "0"), NULL, 10)) {
		game->joystick = al_install_joystick();
	}

	int windowMode = ALLEGRO_FULLSCREEN_WINDOW;
#ifdef ALLEGRO_ANDROID
	windowMode |= ALLEGRO_FRAMELESS;
#endif
#ifdef __EMSCRIPTEN__
	windowMode = ALLEGRO_WINDOWED;
#endif
	if (!game->config.fullscreen) {
		windowMode = ALLEGRO_WINDOWED;
	}
	windowMode |= ALLEGRO_RESIZABLE;

	al_set_new_display_flags(windowMode | ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);
	al_set_new_display_option(ALLEGRO_VSYNC, 2 - strtol(GetConfigOptionDefault(game, "SuperDerpy", "vsync", "1"), NULL, 10), ALLEGRO_SUGGEST);

#ifdef LIBSUPERDERPY_ORIENTATION_LANDSCAPE
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);
#elif defined(LIBSUPERDERPY_ORIENTATION_PORTRAIT)
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_PORTRAIT, ALLEGRO_SUGGEST);
#endif

	if (viewport.depth_buffer) {
		al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 24, ALLEGRO_SUGGEST);
	}

#ifdef ALLEGRO_WINDOWS
	al_set_new_window_position(20, 40); // workaround nasty Windows bug with window being created off-screen
#endif

	al_set_new_window_title(al_get_app_name());
	game->display = al_create_display(game->config.width, game->config.height);
	if (!game->display) {
		fprintf(stderr, "Failed to create display!\n");
		return NULL;
	}

#ifdef ALLEGRO_ANDROID
	al_android_set_apk_file_interface();
	al_android_set_apk_fs_interface();
#endif

#if !defined(ALLEGRO_ANDROID) && !defined(ALLEGRO_IPHONE) && !defined(ALLEGRO_SDL)
	// We're always using OpenGL which already preserves textures on its own, so avoid
	// excessive RAM usage by not backuping the bitmaps when not necessary.
	// Android and iOS can threw out the context, so bitmaps need to be preserved there.
	// SDL2 backend seems to have some workaround for old SDL bug, worth looking into.
	// This will have to be revisited on mobile GNU/Linux platforms.
	al_add_new_bitmap_flag(ALLEGRO_NO_PRESERVE_TEXTURE);
#endif

	PrintConsole(game, "libsuperderpy 2 (rev " LIBSUPERDERPY_GIT_REV ")");
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

	ALLEGRO_BITMAP* icon = al_load_bitmap(GetDataFilePath(game, GetGameName(game, "icons/%s.png")));
	al_set_display_icon(game->display, icon);
	al_destroy_bitmap(icon);

	if (game->config.fullscreen) { al_hide_mouse_cursor(game->display); }
	al_inhibit_screensaver(true);

	SetupViewport(game, viewport);

	al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

	game->_priv.gamestates = NULL;

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
	al_set_mixer_gain(game->audio.mixer, game->config.mute ? 0.0 : 1.0);
	al_set_default_mixer(game->audio.mixer);

	setlocale(LC_NUMERIC, "C");

	game->_priv.argc = argc;
	game->_priv.argv = argv;

	game->data = NULL;

	game->shutting_down = false;
	game->restart = false;

	game->show_loading_on_launch = false;

	game->loading_progress = 0;

	if (game->handlers.compositor) {
		game->loading_fb = CreateNotPreservedBitmap(game->_priv.clip_rect.w, game->_priv.clip_rect.h);
	} else {
		game->loading_fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->_priv.clip_rect.x, game->_priv.clip_rect.y, game->_priv.clip_rect.w, game->_priv.clip_rect.h);
	}

	return game;
}

SYMBOL_EXPORT int libsuperderpy_start(struct Game* game) {
	al_register_event_source(game->_priv.event_queue, al_get_display_event_source(game->display));
	al_register_event_source(game->_priv.event_queue, al_get_keyboard_event_source());
	if (game->mouse) {
		al_register_event_source(game->_priv.event_queue, al_get_mouse_event_source());
	}
	if (game->joystick) {
		al_register_event_source(game->_priv.event_queue, al_get_joystick_event_source());
	}
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

	ReloadShaders(game, false);
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

	ReloadShaders(game, false);

	game->_priv.timestamp = al_get_time();
	game->_priv.paused = false;

#ifdef LIBSUPERDERPY_IMGUI
	igCreateContext(NULL);
	ImGuiIO* io = igGetIO();
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

	// Setup Platform/Renderer bindings
	ImGui_ImplAllegro5_Init(game->display);

	// Setup Style
	igStyleColorsDark(NULL);
#endif

	return 0;
}

#ifdef __EMSCRIPTEN__
SYMBOL_INTERNAL void libsuperderpy_emscripten_mainloop(void* game) {
	if (!libsuperderpy_mainloop(game)) {
		libsuperderpy_destroy(game);
		free(game);
		printf("Halted.\n");
		emscripten_cancel_main_loop();
	}
}
#endif

SYMBOL_EXPORT int libsuperderpy_run(struct Game* game) {
	int ret = libsuperderpy_start(game);
	if (ret) {
		return ret;
	}
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(libsuperderpy_emscripten_mainloop, game, 0, true);
	return 0;
#else
	while (libsuperderpy_mainloop(game)) {};
	libsuperderpy_destroy(game);
	return 0;
#endif
}

SYMBOL_EXPORT void libsuperderpy_destroy(struct Game* game) {
	game->shutting_down = true;

#ifdef LIBSUPERDERPY_IMGUI
	ImGui_ImplAllegro5_Shutdown();
	igDestroyContext(NULL);
#endif

	ClearGarbage(game);

	struct Gamestate *tmp = game->_priv.gamestates, *pom;
	while (tmp) {
		if (tmp->started) {
			PrintConsole(game, "Stopping gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Stop)(game, tmp->data);
			tmp->started = false;
			PrintConsole(game, "Gamestate \"%s\" stopped successfully.", tmp->name);
		}
		if (tmp->loaded) {
			PrintConsole(game, "Unloading gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Unload)(game, tmp->data);
			tmp->loaded = false;
			PrintConsole(game, "Gamestate \"%s\" unloaded successfully.", tmp->name);
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
	DestroyShaders(game);

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
	al_destroy_voice(game->audio.v); // FIXME: doesn't seem to work in Chromium under Emscripten
	al_destroy_cond(game->_priv.texture_sync_cond);
	al_destroy_mutex(game->_priv.texture_sync_mutex);
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

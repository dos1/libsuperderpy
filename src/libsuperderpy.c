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
#include <getopt.h>
#include <libgen.h>
#include <locale.h>
#include <unistd.h>
#ifdef ALLEGRO_MACOSX
#include <mach-o/dyld.h>
#endif

static char* GetDefaultWindowWidth(struct Game* game) {
	char* buf = malloc(sizeof(char) * 255);
	double aspect = game->_priv.params.aspect ? game->_priv.params.aspect : (game->_priv.params.height ? (game->_priv.params.width / (double)game->_priv.params.height) : 1.0);
	if (aspect < 1.0) {
		aspect = 1.0;
	}
	snprintf(buf, 255, "%d", (int)(720 * aspect));
	return AddGarbage(game, buf);
}

static char* GetDefaultWindowHeight(struct Game* game) {
	char* buf = malloc(sizeof(char) * 255);
	double aspect = game->_priv.params.aspect ? game->_priv.params.aspect : (game->_priv.params.height ? (game->_priv.params.width / (double)game->_priv.params.height) : 1.0);
	if (aspect > 1.0) {
		aspect = 1.0;
	}
	snprintf(buf, 255, "%d", (int)(720 / aspect));
	return AddGarbage(game, buf);
}

SYMBOL_EXPORT struct Game* libsuperderpy_init(int argc, char** argv, const char* name, struct Params params) {
	struct Game* game = calloc(1, sizeof(struct Game));

	game->_priv.argc = argc;
	game->_priv.argv = argv;

	game->_priv.name = strdup(name);
	game->_priv.params = params;

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

	al_set_config_value(al_get_system_config(), "ttf", "cache_text", "0123456789QWERTYUIOPASDFGHJKLZXCVBNMĘÓĄŚŁŻŹĆŃqwertyuiopasdfghjklzxcvbnmęóąśłżźćń.@#$%^&*():\"!?\\[];'{}|,<>/~`-=_+ ");

	game->_priv.fps_count.frames_done = 0;
	game->_priv.fps_count.fps = 0;
	game->_priv.fps_count.old_time = 0;

	game->_priv.font_console = NULL;
	game->_priv.font_bsod = NULL;
	game->_priv.console_pos = 0;
	for (size_t i = 0; i < (sizeof(game->_priv.console) / sizeof(game->_priv.console[0])); i++) {
		game->_priv.console[i][0] = '\0';
	}

	game->_priv.garbage = NULL;
	game->_priv.timelines = NULL;
	game->_priv.shaders = NULL;
	game->_priv.gamestates = NULL;

	game->_priv.paused = false;
	game->_priv.started = false;

	game->_priv.texture_sync = false;
	game->_priv.texture_sync_cond = al_create_cond();
	game->_priv.texture_sync_mutex = al_create_mutex();

	game->_priv.in_bsod = false;
	game->_priv.bsod_cond = al_create_cond();
	game->_priv.bsod_mutex = al_create_mutex();

	game->_priv.speed = 1.0;

	game->_priv.transforms = NULL;
	game->_priv.transforms_no = 0;

	game->_priv.window_width = 0;
	game->_priv.window_height = 0;

	game->_priv.mutex = al_create_mutex();

	game->config.fullscreen = strtol(GetConfigOptionDefault(game, "SuperDerpy", "fullscreen", "1"), NULL, 10);
	game->config.music = strtol(GetConfigOptionDefault(game, "SuperDerpy", "music", "10"), NULL, 10);
	game->config.voice = strtol(GetConfigOptionDefault(game, "SuperDerpy", "voice", "10"), NULL, 10);
	game->config.fx = strtol(GetConfigOptionDefault(game, "SuperDerpy", "fx", "10"), NULL, 10);
	game->config.mute = strtol(GetConfigOptionDefault(game, "SuperDerpy", "mute", "0"), NULL, 10);
	game->config.samplerate = strtol(GetConfigOptionDefault(game, "SuperDerpy", "samplerate", "0"), NULL, 10);
	game->config.width = strtol(GetConfigOptionDefault(game, "SuperDerpy", "width", GetDefaultWindowWidth(game)), NULL, 10);
	if (game->config.width < 100) { game->config.width = 100; }
	game->config.height = strtol(GetConfigOptionDefault(game, "SuperDerpy", "height", GetDefaultWindowHeight(game)), NULL, 10);
	if (game->config.height < 100) { game->config.height = 100; }
	game->config.autopause = strtol(GetConfigOptionDefault(game, "SuperDerpy", "autopause", "1"), NULL, 10);

	game->config.debug.enabled = strtol(GetConfigOptionDefault(game, "SuperDerpy", "debug", "0"), NULL, 10);
	game->config.debug.verbose = strtol(GetConfigOptionDefault(game, "debug", "verbose", "0"), NULL, 10);
	game->config.debug.livereload = strtol(GetConfigOptionDefault(game, "debug", "livereload", "0"), NULL, 10);

	if (params.no_autopause) {
		game->config.autopause = false;
	}

#ifdef __EMSCRIPTEN__
	game->config.fullscreen = false; // we can't start fullscreen on emscripten
#endif

	static struct option long_options[] =
		{
			{"debug", no_argument, NULL, 'd'},
			{"fullscreen", no_argument, NULL, 'f'},
			{"windowed", no_argument, NULL, 'w'},
			{NULL, 0, NULL, 0},
		};

	optind = 1;
	int opt = 0;
	while ((opt = getopt_long(argc, argv, "dfw", long_options, NULL)) != -1) {
		switch (opt) {
			case 'd':
				game->config.debug.enabled = true;
				break;
			case 'f':
				if (!game->config.fullscreen) {
					game->config.fullscreen = true;
				}
				break;
			case 'w':
				if (game->config.fullscreen) {
					game->config.fullscreen = false;
				}
				break;
		}
	}
	optind = 1;

	game->show_console = game->config.debug.enabled;
	game->_priv.show_timeline = false;

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

	game->input.available.mouse = al_install_mouse();
	if (!game->input.available.mouse) {
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

	game->input.available.touch = false;

	if (!strtol(GetConfigOptionDefault(game, "SuperDerpy", "disableTouch", "0"), NULL, 10)) {
		game->input.available.touch = al_install_touch_input();
	}

#ifdef LIBSUPERDERPY_MOUSE_EMULATION
	if (game->input.available.touch) {
		al_set_mouse_emulation_mode(ALLEGRO_MOUSE_EMULATION_TRANSPARENT);
	}
#endif

	game->input.available.joystick = false;

	if (!strtol(GetConfigOptionDefault(game, "SuperDerpy", "disableJoystick", "0"), NULL, 10)) {
		game->input.available.joystick = al_install_joystick();
	}

#ifdef ALLEGRO_ANDROID
	int windowMode = ALLEGRO_FULLSCREEN_WINDOW | ALLEGRO_FRAMELESS;
#elif defined(__EMSCRIPTEN__)
	int windowMode = ALLEGRO_WINDOWED;
#else
	int windowMode = ALLEGRO_FULLSCREEN_WINDOW;
#endif

	if (!game->config.fullscreen) {
		windowMode = ALLEGRO_WINDOWED;
	}
	if (!params.fixed_size) {
		windowMode |= ALLEGRO_RESIZABLE;
	}

	al_set_new_display_flags(windowMode | ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE | ALLEGRO_GENERATE_EXPOSE_EVENTS);
	al_set_new_display_option(ALLEGRO_VSYNC, 2 - strtol(GetConfigOptionDefault(game, "SuperDerpy", "vsync", "1"), NULL, 10), ALLEGRO_SUGGEST);

#ifdef LIBSUPERDERPY_ORIENTATION_LANDSCAPE
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);
#elif defined(LIBSUPERDERPY_ORIENTATION_PORTRAIT)
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_PORTRAIT, ALLEGRO_SUGGEST);
#endif

	if (params.depth_buffer) {
		al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 24, ALLEGRO_SUGGEST);
	}

	al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, params.samples ? 1 : 0, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_SAMPLES, params.samples, ALLEGRO_SUGGEST);

#ifdef ALLEGRO_WINDOWS
	al_set_new_window_position(20, 40); // workaround nasty Windows bug with window being created off-screen
#endif

	al_set_new_window_title(game->_priv.params.window_title ? game->_priv.params.window_title : al_get_app_name());

#ifdef __EMSCRIPTEN__
	game->config.width *= emscripten_get_device_pixel_ratio();
	game->config.height *= emscripten_get_device_pixel_ratio();
#endif

	game->display = al_create_display(game->config.width, game->config.height);
	if (!game->display) {
		fprintf(stderr, "Failed to create display!\n");
		return NULL;
	}

#ifdef ALLEGRO_ANDROID
	al_android_set_apk_file_interface();
	al_android_set_apk_fs_interface();
#endif

#if !defined(ALLEGRO_ANDROID) && !defined(ALLEGRO_IPHONE)
	// We're always using OpenGL which already preserves textures on its own, so avoid
	// excessive RAM usage by not backuping the bitmaps when not necessary.
	// Android and iOS can threw out the context, so bitmaps need to be preserved there.
	al_add_new_bitmap_flag(ALLEGRO_NO_PRESERVE_TEXTURE);
#endif

	PrintConsole(game, "%s", al_get_app_name());
	PrintConsole(game, "libsuperderpy 2 (rev " LIBSUPERDERPY_GIT_REV ")");
	{
		uint32_t version = al_get_allegro_version();
		int major = version >> 24;
		int minor = (version >> 16) & 255;
		int revision = (version >> 8) & 255;
		int release = version & 255;
		if (release) {
			PrintConsole(game, "Allegro %d.%d.%d.%d", major, minor, revision, release - 1);
		} else {
			PrintConsole(game, "Allegro %d.%d.%d~", major, minor, revision);
		}
	}
	{
		uint32_t version = al_get_opengl_version();
		int major = version >> 24;
		int minor = (version >> 16) & 255;
		PrintConsole(game, "OpenGL%s %d.%d", al_get_opengl_variant() == ALLEGRO_OPENGL_ES ? " ES" : "", major, minor);
	}

	PrintConsole(game, "Max bitmap size: %d", al_get_display_option(game->display, ALLEGRO_MAX_BITMAP_SIZE));
	PrintConsole(game, "Color buffer bits: %d", al_get_display_option(game->display, ALLEGRO_COLOR_SIZE));
	PrintConsole(game, "Depth buffer bits: %d", al_get_display_option(game->display, ALLEGRO_DEPTH_SIZE));
	PrintConsole(game, "Stencil buffer bits: %d", al_get_display_option(game->display, ALLEGRO_STENCIL_SIZE));
	PrintConsole(game, "NPOT bitmaps: %d", al_get_display_option(game->display, ALLEGRO_SUPPORT_NPOT_BITMAP));
	PrintConsole(game, "Separate alpha blender: %d", al_get_display_option(game->display, ALLEGRO_SUPPORT_SEPARATE_ALPHA));

	PrintConsole(game, "Connected joysticks:%s", al_get_num_joysticks() ? "" : " [none]");
	for (int i = 0; i < al_get_num_joysticks(); i++) {
		ALLEGRO_JOYSTICK* joystick = al_get_joystick(i);
		PrintConsole(game, " - %d: %s", i, al_get_joystick_name(joystick));
		PrintConsole(game, "   - buttons: %d", al_get_joystick_num_buttons(joystick));
		for (int j = 0; j < al_get_joystick_num_sticks(joystick); j++) {
			PrintConsole(game, "   - stick %d: %d axes", j, al_get_joystick_num_axes(joystick, j));
		}
	}

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

	const char* iconpath = FindDataFilePath(game, GetGameName(game, "icons/%s.png"));
	if (iconpath) {
		ALLEGRO_BITMAP* icon = al_load_bitmap(iconpath);
		al_set_display_icon(game->display, icon);
		al_destroy_bitmap(icon);
	}

	if (game->config.fullscreen) {
		al_hide_mouse_cursor(game->display);
	}
	al_inhibit_screensaver(true);

	SetupViewport(game);
	ClearScreen(game);
	al_flip_display();

	al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

	al_init_user_event_source(&(game->event_source));

	game->_priv.event_queue = al_create_event_queue();
	if (!game->_priv.event_queue) {
		FatalError(game, true, "Failed to create event queue.");
		al_destroy_display(game->display);
		return NULL;
	}

	game->_priv.samplerate = game->config.samplerate;
	if (!game->_priv.samplerate) {
		game->_priv.samplerate = params.sample_rate;
		if (!game->_priv.samplerate) {
			game->_priv.samplerate = 44100;
		}
	}
	game->audio.mixer = al_create_mixer(game->_priv.samplerate, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.fx = al_create_mixer(game->_priv.samplerate, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.music = al_create_mixer(game->_priv.samplerate, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	game->audio.voice = al_create_mixer(game->_priv.samplerate, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	al_attach_mixer_to_mixer(game->audio.fx, game->audio.mixer);
	al_attach_mixer_to_mixer(game->audio.music, game->audio.mixer);
	al_attach_mixer_to_mixer(game->audio.voice, game->audio.mixer);
	al_set_mixer_gain(game->audio.fx, game->config.fx / 10.0);
	al_set_mixer_gain(game->audio.music, game->config.music / 10.0);
	al_set_mixer_gain(game->audio.voice, game->config.voice / 10.0);
	al_set_mixer_gain(game->audio.mixer, game->config.mute ? 0.0 : 1.0);
	SetupAudio(game);
	al_set_default_mixer(game->audio.mixer);

	setlocale(LC_NUMERIC, "C");

	game->data = NULL;

	game->_priv.shutting_down = false;
	game->_priv.restart = false;

	game->loading.progress = 0;

#ifdef LIBSUPERDERPY_STATIC_GAMESTATES
	__libsuperderpy_register_gamestate(NULL, NULL, game);
#endif

	PrintConsole(game, "Engine initialized.");

	return game;
}

static void ProgressStub(struct Game* game) {}

SYMBOL_EXPORT int libsuperderpy_start(struct Game* game) {
	al_register_event_source(game->_priv.event_queue, al_get_display_event_source(game->display));
	al_register_event_source(game->_priv.event_queue, al_get_keyboard_event_source());
	if (game->input.available.mouse) {
		al_register_event_source(game->_priv.event_queue, al_get_mouse_event_source());
	}
	if (game->input.available.joystick) {
		al_register_event_source(game->_priv.event_queue, al_get_joystick_event_source());
	}
	if (game->input.available.touch) {
		al_register_event_source(game->_priv.event_queue, al_get_touch_input_event_source());
#ifdef LIBSUPERDERPY_MOUSE_EMULATION
		al_register_event_source(game->_priv.event_queue, al_get_touch_input_mouse_emulation_event_source());
#endif
	}
	al_register_event_source(game->_priv.event_queue, &(game->event_source));

	// HACK: some backends correct the window size only after the display has been already created.
	// if we start loading with wrong size, it won't be corrected until the main loop is entered back,
	// so the loading screen will stay at the wrong size.
	// Turns out SDL/Wayland needs some help with emitting the resize event before it's too late.
	ALLEGRO_EVENT event;
	al_peek_next_event(game->_priv.event_queue, &event);
	ClearScreen(game);
	al_flip_display();

	{
		struct Gamestate* tmp = game->_priv.gamestates;
#ifdef LIBSUPERDERPY_STATIC_GAMESTATES
		if (tmp && strcmp(tmp->name, "loading") == 0) {
			game->_priv.gamestates = tmp->next;
			game->_priv.loading.gamestate = tmp;
			tmp = game->_priv.gamestates;
		}
#endif
		while (tmp) {
			// don't show loading screen on init if requested
			if (tmp->pending_load) {
				tmp->show_loading = game->_priv.params.show_loading_on_launch;
			}
#ifdef LIBSUPERDERPY_STATIC_GAMESTATES
			if (tmp->next && strcmp(tmp->next->name, "loading") == 0) {
				game->_priv.loading.gamestate = tmp->next;
				tmp->next = tmp->next->next;
			}
#endif
			tmp = tmp->next;
		}
	}

	if (!game->_priv.loading.gamestate) {
		game->_priv.loading.gamestate = AllocateGamestate(game, "loading");
	}
	if (OpenGamestate(game, game->_priv.loading.gamestate, false) && LinkGamestate(game, game->_priv.loading.gamestate)) {
		if (game->_priv.params.handlers.compositor) {
			game->_priv.loading.gamestate->fb = CreateNotPreservedBitmap(game->clip_rect.w, game->clip_rect.h);
		} else {
			game->_priv.loading.gamestate->fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->clip_rect.x, game->clip_rect.y, game->clip_rect.w, game->clip_rect.h);
		}

		game->_priv.loading.gamestate->data = (*game->_priv.loading.gamestate->api->load)(game, ProgressStub);
		game->_priv.loading.gamestate->loaded = true;
		PrintConsole(game, "Loading screen registered.");
	} else {
		PrintConsole(game, "No loading screen available.");
	}

	ReloadShaders(game, false);

	if (game->_priv.loading.gamestate->open && game->_priv.loading.gamestate->api->post_load) {
		(*game->_priv.loading.gamestate->api->post_load)(game, game->_priv.loading.gamestate->data);
	}

	game->_priv.timestamp = al_get_time();
	game->_priv.paused = false;
	game->_priv.started = true;

#ifdef LIBSUPERDERPY_IMGUI
	igCreateContext(NULL);
	ImGui_ImplAllegro5_Init(game->display);
	igStyleColorsDark(NULL);
	igGetStyle()->FrameBorderSize = 1.0;
	ImGuiIO* io = igGetIO();
	io->FontGlobalScale = strtod(GetConfigOptionDefault(game, "SuperDerpy", "scale", "1"), NULL);
#endif

	PrintConsole(game, "Engine started.");

	return 0;
}

SYMBOL_EXPORT int libsuperderpy_run(struct Game* game) {
	int ret = libsuperderpy_start(game);
	if (ret) {
		return ret;
	}
#ifdef __EMSCRIPTEN__
	if (game->config.autopause && !EM_ASM_INT({document.hasFocus()})) {
		PrintConsole(game, "Window not focused, autopausing...");
		PauseExecution(game);
	}
#endif
	while (libsuperderpy_mainloop(game)) {}
	libsuperderpy_destroy(game);
	return 0;
}

SYMBOL_EXPORT void libsuperderpy_destroy(struct Game* game) {
	game->_priv.shutting_down = true;

#ifdef LIBSUPERDERPY_IMGUI
	ImGui_ImplAllegro5_Shutdown();
	igDestroyContext(NULL);
#endif

	ClearGarbage(game);

	struct Gamestate *tmp = game->_priv.gamestates, *pom = NULL;
	while (tmp) {
		if (tmp->started) {
			PrintConsole(game, "Stopping gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			(*tmp->api->stop)(game, tmp->data);
			tmp->started = false;
			PrintConsole(game, "Gamestate \"%s\" stopped successfully.", tmp->name);
		}
		if (tmp->loaded) {
			PrintConsole(game, "Unloading gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			(*tmp->api->unload)(game, tmp->data);
			tmp->loaded = false;
			PrintConsole(game, "Gamestate \"%s\" unloaded successfully.", tmp->name);
		}
		CloseGamestate(game, tmp);
		pom = tmp->next;
		free(tmp->name);
		free(tmp);
		tmp = pom;
	}

	if (game->_priv.loading.gamestate->open && game->_priv.loading.gamestate->api) {
		(*game->_priv.loading.gamestate->api->unload)(game, game->_priv.loading.gamestate->data);
	}
	if (game->_priv.loading.gamestate->fb) {
		al_destroy_bitmap(game->_priv.loading.gamestate->fb);
		game->_priv.loading.gamestate->fb = NULL;
	}
	CloseGamestate(game, game->_priv.loading.gamestate);
	free(game->_priv.loading.gamestate->name);
	free(game->_priv.loading.gamestate);

	if (game->_priv.params.handlers.destroy) {
		(*game->_priv.params.handlers.destroy)(game);
	}
	DestroyShaders(game);

	PrintConsole(game, "Shutting down...");

	SetBackgroundColor(game, al_map_rgb(0, 0, 0));
	ClearScreen(game);
	DrawConsole(game);
	al_flip_display();

	while (game->_priv.garbage) {
		free(game->_priv.garbage->data);
		game->_priv.garbage = game->_priv.garbage->next;
	}
	free(game->_priv.transforms);
	Console_Unload(game);

#ifdef __EMSCRIPTEN__
	{
		ALLEGRO_BITMAP* bmp = al_create_bitmap(320, 180);
		al_set_target_bitmap(bmp);
		al_clear_to_color(al_map_rgb(0, 0, 0));
		ALLEGRO_FONT* font = al_create_builtin_font();
		al_draw_text(font, al_map_rgb(228, 127, 59), 320 / 2, 180 / 2 - 8 - 6, ALLEGRO_ALIGN_CENTER, "It's now safe to turn off");
		al_draw_text(font, al_map_rgb(228, 127, 59), 320 / 2, 180 / 2 - 8 + 6, ALLEGRO_ALIGN_CENTER, "your browser.");
		al_set_target_backbuffer(game->display);
		al_reset_clipping_rectangle();
		ALLEGRO_TRANSFORM t;
		al_identity_transform(&t);
		al_use_transform(&t);
		al_draw_scaled_bitmap(bmp, 0, 0, 320, 180, 0, -al_get_display_height(game->display) * 0.2, al_get_display_width(game->display), al_get_display_height(game->display) * 1.4, 0);
		al_flip_display();
		al_destroy_bitmap(bmp);
		al_destroy_font(font);
	}
	printf("Halted.\n");
	return;
#endif

	al_destroy_user_event_source(&(game->event_source));
	al_destroy_event_queue(game->_priv.event_queue);
	al_restore_default_mixer();
	al_destroy_mixer(game->audio.fx);
	al_destroy_mixer(game->audio.music);
	al_destroy_mixer(game->audio.voice);
	al_destroy_mixer(game->audio.mixer);
	al_set_default_voice(NULL); // destroys game->audio.v voice
	al_destroy_cond(game->_priv.texture_sync_cond);
	al_destroy_mutex(game->_priv.texture_sync_mutex);
	al_destroy_cond(game->_priv.bsod_cond);
	al_destroy_mutex(game->_priv.bsod_mutex);
	al_destroy_mutex(game->_priv.mutex);
	DeinitConfig(game);
	al_uninstall_audio();
	al_destroy_display(game->display);
	al_uninstall_system();

#ifndef LIBSUPERDERPY_NO_RESTART
	char** argv = game->_priv.argv;
	bool restart = game->_priv.restart;
	free(game->_priv.name);
	free(game);
	if (restart) {
#ifdef ALLEGRO_MACOSX
		chdir(game->_priv.cwd);
#endif
#ifdef ALLEGRO_WINDOWS
		wchar_t* wargv[game->_priv.argc];
		for (int i = 0; i < game->_priv.argc; i++) {
			size_t size = MultiByteToWideChar(CP_UTF8, 0, argv[i], -1, NULL, 0);
			wargv[i] = malloc(sizeof(wchar_t) * size);
			MultiByteToWideChar(CP_UTF8, 0, argv[i], -1, wargv[i], size);
		}
		_wexecv(wargv[0], (const wchar_t* const*)wargv);
#else
		execv(argv[0], argv);
#endif
	}
#endif
}

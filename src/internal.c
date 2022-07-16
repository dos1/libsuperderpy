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

#include "internal.h"
#include "3rdparty/valgrind.h"

#ifndef LIBSUPERDERPY_STATIC_GAMESTATES
#include <dlfcn.h>
#endif

#ifdef __vita__
int _newlib_heap_size_user = LIBSUPERDERPY_VITA_HEAP_SIZE * 1024 * 1024;

// PIB requires at least 2MB of SceLibc heap size
unsigned int sceLibcHeapSize = 2 * 1024 * 1024;
#endif

SYMBOL_INTERNAL void SimpleCompositor(struct Game* game) {
	struct Gamestate* tmp = GetNextGamestate(game, NULL);
	ClearToColor(game, game->_priv.bg);
	while (tmp) {
		if (IsGamestateVisible(game, tmp)) {
			al_draw_bitmap(GetGamestateFramebuffer(game, tmp), game->clip_rect.x, game->clip_rect.y, 0);
		}
		tmp = GetNextGamestate(game, tmp);
	}
	if (game->loading.shown) {
		al_draw_bitmap(GetGamestateFramebuffer(game, GetGamestate(game, NULL)), game->clip_rect.x, game->clip_rect.y, 0);
	}
}

SYMBOL_INTERNAL void DrawGamestates(struct Game* game) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->loaded && tmp->started && tmp->api->predraw) {
			game->_priv.current_gamestate = tmp;
			tmp->api->predraw(game, tmp->data);
		}
		tmp = tmp->next;
	}
	if (game->loading.shown && game->_priv.loading.gamestate->api->predraw) {
		game->_priv.loading.gamestate->api->predraw(game, game->_priv.loading.gamestate->data);
	}

	if (!game->_priv.params.disable_bg_clear && !game->_priv.params.handlers.compositor) {
		ClearScreen(game);
	}

	if (game->_priv.params.handlers.predraw) {
		game->_priv.params.handlers.predraw(game);
	}

	tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			game->_priv.current_gamestate = tmp;
			SetFramebufferAsTarget(game);
			if (!game->_priv.params.disable_bg_clear && game->_priv.params.handlers.compositor) { // don't clear when uncomposited
				al_reset_clipping_rectangle();
				al_clear_to_color(game->_priv.bg); // even if everything is going to be redrawn, it optimizes tiled rendering
			}
			tmp->api->draw(game, tmp->data);
			// TODO: save and restore more state for careless gamestating
		}
		tmp = tmp->next;
	}

	if (game->loading.shown) {
		// same as above, but for the loading gamestate
		game->_priv.current_gamestate = game->_priv.loading.gamestate;
		SetFramebufferAsTarget(game);
		if (!game->_priv.params.disable_bg_clear && game->_priv.params.handlers.compositor) {
			al_reset_clipping_rectangle();
			al_clear_to_color(game->_priv.bg);
		}
		game->_priv.loading.gamestate->api->draw(game, game->_priv.loading.gamestate->data);
	}

	al_set_target_backbuffer(game->display);

	ALLEGRO_TRANSFORM t;
	// restore full resolution access to the whole screen
	al_identity_transform(&t);
	al_use_transform(&t);
	al_reset_clipping_rectangle();

	if (game->_priv.params.handlers.compositor) {
		game->_priv.params.handlers.compositor(game);
	}

	if (game->_priv.params.handlers.postdraw) {
		game->_priv.params.handlers.postdraw(game);
	}
}

SYMBOL_INTERNAL void LogicGamestates(struct Game* game, double delta) {
	struct Gamestate* tmp = game->_priv.gamestates;
	if (delta > 1) {
		PrintConsole(game, "delta > 1 second!");
		delta = 1;
	}
	int ticks = (int)(floor((game->time + delta) / ALLEGRO_BPS_TO_SECS(60.0)) - floor(game->time / ALLEGRO_BPS_TO_SECS(60.0)));
	game->time += delta;
	if (game->_priv.params.handlers.prelogic) {
		game->_priv.params.handlers.prelogic(game, delta);
	}
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused) && (!tmp->pending_stop)) {
			game->_priv.current_gamestate = tmp;
			if (tmp->api->tick) {
				for (int i = 0; i < ticks; i++) {
					tmp->api->tick(game, tmp->data);
				}
			}
			tmp->api->logic(game, tmp->data, delta);
		}
		tmp = tmp->next;
	}
	if (game->_priv.params.handlers.postlogic) {
		game->_priv.params.handlers.postlogic(game, delta);
	}
}

SYMBOL_INTERNAL void ReloadGamestates(struct Game* game) {
	struct Gamestate* tmp = game->_priv.gamestates;
	ReloadShaders(game, true);
	while (tmp) {
		if (tmp->loaded) {
			game->_priv.current_gamestate = tmp;
			if (tmp->api->reload) {
				tmp->api->reload(game, tmp->data);
			}
		}
		tmp = tmp->next;
	}
	if (game->_priv.loading.gamestate->api->reload) {
		game->_priv.loading.gamestate->api->reload(game, game->_priv.loading.gamestate->data);
	}
}

SYMBOL_INTERNAL void EventGamestates(struct Game* game, ALLEGRO_EVENT* ev) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			game->_priv.current_gamestate = tmp;
			tmp->api->process_event(game, tmp->data, ev);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void FreezeGamestates(struct Game* game) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->started && !tmp->paused) {
			tmp->frozen = true;
			PauseGamestate(game, tmp->name);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void UnfreezeGamestates(struct Game* game) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->frozen) {
			ResumeGamestate(game, tmp->name);
			tmp->frozen = false;
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void ResizeGamestates(struct Game* game) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->started) {
			al_destroy_bitmap(tmp->fb);
			if (game->_priv.params.handlers.compositor) {
				tmp->fb = CreateNotPreservedBitmap(game->clip_rect.w, game->clip_rect.h);
			} else {
				tmp->fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->clip_rect.x, game->clip_rect.y, game->clip_rect.w, game->clip_rect.h);
			}
		}
		tmp = tmp->next;
	}
	if (game->_priv.loading.gamestate && game->_priv.loading.gamestate->open) {
		al_destroy_bitmap(game->_priv.loading.gamestate->fb);
		if (game->_priv.params.handlers.compositor) {
			game->_priv.loading.gamestate->fb = CreateNotPreservedBitmap(game->clip_rect.w, game->clip_rect.h);
		} else {
			game->_priv.loading.gamestate->fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->clip_rect.x, game->clip_rect.y, game->clip_rect.w, game->clip_rect.h);
		}
	}
	if (game->_priv.started) {
		DrawGamestates(game);
	}
}

SYMBOL_INTERNAL int SetupAudio(struct Game* game) {
#ifdef __EMSCRIPTEN__
	game->audio.v = al_create_voice(game->_priv.samplerate, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
#else
	game->audio.v = al_create_voice(game->_priv.samplerate, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
	if (!game->audio.v) {
		// fallback
		game->audio.v = al_create_voice(game->_priv.samplerate, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
	}
#endif
	al_set_default_voice(game->audio.v);

	if (game->audio.v) {
		al_attach_mixer_to_voice(game->audio.mixer, game->audio.v);
	} else {
		PrintConsole(game, "Could not create audio voice!");
		return 1;
	}
	return 0;
}

SYMBOL_INTERNAL void StopAudio(struct Game* game) {
	if (game->audio.v) {
		al_detach_voice(game->audio.v);
	}
	al_set_default_voice(NULL);
	game->audio.v = NULL;
}

SYMBOL_INTERNAL void DrawConsole(struct Game* game) {
	double game_time = al_get_time();
	if (game->show_console) {
		al_set_target_backbuffer(game->display);
		ALLEGRO_TRANSFORM trans;
		al_identity_transform(&trans);
		al_use_transform(&trans);

		al_hold_bitmap_drawing(true);

		int size = sizeof(game->_priv.console) / sizeof(game->_priv.console[0]);
		for (int i = 0; i < size; i++) {
			al_draw_filled_rectangle(0, 0, al_get_display_width(game->display), al_get_font_line_height(game->_priv.font_console) * (size - i), al_map_rgba(0, 0, 0, 80));
		}
		al_lock_mutex(game->_priv.mutex);
		int cur = game->_priv.console_pos + size;
		for (int i = 0; i < size; i++) {
			if (cur >= size) {
				cur -= size;
			}
			al_draw_text(game->_priv.font_console, al_map_rgb(255, 255, 255), (int)(al_get_display_width(game->display) * 0.005), al_get_font_line_height(game->_priv.font_console) * i, ALLEGRO_ALIGN_LEFT, game->_priv.console[cur]);
			cur++;
		}
		al_unlock_mutex(game->_priv.mutex);

		char sfps[16] = {0};
		snprintf(sfps, 6, "%.0f", game->_priv.fps_count.fps);
		DrawTextWithShadow(game->_priv.font_console, al_map_rgb(255, 255, 255), al_get_display_width(game->display), 0, ALLEGRO_ALIGN_RIGHT, sfps);
		snprintf(sfps, 16, "%.2f ms", 1000 * (game_time - game->_priv.fps_count.time));
		DrawTextWithShadow(game->_priv.font_console, al_map_rgb(255, 255, 255), al_get_display_width(game->display), al_get_font_line_height(game->_priv.font_console), ALLEGRO_ALIGN_RIGHT, sfps);

		DrawTimelines(game);

		al_hold_bitmap_drawing(false);
		al_use_transform(&game->_priv.projection);
	}

	if (game_time - game->_priv.fps_count.old_time >= 1.0) {
		game->_priv.fps_count.fps = game->_priv.fps_count.frames_done / (game_time - game->_priv.fps_count.old_time);
		game->_priv.fps_count.frames_done = 0;
		game->_priv.fps_count.old_time = game_time;
	}
	game->_priv.fps_count.time = game_time;
	game->_priv.fps_count.frames_done++;
}

SYMBOL_INTERNAL void Console_Load(struct Game* game) {
	if (FindDataFilePath(game, "fonts/DejaVuSansMono.ttf")) {
		game->_priv.font_console = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), (int)(game->clip_rect.h * 0.025), 0);
	} else {
		PrintConsole(game, "Could not load console font - using built-in fallback.");
		game->_priv.font_console = al_create_builtin_font();
	}

	if (FindDataFilePath(game, "fonts/PerfectDOSVGA437.ttf") && game->clip_rect.h * 0.025 >= 16) {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/PerfectDOSVGA437.ttf"), 16 * ((game->clip_rect.h > 1080) ? 2 : 1), 0);
	} else if (FindDataFilePath(game, "fonts/DejaVuSansMono.ttf")) {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), (int)(game->clip_rect.h * 0.025), 0);
	}
}

SYMBOL_INTERNAL void Console_Unload(struct Game* game) {
	if (game->_priv.font_console) {
		al_destroy_font(game->_priv.font_console);
	}
}

SYMBOL_INTERNAL void* GamestateLoadingThread(void* arg) {
	struct GamestateLoadingThreadData* data = arg;
	data->game->_priv.loading.in_progress = true;
	al_restore_state(&data->state);
	data->gamestate->data = data->gamestate->api->load(data->game, &GamestateProgress);
	if (data->game->_priv.loading.progress != data->gamestate->progress_count) {
		PrintConsole(data->game, "[%s] WARNING: Gamestate_ProgressCount does not match the number of progress invokations (%d)!", data->gamestate->name, data->game->_priv.loading.progress);
#ifndef LIBSUPERDERPY_SINGLE_THREAD
		if (data->game->config.debug.enabled) {
			PrintConsole(data->game, "(sleeping for 3 seconds...)");
			data->game->show_console = true;
			al_rest(3.0);
		}
#endif
	}
	al_store_state(&data->state, ALLEGRO_STATE_NEW_FILE_INTERFACE | ALLEGRO_STATE_NEW_BITMAP_PARAMETERS | ALLEGRO_STATE_BLENDER);

	data->game->_priv.loading.in_progress = false;
	return NULL;
}

SYMBOL_INTERNAL void* ScreenshotThread(void* arg) {
	struct ScreenshotThreadData* data = arg;
	ALLEGRO_PATH* path = al_get_standard_path(ALLEGRO_USER_DOCUMENTS_PATH);
	char filename[255];
	snprintf(filename, 255, "%s_%ju_%ju.png", data->game->_priv.name, (uintmax_t)time(NULL), (uintmax_t)clock());
	al_set_path_filename(path, filename);
	al_save_bitmap(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP), data->bitmap);
	PrintConsole(data->game, "Screenshot stored in %s", al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
	al_destroy_path(path);
	al_destroy_bitmap(data->bitmap);
	free(data);
	return NULL;
}

SYMBOL_INTERNAL void CalculateProgress(struct Game* game) {
	struct Gamestate* tmp = game->_priv.loading.current;
	float progress = ((game->_priv.loading.progress / (float)(tmp->progress_count + 1)) / (float)game->_priv.loading.to_load) + (game->_priv.loading.loaded / (float)game->_priv.loading.to_load);
	if (game->config.debug.enabled) {
		PrintConsole(game, "[%s] Progress: %d%% (%d/%d)", tmp->name, (int)(progress * 100), game->_priv.loading.progress, tmp->progress_count + 1);
	}
	game->loading.progress = progress;
}

SYMBOL_INTERNAL void GamestateProgress(struct Game* game) {
	game->_priv.loading.progress++;
	CalculateProgress(game);
#ifndef LIBSUPERDERPY_SINGLE_THREAD
	// TODO: debounce thread synchronization to reduce overhead
	al_lock_mutex(game->_priv.texture_sync_mutex);
	game->_priv.texture_sync = true;
	while (game->_priv.texture_sync) {
		al_wait_cond(game->_priv.texture_sync_cond, game->_priv.texture_sync_mutex);
	}
	al_unlock_mutex(game->_priv.texture_sync_mutex);
#else
	al_convert_memory_bitmaps();
	double delta = al_get_time() - game->_priv.loading.time;
	game->time += delta; // TODO: ability to disable passing time during loading
	game->_priv.loading.time += delta;
	if (game->loading.shown) {
		game->_priv.loading.gamestate->api->logic(game, game->_priv.loading.gamestate->data, delta);
	}
	DrawGamestates(game);
	DrawConsole(game);
	al_flip_display();
#endif
}

SYMBOL_INTERNAL bool OpenGamestate(struct Game* game, struct Gamestate* gamestate, bool required) {
	PrintConsole(game, "Opening gamestate \"%s\"...", gamestate->name);
#ifndef LIBSUPERDERPY_STATIC_GAMESTATES
	char libname[1024];
	snprintf(libname, 1024, "lib%s-%s" LIBRARY_EXTENSION, game->_priv.name, gamestate->name);
	gamestate->handle = dlopen(AddGarbage(game, GetLibraryPath(game, libname)), RTLD_NOW);
	if (!gamestate->handle) {
		if (required) {
			FatalError(game, false, "Error while opening gamestate \"%s\": %s", gamestate->name, dlerror());
		}
		return false;
	}
#else
	if (gamestate->fromlib) {
		FatalError(game, false, "Tried to open a not registered gamestate \"%s\"!", gamestate->name);
		return false;
	}
#endif
	gamestate->open = true;
	return true;
}

SYMBOL_INTERNAL bool LinkGamestate(struct Game* game, struct Gamestate* gamestate) {
	PrintConsole(game, "Linking gamestate \"%s\"...", gamestate->name);
#ifndef LIBSUPERDERPY_STATIC_GAMESTATES
	gamestate->api = calloc(1, sizeof(struct GamestateAPI));

#define GS_ERROR                                                                                                            \
	FatalError(game, false, "Error on resolving gamestate's %s symbol: %s", gamestate->name, dlerror()); /* TODO: move out */ \
	free(gamestate->api);                                                                                                     \
	return false

	if (!(gamestate->api->draw = dlsym(gamestate->handle, "Gamestate_Draw"))) { GS_ERROR; }
	if (!(gamestate->api->logic = dlsym(gamestate->handle, "Gamestate_Logic"))) { GS_ERROR; }
	if (!(gamestate->api->load = dlsym(gamestate->handle, "Gamestate_Load"))) { GS_ERROR; }
	if (!(gamestate->api->unload = dlsym(gamestate->handle, "Gamestate_Unload"))) { GS_ERROR; }
	if (!(gamestate->api->start = dlsym(gamestate->handle, "Gamestate_Start"))) { GS_ERROR; }
	if (!(gamestate->api->stop = dlsym(gamestate->handle, "Gamestate_Stop"))) { GS_ERROR; }
	if (!(gamestate->api->process_event = dlsym(gamestate->handle, "Gamestate_ProcessEvent"))) { GS_ERROR; }

	// optional
	gamestate->api->predraw = dlsym(gamestate->handle, "Gamestate_PreDraw");
	gamestate->api->tick = dlsym(gamestate->handle, "Gamestate_Tick");
	gamestate->api->post_load = dlsym(gamestate->handle, "Gamestate_PostLoad");
	gamestate->api->pause = dlsym(gamestate->handle, "Gamestate_Pause");
	gamestate->api->resume = dlsym(gamestate->handle, "Gamestate_Resume");
	gamestate->api->reload = dlsym(gamestate->handle, "Gamestate_Reload");
	gamestate->api->progress_count = dlsym(gamestate->handle, "Gamestate_ProgressCount");

#undef GS_ERROR

#else
	if (!gamestate->api) {
		return false;
	}
#endif

	if (gamestate->api->progress_count) {
		gamestate->progress_count = *gamestate->api->progress_count;
	}

	return true;
}

SYMBOL_INTERNAL struct Gamestate* AllocateGamestate(struct Game* game, const char* name) {
	struct Gamestate* tmp = malloc(sizeof(struct Gamestate));
	tmp->name = strdup(name);
	tmp->handle = NULL;
	tmp->loaded = false;
	tmp->paused = false;
	tmp->frozen = false;
	tmp->started = false;
	tmp->pending_load = false;
	tmp->pending_start = false;
	tmp->pending_stop = false;
	tmp->pending_unload = false;
	tmp->next = NULL;
	tmp->api = NULL;
	tmp->fromlib = true;
	tmp->progress_count = 0;
	tmp->open = false;
	tmp->fb = NULL;
	tmp->show_loading = true;
	tmp->data = NULL;
	return tmp;
}

SYMBOL_INTERNAL void CloseGamestate(struct Game* game, struct Gamestate* gamestate) {
	PrintConsole(game, "Closing gamestate \"%s\"...", gamestate->name);
	if (gamestate->api) {
		free(gamestate->api);
		gamestate->api = NULL;
	}
	if (!gamestate->open) {
		PrintConsole(game, "Gamestate \"%s\" already closed.", gamestate->name);
		return;
	}
#ifndef LIBSUPERDERPY_STATIC_GAMESTATES
	if (gamestate->handle && !RUNNING_ON_VALGRIND) {
#ifndef LEAK_SANITIZER
		dlclose(gamestate->handle);
		gamestate->handle = NULL;
#endif
	}
#endif
	gamestate->open = false;
}

SYMBOL_INTERNAL struct List* AddToList(struct List* list, void* data) {
	if (!list) {
		list = malloc(sizeof(struct List));
		list->data = data;
		list->next = NULL;
	} else {
		struct List* elem = malloc(sizeof(struct List));
		elem->next = list;
		elem->data = data;
		list = elem;
	}
	return list;
}

static bool Identity(struct List* elem, void* data) {
	return elem->data == data;
}

SYMBOL_INTERNAL struct List* FindInList(struct List* list, void* data, bool (*identity)(struct List* elem, void* data)) {
	struct List* tmp = list;
	if (!identity) {
		identity = Identity;
	}
	while (tmp) {
		if (identity(tmp, data)) {
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;
}

SYMBOL_INTERNAL void* RemoveFromList(struct List** list, void* data, bool (*identity)(struct List* elem, void* data)) {
	struct List *prev = NULL, *tmp = *list, *start = NULL;
	void* d = NULL;
	if (!identity) {
		identity = Identity;
	}
	while (tmp) {
		if (identity(tmp, data)) {
			if (prev) {
				prev->next = tmp->next;
				d = tmp->data;
				free(tmp);
				return d;
			}
			start = tmp->next;
			d = tmp->data;
			free(tmp);
			*list = start;
			return d;
		}
		prev = tmp;
		tmp = tmp->next;
	}
	return NULL;
}

// TODO: maybe make external?
SYMBOL_INTERNAL void* AddGarbage(struct Game* game, void* data) {
	game->_priv.garbage = AddToList(game->_priv.garbage, data);
	return data;
}

SYMBOL_INTERNAL void ClearGarbage(struct Game* game) {
	struct List* tmp = NULL;
	while (game->_priv.garbage) {
		free(game->_priv.garbage->data);
		tmp = game->_priv.garbage->next;
		free(game->_priv.garbage);
		game->_priv.garbage = tmp;
	}
}

SYMBOL_INTERNAL void AddTimeline(struct Game* game, struct Timeline* timeline) {
	game->_priv.timelines = AddToList(game->_priv.timelines, timeline);
}

SYMBOL_INTERNAL void RemoveTimeline(struct Game* game, struct Timeline* timeline) {
	struct List* tmp = game->_priv.timelines;
	if (tmp->data == timeline) {
		struct List* next = tmp->next;
		free(tmp);
		game->_priv.timelines = next;
		return;
	}
	while (tmp->next) {
		if (tmp->next->data == timeline) {
			struct List* next = tmp->next->next;
			free(tmp->next);
			tmp->next = next;
			return;
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void ClearScreen(struct Game* game) {
	al_set_target_backbuffer(game->display);
	al_reset_clipping_rectangle();
	al_clear_to_color(game->_priv.bg);
	if (game->_priv.params.depth_buffer) {
		al_clear_depth_buffer(1.0);
	}
	al_set_clipping_rectangle(game->clip_rect.x, game->clip_rect.y, game->clip_rect.w, game->clip_rect.h);
}

static void DrawQueue(struct Game* game, struct TM_Action* queue, int clipX, int clipY) {
	int pos = clipX;

	struct TM_Action* pom = queue;
	while (pom != NULL) {
		int width = al_get_text_width(game->_priv.font_console, pom->name);
		al_draw_filled_rectangle(pos - (10 / 3200.0) * game->clip_rect.w, clipY, pos + width + (10 / 3200.0) * game->clip_rect.w, clipY + (60 / 1800.0) * game->clip_rect.h, pom->started ? al_map_rgba(255, 255, 255, 192) : al_map_rgba(0, 0, 0, 0));
		al_draw_rectangle(pos - (10 / 3200.0) * game->clip_rect.w, clipY, pos + width + (10 / 3200.0) * game->clip_rect.w, clipY + (60 / 1800.0) * game->clip_rect.h, al_map_rgb(255, 255, 255), 2);
		al_draw_text(game->_priv.font_console, pom->started ? al_map_rgb(0, 0, 0) : al_map_rgb(255, 255, 255), pos, clipY, ALLEGRO_ALIGN_LEFT, pom->name);

		if (pom->delay) {
			al_draw_textf(game->_priv.font_console, al_map_rgb(255, 255, 255), pos, clipY - (50 / 1800.0) * game->clip_rect.h, ALLEGRO_ALIGN_LEFT, "%d", (int)(pom->delay * 1000));
		}

		if (strncmp(pom->name, "TM_RunInBackground", 18) == 0) { // FIXME: this is crappy way to detect queued background actions
			al_draw_textf(game->_priv.font_console, al_map_rgb(255, 255, 255), pos, clipY - (50 / 1800.0) * game->clip_rect.h, ALLEGRO_ALIGN_LEFT, "%s", (char*)pom->arguments->next->next->value);
		}

		pos += width + (int)((20 / 3200.0) * game->clip_rect.w);
		pom = pom->next;
	}
}

static void DrawTimeline(struct Game* game, struct Timeline* timeline, int pos) {
	al_draw_filled_rectangle(0, al_get_display_height(game->display) - (340 / 1800.0) * al_get_display_height(game->display) * (pos + 1), al_get_display_width(game->display), al_get_display_height(game->display) - (340 / 1800.0) * al_get_display_height(game->display) * pos, al_map_rgba(0, 0, 0, 92));

	al_draw_textf(game->_priv.font_console, al_map_rgb(255, 255, 255), al_get_display_width(game->display) / 2.0, al_get_display_height(game->display) - (340 / 1800.0) * al_get_display_height(game->display) * (pos + 1) + (10 / 1800.0) * al_get_display_height(game->display), ALLEGRO_ALIGN_CENTER, "Timeline: %s", timeline->name);

	DrawQueue(game, timeline->queue, (int)((25 / 3200.0) * al_get_display_width(game->display)), al_get_display_height(game->display) - (int)((220 / 1800.0) * al_get_display_height(game->display)) - (int)((340 / 1800.0) * al_get_display_height(game->display) * pos));
	DrawQueue(game, timeline->background, (int)((25 / 3200.0) * al_get_display_width(game->display)), al_get_display_height(game->display) - (int)((100 / 1800.0) * al_get_display_height(game->display)) - (int)((340 / 1800.0) * al_get_display_height(game->display) * pos));
}

SYMBOL_INTERNAL void DrawTimelines(struct Game* game) {
	if (!game->_priv.show_timeline) {
		return;
	}
	struct List* tmp = game->_priv.timelines;
	int i = 0;
	while (tmp) {
		DrawTimeline(game, tmp->data, i);
		i++;
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL char* GetLibraryPath(struct Game* game, char* filename) {
	char* result = NULL;
	ALLEGRO_PATH* path = al_get_standard_path(ALLEGRO_EXENAME_PATH);
	al_set_path_filename(path, filename);
	if (al_filename_exists(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP))) {
		result = strdup(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
	} else {
		al_append_path_component(path, "gamestates");
		if (al_filename_exists(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP))) {
			result = strdup(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
		} else {
			result = strdup(filename);
		}
	}
	al_destroy_path(path);
	return result;
}

SYMBOL_INTERNAL void PauseExecution(struct Game* game) {
	if (game->_priv.paused) {
		return;
	}
	game->_priv.paused = true;
	if (game->audio.v) {
		al_detach_voice(game->audio.v);
	}
	al_set_default_voice(NULL);
	game->audio.v = NULL;
	FreezeGamestates(game);
	PrintConsole(game, "Engine halted.");
	RedrawScreen(game);
}

SYMBOL_INTERNAL void ReloadCode(struct Game* game) {
	ReloadShaders(game, true);
	PrintConsole(game, "DEBUG: Reloading the gamestates...");
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->open && tmp->fromlib) {
			CloseGamestate(game, tmp);
			if (OpenGamestate(game, tmp, true) && LinkGamestate(game, tmp) && tmp->loaded) {
				if (tmp->api->reload) {
					PrintConsole(game, "[%s] Reloading...", tmp->name);
					tmp->api->reload(game, tmp->data);
				}
			} else {
				// live-reload failed
				tmp->loaded = false;
			}
		}
		tmp = tmp->next;
	}
	PrintConsole(game, "DEBUG: Reloading done.");
}

SYMBOL_INTERNAL void ResumeExecution(struct Game* game) {
	if (!game->_priv.paused) {
		return;
	}
	UnfreezeGamestates(game);
	SetupAudio(game);
	game->_priv.paused = false;
	game->_priv.timestamp = al_get_time();
	PrintConsole(game, "Engine resumed.");
}

SYMBOL_INTERNAL char* GetGameName(struct Game* game, const char* format) {
	char* result = malloc(sizeof(char) * 255);
	SUPPRESS_WARNING("-Wformat-nonliteral")
	snprintf(result, 255, format, game->_priv.name);
	SUPPRESS_END
	return AddGarbage(game, result);
}

static int HashString(struct Game* game, const char* str) {
	unsigned long hash = 5381;
	char c = 0;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	//PrintConsole(game, "sum %d, bucket %d", hash, hash % LIBSUPERDERPY_BITMAP_HASHMAP_BUCKETS);
	return hash % LIBSUPERDERPY_BITMAP_HASHMAP_BUCKETS;
}

static bool RefCountIdentity(struct List* elem, void* data) {
	struct RefCount* item = elem->data;
	return strcmp(data, item->id) == 0;
}

SYMBOL_INTERNAL void RedrawScreen(struct Game* game) {
#ifdef LIBSUPERDERPY_IMGUI
	ImGui_ImplAllegro5_NewFrame();
	igNewFrame();
#endif

	DrawGamestates(game);

#ifdef LIBSUPERDERPY_IMGUI
	igRender();
	ImGui_ImplAllegro5_RenderDrawData(igGetDrawData());
#endif

	DrawConsole(game);

	al_flip_display();
}

SYMBOL_INTERNAL ALLEGRO_BITMAP* AddBitmap(struct Game* game, char* filename) {
	int bucket = HashString(game, filename);
	struct List* item = FindInList(game->_priv.bitmaps[bucket], filename, RefCountIdentity);
	struct RefCount* rc = NULL;
	if (item) {
		rc = item->data;
		rc->counter++;
	} else {
		rc = malloc(sizeof(struct RefCount));
		rc->counter = 1;
		rc->id = strdup(filename);
		rc->data = al_load_bitmap(GetDataFilePath(game, filename));
		if (!rc->data) {
			FatalError(game, false, "Bitmap %s (%s) failed to load.", filename, GetDataFilePath(game, filename));
		}
		game->_priv.bitmaps[bucket] = AddToList(game->_priv.bitmaps[bucket], rc);
	}
	return rc->data;
}

SYMBOL_INTERNAL void RemoveBitmap(struct Game* game, char* filename) {
	int bucket = HashString(game, filename);
	struct List* item = FindInList(game->_priv.bitmaps[bucket], filename, RefCountIdentity);
	if (item) {
		struct RefCount* rc = item->data;
		rc->counter--;
		if (rc->counter == 0) {
			RemoveFromList(&game->_priv.bitmaps[bucket], filename, RefCountIdentity);
			al_destroy_bitmap(rc->data);
			free(rc->id);
			free(rc);
		}
	} else {
		PrintConsole(game, "Tried to remove non-existent bitmap %s!", filename);
	}
}

SYMBOL_INTERNAL void SetupViewport(struct Game* game) {
	game->viewport.width = game->_priv.params.width;
	game->viewport.height = game->_priv.params.height;

	game->_priv.window_width = al_get_display_width(game->display);
	game->_priv.window_height = al_get_display_height(game->display);

	if (game->viewport.width == 0 && game->viewport.height == 0) {
		game->viewport.width = game->_priv.window_width;
		game->viewport.height = game->_priv.window_height;
	} else if (game->viewport.width == 0 || game->viewport.height == 0) {
		game->viewport.height = al_get_display_height(game->display);
		game->viewport.width = (int)(game->_priv.params.aspect * game->viewport.height);
		if (game->viewport.width > al_get_display_width(game->display)) {
			game->viewport.width = al_get_display_width(game->display);
			game->viewport.height = (int)(game->viewport.width / game->_priv.params.aspect);
		}
	}

	al_set_target_backbuffer(game->display);
	al_identity_transform(&game->_priv.projection);
	al_use_transform(&game->_priv.projection);
	al_reset_clipping_rectangle();

	float resolution = al_get_display_height(game->display) / (float)game->viewport.height;
	if (al_get_display_width(game->display) / (float)game->viewport.width < resolution) {
		resolution = al_get_display_width(game->display) / (float)game->viewport.width;
	}
	if (game->_priv.params.integer_scaling) {
		resolution = floorf(resolution);
		if (floorf(resolution) == 0) {
			resolution = 1;
		}
	}
	if ((!strtol(GetConfigOptionDefault(game, "SuperDerpy", "downscale", "1"), NULL, 10)) && (resolution < 1)) {
		resolution = 1;
	}
	if (!strtol(GetConfigOptionDefault(game, "SuperDerpy", "scaling", "1"), NULL, 10)) {
		resolution = 1;
	}

	int clipWidth = (int)(game->viewport.width * resolution);
	int clipHeight = (int)(game->viewport.height * resolution);
	if (strtol(GetConfigOptionDefault(game, "SuperDerpy", "letterbox", "1"), NULL, 10)) {
		int clipX = (al_get_display_width(game->display) - clipWidth) / 2;
		int clipY = (al_get_display_height(game->display) - clipHeight) / 2;
		al_build_transform(&game->_priv.projection, clipX, clipY, resolution, resolution, 0.0);
		al_set_clipping_rectangle(clipX, clipY, clipWidth, clipHeight);
		game->clip_rect.x = clipX;
		game->clip_rect.y = clipY;
		game->clip_rect.w = clipWidth;
		game->clip_rect.h = clipHeight;
	} else if (strtol(GetConfigOptionDefault(game, "SuperDerpy", "scaling", "1"), NULL, 10)) {
		al_build_transform(&game->_priv.projection, 0, 0, al_get_display_width(game->display) / (float)game->viewport.width, al_get_display_height(game->display) / (float)game->viewport.height, 0.0);
	}
	al_use_transform(&game->_priv.projection);
	Console_Unload(game);
	Console_Load(game);
	ResizeGamestates(game);

	PrintConsole(game, "Viewport %dx%d; display %dx%d", game->viewport.width, game->viewport.height, al_get_display_width(game->display), al_get_display_height(game->display));
}

#ifdef LIBSUPERDERPY_STATIC_GAMESTATES

SYMBOL_EXPORT void __libsuperderpy_register_gamestate(const char* name, struct GamestateAPI* api, struct Game* game) {
	static int counter = 0;
	static struct GamestateAPI apis[256];
	static const char* names[256];
	if (counter == 255) {
		return;
	}
	if (api) {
		names[counter] = name;
		apis[counter++] = *api;
	}
	if (game) {
		for (int i = 0; i < counter; i++) {
			RegisterGamestate(game, names[i], &apis[i]);
		}
	}
}
#endif

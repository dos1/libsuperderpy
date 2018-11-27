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
#include <dlfcn.h>

SYMBOL_INTERNAL void SimpleCompositor(struct Game* game, struct Gamestate* gamestates) {
	struct Gamestate* tmp = gamestates;
	ClearToColor(game, al_map_rgb(0, 0, 0));
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			al_draw_bitmap(tmp->fb, game->_priv.clip_rect.x, game->_priv.clip_rect.y, 0);
		}
		tmp = tmp->next;
	}
	if (game->_priv.loading.inProgress) {
		al_draw_bitmap(game->loading_fb, game->_priv.clip_rect.x, game->_priv.clip_rect.y, 0);
	}
}

SYMBOL_INTERNAL void DrawGamestates(struct Game* game) {
	if (!game->handlers.compositor) {
		ClearScreen(game);
	}
	struct Gamestate* tmp = game->_priv.gamestates;
	if (game->handlers.predraw) {
		game->handlers.predraw(game);
	}
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			game->_priv.current_gamestate = tmp;
			SetFramebufferAsTarget(game);
			if (game->handlers.compositor) { // don't clear when uncomposited
				al_reset_clipping_rectangle();
				al_clear_to_color(al_map_rgb(0, 0, 0)); // even if everything is going to be redrawn, it optimizes tiled rendering
			}
			tmp->api->Gamestate_Draw(game, tmp->data);
			// TODO: save and restore more state for careless gamestating
		}
		tmp = tmp->next;
	}

	if (game->_priv.loading.inProgress) {
		// same as above, but for the loading gamestate
		game->_priv.current_gamestate = NULL;
		SetFramebufferAsTarget(game);
		if (game->handlers.compositor) {
			al_reset_clipping_rectangle();
			al_clear_to_color(al_map_rgb(0, 0, 0));
		}
		game->_priv.loading.gamestate->api->Gamestate_Draw(game, game->_priv.loading.gamestate->data);
	}

	al_set_target_backbuffer(game->display);

	ALLEGRO_TRANSFORM t;
	// restore full resolution access to the whole screen
	al_identity_transform(&t);
	al_use_transform(&t);
	al_reset_clipping_rectangle();

	if (game->handlers.compositor) {
		game->handlers.compositor(game, game->_priv.gamestates);
	}

	if (game->handlers.postdraw) {
		game->handlers.postdraw(game);
	}
}

SYMBOL_INTERNAL void LogicGamestates(struct Game* game, double delta) {
	struct Gamestate* tmp = game->_priv.gamestates;
	if (game->handlers.prelogic) {
		game->handlers.prelogic(game, delta);
	}
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			game->_priv.current_gamestate = tmp;
			tmp->api->Gamestate_Logic(game, tmp->data, delta);
		}
		tmp = tmp->next;
	}
	if (game->handlers.postlogic) {
		game->handlers.postlogic(game, delta);
	}
}

SYMBOL_INTERNAL void TickGamestates(struct Game* game) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			game->_priv.current_gamestate = tmp;
			if (tmp->api->Gamestate_Tick) {
				tmp->api->Gamestate_Tick(game, tmp->data);
			}
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void ReloadGamestates(struct Game* game) {
	struct Gamestate* tmp = game->_priv.gamestates;
	ReloadShaders(game, true);
	while (tmp) {
		if (tmp->loaded) {
			game->_priv.current_gamestate = tmp;
			if (tmp->api->Gamestate_Reload) {
				tmp->api->Gamestate_Reload(game, tmp->data);
			}
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void EventGamestates(struct Game* game, ALLEGRO_EVENT* ev) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			game->_priv.current_gamestate = tmp;
			tmp->api->Gamestate_ProcessEvent(game, tmp->data, ev);
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
		al_destroy_bitmap(tmp->fb);
		if (game->handlers.compositor) {
			tmp->fb = CreateNotPreservedBitmap(game->_priv.clip_rect.w, game->_priv.clip_rect.h);
		} else {
			tmp->fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->_priv.clip_rect.x, game->_priv.clip_rect.y, game->_priv.clip_rect.w, game->_priv.clip_rect.h);
		}
		tmp = tmp->next;
	}
	al_destroy_bitmap(game->loading_fb);
	if (game->handlers.compositor) {
		game->loading_fb = CreateNotPreservedBitmap(game->_priv.clip_rect.w, game->_priv.clip_rect.h);
	} else {
		game->loading_fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->_priv.clip_rect.x, game->_priv.clip_rect.y, game->_priv.clip_rect.w, game->_priv.clip_rect.h);
	}
}

SYMBOL_INTERNAL void DrawConsole(struct Game* game) {
	double game_time = al_get_time();
	if (game->_priv.showconsole) {
		al_set_target_backbuffer(game->display);
		ALLEGRO_TRANSFORM trans;
		al_identity_transform(&trans);
		al_use_transform(&trans);

		al_hold_bitmap_drawing(true);

		int size = sizeof(game->_priv.console) / sizeof(game->_priv.console[0]);
		for (int i = 0; i < size; i++) {
			al_draw_filled_rectangle(0, 0, al_get_display_width(game->display), al_get_font_line_height(game->_priv.font_console) * (size - i), al_map_rgba(0, 0, 0, 80));
		}
		int cur = game->_priv.console_pos + size;
		for (int i = 0; i < size; i++) {
			if (cur >= size) {
				cur -= size;
			}
			al_draw_text(game->_priv.font_console, al_map_rgb(255, 255, 255), (int)(al_get_display_width(game->display) * 0.005), al_get_font_line_height(game->_priv.font_console) * i, ALLEGRO_ALIGN_LEFT, game->_priv.console[cur]);
			cur++;
		}

		char sfps[16] = {0};
		snprintf(sfps, 6, "%.0f", game->_priv.fps_count.fps);
		DrawTextWithShadow(game->_priv.font_console, al_map_rgb(255, 255, 255), al_get_display_width(game->display), 0, ALLEGRO_ALIGN_RIGHT, sfps);
		snprintf(sfps, 16, "%.2f ms", 1000 * (game_time - game->_priv.fps_count.time));
		DrawTextWithShadow(game->_priv.font_console, al_map_rgb(255, 255, 255), al_get_display_width(game->display), al_get_font_line_height(game->_priv.font_console), ALLEGRO_ALIGN_RIGHT, sfps);

		DrawTimelines(game);

		al_hold_bitmap_drawing(false);
		al_use_transform(&game->projection);
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
	game->_priv.font_console = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), (int)(game->_priv.clip_rect.h * 0.025), 0);
	if (game->_priv.clip_rect.h * 0.025 >= 16) {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/PerfectDOSVGA437.ttf"), 16 * ((game->_priv.clip_rect.h > 1080) ? 2 : 1), 0);
	} else {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), (int)(game->_priv.clip_rect.h * 0.025), 0);
	}
}

SYMBOL_INTERNAL void Console_Unload(struct Game* game) {
	if (game->_priv.font_console) {
		al_destroy_font(game->_priv.font_console);
	}
}

SYMBOL_INTERNAL void* GamestateLoadingThread(void* arg) {
	struct GamestateLoadingThreadData* data = arg;
	data->game->_priv.loading.inProgress = true;
	al_set_new_bitmap_flags(data->bitmap_flags);
	data->gamestate->data = data->gamestate->api->Gamestate_Load(data->game, &GamestateProgress);
	if (data->game->_priv.loading.progress != data->gamestate->progressCount) {
		PrintConsole(data->game, "[%s] WARNING: Gamestate_ProgressCount does not match the number of progress invokations (%d)!", data->gamestate->name, data->game->_priv.loading.progress);
		if (data->game->config.debug) {
			PrintConsole(data->game, "(sleeping for 3 seconds...)");
			data->game->_priv.showconsole = true;
			al_rest(3.0);
		}
	}
	data->bitmap_flags = al_get_new_bitmap_flags();
	data->game->_priv.loading.inProgress = false;
	return NULL;
}

SYMBOL_INTERNAL void* ScreenshotThread(void* arg) {
	struct ScreenshotThreadData* data = arg;
	ALLEGRO_PATH* path = al_get_standard_path(ALLEGRO_USER_DOCUMENTS_PATH);
	char filename[255];
	snprintf(filename, 255, "%s_%ju_%ju.png", data->game->name, (uintmax_t)time(NULL), (uintmax_t)clock());
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
	float progress = ((game->_priv.loading.progress / (float)(tmp->progressCount + 1)) / (float)game->_priv.loading.toLoad) + (game->_priv.loading.loaded / (float)game->_priv.loading.toLoad);
	if (game->config.debug) {
		PrintConsole(game, "[%s] Progress: %d%% (%d/%d)", tmp->name, (int)(progress * 100), game->_priv.loading.progress, tmp->progressCount + 1);
	}
	game->loading_progress = progress;
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
	if (game->_priv.loading.current->showLoading) {
		game->_priv.loading.gamestate->api->Gamestate_Logic(game, game->_priv.loading.gamestate->data, delta);
		DrawGamestates(game);
	}
	game->_priv.loading.time += delta;
	DrawConsole(game);
	al_flip_display();
#endif
}

SYMBOL_INTERNAL bool OpenGamestate(struct Game* game, struct Gamestate* gamestate) {
	PrintConsole(game, "Opening gamestate \"%s\"...", gamestate->name);
	char libname[1024];
	snprintf(libname, 1024, "libsuperderpy-%s-%s" LIBRARY_EXTENSION, game->name, gamestate->name);
	gamestate->handle = dlopen(AddGarbage(game, GetLibraryPath(game, libname)), RTLD_NOW);
	if (!gamestate->handle) {
		FatalError(game, false, "Error while opening gamestate \"%s\": %s", gamestate->name, dlerror()); // TODO: move out
		return false;
	}
	if (game->handlers.compositor) {
		gamestate->fb = CreateNotPreservedBitmap(game->_priv.clip_rect.w, game->_priv.clip_rect.h);
	} else {
		gamestate->fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->_priv.clip_rect.x, game->_priv.clip_rect.y, game->_priv.clip_rect.w, game->_priv.clip_rect.h);
	}
	gamestate->open = true;
	return true;
}

SYMBOL_INTERNAL bool LinkGamestate(struct Game* game, struct Gamestate* gamestate) {
	gamestate->api = calloc(1, sizeof(struct Gamestate_API));

#define GS_ERROR                                                                                                            \
	FatalError(game, false, "Error on resolving gamestate's %s symbol: %s", gamestate->name, dlerror()); /* TODO: move out */ \
	free(gamestate->api);                                                                                                     \
	return false;

	if (!(gamestate->api->Gamestate_Draw = dlsym(gamestate->handle, "Gamestate_Draw"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Logic = dlsym(gamestate->handle, "Gamestate_Logic"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Load = dlsym(gamestate->handle, "Gamestate_Load"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Unload = dlsym(gamestate->handle, "Gamestate_Unload"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Start = dlsym(gamestate->handle, "Gamestate_Start"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Stop = dlsym(gamestate->handle, "Gamestate_Stop"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_ProcessEvent = dlsym(gamestate->handle, "Gamestate_ProcessEvent"))) { GS_ERROR; }

	// optional
	gamestate->api->Gamestate_Tick = dlsym(gamestate->handle, "Gamestate_Tick");
	gamestate->api->Gamestate_PostLoad = dlsym(gamestate->handle, "Gamestate_PostLoad");
	gamestate->api->Gamestate_Pause = dlsym(gamestate->handle, "Gamestate_Pause");
	gamestate->api->Gamestate_Resume = dlsym(gamestate->handle, "Gamestate_Resume");
	gamestate->api->Gamestate_Reload = dlsym(gamestate->handle, "Gamestate_Reload");
	gamestate->api->Gamestate_ProgressCount = dlsym(gamestate->handle, "Gamestate_ProgressCount");

	if (gamestate->api->Gamestate_ProgressCount) {
		gamestate->progressCount = *gamestate->api->Gamestate_ProgressCount;
	}

#undef GS_ERROR

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
	tmp->progressCount = 0;
	tmp->open = false;
	tmp->fb = NULL;
	tmp->showLoading = true;
	tmp->data = NULL;
	return tmp;
}

SYMBOL_INTERNAL void CloseGamestate(struct Game* game, struct Gamestate* gamestate) {
	if (!gamestate->open) {
		return;
	}
	if (gamestate->handle && !RUNNING_ON_VALGRIND) {
#ifndef LEAK_SANITIZER
		PrintConsole(game, "Closing gamestate \"%s\"...", gamestate->name);
		dlclose(gamestate->handle);
#endif
	}
	free(gamestate->name);
	if (gamestate->api) {
		free(gamestate->api);
	}
	al_destroy_bitmap(gamestate->fb);
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
	struct List *prev = NULL, *tmp = *list, *start;
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
	struct List* tmp;
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
	al_clear_to_color(al_map_rgb(0, 0, 0));
	if (game->viewport_config.depth_buffer) {
		al_clear_depth_buffer(1.0);
	}
	al_set_clipping_rectangle(game->_priv.clip_rect.x, game->_priv.clip_rect.y, game->_priv.clip_rect.w, game->_priv.clip_rect.h);
}

static void DrawQueue(struct Game* game, struct TM_Action* queue, int clipX, int clipY) {
	int pos = clipX;

	struct TM_Action* pom = queue;
	while (pom != NULL) {
		int width = al_get_text_width(game->_priv.font_console, pom->name);
		al_draw_filled_rectangle(pos - (10 / 3200.0) * game->_priv.clip_rect.w, clipY, pos + width + (10 / 3200.0) * game->_priv.clip_rect.w, clipY + (60 / 1800.0) * game->_priv.clip_rect.h, pom->started ? al_map_rgba(255, 255, 255, 192) : al_map_rgba(0, 0, 0, 0));
		al_draw_rectangle(pos - (10 / 3200.0) * game->_priv.clip_rect.w, clipY, pos + width + (10 / 3200.0) * game->_priv.clip_rect.w, clipY + (60 / 1800.0) * game->_priv.clip_rect.h, al_map_rgb(255, 255, 255), 2);
		al_draw_text(game->_priv.font_console, pom->started ? al_map_rgb(0, 0, 0) : al_map_rgb(255, 255, 255), pos, clipY, ALLEGRO_ALIGN_LEFT, pom->name);

		if (pom->delay) {
			al_draw_textf(game->_priv.font_console, al_map_rgb(255, 255, 255), pos, clipY - (50 / 1800.0) * game->_priv.clip_rect.h, ALLEGRO_ALIGN_LEFT, "%d", (int)(pom->delay * 1000));
		}

		if (strncmp(pom->name, "TM_RunInBackground", 18) == 0) { // FIXME: this is crappy way to detect queued background actions
			al_draw_textf(game->_priv.font_console, al_map_rgb(255, 255, 255), pos, clipY - (50 / 1800.0) * game->_priv.clip_rect.h, ALLEGRO_ALIGN_LEFT, "%s", (char*)pom->arguments->next->next->value);
		}

		pos += width + (int)((20 / 3200.0) * game->_priv.clip_rect.w);
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
	if (!game->_priv.showtimeline) {
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
	al_stop_timer(game->_priv.timer);
	al_detach_voice(game->audio.v);
	FreezeGamestates(game);
	PrintConsole(game, "Engine halted.");
}

SYMBOL_INTERNAL void ReloadCode(struct Game* game) {
	ReloadShaders(game, true);
	PrintConsole(game, "DEBUG: Reloading the gamestates...");
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->open && tmp->fromlib) {
			char* name = strdup(tmp->name);
			CloseGamestate(game, tmp);
			tmp->name = name;
			if (OpenGamestate(game, tmp) && LinkGamestate(game, tmp) && tmp->loaded) {
				if (tmp->api->Gamestate_Reload) {
					PrintConsole(game, "[%s] Reloading...", tmp->name);
					tmp->api->Gamestate_Reload(game, tmp->data);
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
	al_attach_mixer_to_voice(game->audio.mixer, game->audio.v);
	al_resume_timer(game->_priv.timer);
	game->_priv.paused = false;
	game->_priv.timestamp = al_get_time();
	PrintConsole(game, "Engine resumed.");
}

SYMBOL_INTERNAL char* GetGameName(struct Game* game, const char* format) {
	char* result = malloc(sizeof(char) * 255);
	SUPPRESS_WARNING("-Wformat-nonliteral")
	snprintf(result, 255, format, game->name);
	SUPPRESS_END
	return AddGarbage(game, result);
}

static int HashString(struct Game* game, const char* str) {
	unsigned long hash = 5381;
	int c;

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

SYMBOL_INTERNAL ALLEGRO_BITMAP* AddBitmap(struct Game* game, char* filename) {
	int bucket = HashString(game, filename);
	struct List* item = FindInList(game->_priv.bitmaps[bucket], filename, RefCountIdentity);
	struct RefCount* rc;
	if (item) {
		rc = item->data;
		rc->counter++;
	} else {
		rc = malloc(sizeof(struct RefCount));
		rc->counter = 1;
		rc->id = strdup(filename);
		rc->data = al_load_bitmap(GetDataFilePath(game, filename));
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

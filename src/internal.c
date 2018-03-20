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
#include "libsuperderpy.h"
#include <allegro5/allegro_ttf.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>

SYMBOL_INTERNAL void SimpleCompositor(struct Game* game, struct Gamestate* gamestates) {
	struct Gamestate* tmp = gamestates;
	al_clear_to_color(al_map_rgb(0, 0, 0));
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			al_draw_bitmap(tmp->fb, 0, 0, 0);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void DrawGamestates(struct Game* game) {
	if (!game->handlers.compositor) {
		ClearScreen(game);
	}
	struct Gamestate* tmp = game->_priv.gamestates;
	if (game->handlers.predraw) {
		(*game->handlers.predraw)(game);
	}
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			game->_priv.current_gamestate = tmp;
			SetFramebufferAsTarget(game);
			if (game->handlers.compositor) { // don't clear when uncomposited
				al_clear_to_color(al_map_rgb(0, 0, 0)); // even if everything is going to be redrawn, it optimizes tiled rendering
			}
			(*tmp->api->Gamestate_Draw)(game, tmp->data);
			// TODO: save and restore more state for careless gamestating
		}
		tmp = tmp->next;
	}

	if (game->handlers.compositor) {
		ALLEGRO_TRANSFORM t;
		al_set_target_backbuffer(game->display);
		ClearScreen(game);
		al_identity_transform(&t);
		/*		double factor = (sin(al_get_time()) / 2.0 + 1.0) * 2;
		al_translate_transform(&t, -game->_priv.clip_rect.w / factor, -game->_priv.clip_rect.h / factor);
		al_scale_transform(&t, factor, factor);
		al_translate_transform(&t, game->_priv.clip_rect.w / factor, game->_priv.clip_rect.h / factor);*/
		al_translate_transform(&t, game->_priv.clip_rect.x, game->_priv.clip_rect.y);
		al_use_transform(&t);
		game->handlers.compositor(game, game->_priv.gamestates);
	}
	if (game->handlers.postdraw) {
		(*game->handlers.postdraw)(game);
	}
}

SYMBOL_INTERNAL void LogicGamestates(struct Game* game, double delta) {
	struct Gamestate* tmp = game->_priv.gamestates;
	if (game->handlers.prelogic) {
		(*game->handlers.prelogic)(game, delta);
	}
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Logic)(game, tmp->data, delta);
		}
		tmp = tmp->next;
	}
	if (game->handlers.postlogic) {
		(*game->handlers.postlogic)(game, delta);
	}
}

SYMBOL_INTERNAL void ReloadGamestates(struct Game* game) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->loaded) {
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Reload)(game, tmp->data);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void EventGamestates(struct Game* game, ALLEGRO_EVENT* ev) {
	struct Gamestate* tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_ProcessEvent)(game, tmp->data, ev);
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
}

SYMBOL_INTERNAL void DrawConsole(struct Game* game) {
	double game_time = al_get_time();
	if (game->_priv.showconsole) {
		al_set_target_backbuffer(game->display);
		ALLEGRO_TRANSFORM trans;
		al_identity_transform(&trans);
		int clipX, clipY, clipWidth, clipHeight;
		al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
		al_use_transform(&trans);
		al_hold_bitmap_drawing(true);

		int width = (al_get_display_width(game->display) / game->viewport.width) * game->viewport.width;
		if (!game->viewport.integer_scaling) {
			width = (al_get_display_width(game->display) / (float)game->viewport.width) * game->viewport.width;
		}

		int size = sizeof(game->_priv.console) / sizeof(game->_priv.console[0]);
		for (int i = 0; i < size; i++) {
			al_draw_filled_rectangle(clipX, clipY, clipX + width, clipY + al_get_font_line_height(game->_priv.font_console) * (size - i), al_map_rgba(0, 0, 0, 80));
		}
		int cur = game->_priv.console_pos + size;
		for (int i = 0; i < size; i++) {
			if (cur >= size) {
				cur -= size;
			}
			al_draw_text(game->_priv.font_console, al_map_rgb(255, 255, 255), clipX + (int)(game->viewport.width * 0.005), clipY + al_get_font_line_height(game->_priv.font_console) * i, ALLEGRO_ALIGN_LEFT, game->_priv.console[cur]);
			cur++;
		}

		char sfps[16] = {0};
		snprintf(sfps, 6, "%.0f", game->_priv.fps_count.fps);
		DrawTextWithShadow(game->_priv.font_console, al_map_rgb(255, 255, 255), clipX + clipWidth, clipY, ALLEGRO_ALIGN_RIGHT, sfps);
		snprintf(sfps, 16, "%.2f ms", 1000 * (game_time - game->_priv.fps_count.time));
		DrawTextWithShadow(game->_priv.font_console, al_map_rgb(255, 255, 255), clipX + clipWidth, clipY + al_get_font_line_height(game->_priv.font_console), ALLEGRO_ALIGN_RIGHT, sfps);

		al_use_transform(&game->projection);

		DrawTimelines(game);
	}
	al_hold_bitmap_drawing(false);

	if (game_time - game->_priv.fps_count.old_time >= 1.0) {
		game->_priv.fps_count.fps = game->_priv.fps_count.frames_done / (game_time - game->_priv.fps_count.old_time);
		game->_priv.fps_count.frames_done = 0;
		game->_priv.fps_count.old_time = game_time;
	}
	game->_priv.fps_count.time = game_time;
	game->_priv.fps_count.frames_done++;
}

SYMBOL_INTERNAL void Console_Load(struct Game* game) {
	game->_priv.font_console = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), al_get_display_height(game->display) * 0.025, 0);
	if (al_get_display_height(game->display) * 0.025 >= 16) {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/PerfectDOSVGA437.ttf"), 16 * ((al_get_display_height(game->display) > 1080) ? 2 : 1), 0);
	} else {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), al_get_display_height(game->display) * 0.025, 0);
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
	GamestateProgress(data->game);
	data->gamestate->data = (*data->gamestate->api->Gamestate_Load)(data->game, &GamestateProgress);
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

SYMBOL_INTERNAL void GamestateProgress(struct Game* game) {
	struct Gamestate* tmp = game->_priv.loading.current;
	game->_priv.loading.progress++;
	float progressCount = *(tmp->api->Gamestate_ProgressCount) ? (float)*(tmp->api->Gamestate_ProgressCount) : 1;
	float progress = ((game->_priv.loading.progress / progressCount) / (float)game->_priv.loading.toLoad) + (game->_priv.loading.loaded / (float)game->_priv.loading.toLoad);
	game->loading_progress = progress;
	if (game->config.debug) {
		PrintConsole(game, "[%s] Progress: %d%% (%d/%d)", tmp->name, (int)(progress * 100), game->_priv.loading.progress, *(tmp->api->Gamestate_ProgressCount));
	}
#ifdef LIBSUPERDERPY_SINGLE_THREAD
	DrawGamestates(game);
	if (tmp->showLoading) {
		(*game->_priv.loading.gamestate->api->Gamestate_Draw)(game, game->_priv.loading.gamestate->data);
	}
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
	return true;
}

SYMBOL_INTERNAL bool LinkGamestate(struct Game* game, struct Gamestate* gamestate) {
	gamestate->api = malloc(sizeof(struct Gamestate_API));

#define GS_ERROR                                                                                                            \
	FatalError(game, false, "Error on resolving gamestate's %s symbol: %s", gamestate->name, dlerror()); /* TODO: move out */ \
	free(gamestate->api);                                                                                                     \
	return false;

	if (!(gamestate->api->Gamestate_Draw = dlsym(gamestate->handle, "Gamestate_Draw"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Logic = dlsym(gamestate->handle, "Gamestate_Logic"))) { GS_ERROR; }

	if (!(gamestate->api->Gamestate_Load = dlsym(gamestate->handle, "Gamestate_Load"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Start = dlsym(gamestate->handle, "Gamestate_Start"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Pause = dlsym(gamestate->handle, "Gamestate_Pause"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Resume = dlsym(gamestate->handle, "Gamestate_Resume"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Stop = dlsym(gamestate->handle, "Gamestate_Stop"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Unload = dlsym(gamestate->handle, "Gamestate_Unload"))) { GS_ERROR; }

	if (!(gamestate->api->Gamestate_ProcessEvent = dlsym(gamestate->handle, "Gamestate_ProcessEvent"))) { GS_ERROR; }
	if (!(gamestate->api->Gamestate_Reload = dlsym(gamestate->handle, "Gamestate_Reload"))) { GS_ERROR; }

	if (!(gamestate->api->Gamestate_ProgressCount = dlsym(gamestate->handle, "Gamestate_ProgressCount"))) { GS_ERROR; }

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
	if (game->handlers.compositor) {
		tmp->fb = CreateNotPreservedBitmap(game->_priv.clip_rect.w, game->_priv.clip_rect.h);
	} else {
		tmp->fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->_priv.clip_rect.x, game->_priv.clip_rect.y, game->_priv.clip_rect.w, game->_priv.clip_rect.h);
	}
	return tmp;
}

SYMBOL_INTERNAL void CloseGamestate(struct Game* game, struct Gamestate* gamestate) {
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

SYMBOL_INTERNAL struct libsuperderpy_list* AddToList(struct libsuperderpy_list* list, void* data) {
	if (!list) {
		list = malloc(sizeof(struct libsuperderpy_list));
		list->data = data;
		list->next = NULL;
	} else {
		struct libsuperderpy_list* elem = malloc(sizeof(struct libsuperderpy_list));
		elem->next = list;
		elem->data = data;
		list = elem;
	}
	return list;
}

SYMBOL_INTERNAL struct libsuperderpy_list* RemoveFromList(struct libsuperderpy_list** list, bool (*identity)(struct libsuperderpy_list* elem, void* data), void* data) {
	struct libsuperderpy_list *prev = NULL, *tmp = *list, *start;
	void* d = NULL;
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
	struct libsuperderpy_list* tmp;
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
	struct libsuperderpy_list* tmp = game->_priv.timelines;
	if (tmp->data == timeline) {
		struct libsuperderpy_list* next = tmp->next;
		free(tmp);
		game->_priv.timelines = next;
		return;
	}
	while (tmp->next) {
		if (tmp->next->data == timeline) {
			struct libsuperderpy_list* next = tmp->next->next;
			free(tmp->next);
			tmp->next = next;
			return;
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void ClearScreen(struct Game* game) {
	al_set_target_backbuffer(game->display);
	al_set_clipping_rectangle(0, 0, al_get_display_width(game->display), al_get_display_height(game->display));
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_clear_depth_buffer(1.0);
	al_set_clipping_rectangle(game->_priv.clip_rect.x, game->_priv.clip_rect.y, game->_priv.clip_rect.w, game->_priv.clip_rect.h);
}

static void DrawQueue(struct Game* game, struct TM_Action* queue, int clipX, int clipY) {
	int pos = clipX;

	struct TM_Action* pom = queue;
	while (pom != NULL) {
		int width = al_get_text_width(game->_priv.font_console, pom->name);
		al_draw_filled_rectangle(pos - (10 / 3200.0) * al_get_display_width(game->display), clipY, pos + width + (10 / 3200.0) * al_get_display_width(game->display), clipY + (60 / 1800.0) * al_get_display_height(game->display), pom->active ? al_map_rgba(255, 255, 255, 192) : al_map_rgba(0, 0, 0, 0));
		al_draw_rectangle(pos - (10 / 3200.0) * al_get_display_width(game->display), clipY, pos + width + (10 / 3200.0) * al_get_display_width(game->display), clipY + (60 / 1800.0) * al_get_display_height(game->display), al_map_rgb(255, 255, 255), 2);
		al_draw_text(game->_priv.font_console, pom->active ? al_map_rgb(0, 0, 0) : al_map_rgb(255, 255, 255), pos, clipY, ALLEGRO_ALIGN_LEFT, pom->name);

		if (pom->delay) {
			al_draw_textf(game->_priv.font_console, al_map_rgb(255, 255, 255), pos, clipY - (50 / 1800.0) * al_get_display_height(game->display), ALLEGRO_ALIGN_LEFT, "%d", pom->delay);
		}

		if (strncmp(pom->name, "TM_BackgroundAction", 19) == 0) {
			al_draw_textf(game->_priv.font_console, al_map_rgb(255, 255, 255), pos, clipY - (50 / 1800.0) * al_get_display_height(game->display), ALLEGRO_ALIGN_LEFT, "%s", (char*)pom->arguments->next->next->value);
		}

		pos += width + (20 / 3200.0) * al_get_display_width(game->display);
		pom = pom->next;
	}
}

static void DrawTimeline(struct Game* game, struct Timeline* timeline, int pos) {
	al_set_target_backbuffer(game->display);
	ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	int clipX, clipY, clipWidth, clipHeight;
	al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
	al_use_transform(&trans);

	al_draw_filled_rectangle(clipX, clipY + clipHeight - (340 / 1800.0) * al_get_display_height(game->display) * (pos + 1), clipX + clipWidth, clipY + clipHeight - (340 / 1800.0) * al_get_display_height(game->display) * pos, al_map_rgba(0, 0, 0, 92));

	al_draw_textf(game->_priv.font_console, al_map_rgb(255, 255, 255), clipX + clipWidth / 2, clipY + clipHeight - (340 / 1800.0) * al_get_display_height(game->display) * (pos + 1) + (10 / 1800.0) * al_get_display_height(game->display), ALLEGRO_ALIGN_CENTER, "Timeline: %s", timeline->name);

	DrawQueue(game, timeline->queue, clipX + (25 / 3200.0) * al_get_display_width(game->display), clipY + clipHeight - (220 / 1800.0) * al_get_display_height(game->display) - (340 / 1800.0) * al_get_display_height(game->display) * pos);
	DrawQueue(game, timeline->background, clipX + (25 / 3200.0) * al_get_display_width(game->display), clipY + clipHeight - (100 / 1800.0) * al_get_display_height(game->display) - (340 / 1800.0) * al_get_display_height(game->display) * pos);

	al_use_transform(&game->projection);
}

SYMBOL_INTERNAL void DrawTimelines(struct Game* game) {
	if (!game->_priv.showtimeline) {
		return;
	}
	struct libsuperderpy_list* tmp = game->_priv.timelines;
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

/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Also, ponies.
 */

#include <stdio.h>
#include <allegro5/allegro_ttf.h>
#include "libsuperderpy.h"
#include "internal.h"

SYMBOL_INTERNAL void DrawGamestates(struct Game *game) {
	al_set_target_backbuffer(game->display);
	al_clear_to_color(al_map_rgb(0,0,0));
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			(*tmp->api.Gamestate_Draw)(game, tmp->data);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void LogicGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			(*tmp->api.Gamestate_Logic)(game, tmp->data);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void EventGamestates(struct Game *game, ALLEGRO_EVENT *ev) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			(*tmp->api.Gamestate_ProcessEvent)(game, tmp->data, ev);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void PauseGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			(*tmp->api.Gamestate_Pause)(game, tmp->data);
		}
		tmp = tmp->next;
	}
}


SYMBOL_INTERNAL void ResumeGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			(*tmp->api.Gamestate_Resume)(game, tmp->data);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void DrawConsole(struct Game *game) {
	if (game->_priv.showconsole) {
		al_set_target_backbuffer(game->display);
		ALLEGRO_TRANSFORM trans;
		al_identity_transform(&trans);
		int clipX, clipY, clipWidth, clipHeight;
		al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
		al_use_transform(&trans);

		al_draw_bitmap(game->_priv.console, clipX, clipY, 0);
		double game_time = al_get_time();
		if(game_time - game->_priv.fps_count.old_time >= 1.0) {
			game->_priv.fps_count.fps = game->_priv.fps_count.frames_done / (game_time - game->_priv.fps_count.old_time);
			game->_priv.fps_count.frames_done = 0;
			game->_priv.fps_count.old_time = game_time;
		}
		char sfps[6] = { };
		snprintf(sfps, 6, "%.0f", game->_priv.fps_count.fps);
		DrawTextWithShadow(game->_priv.font_console, al_map_rgb(255,255,255), clipX + clipWidth, clipY, ALLEGRO_ALIGN_RIGHT, sfps);

		al_use_transform(&game->projection);

	}
	game->_priv.fps_count.frames_done++;
}

SYMBOL_INTERNAL void Console_Load(struct Game *game) {
	game->_priv.font_console = NULL;
	game->_priv.console = NULL;
	game->_priv.font_console = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"),al_get_display_height(game->display)*0.025,0 );
	if (al_get_display_height(game->display)*0.025 >= 16) {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/PerfectDOSVGA437.ttf"),16 * ((al_get_display_height(game->display) > 1080) ? 2 : 1) ,0 );
	} else {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), al_get_display_height(game->display)*0.025,0 );
	}
	game->_priv.console = al_create_bitmap((al_get_display_width(game->display) / 320) * 320, al_get_font_line_height(game->_priv.font_console)*5);
	al_set_target_bitmap(game->_priv.console);
	al_clear_to_color(al_map_rgba(0,0,0,80));
	al_set_target_bitmap(al_get_backbuffer(game->display));
}

SYMBOL_INTERNAL void Console_Unload(struct Game *game) {
	al_destroy_font(game->_priv.font_console);
	al_destroy_bitmap(game->_priv.console);
}

SYMBOL_INTERNAL void GamestateProgress(struct Game *game) {
	struct Gamestate *tmp = game->_priv.cur_gamestate.tmp;
	game->_priv.cur_gamestate.p++;
	DrawGamestates(game);
	float progressCount = *(tmp->api.Gamestate_ProgressCount) ? (float)*(tmp->api.Gamestate_ProgressCount) : 1;
	float progress = ((game->_priv.cur_gamestate.p / progressCount) / (float)game->_priv.cur_gamestate.toLoad) + (game->_priv.cur_gamestate.loaded/(float)game->_priv.cur_gamestate.toLoad);
	if (game->config.debug) PrintConsole(game, "[%s] Progress: %d% (%d/%d)", tmp->name, (int)(progress*100), game->_priv.cur_gamestate.p, *(tmp->api.Gamestate_ProgressCount));
	if (tmp->showLoading) (*game->_priv.loading.Draw)(game, game->_priv.loading.data, progress);
	DrawConsole(game);
	if (al_get_time() - game->_priv.cur_gamestate.t >= 1/60.0) {
		al_flip_display();
		game->_priv.cur_gamestate.t = al_get_time();
	}
}

SYMBOL_INTERNAL void* AddGarbage(struct Game *game, void* data) {
	if (!game->_priv.garbage) {
		game->_priv.garbage = malloc(sizeof(struct libsuperderpy_list));
		game->_priv.garbage->data = data;
		game->_priv.garbage->next = NULL;
	} else {
		struct libsuperderpy_list *garbage = malloc(sizeof(struct libsuperderpy_list));
		garbage->next = game->_priv.garbage;
		garbage->data = data;
		game->_priv.garbage = garbage;
	}
	return data;
}

SYMBOL_INTERNAL void ClearGarbage(struct Game *game) {
	while (game->_priv.garbage) {
		free(game->_priv.garbage->data);
		game->_priv.garbage = game->_priv.garbage->next;
	}
}

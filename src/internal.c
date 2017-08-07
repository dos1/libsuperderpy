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

#include <stdio.h>
#include <allegro5/allegro_ttf.h>
#include "internal.h"
#include "libsuperderpy.h"

SYMBOL_INTERNAL void DrawGamestates(struct Game *game) {
	ClearScreen(game);
	al_set_target_backbuffer(game->display);
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Draw)(game, tmp->data);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void LogicGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Logic)(game, tmp->data);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void ReloadGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->loaded) {
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Reload)(game, tmp->data);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void EventGamestates(struct Game *game, ALLEGRO_EVENT *ev) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started) && (!tmp->paused)) {
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_ProcessEvent)(game, tmp->data, ev);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void FreezeGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->started && !tmp->paused) {
			tmp->frozen = true;
			PauseGamestate(game, tmp->name);
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void UnfreezeGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->frozen) {
			ResumeGamestate(game, tmp->name);
			tmp->frozen = false;
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
	DrawTimelines(game);
}

SYMBOL_INTERNAL void Console_Load(struct Game *game) {
	game->_priv.font_console = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"),al_get_display_height(game->display)*0.025,0 );
	if (al_get_display_height(game->display)*0.025 >= 16) {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/PerfectDOSVGA437.ttf"),16 * ((al_get_display_height(game->display) > 1080) ? 2 : 1) ,0 );
	} else {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), al_get_display_height(game->display)*0.025,0 );
	}
	int width = (al_get_display_width(game->display) / game->viewport.width) * game->viewport.width;
	if (!game->viewport.integer_scaling) {
		width = (al_get_display_width(game->display) / (float)game->viewport.width) * game->viewport.width;
	}
	game->_priv.console = CreateNotPreservedBitmap(width, al_get_font_line_height(game->_priv.font_console)*5);
	game->_priv.console_tmp = CreateNotPreservedBitmap(width, al_get_font_line_height(game->_priv.font_console)*5);
	al_set_target_bitmap(game->_priv.console);
	al_clear_to_color(al_map_rgba(0,0,0,80));
	al_set_target_bitmap(al_get_backbuffer(game->display));
}

SYMBOL_INTERNAL void Console_Unload(struct Game *game) {
	al_destroy_font(game->_priv.font_console);
	al_destroy_bitmap(game->_priv.console);
	al_destroy_bitmap(game->_priv.console_tmp);
}

SYMBOL_INTERNAL void GamestateProgress(struct Game *game) {
	struct Gamestate *tmp = game->_priv.tmp_gamestate.tmp;
	game->_priv.tmp_gamestate.p++;
	DrawGamestates(game);
	float progressCount = *(tmp->api->Gamestate_ProgressCount) ? (float)*(tmp->api->Gamestate_ProgressCount) : 1;
	float progress = ((game->_priv.tmp_gamestate.p / progressCount) / (float)game->_priv.tmp_gamestate.toLoad) + (game->_priv.tmp_gamestate.loaded/(float)game->_priv.tmp_gamestate.toLoad);
	if (game->config.debug) PrintConsole(game, "[%s] Progress: %d% (%d/%d)", tmp->name, (int)(progress*100), game->_priv.tmp_gamestate.p, *(tmp->api->Gamestate_ProgressCount));
	if (tmp->showLoading) (*game->_priv.loading.Draw)(game, game->_priv.loading.data, progress);
	DrawConsole(game);
	if (al_get_time() - game->_priv.tmp_gamestate.t >= 1/60.0) {
		al_flip_display();
		game->_priv.tmp_gamestate.t = al_get_time();
	}
}

SYMBOL_INTERNAL struct libsuperderpy_list* AddToList(struct libsuperderpy_list *list, void* data) {
	if (!list) {
		list = malloc(sizeof(struct libsuperderpy_list));
		list->data = data;
		list->next = NULL;
	} else {
		struct libsuperderpy_list *elem = malloc(sizeof(struct libsuperderpy_list));
		elem->next = list;
		elem->data = data;
		list = elem;
	}
	return list;
}

SYMBOL_INTERNAL struct libsuperderpy_list* RemoveFromList(struct libsuperderpy_list **list, bool (*identity)(struct libsuperderpy_list* elem, void* data), void* data) {
	struct libsuperderpy_list *prev = NULL, *tmp = *list, *start = *list;
	void* d = NULL;
	while (tmp) {
		if (identity(tmp, data)) {
			if (prev) {
				prev->next = tmp->next;
				d = tmp->data;
				free(tmp);
				return d;
			} else {
				start = tmp->next;
				d = tmp->data;
				free(tmp);
				*list = start;
				return d;
			}
		}
		prev = tmp;
		tmp = tmp->next;
	}
	return NULL;
}

SYMBOL_INTERNAL void* AddGarbage(struct Game *game, void* data) {
	game->_priv.garbage = AddToList(game->_priv.garbage, data);
	return data;
}

SYMBOL_INTERNAL void ClearGarbage(struct Game *game) {
	struct libsuperderpy_list *tmp;
	while (game->_priv.garbage) {
		free(game->_priv.garbage->data);
		tmp = game->_priv.garbage->next;
		free(game->_priv.garbage);
		game->_priv.garbage = tmp;
	}
}

SYMBOL_INTERNAL void AddTimeline(struct Game *game, struct Timeline *timeline) {
	game->_priv.timelines = AddToList(game->_priv.timelines, timeline);
}

SYMBOL_INTERNAL void RemoveTimeline(struct Game *game, struct Timeline *timeline) {
	struct libsuperderpy_list *tmp = game->_priv.timelines;
	if (tmp->data == timeline) {
		struct libsuperderpy_list *next = tmp->next;
		free(tmp);
		game->_priv.timelines = next;
		return;
	}
	while (tmp->next) {
		if (tmp->next->data == timeline) {
			struct libsuperderpy_list *next = tmp->next->next;
			free(tmp->next);
			tmp->next = next;
			return;
		}
		tmp = tmp->next;
	}
}

SYMBOL_INTERNAL void ClearScreen(struct Game *game) {
	ALLEGRO_TRANSFORM identity;
	int clipX, clipY, clipWidth, clipHeight;
	al_set_target_backbuffer(game->display);
	al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
	al_set_clipping_rectangle(0, 0, al_get_display_width(game->display), al_get_display_height(game->display));
	al_identity_transform(&identity);
	al_use_transform(&identity);
	al_clear_to_color(al_map_rgb(0,0,0));
	al_use_transform(&game->projection);
	al_set_clipping_rectangle(clipX, clipY, clipWidth, clipHeight);
}

SYMBOL_INTERNAL void DrawQueue(struct Game *game, struct TM_Action* queue, int clipX, int clipY) {

	int pos = clipX;

	struct TM_Action *pom = queue;
	while (pom!=NULL) {

		int width = al_get_text_width(game->_priv.font_console, pom->name);
		al_draw_filled_rectangle(pos-(10/3200.0)*al_get_display_width(game->display), clipY, pos+width+(10/3200.0)*al_get_display_width(game->display), clipY+ (60/1800.0)*al_get_display_height(game->display), pom->active ? al_map_rgba(255,255,255,192) : al_map_rgba(0, 0, 0, 0) );
		al_draw_rectangle(pos-(10/3200.0)*al_get_display_width(game->display), clipY, pos+width+(10/3200.0)*al_get_display_width(game->display), clipY+ (60/1800.0)*al_get_display_height(game->display), al_map_rgb(255,255,255), 2);
		al_draw_text(game->_priv.font_console, pom->active ? al_map_rgb(0,0,0) : al_map_rgb(255,255,255), pos, clipY, ALLEGRO_ALIGN_LEFT, pom->name);

		if (pom->delay) {
			al_draw_textf(game->_priv.font_console, al_map_rgb(255,255,255), pos, clipY - (50/1800.0)*al_get_display_height(game->display), ALLEGRO_ALIGN_LEFT, "%d", pom->delay);
		}

		if (strncmp(pom->name, "TM_BackgroundAction", 19) == 0) {
			al_draw_textf(game->_priv.font_console, al_map_rgb(255,255,255), pos, clipY - (50/1800.0)*al_get_display_height(game->display), ALLEGRO_ALIGN_LEFT, "%s", (char*)pom->arguments->next->next->value);
		}

		pos += width + (20/3200.0)*al_get_display_width(game->display);
		pom = pom->next;
	}
}

SYMBOL_INTERNAL void DrawTimeline(struct Game *game, struct Timeline* timeline, int pos) {
	al_set_target_backbuffer(game->display);
	ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	int clipX, clipY, clipWidth, clipHeight;
	al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
	al_use_transform(&trans);

	al_draw_filled_rectangle(clipX, clipY+clipHeight-(340/1800.0)*al_get_display_height(game->display)*(pos+1), clipX + clipWidth, clipY+clipHeight-(340/1800.0)*al_get_display_height(game->display)*pos, al_map_rgba(0,0,0,92));

	al_draw_textf(game->_priv.font_console, al_map_rgb(255,255,255), clipX + clipWidth / 2, clipY+clipHeight-(340/1800.0)*al_get_display_height(game->display)*(pos+1) + (10/1800.0)*al_get_display_height(game->display), ALLEGRO_ALIGN_CENTER, "Timeline: %s", timeline->name);

	DrawQueue(game, timeline->queue, clipX + (25/3200.0)*al_get_display_width(game->display), clipY + clipHeight - (220/1800.0)*al_get_display_height(game->display) - (340/1800.0)*al_get_display_height(game->display)*pos);
	DrawQueue(game, timeline->background, clipX + (25/3200.0)*al_get_display_width(game->display), clipY + clipHeight - (100/1800.0)*al_get_display_height(game->display) - (340/1800.0)*al_get_display_height(game->display)*pos);

	al_use_transform(&game->projection);
}

SYMBOL_INTERNAL void DrawTimelines(struct Game *game) {
	if ((!game->_priv.showconsole) || (!game->_priv.showtimeline)) {
		return;
	}
	struct libsuperderpy_list *tmp = game->_priv.timelines;
	int i=0;
	while (tmp) {
		DrawTimeline(game, tmp->data, i);
		i++;
		tmp = tmp->next;
	}
}

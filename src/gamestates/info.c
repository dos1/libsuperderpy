/*! \file menu.c
 *  \brief Main Menu view.
 */
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
 */
#include <stdio.h>
#include <math.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include "../config.h"
#include "../utils.h"
#include "../timeline.h"
#include "level.h"

#define TILE_SIZE 20
#define MAX_FUN 250.0

int Gamestate_ProgressCount = 4;


void Gamestate_Draw(struct Game *game, struct LevelResources* data) {

	al_set_target_bitmap(al_get_backbuffer(game->display));

	al_clear_to_color(al_map_rgb(3, 213, 255));

	al_draw_bitmap(data->bg,0, 0,0);
	al_draw_bitmap(data->buildings,0, 0,0);

	al_draw_filled_rectangle(0, 0, 320, 180, al_map_rgba(0,0,0,64));


	DrawCharacter(game, data->monster, al_map_rgb(255,255,255), 0);

	al_draw_bitmap(data->meter,0, 0,0);

	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, 19, ALLEGRO_ALIGN_CENTRE, "You're the TICKLE MONSTER!");
	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, 29, ALLEGRO_ALIGN_CENTRE, "You tickle kids to ensure they ");
	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, 39, ALLEGRO_ALIGN_CENTRE, "are raised with proper amounts of fun!");
	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, 54, ALLEGRO_ALIGN_CENTRE, "Use ARROWS to move and SPACE to tickle!");

	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, 104, ALLEGRO_ALIGN_CENTRE, "Beware - if you give them wrong amount");
	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, 114, ALLEGRO_ALIGN_CENTRE, "of fun, they turn into fun hating");
	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, 124, ALLEGRO_ALIGN_CENTRE, "grown ups! Don't let them catch you!");

	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 5, 162, ALLEGRO_ALIGN_LEFT, "Press ENTER to start!");

}


void Gamestate_Logic(struct Game *game, struct LevelResources* data) {

	AnimateCharacter(game, data->monster, 1);

}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {

	struct LevelResources *data = malloc(sizeof(struct LevelResources));

	data->timer = al_create_timer(1);
	al_register_event_source(game->_priv.event_queue, al_get_timer_event_source(data->timer));

	data->timeline = TM_Init(game, "main");
	(*progress)(game);

	data->bg = al_load_bitmap( GetDataFilePath(game, "bg2.png") );
	data->buildings = al_load_bitmap( GetDataFilePath(game, "buildings.png") );
	data->hid = al_load_bitmap( GetDataFilePath(game, "hid.png") );
	data->meter = al_load_bitmap( GetDataFilePath(game, "meter.png") );
	(*progress)(game);

	data->font_title = al_load_ttf_font(GetDataFilePath(game, "fonts/MonkeyIsland.ttf"),game->viewport.height*0.16,0 );
	data->font = al_load_ttf_font(GetDataFilePath(game, "fonts/MonkeyIsland.ttf"),12,0 );
	(*progress)(game);

	data->monster = CreateCharacter(game, "monster");
	RegisterSpritesheet(game, data->monster, "stand");
	RegisterSpritesheet(game, data->monster, "tickle");
	RegisterSpritesheet(game, data->monster, "ticklefail");
	RegisterSpritesheet(game, data->monster, "fail");
	RegisterSpritesheet(game, data->monster, "jump");
	LoadSpritesheets(game, data->monster);
	(*progress)(game);

	al_set_target_backbuffer(game->display);
	return data;
}


void Gamestate_Stop(struct Game *game, struct LevelResources* data) {

}

void Gamestate_Unload(struct Game *game, struct LevelResources* data) {
	al_destroy_bitmap(data->bg);
	al_destroy_bitmap(data->buildings);
	al_destroy_bitmap(data->meter);
	al_destroy_font(data->font_title);
	al_destroy_font(data->font);
	DestroyCharacter(game, data->monster);
}


void StartGame(struct Game *game, struct LevelResources *data) {
	TM_CleanQueue(data->timeline);
	TM_CleanBackgroundQueue(data->timeline);
	ChangeSpritesheet(game, data->monster, "stand");
	ChangeSpritesheet(game, data->suit, "stand");
 }

void Gamestate_Start(struct Game *game, struct LevelResources* data) {
	data->cloud_position = 100;
	SetCharacterPosition(game, data->monster, 150, 73, 0);

	data->score = 0;
	data->time = 0;

	data->lost = false;
	data->tickling = false;
	data->haskid = false;

	data->movedown = false;
	data->moveup = false;

	data->markx = 119;
	data->marky = 2;

	data->soloactive = false;
	data->soloanim = 0;
	data->soloflash = 0;
	data->soloready = 0;

	data->keys.key = 0;
	data->keys.delay = 0;
	data->keys.shift = false;
	data->keys.lastkey = -1;

	data->lightanim=0;

	data->kidSpeed = 0.8;

	data->usage = 0;

	SelectSpritesheet(game, data->monster, "stand");

	//TM_AddQueuedBackgroundAction(data->timeline, &Anim_FixGuitar, TM_AddToArgs(NULL, 1, data), 15*1000, "fix_guitar");
	//TM_AddQueuedBackgroundAction(data->timeline, &Anim_CowLook, TM_AddToArgs(NULL, 1, data), 5*1000, "cow_look");

	data->kids[0] = NULL;
	data->kids[1] = NULL;
	data->kids[2] = NULL;
	data->kids[3] = NULL;
	data->kids[4] = NULL;
	data->kids[5] = NULL;
	data->destroyQueue = NULL;

	data->kidRate = 100;
	data->timeTillNextBadguy = 0;
}

void Gamestate_ProcessEvent(struct Game *game, struct LevelResources* data, ALLEGRO_EVENT *ev) {
	TM_HandleEvent(data->timeline, ev);

 if (ev->type == ALLEGRO_EVENT_KEY_DOWN) {
	 if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
		 SwitchGamestate(game, "info", "level");
		 return;
	 }
	 if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER) {
		 SwitchGamestate(game, "info", "level");
		 return;
	 }
 }

}

void Gamestate_Pause(struct Game *game, struct LevelResources* data) {
	TM_Pause(data->timeline);
}
void Gamestate_Resume(struct Game *game, struct LevelResources* data) {
	TM_Resume(data->timeline);
}
void Gamestate_Reload(struct Game *game, struct LevelResources* data) {}

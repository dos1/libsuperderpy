/*! \file dosowisko.c
 *  \brief Init animation with dosowisko.net logo.
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

#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include "../utils.h"
#include "../timeline.h"
#include "burndt.h"

int Gamestate_ProgressCount = 1;

void Gamestate_Logic(struct Game *game, struct burndtResources* data) {
	data->tick++;
    if (data->tick > 260) {
        SwitchGamestate(game, "burndt", "menu");
    }
}

void Gamestate_Draw(struct Game *game, struct burndtResources* data) {

    float light = 1 - data->tick / 20.0;

    if (data->tick >= 190) {
        light = 1 - (data->tick-190) / 20.0;
    }

    if (light > 1) {
        light = 1;
    }
    if (light < 0) {
        light = 0;
    }

    if (data->tick < 190) {
        al_clear_to_color(al_map_rgb(82 + (255-82) * light, 82 + (255-82) * light, 186 + (255-186) * light));
        al_draw_bitmap(data->bitmap, 0, 0, 0);
    } else {
        al_clear_to_color(al_map_rgb(255 * light, 255 * light, 255 * light));

    }

}

void Gamestate_Start(struct Game *game, struct burndtResources* data) {
    data->tick = 0;
	al_play_sample_instance(data->sound);
}

void Gamestate_ProcessEvent(struct Game *game, struct burndtResources* data, ALLEGRO_EVENT *ev) {
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
        SwitchGamestate(game, "burndt", "menu");
	}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
    struct burndtResources *data = malloc(sizeof(struct burndtResources));
    data->bitmap = al_load_bitmap( GetDataFilePath(game, "burndt.png") );
    (*progress)(game);

    data->sample = al_load_sample( GetDataFilePath(game, "burndt.flac") );
	data->sound = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->sound, game->audio.music);
	al_set_sample_instance_playmode(data->sound, ALLEGRO_PLAYMODE_ONCE);

	return data;
}

void Gamestate_Stop(struct Game *game, struct burndtResources* data) {
	al_stop_sample_instance(data->sound);
}

void Gamestate_Unload(struct Game *game, struct burndtResources* data) {
	al_destroy_sample_instance(data->sound);
	al_destroy_sample(data->sample);
    al_destroy_bitmap(data->bitmap);
	free(data);
}

void Gamestate_Reload(struct Game *game, struct burndtResources* data) {}

void Gamestate_Resume(struct Game *game, struct burndtResources* data) {}
void Gamestate_Pause(struct Game *game, struct burndtResources* data) {}

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
#include "lollipop.h"

int Gamestate_ProgressCount = 4;


bool switchMinigame(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
        if (state == TM_ACTIONSTATE_START) {
            StopGamestate(game, "lollipop");
						game->mediator.next = "riots";
						StartGamestate(game, GetAbstractIsItBonusLevelTimeNowFactoryProvider(game) ? "bonus" : "riots");
        }
        return true;
}

bool theEnd(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
        if (state == TM_ACTIONSTATE_START) {
            StopGamestate(game, "lollipop");
            StartGamestate(game, "theend");
        }
        return true;
}

void Gamestate_Logic(struct Game *game, struct RocketsResources* data) {

    if ((data->spawncounter == data->currentspawn) && ((data->counter < data->timelimit) || (data->lost))) {
				data->dx = (( (rand() / (float)RAND_MAX) - 0.5)/ 200.0) * game->mediator.modificator;
        PrintConsole(game, "DX %f", data->dx);
        data->spawncounter = 0;
    }

    if ((data->lost) && (data->hearts > 80)) {
        AnimateCharacter(game, game->mediator.heart, 1);
        if (game->mediator.heart->pos == 6) {
            al_play_sample_instance(data->jump_sound);
        }
    }

    if (data->lost) {
        data->hearts++;
    }

    if ((data->counter >= data->timelimit) && (!data->lost) && (!data->won)) {
            al_play_sample_instance(data->rainbow_sound);
            data->won = true;
            AdvanceLevel(game, true);
            SelectSpritesheet(game, data->riot, "win");

            TM_AddDelay(data->timeline, 2500);
            TM_AddAction(data->timeline, switchMinigame, NULL, "switchMinigame");
    }

    if (data->won) {
        AnimateCharacter(game, data->riot, 1);
    } else {

    if ((data->currentpos < -0.15) || (data->currentpos > 0.15)) {
        if (!data->lost) {
            AdvanceLevel(game, false);
            data->lost = true;
            SelectSpritesheet(game, data->riot, "end");
            TM_AddDelay(data->timeline, 3500);
            if (game->mediator.lives > 0) {
                TM_AddAction(data->timeline, switchMinigame, NULL, "switchMinigame");
            } else {

                TM_AddAction(data->timeline, theEnd, NULL, "switchMinigame");
            }
            al_play_sample_instance(data->boom_sound);
        }

    }

    }

    int newstate = 0;
    if (data->currentpos < -0.05) {
        newstate = -1;
    } else if (data->currentpos > 0.05) {
        newstate = 1;
    }

    if (data->oldstate != newstate) {
        if (newstate == 0) {
            SelectSpritesheet(game, data->faces, "center");
        } else if (newstate == 1) {
            SelectSpritesheet(game, data->faces, "right");
        } else {
            SelectSpritesheet(game, data->faces, "left");
        }
        data->oldstate = newstate;
    }

    data->counter++;
    data->spawncounter++;

    if (!data->lost) {
        data->currentpos += data->dx;
    }

    TM_Process(data->timeline);
}

void Gamestate_Draw(struct Game *game, struct RocketsResources* data) {

    al_set_target_bitmap(data->pixelator);
    al_clear_to_color(al_map_rgba(128,192,255,0));
    al_draw_bitmap(data->bg, 0, 0, 0);


    if (data->won) {
        //DrawCharacter(game, data->euro, al_map_rgb(255,255,255), 0);
    }

    if ((!data->lost) && (!data->won)) {
        if (data->counter / (float)data->timelimit < 0.5) {
            al_draw_rotated_bitmap(data->earth2, 158, 140, 158, 140, data->currentpos, 0);
        } else if (data->counter / (float)data->timelimit < 0.8) {
            al_draw_rotated_bitmap(data->earth3, 158, 140, 158, 140, data->currentpos, 0);
        } else {
            al_draw_rotated_bitmap(data->earth4, 158, 140, 158, 140, data->currentpos, 0);
        }
        al_draw_bitmap(data->earth, 0, 0, 0);
        DrawCharacter(game, data->faces, al_map_rgb(255,255,255), 0);

        //DrawCharacter(game, data->cursor, al_map_rgb(255,255,255), 0);
    }

    if ((!data->lost) && (!data->won)) {
        al_draw_filled_rectangle(78, 5, 78+164, 5+5, al_map_rgb(155, 142, 142));
        al_draw_filled_rectangle(80, 6, 80+160, 6+3, al_map_rgb(66, 55, 30));
        al_draw_filled_rectangle(80, 6, (data->counter < data->timelimit) ? (80+ 160 * (1 - (data->counter / (float)data->timelimit))) : 80, 6+3, al_map_rgb(225,182, 80));
    }

    al_set_target_backbuffer(game->display);
    al_draw_bitmap(data->pixelator, 0, 0, 0);

    if ((!data->lost) && (!data->won)) {
        al_draw_filled_rectangle(78, 5, 78+164, 5+5, al_map_rgb(155, 142, 142));
        al_draw_filled_rectangle(80, 6, 80+160, 6+3, al_map_rgb(66, 55, 30));
        al_draw_filled_rectangle(80, 6, (data->counter < data->timelimit) ? (80+160 * (1 - (data->counter / (float)data->timelimit))) : 80, 6+3, al_map_rgb(225,182, 80));
    }

    if (data->won) {
        DrawCharacter(game, data->riot, al_map_rgb(255,255,255), 0);
    }

    if (data->lost) {
        al_draw_bitmap(data->currentpos < 0 ? data->clouds : data->combined, 0, 0, 0);
        if (data->hearts > 80) {
            ShowLevelStatistics(game);
        }
    }

    //Gamestate_Logic(game, data);

}

void Gamestate_Start(struct Game *game, struct RocketsResources* data) {

    data->timelimit = 400 * game->mediator.modificator;
		data->spawnspeed = 40 / game->mediator.modificator;
    data->currentspawn = data->spawnspeed;
		data->spawncounter = data->spawnspeed;

    data->lost = false;
    data->won = false;

    data->hearts = 0;

    data->currentpos = 0;
    data->dx = 0;

    SetCharacterPosition(game, data->riot, 0, 0, 0);
    SelectSpritesheet(game, data->riot, "win");
    SelectSpritesheet(game, data->faces, "center");
    SetCharacterPosition(game, data->faces, 0, 0, 0);

    data->oldstate = 0;

    data->counter = 0;

    al_set_mouse_xy(game->display, al_get_display_width(game->display) / 2, al_get_display_height(game->display) / 2);

}

void Gamestate_ProcessEvent(struct Game *game, struct RocketsResources* data, ALLEGRO_EVENT *ev) {
    TM_HandleEvent(data->timeline, ev);
    if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
				SwitchGamestate(game, "lollipop", "theend");
    } else if (ev->type == ALLEGRO_EVENT_MOUSE_AXES) {
        int mousex = ev->mouse.dx / (al_get_display_width(game->display) / 320);

				if (!data->lost) {
					data->currentpos += mousex / 5000.0 / game->mediator.modificator;
				}
        al_set_mouse_xy(game->display, al_get_display_width(game->display) / 2, al_get_display_height(game->display) / 2);
    }
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
    struct RocketsResources *data = malloc(sizeof(struct RocketsResources));

    data->timeline = TM_Init(game, "lollipop");

    data->bg = al_load_bitmap( GetDataFilePath(game, "lollipop/bg.png"));
    (*progress)(game);

    data->earth = al_load_bitmap( GetDataFilePath(game, "lollipop/peoples.png"));
    data->earth2 = al_load_bitmap( GetDataFilePath(game, "lollipop/lollipop.png"));
    data->earth3 = al_load_bitmap( GetDataFilePath(game, "lollipop/lollipop2.png"));
    data->earth4 = al_load_bitmap( GetDataFilePath(game, "lollipop/lollipop3.png"));

    data->clouds = al_load_bitmap( GetDataFilePath(game, "lollipop/przegrywdziew.png"));
    data->combined = al_load_bitmap( GetDataFilePath(game, "lollipop/przegrywchop.png"));

    data->boom_sample = al_load_sample( GetDataFilePath(game, "lollipop/lost.flac") );
    data->jump_sample = al_load_sample( GetDataFilePath(game, "boom.flac") );
		data->rainbow_sample = al_load_sample( GetDataFilePath(game, "lollipop/success.flac") );
    (*progress)(game);

    data->boom_sound = al_create_sample_instance(data->boom_sample);
    al_attach_sample_instance_to_mixer(data->boom_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->boom_sound, ALLEGRO_PLAYMODE_ONCE);

    data->rainbow_sound = al_create_sample_instance(data->rainbow_sample);
    al_attach_sample_instance_to_mixer(data->rainbow_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->rainbow_sound, ALLEGRO_PLAYMODE_ONCE);
		al_set_sample_instance_gain(data->rainbow_sound, 1.25);

    data->jump_sound = al_create_sample_instance(data->jump_sample);
    al_attach_sample_instance_to_mixer(data->jump_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->jump_sound, ALLEGRO_PLAYMODE_ONCE);

    (*progress)(game);

    data->riot = CreateCharacter(game, "loliwin");
    RegisterSpritesheet(game, data->riot, "win");
    RegisterSpritesheet(game, data->riot, "end");
    LoadSpritesheets(game, data->riot);
    (*progress)(game);

    data->faces = CreateCharacter(game, "faces");
    RegisterSpritesheet(game, data->faces, "left");
    RegisterSpritesheet(game, data->faces, "center");
    RegisterSpritesheet(game, data->faces, "right");
    LoadSpritesheets(game, data->faces);
    (*progress)(game);

    data->pixelator = al_create_bitmap(320, 180);
    al_set_target_bitmap(data->pixelator);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_set_target_backbuffer(game->display);

    return data;
}

void Gamestate_Stop(struct Game *game, struct RocketsResources* data) {
    TM_CleanQueue(data->timeline);
}

void Gamestate_Unload(struct Game *game, struct RocketsResources* data) {
    al_destroy_bitmap(data->bg);
    al_destroy_bitmap(data->earth);
    al_destroy_bitmap(data->earth2);
    al_destroy_bitmap(data->earth3);
    al_destroy_bitmap(data->earth4);
    al_destroy_bitmap(data->clouds);
    al_destroy_bitmap(data->combined);
    al_destroy_bitmap(data->pixelator);
    al_destroy_sample_instance(data->boom_sound);
    al_destroy_sample(data->boom_sample);
    DestroyCharacter(game, data->faces);
    DestroyCharacter(game, data->riot);
    // TODO: Destroy all the stuff
    free(data);
}

void Gamestate_Reload(struct Game *game, struct RocketsResources* data) {}

void Gamestate_Resume(struct Game *game, struct RocketsResources* data) {
	TM_Resume(data->timeline);
}
void Gamestate_Pause(struct Game *game, struct RocketsResources* data) {
	TM_Pause(data->timeline);
}

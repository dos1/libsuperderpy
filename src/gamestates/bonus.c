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
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include "../utils.h"
#include "../timeline.h"
#include "bonus.h"

int Gamestate_ProgressCount = 11;

struct Rocket* CreateRocket(struct Game *game, struct RocketsResources* data, struct Rocket* rockets, bool right) {
    struct Rocket *n = malloc(sizeof(struct Rocket));
    n->next = NULL;
    n->prev = NULL;
    n->dx = (right ? -1.5 : 1.5) + (rand() / (float)RAND_MAX) * 0.6 - 0.3;
    n->dy = -2.1 + (rand() / (float)RAND_MAX) * 0.5 - 0.25;
    n->modifier = 0.025;
    n->blown = false;
    n->character = CreateCharacter(game, "rocket");
    n->character->spritesheets = data->rocket_template->spritesheets;
    n->character->shared = true;
		SelectSpritesheet(game, n->character, rand() % 2 ? "usa" : "ru");
		SetCharacterPosition(game, n->character, (data->ru_flag->x + 40) + rand() % 100 - 50, 100, right ? -0.33 : 0.33);
    al_play_sample_instance(data->jump_sound);
    data->currentspawn = data->spawnspeed + (data->spawnspeed * 0.1) * (float)(rand() / (float)RAND_MAX * 2) - (data->spawnspeed * 0.05);

    if (rockets) {
        struct Rocket *tmp = rockets;
        while (tmp->next) {
            tmp=tmp->next;
        }
        tmp->next = n;
        n->prev = tmp;
        return rockets;
    } else {
        return n;
    }
}


void DrawRockets(struct Game *game, struct RocketsResources* data, struct Rocket* rockets) {
    struct Rocket *tmp = rockets;
    while (tmp) {
        DrawCharacter(game, tmp->character, al_map_rgb(255,255,255), 0);

        tmp=tmp->next;
    }
}

bool switchMinigame(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
        if (state == TM_ACTIONSTATE_START) {
						StopGamestate(game, "bonus");
						StartGamestate(game, game->mediator.next);
        }
        return true;
}

bool theEnd(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
        if (state == TM_ACTIONSTATE_START) {
						StopGamestate(game, "bonus");
            StartGamestate(game, "theend");
        }
        return true;
}

void UpdateRockets(struct Game *game, struct RocketsResources *data, struct Rocket* rockets) {
    struct Rocket *tmp = rockets;
    while (tmp) {
        tmp->dy+= tmp->modifier;
        if (!tmp->blown) {
            tmp->dx+= (tmp->dx > 0) ? (-tmp->modifier / 5 + 0.001) : (tmp->modifier / 5 - 0.001);
        }
        MoveCharacter(game, tmp->character, tmp->dx, tmp->dy, tmp->blown ? 0 : ((tmp->dx > 0) ? 0.0166 : -0.0166));
        AnimateCharacter(game, tmp->character, 1);

				if (!tmp->blown) {

						//if (((((tmp->character->y > 90) && (rand() % 4 == 0) && (tmp->dy > 0)))) && (tmp->character->x > -20 && tmp->character->x < 320)) {
						if ((tmp->character->x > 200) && (tmp->character->y < 50) && (tmp->character->x < 260) && (rand() % 4 == 0)) {
								tmp->blown = true;
                tmp->modifier = 0;
                tmp->character->angle = 0;
                tmp->dx = 0;
                tmp->dy = 0;
                SelectSpritesheet(game, tmp->character, "boom");
                MoveCharacter(game, tmp->character, 5, 5, 0);

								data->lizakpowa--;

						}
				}
        tmp=tmp->next;
    }
}

void Gamestate_Logic(struct Game *game, struct RocketsResources* data) {

    if (!data->flash) {
			if ((!data->lost) && (!data->won)) {

        UpdateRockets(game, data, data->rockets_left);
        UpdateRockets(game, data, data->rockets_right);
			}
    } else {
        data->flash--;
    }

    if (data->lost) {
        data->zadyma++;
        if (data->zadyma >= 255) {
            data->zadyma = 255;
        }
    }

    AnimateCharacter(game, data->usa_flag, 1);
    AnimateCharacter(game, data->ru_flag, 1);
    AnimateCharacter(game, data->riot, 1);
    if ((data->lost) && (data->hearts > 80)) {
        AnimateCharacter(game, game->mediator.heart, 1);
        if (game->mediator.heart->pos == 6) {
            al_play_sample_instance(data->boom_sound);
        }
    }


		if ((data->lizakpowa <= 0) && (!data->won) && (!data->lost)) {
			data->won = true;
			al_play_sample_instance(data->rainbow_sound);
			al_play_sample_instance(data->boom_sound);
			data->flash = 4;
			game->mediator.lives++;
			AdvanceLevel(game, true);
			MoveCharacter(game, data->ru_flag, 0, -24, 0);
			SelectSpritesheet(game, data->ru_flag, "lollipop");
			TM_AddDelay(data->timeline, 2500);
			TM_AddAction(data->timeline, switchMinigame, NULL, "switchMinigame");
		}

    if ((data->counter >= data->timelimit) && (!data->lost) && (!data->won)) {

				data->lost = true;
				game->mediator.lives++;
				AdvanceLevel(game, false);

				SelectSpritesheet(game, data->ru_flag, "cry");
				al_play_sample_instance(data->wuwu_sound);

				TM_AddDelay(data->timeline, 1500);
				TM_AddAction(data->timeline, switchMinigame, NULL, "switchMinigame");


    }

		if (!data->won) {
			data->counter++;
		}
    data->spawncounter++;
    data->cloud_rotation += 0.002;
		data->tick ++;

		data->color += 2;
		if (data->color >= 360) {
			data->color = 0;
		}

    TM_Process(data->timeline);
}

void Gamestate_Draw(struct Game *game, struct RocketsResources* data) {

    al_set_target_bitmap(data->pixelator);

		al_draw_tinted_bitmap(data->bg, al_color_hsv(abs(data->color), 0.498, 1.0), 0, 0, 0);


    if (data->won) {
			 // DrawCharacter(game, data->euro, al_map_rgb(255,255,255), 0);
    }

		if (data->lizakpowa > 30) {
			al_draw_rotated_bitmap(data->loli, 200, 200, 370, 65, -1, 0);
		} else if (data->lizakpowa > 15) {
			al_draw_rotated_bitmap(data->loli2, 200, 200, 370, 65, -1, 0);
		} else if (data->lizakpowa > 0) {
			al_draw_rotated_bitmap(data->loli3, 200, 200, 370, 65, -1, 0);
		}

		if ((!data->lost) && (!data->won)) {


		DrawRockets(game, data, data->rockets_left);
    DrawRockets(game, data, data->rockets_right);

		}


        DrawCharacter(game, data->ru_flag, al_map_rgb(255,255,255), 0);

    if ((!data->lost) && (!data->won)) {
        al_draw_filled_rectangle(78, 5, 78+164, 5+5, al_map_rgb(155, 142, 142));
        al_draw_filled_rectangle(80, 6, 80+160, 6+3, al_map_rgb(66, 55, 30));
        al_draw_filled_rectangle(80, 6, (data->counter < data->timelimit) ? (80+ 160 * (1 - (data->counter / (float)data->timelimit))) : 80, 6+3, al_map_rgb(225,182, 80));
    }

    if (data->flash) {
        al_draw_filled_rectangle(0, 0, 320, 180, al_map_rgb(255, 255, 255));
    }

    al_set_target_backbuffer(game->display);
    al_draw_bitmap(data->pixelator, 0, 0, 0);

    if ((data->lost) && (data->hearts > 80)) {
        ShowLevelStatistics(game);
    }

		if (data->tick < 86) {

			if ((data->tick / 11) % 2 == 0) {
					DrawTextWithShadow(game->_priv.font, al_map_rgb(255,255,255), 320 /2 , 45 , ALLEGRO_ALIGN_CENTER, "BONUS LEVEL");
			}
		}

    //Gamestate_Logic(game, data);

}

void Gamestate_Start(struct Game *game, struct RocketsResources* data) {
    data->rockets_left = NULL;
    data->rockets_right = NULL;

		data->tick = 0;

		data->timelimit = 420 / game->mediator.modificator;
    data->spawnspeed = 80 / game->mediator.modificator;
    data->currentspawn = data->spawnspeed;
    data->spawncounter = data->spawnspeed - 20;

    data->lost = false;
    data->won = false;
    data->hearts = 0;

    data->flash = 0;
    data->zadyma = 16;
		data->lizakpowa = 60;

    SetCharacterPosition(game, data->usa_flag, 185, 80, 0);
    SetCharacterPosition(game, data->ru_flag, 25, 80, 0);

		SetCharacterPosition(game, data->cursor, -320, 50, 0);

    SetCharacterPosition(game, data->riot, 0, 0, 0);
    SelectSpritesheet(game, data->riot, "riot");

    SelectSpritesheet(game, data->usa_flag, "legia");
    SelectSpritesheet(game, data->ru_flag, "lech");

    SelectSpritesheet(game, data->cursor, "hand");

    data->counter = 0;
    data->cloud_rotation = 0;

    data->mousemove.bottom = false;
    data->mousemove.top = false;
    data->mousemove.left = false;
    data->mousemove.right = false;

    al_set_mouse_xy(game->display, al_get_display_width(game->display) / 2, al_get_display_height(game->display) / 2);

		al_set_sample_instance_gain(game->muzyczka.instance.bg, 0.0);
		al_set_sample_instance_gain(game->muzyczka.instance.fg, 0.0);
		al_set_sample_instance_gain(data->riot_sound, 1.5);
		al_set_sample_instance_gain(data->wuwu_sound, 1.25);
		al_play_sample_instance(data->riot_sound);
		al_play_sample_instance(data->wuwu_sound);
}

void Gamestate_ProcessEvent(struct Game *game, struct RocketsResources* data, ALLEGRO_EVENT *ev) {
    TM_HandleEvent(data->timeline, ev);
    if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
				SwitchGamestate(game, "bonus", "theend");
    } else if (ev->type == ALLEGRO_EVENT_MOUSE_AXES) {
        int mousex = ev->mouse.x / (al_get_display_width(game->display) / 320);
				int mousey = 80;
        data->mousemove.right = mousex > data->cursor->x;
        data->mousemove.top = mousey < data->cursor->y;
        data->mousemove.left = mousex < data->cursor->x;
        data->mousemove.bottom = mousey > data->cursor->y;
				if ((!data->won)&& (!data->lost)) {
					SetCharacterPosition(game, data->ru_flag, mousex, mousey , 0); // FIXMEEEE!
				}
		} else if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			if ((!data->won)&& (!data->lost)) {
			 data->rockets_left = CreateRocket(game, data, data->rockets_left, false);
			}
		}
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
    struct RocketsResources *data = malloc(sizeof(struct RocketsResources));

		data->timeline = TM_Init(game, "bonus");

		data->bg = al_load_bitmap( GetDataFilePath(game, "bg.png"));

    data->earth = al_load_bitmap( GetDataFilePath(game, "riots/separator.png"));

		data->loli = al_load_bitmap( GetDataFilePath(game, "lollipop/lollipop.png"));
		data->loli2 = al_load_bitmap( GetDataFilePath(game, "lollipop/lollipop2.png"));
		data->loli3 = al_load_bitmap( GetDataFilePath(game, "lollipop/lollipop3.png"));

    (*progress)(game);

    data->rocket_sample = al_load_sample( GetDataFilePath(game, "bump.flac") );
    (*progress)(game);
    data->boom_sample = al_load_sample( GetDataFilePath(game, "boom.flac") );
    (*progress)(game);
    data->jump_sample = al_load_sample( GetDataFilePath(game, "launch.flac") );
    (*progress)(game);
    data->rainbow_sample = al_load_sample( GetDataFilePath(game, "win.flac") );
    (*progress)(game);
		data->wuwu_sample = al_load_sample( GetDataFilePath(game, "warning.flac") );
    (*progress)(game);
		data->riot_sample = al_load_sample( GetDataFilePath(game, "bonus.flac") );
    (*progress)(game);

    data->rocket_sound = al_create_sample_instance(data->rocket_sample);
    al_attach_sample_instance_to_mixer(data->rocket_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->rocket_sound, ALLEGRO_PLAYMODE_ONCE);

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

    data->riot_sound = al_create_sample_instance(data->riot_sample);
		al_attach_sample_instance_to_mixer(data->riot_sound, game->audio.music);
		al_set_sample_instance_playmode(data->riot_sound, ALLEGRO_PLAYMODE_LOOP);

    data->wuwu_sound = al_create_sample_instance(data->wuwu_sample);
    al_attach_sample_instance_to_mixer(data->wuwu_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->wuwu_sound, ALLEGRO_PLAYMODE_ONCE);


    data->cursor = CreateCharacter(game, "cursor");
    RegisterSpritesheet(game, data->cursor, "hand");
    LoadSpritesheets(game, data->cursor);
    (*progress)(game);

    data->pixelator = al_create_bitmap(320, 180);
    al_set_target_bitmap(data->pixelator);
    al_clear_to_color(al_map_rgb(0, 0, 0));

    al_set_target_backbuffer(game->display);

    data->rocket_template = CreateCharacter(game, "rocket");
		RegisterSpritesheet(game, data->rocket_template, "usa");
		RegisterSpritesheet(game, data->rocket_template, "ru");
		RegisterSpritesheet(game, data->rocket_template, "atom");
    RegisterSpritesheet(game, data->rocket_template, "boom");
    RegisterSpritesheet(game, data->rocket_template, "blank");
    LoadSpritesheets(game, data->rocket_template);
    (*progress)(game);

    data->usa_flag = CreateCharacter(game, "kibols");
    RegisterSpritesheet(game, data->usa_flag, "legia");
		RegisterSpritesheet(game, data->usa_flag, "lollipop");
		RegisterSpritesheet(game, data->usa_flag, "cry");
		LoadSpritesheets(game, data->usa_flag);

    data->ru_flag = CreateCharacter(game, "kibols");
    RegisterSpritesheet(game, data->ru_flag, "lech");
		RegisterSpritesheet(game, data->ru_flag, "lollipop");
		RegisterSpritesheet(game, data->ru_flag, "cry");
		LoadSpritesheets(game, data->ru_flag);
    (*progress)(game);

    data->rainbow = CreateCharacter(game, "rainbow");
    RegisterSpritesheet(game, data->rainbow, "shine");
    RegisterSpritesheet(game, data->rainbow, "be");
    LoadSpritesheets(game, data->rainbow);

    data->riot = CreateCharacter(game, "riot");
    RegisterSpritesheet(game, data->riot, "riot");
    LoadSpritesheets(game, data->riot);
    (*progress)(game);

    data->euro = CreateCharacter(game, "euro");
    RegisterSpritesheet(game, data->euro, "euro");
    LoadSpritesheets(game, data->euro);

    return data;
}

void Gamestate_Stop(struct Game *game, struct RocketsResources* data) {
    TM_CleanQueue(data->timeline);
		al_set_sample_instance_gain(game->muzyczka.instance.bg, 1.5);
		al_set_sample_instance_gain(game->muzyczka.instance.fg, 1.5);
		al_stop_sample_instance(data->riot_sound);
}

void Gamestate_Unload(struct Game *game, struct RocketsResources* data) {
    al_destroy_bitmap(data->bg);
    al_destroy_bitmap(data->earth);
    al_destroy_bitmap(data->pixelator);
		al_destroy_bitmap(data->loli);
		al_destroy_bitmap(data->loli2);
		al_destroy_bitmap(data->loli3);
		al_destroy_sample_instance(data->rocket_sound);
    al_destroy_sample_instance(data->boom_sound);
    al_destroy_sample(data->rocket_sample);
    al_destroy_sample(data->boom_sample);
    // TODO: DestroyCharacters
    free(data);
}

void Gamestate_Reload(struct Game *game, struct RocketsResources* data) {}

void Gamestate_Resume(struct Game *game, struct RocketsResources* data) {
	TM_Resume(data->timeline);
}
void Gamestate_Pause(struct Game *game, struct RocketsResources* data) {
	TM_Pause(data->timeline);
}

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
#include "riots.h"

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
    SelectSpritesheet(game, n->character, rand() % 2 ? "rock" : rand() % 2 ? "bottle" : "bottle2");
		SetCharacterPosition(game, n->character, (right ? 250 : 50) + rand() % 90 - 40, 100, right ? -0.33 : 0.33);
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
            StopGamestate(game, "riots");
						game->mediator.next = "rockets";
						StartGamestate(game, GetAbstractIsItBonusLevelTimeNowFactoryProvider(game) ? "bonus" : "rockets");
        }
        return true;
}

bool theEnd(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
        if (state == TM_ACTIONSTATE_START) {
            StopGamestate(game, "riots");
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

            if (((((tmp->character->y > 90) && (rand() % 4 == 0) && (tmp->dy > 0)))) && (tmp->character->x > -20 && tmp->character->x < 320)) {
                tmp->blown = true;
                tmp->modifier = 0;
                tmp->character->angle = 0;
                tmp->dx = 0;
                tmp->dy = 0;
                SelectSpritesheet(game, tmp->character, "boom");
                MoveCharacter(game, tmp->character, 5, 5, 0);

                if (!((tmp->character->x > 140) && (tmp->character->x < 180))) {
                    if (!data->lost) {
                        AdvanceLevel(game, false);

                    data->lost = true;
                    data->flash = 4;
										data->counter = 0;
                    TM_AddDelay(data->timeline, 3500);
                    if (game->mediator.lives > 0) {
                        TM_AddAction(data->timeline, switchMinigame, NULL, "switchMinigame");
                    } else {
                        TM_AddAction(data->timeline, theEnd, NULL, "theEnd");
                    }
                    data->spawnspeed = 10;

                    al_play_sample_instance(data->riot_sound);

                    }
                    al_play_sample_instance(data->boom_sound);

                }

            } else if (tmp->character->x < -20 || tmp->character->x > 320) {
                tmp->blown = true;
                tmp->modifier = 0;
                tmp->character->angle = 0;
                tmp->dx = 0;
                tmp->dy = 0;
                SelectSpritesheet(game, tmp->character, "blank");
                al_play_sample_instance(data->boom_sound);
            }
        }
        tmp=tmp->next;
    }
}

void Gamestate_Logic(struct Game *game, struct RocketsResources* data) {

    if ((data->spawncounter == data->currentspawn) && ((data->counter < data->timelimit) || (data->lost))) {
        if (rand() % 2 == 0) {
            data->rockets_left = CreateRocket(game, data, data->rockets_left, false);
        } else {
            data->rockets_right = CreateRocket(game, data, data->rockets_right, true);
        }
        data->spawncounter = 0;
    }

    if (!data->flash) {
        UpdateRockets(game, data, data->rockets_left);
        UpdateRockets(game, data, data->rockets_right);
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

    if (data->lost) {
        data->hearts++;
    }

    if (data->won) {
        AnimateCharacter(game, data->euro, 1);
    }

    if ((data->counter >= data->timelimit) && (!data->lost) && (!data->won)) {
        bool stillthere = false;
        struct Rocket *tmp = data->rockets_left;
        while (tmp) {
            if (!tmp->blown) {
                stillthere = true;
                break;
            }
            tmp = tmp->next;
        }
        tmp = data->rockets_right;
        while (tmp) {
            if (!tmp->blown) {
                stillthere = true;
                break;
            }
            tmp = tmp->next;
        }
        if (!stillthere) {
            SelectSpritesheet(game, data->euro, "euro");
            SetCharacterPosition(game, data->euro, 0, 0, 0);
            al_play_sample_instance(data->wuwu_sound);
            data->won = true;
            AdvanceLevel(game, true);
            SelectSpritesheet(game, data->usa_flag, "poland");
            SelectSpritesheet(game, data->ru_flag, "poland");
            TM_AddDelay(data->timeline, 2500);
            TM_AddAction(data->timeline, switchMinigame, NULL, "switchMinigame");
        }
    }


    void iterate(struct Rocket *start) {
        struct Rocket *tmp1 = start;
        while (tmp1) {
            if (!tmp1->blown) {
              //  if (CheckCollision(game, data, data->cursor, tmp1->character)) {

                if ( (abs(tmp1->character->y - data->cursor->y) <= 10) &&
                     (((tmp1->character->x <= data->cursor->x + al_get_bitmap_width(data->cursor->bitmap)) && (tmp1->character->x + al_get_bitmap_width(tmp1->character->bitmap) >= data->cursor->x + al_get_bitmap_width(data->cursor->bitmap)))  ||

                     ((tmp1->character->x + al_get_bitmap_width(tmp1->character->bitmap) >= data->cursor->x) && (tmp1->character->x + al_get_bitmap_width(tmp1->character->bitmap) <= data->cursor->x + al_get_bitmap_width(data->cursor->bitmap))) )) {

                    if (tmp1->character->y < data->cursor->y) {
                        tmp1->dx = 0;
                        tmp1->dy = 0;
                        tmp1->modifier = 0;
                        tmp1->blown = true;
                        SelectSpritesheet(game, tmp1->character, "blank");
                    } else if (tmp1->dy < 0) {
                        tmp1->dy *= -1;
                    }

                    al_play_sample_instance(data->rocket_sound);

                }

            }
            tmp1 = tmp1->next;
        }

    }
    iterate(data->rockets_left);
    iterate(data->rockets_right);

    data->counter++;
    data->spawncounter++;
    data->cloud_rotation += 0.002;

    TM_Process(data->timeline);
}

void Gamestate_Draw(struct Game *game, struct RocketsResources* data) {

    al_set_target_bitmap(data->pixelator);

    if (!data->lost) {
        al_draw_bitmap(data->bg, 0, 0, 0);
        //al_draw_bitmap(data->earth2, 0, 0, 0);
        //al_draw_bitmap(data->combined, 0, 0, 0);
    } else if (!data->won) {
        al_draw_tinted_bitmap(data->bg, al_map_rgb(255, 192, 128), 0, 0, 0);
        //al_draw_tinted_bitmap(data->earth2, al_map_rgb(255, 192, 128), 0, 0, 0);
        //al_draw_tinted_bitmap(data->combined, al_map_rgb(255, 192, 128),  0, 0, 0);
    } else {
        al_draw_bitmap(data->bg, 0, 0, 0);
        al_draw_filled_rectangle(0, 0, 320, 180, al_map_rgba(255,255,255, 64));
    }

    if (data->won) {
        DrawCharacter(game, data->euro, al_map_rgb(255,255,255), 0);
    }


    if ((!data->lost) && (!data->won)) {
        DrawCharacter(game, data->cursor, al_map_rgb(255,255,255), 0);
    }

    DrawRockets(game, data, data->rockets_left);
    DrawRockets(game, data, data->rockets_right);

    if (data->lost) {
        DrawCharacter(game, data->riot, al_map_rgb(255,255,255), 0);
    } else {
        DrawCharacter(game, data->usa_flag, al_map_rgb(255,255,255), 1);
        DrawCharacter(game, data->ru_flag, al_map_rgb(255,255,255), 0);
        al_draw_bitmap(data->earth, 5, 50, 0);
    }


    if (data->lost) {
				al_draw_tinted_bitmap(data->clouds, al_map_rgba(data->zadyma, data->zadyma, data->zadyma, data->zadyma), -data->counter / 2, 0, 0);
				al_draw_tinted_bitmap(data->clouds, al_map_rgba(data->zadyma, data->zadyma, data->zadyma, data->zadyma), -data->counter / 2 + 640, 0, 0);
		} else {
				al_draw_tinted_bitmap(data->clouds, al_map_rgba(data->zadyma, data->zadyma, data->zadyma, data->zadyma), -data->counter / 3, 0, 0);
				al_draw_tinted_bitmap(data->clouds, al_map_rgba(data->zadyma, data->zadyma, data->zadyma, data->zadyma), -data->counter / 3 + 640, 0, 0);
		}


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

    //Gamestate_Logic(game, data);

}

void Gamestate_Start(struct Game *game, struct RocketsResources* data) {
    data->rockets_left = NULL;
    data->rockets_right = NULL;

    data->timelimit = 400 * game->mediator.modificator;
    data->spawnspeed = 80 / game->mediator.modificator;
    data->currentspawn = data->spawnspeed;
    data->spawncounter = data->spawnspeed - 20;

    data->lost = false;
    data->won = false;
    data->hearts = 0;

    data->flash = 0;
    data->zadyma = 16;

    SetCharacterPosition(game, data->usa_flag, 185, 80, 0);
    SetCharacterPosition(game, data->ru_flag, 25, 80, 0);

    SetCharacterPosition(game, data->cursor, 320/2, 50, 0);

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

}

void Gamestate_ProcessEvent(struct Game *game, struct RocketsResources* data, ALLEGRO_EVENT *ev) {
    TM_HandleEvent(data->timeline, ev);
    if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
				SwitchGamestate(game, "riots", "theend");
    } else if (ev->type == ALLEGRO_EVENT_MOUSE_AXES) {
        int mousex = ev->mouse.x / (al_get_display_width(game->display) / 320);
        int mousey = 50;
        data->mousemove.right = mousex > data->cursor->x;
        data->mousemove.top = mousey < data->cursor->y;
        data->mousemove.left = mousex < data->cursor->x;
        data->mousemove.bottom = mousey > data->cursor->y;
        SetCharacterPosition(game, data->cursor, mousex, mousey , 0); // FIXMEEEE!
    }
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
    struct RocketsResources *data = malloc(sizeof(struct RocketsResources));

    data->timeline = TM_Init(game, "riots");

    data->bg = al_load_bitmap( GetDataFilePath(game, "riots/bg.png"));

    data->earth = al_load_bitmap( GetDataFilePath(game, "riots/separator.png"));

    data->clouds = al_load_bitmap( GetDataFilePath(game, "riots/fog.png"));
    (*progress)(game);

    data->rocket_sample = al_load_sample( GetDataFilePath(game, "bump.flac") );
    (*progress)(game);
    data->boom_sample = al_load_sample( GetDataFilePath(game, "boom.flac") );
    (*progress)(game);
    data->jump_sample = al_load_sample( GetDataFilePath(game, "launch.flac") );
    (*progress)(game);
    data->rainbow_sample = al_load_sample( GetDataFilePath(game, "win.flac") );
    (*progress)(game);
    data->wuwu_sample = al_load_sample( GetDataFilePath(game, "riots/vuvu.flac") );
    (*progress)(game);
    data->riot_sample = al_load_sample( GetDataFilePath(game, "riots/riot.flac") );
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

    data->jump_sound = al_create_sample_instance(data->jump_sample);
    al_attach_sample_instance_to_mixer(data->jump_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->jump_sound, ALLEGRO_PLAYMODE_ONCE);

    data->riot_sound = al_create_sample_instance(data->riot_sample);
    al_attach_sample_instance_to_mixer(data->riot_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->riot_sound, ALLEGRO_PLAYMODE_ONCE);

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
    RegisterSpritesheet(game, data->rocket_template, "rock");
    RegisterSpritesheet(game, data->rocket_template, "bottle");
    RegisterSpritesheet(game, data->rocket_template, "bottle2");
    RegisterSpritesheet(game, data->rocket_template, "atom");
    RegisterSpritesheet(game, data->rocket_template, "boom");
    RegisterSpritesheet(game, data->rocket_template, "blank");
    LoadSpritesheets(game, data->rocket_template);
    (*progress)(game);

    data->usa_flag = CreateCharacter(game, "kibols");
    RegisterSpritesheet(game, data->usa_flag, "legia");
    RegisterSpritesheet(game, data->usa_flag, "poland");
    LoadSpritesheets(game, data->usa_flag);

    data->ru_flag = CreateCharacter(game, "kibols");
    RegisterSpritesheet(game, data->ru_flag, "lech");
    RegisterSpritesheet(game, data->ru_flag, "poland");
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
}

void Gamestate_Unload(struct Game *game, struct RocketsResources* data) {
    al_destroy_bitmap(data->bg);
    al_destroy_bitmap(data->earth);
    al_destroy_bitmap(data->clouds);
    al_destroy_bitmap(data->pixelator);
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

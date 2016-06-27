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
#include "rockets.h"

int Gamestate_ProgressCount = 9;

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
    SelectSpritesheet(game, n->character, right ? "usa" : "ru");
    SetCharacterPosition(game, n->character, right ? 270 : 48, right ? 136 : 122, right ? -0.33 : 0.33);
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

bool CheckCollision(struct Game *game, struct RocketsResources* data, struct Character* character1, struct Character* character2) {

    bool check(int x, int y, int w, int h, int x2, int y2, int w2, int h2) {

        // HACK LOL I DON'T EVEN
        w /= character2->spritesheet->cols;
        h /= character2->spritesheet->cols;
        w2 /= character1->spritesheet->cols;
        h2 /= character1->spritesheet->cols;

        //if ((((x>=derpyx+0.38*derpyw+derpyo) && (x<=derpyx+0.94*derpyw+derpyo)) || ((x+w>=derpyx+0.38*derpyw+derpyo) && (x+w<=derpyx+0.94*derpyw+derpyo)) || ((x<=derpyx+0.38*derpyw+derpyo) && (x+w>=derpyx+0.94*derpyw+derpyo))) &&
        //        (((y>=derpyy+0.26*derpyh) && (y<=derpyy+0.76*derpyh)) || ((y+h>=derpyy+0.26*derpyh) && (y+h<=derpyy+0.76*derpyh)) || ((y<=derpyy+0.26*derpyh) && (y+h>=derpyy+0.76*derpyh)))) {
        al_draw_rectangle(x, y, x+w, y+h, al_map_rgb(255, 255, 255), 1);
        al_draw_rectangle(x2, y2, x2+w2, y2+h2, al_map_rgb(255, 255, 255), 1);
        if ((((x>=x2) && (x<=x2+w2)) || ((x+w>=x2) && (x+w<=x2+w2))) && (((y>=y2) && (y<=y2+h2)) || ((y+h>=y2) && (y+h<=y2+h2)))) {
            al_draw_rectangle(x, y, x+w, y+h, al_map_rgb(255, 0, 0), 1);
            al_draw_rectangle(x2, y2, x2+w2, y2+h2, al_map_rgb(0, 255, 0), 1);
            return true;
        }

        return false;
    }

    bool pointInside(int x, int y, int w, int h) {

        bool value = false;

        if ((character2->angle <= 1.0 && character2->angle >= 0.0) || (character2->angle < -2.0 && character2->angle > -2.6)) {
            value = value || check(character2->x + 8, character2->y + 15, al_get_bitmap_width(character2->bitmap) - 16, al_get_bitmap_height(character2->bitmap) - 9, x, y, w, h);
            value = value || check(character2->x + 15, character2->y + 7,  al_get_bitmap_width(character2->bitmap) - 9, al_get_bitmap_height(character2->bitmap) - 15, x, y, w, h);
        } else if ((character2->angle >= -1.0 && character2->angle <= 0.0) || (character2->angle > 2.0 && character2->angle < 2.6)) {
            value = value || check(character2->x + 14, character2->y + 15,  al_get_bitmap_width(character2->bitmap) - 9,al_get_bitmap_height(character2->bitmap) - 10, x, y, w, h);
            value = value || check(character2->x + 7, character2->y + 7, al_get_bitmap_width(character2->bitmap) - 16, al_get_bitmap_height(character2->bitmap) - 16, x, y, w, h);
        } else if ((character2->angle > 1.0 && character2->angle < 2.0) || (character2->angle < -1.0 && character2->angle > -2.0)) {
            value = value || check(character2->x + 6, character2->y + 12, al_get_bitmap_width(character2->bitmap) + 10, al_get_bitmap_height(character2->bitmap) - 18, x, y, w, h);
        } else {
            value = value || check(character2->x + 13, character2->y + 5,  al_get_bitmap_width(character2->bitmap) - 18,al_get_bitmap_height(character2->bitmap) + 8, x, y, w, h);
        }

        return value;

    }

    bool value = false;

    if (character1 == data->cursor) {
        return pointInside(character1->x, character1->y, al_get_bitmap_width(character1->bitmap), al_get_bitmap_height(character1->bitmap));
    }

    if ((character1->angle <= 1.0 && character1->angle >= 0.0) || (character1->angle < -2.0 && character1->angle > -2.6)) {
        value = value || pointInside(character1->x + 8, character1->y + 15, al_get_bitmap_width(character1->bitmap) - 16, al_get_bitmap_height(character1->bitmap) - 9);
        value = value || pointInside(character1->x + 15, character1->y + 7, al_get_bitmap_width(character1->bitmap) - 9, al_get_bitmap_height(character1->bitmap) - 15);
    } else if ((character1->angle >= -1.0 && character1->angle <= 0.0) || (character1->angle > 2.0 && character1->angle < 2.6)) {
        value = value || pointInside(character1->x + 14, character1->y + 15, al_get_bitmap_width(character1->bitmap) - 9, al_get_bitmap_height(character1->bitmap) - 10);
        value = value || pointInside(character1->x + 7, character1->y + 7,al_get_bitmap_width(character1->bitmap) - 16,al_get_bitmap_height(character1->bitmap) - 16);
    } else if ((character1->angle > 1.0 && character1->angle < 2.0) || (character1->angle < -1.0 && character1->angle > -2.0)) {
        value = value || pointInside(character1->x + 6, character1->y + 12, al_get_bitmap_width(character1->bitmap) + 10,al_get_bitmap_height(character1->bitmap) - 18);
    } else {
        value = value || pointInside(character1->x + 13, character1->y + 5, al_get_bitmap_width(character1->bitmap) - 18,al_get_bitmap_height(character1->bitmap) + 8);
    }

    return value;
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
            StopGamestate(game, "rockets");
						game->mediator.next = "lollipop";
						StartGamestate(game, GetAbstractIsItBonusLevelTimeNowFactoryProvider(game) ? "bonus" : "lollipop");
        }
        return true;
}

bool theEnd(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
        if (state == TM_ACTIONSTATE_START) {
            StopGamestate(game, "rockets");
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

            bool dupy = false; // nice sos, sos

            void iterate(struct Rocket *start) {
                struct Rocket *tmp1 = start;
                while (tmp1) {
                    if ((tmp != tmp1) && (!tmp1->blown)) {
                        if (CheckCollision(game, data, tmp->character, tmp1->character)) {
                            dupy = true;

                            tmp1->blown = true;
                            tmp1->modifier = 0;
                            tmp1->character->angle = 0;
                            tmp1->dx = 0;
                            tmp1->dy = 0;
                            SelectSpritesheet(game, tmp1->character, "boom");
                            MoveCharacter(game, tmp1->character, 5, 5, 0);
                            //DrawCharacter(game, tmp->character, al_map_rgb(255,0,0), 0);
                            al_play_sample_instance(data->boom_sound);
                        }

                    }
                    tmp1 = tmp1->next;
                }

            }
            iterate(data->rockets_left);
            iterate(data->rockets_right);


						if (((((tmp->character->y > 120) && (rand() % 4 == 0) && (tmp->dy > 0)) || (dupy))) && (tmp->character->x > -20 && tmp->character->x < 310)) {
                tmp->blown = true;
                tmp->modifier = 0;
                tmp->character->angle = 0;
                tmp->dx = 0;
                tmp->dy = 0;
                SelectSpritesheet(game, tmp->character, "boom");
                MoveCharacter(game, tmp->character, 5, 5, 0);

                if (!dupy) {
                    if (!data->lost) {
                        AdvanceLevel(game, false);
                    }
                    data->lost = true;
                    data->flash = 4;
                    TM_AddDelay(data->timeline, 3500);
                    if (game->mediator.lives > 0) {
                        TM_AddAction(data->timeline, switchMinigame, NULL, "switchMinigame");
                    } else {

                        TM_AddAction(data->timeline, theEnd, NULL, "switchMinigame");
                    }
                    data->spawnspeed = 10;
                    al_play_sample_instance(data->atom_sound);
                    SelectSpritesheet(game, data->usa_flag, "brokenusa");
                    SelectSpritesheet(game, data->ru_flag, "brokenru");
                }
                if (!dupy) {
                    SelectSpritesheet(game, tmp->character, "atom");
                    MoveCharacter(game, tmp->character, 0, -11, 0);
                    tmp->character->angle = ((tmp->character->x - 160) / 160) * 0.8;
                }
						} else if (tmp->character->x <= -20 || tmp->character->x >= 310) {
                tmp->blown = true;
                tmp->modifier = 0;
                tmp->character->angle = 0;
                tmp->dx = 0;
                tmp->dy = 0;
                SelectSpritesheet(game, tmp->character, "blank");
            }
        }
        tmp=tmp->next;
    }
}

void Gamestate_Logic(struct Game *game, struct RocketsResources* data) {

    if ((data->spawncounter == data->currentspawn) && ((data->counter < data->timelimit) || (data->lost))) {
				data->next = !data->next;
				if (data->next) {
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
        data->hearts++;
    }

    AnimateCharacter(game, data->usa_flag, 1);
    AnimateCharacter(game, data->ru_flag, 1);
    if ((data->lost) && (data->hearts > 80)) {
        AnimateCharacter(game, game->mediator.heart, 1);
        if (game->mediator.heart->pos == 6) {
            al_play_sample_instance(data->boom_sound);
        }
    }

    if (data->won) {
        AnimateCharacter(game, data->rainbow, 1);
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
            SelectSpritesheet(game, data->rainbow, "shine");
            SetCharacterPosition(game, data->rainbow, 89, 42, 0);
            al_play_sample_instance(data->rainbow_sound);
            data->won = true;
            AdvanceLevel(game, true);
            TM_AddDelay(data->timeline, 2500);
            TM_AddAction(data->timeline, switchMinigame, NULL, "switchMinigame");
        }
    }


    void iterate(struct Rocket *start) {
        struct Rocket *tmp1 = start;
        while (tmp1) {
            if (!tmp1->blown) {
                if (CheckCollision(game, data, data->cursor, tmp1->character)) {

                    if (!tmp1->bumped) {
                        tmp1->bumped = true;
                        al_play_sample_instance(data->rocket_sound);
                        if (data->mousemove.top || data->mousemove.bottom) {
                            tmp1->dy = (data->mousemove.bottom * 2) - 1;
                        }
                        tmp1->dx += (data->mousemove.left * -0.3) + (data->mousemove.right * 0.3);
                        tmp1->character->angle += (tmp1->character->angle < 0) ? 0.25 : -0.25;

                        //PrintConsole(game, "collision TOP %d rIGHT %d", data->mousemove.top, data->mousemove.right);

                        int movex = 0, movey = 0;

                        if (data->mousemove.right) {
                            movex = -3;
                        }
                        if (data->mousemove.left) {
                            movex = 3;
                        }
                        if (data->mousemove.top) {
                            movey = 3;
                        }
                        if (data->mousemove.bottom) {
                            movey =-3;
                        }

                        MoveCharacter(game, data->cursor, movex, movey, 0);
                        al_set_mouse_xy(game->display, data->cursor->x * (al_get_display_width(game->display) / 320), data->cursor->y * (al_get_display_height(game->display) / 180));

                        data->mousemove.top = false;
                        data->mousemove.bottom = false;
                        data->mousemove.left = false;
                        data->mousemove.right = false;

                    }
                } else {
                    tmp1->bumped = false;
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

    al_set_target_bitmap(data->combined);
    al_clear_to_color(al_map_rgba(0,0,0,0));
    al_draw_bitmap(data->earth, 0, 0, 0);
    //al_draw_bitmap(data->clouds, -140, -210, 0);
    //al_set_separate_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ZERO);
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
    al_draw_rotated_bitmap(data->clouds, 250, 250, -90 + 250, -160 + 250 + 20, data->cloud_rotation, 0);
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

    al_set_target_bitmap(data->pixelator);

    if (!data->lost) {
        al_draw_bitmap(data->bg, 0, 0, 0);
        al_draw_bitmap(data->earth2, 0, 0, 0);
        al_draw_bitmap(data->combined, 0, 0, 0);
    } else {
        al_draw_tinted_bitmap(data->bg, al_map_rgb(255, 192, 128), 0, 0, 0);
        //al_draw_tinted_bitmap(data->earth2, al_map_rgb(255, 192, 128), 0, 0, 0);
        al_draw_tinted_bitmap(data->combined, al_map_rgb(255, 192, 128),  0, 0, 0);
    }

    if (data->won) {
        DrawCharacter(game, data->rainbow, al_map_rgb(255,255,255), 0);
    }

    DrawCharacter(game, data->usa_flag, al_map_rgb(255,255,255), 0);
    DrawCharacter(game, data->ru_flag, al_map_rgb(255,255,255), 0);

    if ((!data->lost) && (!data->won)) {
        DrawCharacter(game, data->cursor, al_map_rgb(255,255,255), 0);
    }

    DrawRockets(game, data, data->rockets_left);
    DrawRockets(game, data, data->rockets_right);

    if ((!data->lost) && (!data->won)) {
        al_draw_filled_rectangle(78, 5, 78+164, 5+5, al_map_rgb(155, 142, 142));
        al_draw_filled_rectangle(80, 6, 80+160, 6+3, al_map_rgb(66, 55, 30));
        al_draw_filled_rectangle(80, 6, (data->counter < data->timelimit) ? (80+160 * (1 - (data->counter / (float)data->timelimit))) : 80, 6+3, al_map_rgb(225,182, 80));
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

    data->timelimit = 480 * game->mediator.modificator;
    data->spawnspeed = 60 / game->mediator.modificator;
    data->currentspawn = data->spawnspeed;
    data->spawncounter = data->spawnspeed - 20;

    data->lost = false;
    data->won = false;
    data->hearts = 0;

    data->flash = 0;

		data->next = rand() % 2;

    SetCharacterPosition(game, data->usa_flag, 266, 105, 0);
    SetCharacterPosition(game, data->ru_flag, 13, 103, 0);

    SetCharacterPosition(game, data->cursor, 320 / 2, 180 / 2, 0);

    SelectSpritesheet(game, data->usa_flag, "usa");
    SelectSpritesheet(game, data->ru_flag, "ru");

    SelectSpritesheet(game, data->cursor, "be");

    data->counter = 0;
    data->cloud_rotation = 0;

    al_set_mouse_xy(game->display, al_get_display_width(game->display) / 2, al_get_display_height(game->display) / 2);

    data->mousemove.bottom = false;
    data->mousemove.top = false;
    data->mousemove.left = false;
    data->mousemove.right = false;
}

void Gamestate_ProcessEvent(struct Game *game, struct RocketsResources* data, ALLEGRO_EVENT *ev) {
    TM_HandleEvent(data->timeline, ev);
	if ((ev->type==ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
				SwitchGamestate(game, "rockets", "theend");
    } else if (ev->type == ALLEGRO_EVENT_MOUSE_AXES) {
        int mousex = ev->mouse.x / (al_get_display_width(game->display) / 320);
        int mousey = ev->mouse.y / (al_get_display_height(game->display) / 180);
        data->mousemove.right = mousex > data->cursor->x;
        data->mousemove.top = mousey < data->cursor->y;
        data->mousemove.left = mousex < data->cursor->x;
        data->mousemove.bottom = mousey > data->cursor->y;
        SetCharacterPosition(game, data->cursor, mousex, mousey , 0); // FIXMEEEE!
    }
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {
    struct RocketsResources *data = malloc(sizeof(struct RocketsResources));

    data->timeline = TM_Init(game, "rockets");

    data->bg = al_load_bitmap( GetDataFilePath(game, "rockets/bg.png"));

    data->earth = al_load_bitmap( GetDataFilePath(game, "rockets/earth.png"));
    data->earth2 = al_load_bitmap( GetDataFilePath(game, "rockets/earth2.png"));

    data->clouds = al_load_bitmap( GetDataFilePath(game, "rockets/clouds.png"));
    (*progress)(game);

    data->rocket_sample = al_load_sample( GetDataFilePath(game, "bump.flac") );
    (*progress)(game);
    data->boom_sample = al_load_sample( GetDataFilePath(game, "boom.flac") );
    (*progress)(game);
    data->atom_sample = al_load_sample( GetDataFilePath(game, "rockets/atom.flac") );
    (*progress)(game);
    data->jump_sample = al_load_sample( GetDataFilePath(game, "launch.flac") );
    (*progress)(game);
    data->rainbow_sample = al_load_sample( GetDataFilePath(game, "win.flac") );
    (*progress)(game);

    data->rocket_sound = al_create_sample_instance(data->rocket_sample);
    al_attach_sample_instance_to_mixer(data->rocket_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->rocket_sound, ALLEGRO_PLAYMODE_ONCE);

    data->atom_sound = al_create_sample_instance(data->atom_sample);
    al_attach_sample_instance_to_mixer(data->atom_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->atom_sound, ALLEGRO_PLAYMODE_ONCE);

    data->boom_sound = al_create_sample_instance(data->boom_sample);
    al_attach_sample_instance_to_mixer(data->boom_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->boom_sound, ALLEGRO_PLAYMODE_ONCE);

    data->jump_sound = al_create_sample_instance(data->jump_sample);
    al_attach_sample_instance_to_mixer(data->jump_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->jump_sound, ALLEGRO_PLAYMODE_ONCE);

    data->rainbow_sound = al_create_sample_instance(data->rainbow_sample);
    al_attach_sample_instance_to_mixer(data->rainbow_sound, game->audio.fx);
    al_set_sample_instance_playmode(data->rainbow_sound, ALLEGRO_PLAYMODE_ONCE);

    data->cursor = CreateCharacter(game, "cursor");
    RegisterSpritesheet(game, data->cursor, "be");
    LoadSpritesheets(game, data->cursor);
    (*progress)(game);

    data->pixelator = al_create_bitmap(320, 180);
    al_set_target_bitmap(data->pixelator);
    al_clear_to_color(al_map_rgb(0, 0, 0));

    data->combined = al_create_bitmap(320, 180);
    al_set_target_bitmap(data->combined);
    al_clear_to_color(al_map_rgba(0, 0, 0, 0));

    al_set_target_backbuffer(game->display);

    data->rocket_template = CreateCharacter(game, "rocket");
    RegisterSpritesheet(game, data->rocket_template, "usa");
    RegisterSpritesheet(game, data->rocket_template, "ru");
    RegisterSpritesheet(game, data->rocket_template, "atom");
    RegisterSpritesheet(game, data->rocket_template, "boom");
    RegisterSpritesheet(game, data->rocket_template, "blank");
    LoadSpritesheets(game, data->rocket_template);
    (*progress)(game);

    data->usa_flag = CreateCharacter(game, "flag");
    RegisterSpritesheet(game, data->usa_flag, "usa");
    RegisterSpritesheet(game, data->usa_flag, "brokenusa");
    LoadSpritesheets(game, data->usa_flag);

    data->ru_flag = CreateCharacter(game, "flag");
    RegisterSpritesheet(game, data->ru_flag, "ru");
    RegisterSpritesheet(game, data->ru_flag, "brokenru");
    LoadSpritesheets(game, data->ru_flag);
    (*progress)(game);

    data->rainbow = CreateCharacter(game, "rainbow");
    RegisterSpritesheet(game, data->rainbow, "shine");
    RegisterSpritesheet(game, data->rainbow, "be");
    LoadSpritesheets(game, data->rainbow);

    return data;
}

void Gamestate_Stop(struct Game *game, struct RocketsResources* data) {
    TM_CleanQueue(data->timeline);
}

void Gamestate_Unload(struct Game *game, struct RocketsResources* data) {
    al_destroy_bitmap(data->bg);
    al_destroy_bitmap(data->earth);
    al_destroy_bitmap(data->earth2);
    al_destroy_bitmap(data->clouds);
    al_destroy_bitmap(data->combined);
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

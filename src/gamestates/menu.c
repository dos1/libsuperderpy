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
#include "menu.h"

int Gamestate_ProgressCount = 4;

void About(struct Game *game, struct MenuResources* data) {
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_use_transform(&trans);

    if (!game->_priv.font_bsod) {
        game->_priv.font_bsod = al_create_builtin_font();
    }

    al_set_target_backbuffer(game->display);
    al_clear_to_color(al_map_rgb(0,0,170));

    char *header = "MEDIATOR";

    al_draw_filled_rectangle(al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header)/2 - 4, (int)(al_get_display_height(game->display) * 0.32), 4 + al_get_display_width(game->display)/2 + al_get_text_width(game->_priv.font_bsod, header)/2, (int)(al_get_display_height(game->display) * 0.32) + al_get_font_line_height(game->_priv.font_bsod), al_map_rgb(170,170,170));

    al_draw_text(game->_priv.font_bsod, al_map_rgb(0, 0, 170), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32), ALLEGRO_ALIGN_CENTRE, header);

    char *header2 = "A fatal exception 0xD3RP has occured at 0028:M00F11NZ in GST SD(01) +";

    al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+2*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, header2);
    al_draw_textf(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+3*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "%p and system just doesn't know what went wrong.", (void*)game);

    al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+5*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, 	"About screen not implemented!");
    al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+7*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, 	"Made in the theater by Sebastian Krzyszkowiak and Konrad Burandt");

    al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+9*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Press any key to terminate this error.");
    al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+10*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Press any key to destroy all muffins in the world.");
    al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+11*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Just kidding, please press any key anyway.");

    al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+13*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, "Press any key to continue _");

    al_use_transform(&game->projection);
}

void DrawMenuState(struct Game *game, struct MenuResources *data) {

    const ALLEGRO_TRANSFORM *tmp_trans = al_get_current_transform();
    ALLEGRO_TRANSFORM trans, cur_trans;
    al_copy_transform(&trans, tmp_trans);
    al_copy_transform(&cur_trans, tmp_trans);
    al_translate_transform(&trans, (al_get_display_width(game->display) / 320.0) * 80, (al_get_display_height(game->display) / 260.0) * ((180-data->screen_pos) - 48));
    al_use_transform(&trans);

    ALLEGRO_FONT *font = data->font;
    char* text = malloc(255*sizeof(char));
    struct ALLEGRO_COLOR color;
    switch (data->menustate) {
        case MENUSTATE_MAIN:
        case MENUSTATE_HIDDEN:
            if (!data->invisible) {
                DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "Start game");
                DrawTextWithShadow(font, data->selected==1 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, "Options");
                DrawTextWithShadow(font, data->selected==2 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.7, ALLEGRO_ALIGN_CENTRE, "About");
                DrawTextWithShadow(font, data->selected==3 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Exit");
            }
            break;
        case MENUSTATE_OPTIONS:
            DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "Video settings");
            DrawTextWithShadow(font, data->selected==1 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, "Audio settings");
            DrawTextWithShadow(font, data->selected==3 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Back");
            break;
        case MENUSTATE_AUDIO:
            if (game->config.music) snprintf(text, 255, "Music volume: %d0%%", game->config.music);
            else sprintf(text, "Music disabled");
            DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, text);
            if (game->config.fx) snprintf(text, 255, "Effects volume: %d0%%", game->config.fx);
            else sprintf(text, "Effects disabled");
            DrawTextWithShadow(font, data->selected==1 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, text);
            DrawTextWithShadow(font, data->selected==3 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Back");
            break;
        case MENUSTATE_ABOUT:
            al_use_transform(&cur_trans);
            About(game, data);
            break;
        case MENUSTATE_VIDEO:
            if (data->options.fullscreen) {
                sprintf(text, "Fullscreen: yes");
                color = al_map_rgba(0,0,0,128);
            }
            else {
                sprintf(text, "Fullscreen: no");
                color = data->selected==1 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255);
            }
            DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, text);
            sprintf(text, "Resolution: %dx", data->options.resolution);
            DrawTextWithShadow(font, color, game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, text);
            DrawTextWithShadow(font, data->selected==3 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Back");
            break;
        default:
            data->selected=0;
            DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,222, 120) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "Not implemented yet");
            break;
    }
    free(text);

    al_use_transform(&cur_trans);
}



void ChangeMenuState(struct Game *game, struct MenuResources* data, enum menustate_enum state) {
    data->menustate=state;
    data->selected=0;
    if (state == MENUSTATE_HIDDEN) {
        al_set_sample_instance_gain(game->muzyczka.instance.fg, 0.0);
    } else {
        al_set_sample_instance_gain(game->muzyczka.instance.fg, 1.5);
    }
    PrintConsole(game, "menu state changed %d", state);
}

void Gamestate_Draw(struct Game *game, struct MenuResources* data) {

    al_set_target_bitmap(al_get_backbuffer(game->display));

    al_clear_to_color(al_map_rgb(3, 213, 255));

    al_draw_bitmap(data->bg, 0, 0, 0);
    al_draw_bitmap(data->monster, data->monster_pos, 10, 0);
    if (!data->starting) {
        al_draw_bitmap(data->title, 12, 25 - (pow(sin(data->title_pos), 2) * 16) - data->screen_pos, 0);
    }

    if ((data->menustate == MENUSTATE_HIDDEN) && (!data->starting)) {
        DrawTextWithShadow(game->_priv.font, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.9, ALLEGRO_ALIGN_CENTRE, "Press RETURN");
    }

    DrawMenuState(game, data);
}


void Gamestate_Logic(struct Game *game, struct MenuResources* data) {

    data->title_pos += 0.05;

    if (data->starting) {
        data->monster_pos -= 6;

        if (data->monster_pos < -202) {
            data->starting = false;
            LoadGamestate(game, "theend");
            LoadGamestate(game, "info");
            LoadGamestate(game, "rockets");
            LoadGamestate(game, "riots");
            LoadGamestate(game, "lollipop");
						LoadGamestate(game, "bonus");
						StartGamestate(game, "info");
            StopGamestate(game, "menu");
        }

    } else {
        data->monster_pos += 6;
        if (data->monster_pos > -40) {
            data->monster_pos = -40;
        }
    }

    if (data->menustate == MENUSTATE_HIDDEN) {
        data->screen_pos -= (180 - data->screen_pos) / 4 + 1;
        if (data->screen_pos < 0) {
            data->screen_pos = 0;
            data->invisible = false;
        }
    } else {
        data->invisible = false;
        data->screen_pos += (data->screen_pos) / 4 + 1;
        if (data->screen_pos > 180) {
            data->screen_pos = 180;
        }
    }

}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {

    struct MenuResources *data = malloc(sizeof(struct MenuResources));

    data->options.fullscreen = game->config.fullscreen;
    data->options.fps = game->config.fps;
    data->options.width = game->config.width;
    data->options.height = game->config.height;
    data->options.resolution = game->config.width / 320;
    if (game->config.height / 180 < data->options.resolution) data->options.resolution = game->config.height / 180;
    (*progress)(game);

    data->bg = al_load_bitmap( GetDataFilePath(game, "bg.png") );
    data->monster = al_load_bitmap( GetDataFilePath(game, "light.png") );
    data->title = al_load_bitmap( GetDataFilePath(game, "title.png") );
    (*progress)(game);

    game->muzyczka.sample.fg = al_load_sample( GetDataFilePath(game, "song-fg.flac") );
    game->muzyczka.sample.bg = al_load_sample( GetDataFilePath(game, "song-bg.flac") );
    game->muzyczka.sample.drums = al_load_sample( GetDataFilePath(game, "song-drums.flac") );
    data->click_sample = al_load_sample( GetDataFilePath(game, "click.flac") );
    (*progress)(game);

    game->muzyczka.instance.fg = al_create_sample_instance(game->muzyczka.sample.fg);
    al_attach_sample_instance_to_mixer(game->muzyczka.instance.fg, game->audio.music);
    al_set_sample_instance_playmode(game->muzyczka.instance.fg, ALLEGRO_PLAYMODE_LOOP);

    game->muzyczka.instance.bg = al_create_sample_instance(game->muzyczka.sample.bg);
    al_attach_sample_instance_to_mixer(game->muzyczka.instance.bg, game->audio.music);
    al_set_sample_instance_playmode(game->muzyczka.instance.bg, ALLEGRO_PLAYMODE_LOOP);

    game->muzyczka.instance.drums = al_create_sample_instance(game->muzyczka.sample.drums);
    al_attach_sample_instance_to_mixer(game->muzyczka.instance.drums, game->audio.music);
    al_set_sample_instance_playmode(game->muzyczka.instance.drums, ALLEGRO_PLAYMODE_LOOP);

    data->click = al_create_sample_instance(data->click_sample);
    al_attach_sample_instance_to_mixer(data->click, game->audio.fx);
    al_set_sample_instance_playmode(data->click, ALLEGRO_PLAYMODE_ONCE);

    if (!data->click_sample){
        fprintf(stderr, "Audio clip sample not loaded!\n" );
        exit(-1);
    }
    (*progress)(game);

    data->font = al_load_ttf_font(GetDataFilePath(game, "fonts/MonkeyIsland.ttf"),game->viewport.height*0.05,0 );

    al_set_target_backbuffer(game->display);
    return data;
}


void Gamestate_Stop(struct Game *game, struct MenuResources* data) {
    //al_stop_sample_instance(data->music);
}

void Gamestate_Unload(struct Game *game, struct MenuResources* data) {
    //al_stop_sample_instance(data->music);
    al_destroy_bitmap(data->bg);
    al_destroy_bitmap(data->title);
    al_destroy_bitmap(data->monster);
    al_destroy_font(data->font);
    //al_destroy_sample_instance(data->music);
    al_destroy_sample_instance(data->click);
    //al_destroy_sample(data->sample);
    al_destroy_sample(data->click_sample);
}


void StartGame(struct Game *game, struct MenuResources *data) {

    game->mediator.lives = 3;
    game->mediator.score = 0;
        game->mediator.modificator = 0.9756;

    ChangeMenuState(game,data,MENUSTATE_HIDDEN);
    al_set_sample_instance_gain(game->muzyczka.instance.drums, 0.0);
    al_set_sample_instance_gain(game->muzyczka.instance.fg, 1.5);
    al_set_sample_instance_gain(game->muzyczka.instance.bg, 1.5);
    data->starting = true;
}


void Gamestate_Start(struct Game *game, struct MenuResources* data) {

    al_ungrab_mouse();
    if (!game->config.fullscreen) al_show_mouse_cursor(game->display);

    data->title_pos = 0;
    data->screen_pos = 180;
    data->invisible = true;
    data->monster_pos = -202;
    data->starting = false;

    ChangeMenuState(game,data,MENUSTATE_HIDDEN);
    al_play_sample_instance(game->muzyczka.instance.fg);
    al_play_sample_instance(game->muzyczka.instance.bg);
    al_play_sample_instance(game->muzyczka.instance.drums);
    al_set_sample_instance_gain(game->muzyczka.instance.fg, 0.0);
    al_set_sample_instance_gain(game->muzyczka.instance.bg, 1.5);
    al_set_sample_instance_gain(game->muzyczka.instance.drums, 1.5);


}

void Gamestate_ProcessEvent(struct Game *game, struct MenuResources* data, ALLEGRO_EVENT *ev) {

    if ((data->menustate == MENUSTATE_ABOUT) && (ev->type == ALLEGRO_EVENT_KEY_DOWN)) {
        ChangeMenuState(game, data, MENUSTATE_MAIN);
        return;
    }

    if (ev->type != ALLEGRO_EVENT_KEY_DOWN) return;

    if (data->starting) return;

    if (ev->keyboard.keycode==ALLEGRO_KEY_UP) {
        data->selected--;
        if ((data->selected == 2) && ((data->menustate==MENUSTATE_VIDEO) || (data->menustate==MENUSTATE_OPTIONS) || (data->menustate==MENUSTATE_AUDIO))) {
            data->selected --;
        }
        if ((data->menustate==MENUSTATE_VIDEO) && (data->selected==1) && (data->options.fullscreen)) data->selected--;
        al_play_sample_instance(data->click);
    } else if (ev->keyboard.keycode==ALLEGRO_KEY_DOWN) {
        data->selected++;
        if ((data->menustate==MENUSTATE_VIDEO) && (data->selected==1) && (data->options.fullscreen)) data->selected++;
        if ((data->selected == 2) && ((data->menustate==MENUSTATE_VIDEO) || (data->menustate==MENUSTATE_OPTIONS) || (data->menustate==MENUSTATE_AUDIO))) {
            data->selected ++;
        }


        al_play_sample_instance(data->click);
    }

    if (ev->keyboard.keycode==ALLEGRO_KEY_ENTER) {
        char *text;
        al_play_sample_instance(data->click);
        switch (data->menustate) {
            case MENUSTATE_MAIN:
                switch (data->selected) {
                    case 0:
                        StartGame(game, data);
                        break;
                    case 1:
                        ChangeMenuState(game,data,MENUSTATE_OPTIONS);
                        break;
                    case 2:
                        ChangeMenuState(game,data,MENUSTATE_ABOUT);
                        break;
                    case 3:
                        UnloadGamestate(game, "menu");
                        break;
                }
                break;
            case MENUSTATE_HIDDEN:
                ChangeMenuState(game,data,MENUSTATE_MAIN);
                break;
            case MENUSTATE_AUDIO:
                text = malloc(255*sizeof(char));
                switch (data->selected) {
                    case 0:
                        game->config.music--;
                        if (game->config.music<0) game->config.music=10;
                        snprintf(text, 255, "%d", game->config.music);
                        SetConfigOption(game, "SuperDerpy", "music", text);
                        al_set_mixer_gain(game->audio.music, game->config.music/10.0);
                        break;
                    case 1:
                        game->config.fx--;
                        if (game->config.fx<0) game->config.fx=10;
                        snprintf(text, 255, "%d", game->config.fx);
                        SetConfigOption(game, "SuperDerpy", "fx", text);
                        al_set_mixer_gain(game->audio.fx, game->config.fx/10.0);
                        break;
                    case 2:
                        game->config.voice--;
                        if (game->config.voice<0) game->config.voice=10;
                        snprintf(text, 255, "%d", game->config.voice);
                        SetConfigOption(game, "SuperDerpy", "voice", text);
                        al_set_mixer_gain(game->audio.voice, game->config.voice/10.0);
                        break;
                    case 3:
                        ChangeMenuState(game,data,MENUSTATE_OPTIONS);
                        break;
                }
                free(text);
                break;
            case MENUSTATE_OPTIONS:
                switch (data->selected) {
                    case 0:
                        ChangeMenuState(game,data,MENUSTATE_VIDEO);
                        break;
                    case 1:
                        ChangeMenuState(game,data,MENUSTATE_AUDIO);
                        break;
                    case 3:
                        ChangeMenuState(game,data,MENUSTATE_MAIN);
                        break;
                    default:
                        break;
                }
                break;
            case MENUSTATE_VIDEO:
                switch (data->selected) {
                    case 0:
                        data->options.fullscreen = !data->options.fullscreen;
                        if (data->options.fullscreen)
                            SetConfigOption(game, "SuperDerpy", "fullscreen", "1");
                        else
                            SetConfigOption(game, "SuperDerpy", "fullscreen", "0");
                        al_set_display_flag(game->display, ALLEGRO_FULLSCREEN_WINDOW, data->options.fullscreen);
                        game->config.fullscreen = data->options.fullscreen;
                        if (!data->options.fullscreen) {
                            al_show_mouse_cursor(game->display);
                        } else {
                            al_hide_mouse_cursor(game->display);
                        }
                        SetupViewport(game);
                        PrintConsole(game, "Fullscreen toggled");
                        break;
                    case 1:
                        data->options.resolution++;

                        int max = 0, i = 0;

                        for (i=0; i < al_get_num_video_adapters(); i++) {
                            ALLEGRO_MONITOR_INFO aminfo;
                            al_get_monitor_info(i , &aminfo);
                            int desktop_width = aminfo.x2 - aminfo.x1 + 1;
                            int desktop_height = aminfo.y2 - aminfo.y1 + 1;
                            int localmax = desktop_width / 320;
                            if (desktop_height / 180 < localmax) localmax = desktop_height / 180;
                            if (localmax > max) max = localmax;
                        }


                        if (data->options.resolution > max) data->options.resolution = 1;
                        text = malloc(255*sizeof(char));
                        snprintf(text, 255, "%d", data->options.resolution * 320);
                        SetConfigOption(game, "SuperDerpy", "width", text);
                        snprintf(text, 255, "%d", data->options.resolution * 180);
                        SetConfigOption(game, "SuperDerpy", "height", text);
                        free(text);
                        al_resize_display(game->display, data->options.resolution * 320, data->options.resolution * 180);

                        if ((al_get_display_width(game->display) < (data->options.resolution * 320)) || (al_get_display_height(game->display) < (data->options.resolution * 180))) {
                            SetConfigOption(game, "SuperDerpy", "width", "320");
                            SetConfigOption(game, "SuperDerpy", "height", "180");
                            data->options.resolution = 1;
                            al_resize_display(game->display, 320, 180);
                        }

                        SetupViewport(game);
                        PrintConsole(game, "Resolution changed");
                        break;
                    case 3:
                        ChangeMenuState(game,data,MENUSTATE_OPTIONS);
                        break;
                    default:
                        break;
                }
                break;
            case MENUSTATE_ABOUT:
                break;
            default:
                UnloadGamestate(game, "menu");
                return;
                break;
        }
    } else if (ev->keyboard.keycode==ALLEGRO_KEY_ESCAPE) {
        switch (data->menustate) {
            case MENUSTATE_OPTIONS:
                ChangeMenuState(game,data,MENUSTATE_MAIN);
                break;
            case MENUSTATE_ABOUT:
                ChangeMenuState(game,data,MENUSTATE_MAIN);
                break;
            case MENUSTATE_HIDDEN:
                UnloadGamestate(game, "menu");
                break;
            case MENUSTATE_VIDEO:
                ChangeMenuState(game,data,MENUSTATE_OPTIONS);
                break;
            case MENUSTATE_AUDIO:
                ChangeMenuState(game,data,MENUSTATE_OPTIONS);
                break;
            default:
                ChangeMenuState(game,data,MENUSTATE_HIDDEN);
                data->selected = -1;
                data->title_pos = 0;
                return;
        }
    }

    if (data->selected==-1) data->selected=3;
    if (data->selected==4) data->selected=0;
    return;
}

void Gamestate_Pause(struct Game *game, struct MenuResources* data) {}
void Gamestate_Resume(struct Game *game, struct MenuResources* data) {}
void Gamestate_Reload(struct Game *game, struct MenuResources* data) {}

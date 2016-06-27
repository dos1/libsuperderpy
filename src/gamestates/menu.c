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

#define SOLO_MIN 20

int Gamestate_ProgressCount = 5;

void About(struct Game *game, struct MenuResources* data) {
	ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	al_use_transform(&trans);

	if (!game->_priv.font_bsod) {
		game->_priv.font_bsod = al_create_builtin_font();
	}

	al_set_target_backbuffer(game->display);
	al_clear_to_color(al_map_rgb(0,0,170));

	char *header = "RADIO EDIT";

	al_draw_filled_rectangle(al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header)/2 - 4, (int)(al_get_display_height(game->display) * 0.32), 4 + al_get_display_width(game->display)/2 + al_get_text_width(game->_priv.font_bsod, header)/2, (int)(al_get_display_height(game->display) * 0.32) + al_get_font_line_height(game->_priv.font_bsod), al_map_rgb(170,170,170));

	al_draw_text(game->_priv.font_bsod, al_map_rgb(0, 0, 170), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32), ALLEGRO_ALIGN_CENTRE, header);

	char *header2 = "A fatal exception 0xD3RP has occured at 0028:M00F11NZ in GST SD(01) +";

	al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+2*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, header2);
	al_draw_textf(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+3*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "%p and system just doesn't know what went wrong.", (void*)game);

	al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+5*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, 	"About screen not implemented!");
	al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+6*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, 	"See http://dosowisko.net/radioedit/");
	al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+7*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, 	"Made for Ludum Dare 32");

	al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+9*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Press any key to terminate this error.");
	al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+10*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Press any key to destroy all muffins in the world.");
	al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+11*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Just kidding, please press any key anyway.");

	al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+13*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, "Press any key to continue _");

	al_use_transform(&game->projection);
}

void DrawMenuState(struct Game *game, struct MenuResources *data) {
	ALLEGRO_FONT *font = data->font;
	char* text = malloc(255*sizeof(char));
	struct ALLEGRO_COLOR color;
	switch (data->menustate) {
		case MENUSTATE_MAIN:
			DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "Start game");
			DrawTextWithShadow(font, data->selected==1 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, "Options");
			DrawTextWithShadow(font, data->selected==2 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.7, ALLEGRO_ALIGN_CENTRE, "About");
			DrawTextWithShadow(font, data->selected==3 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Exit");
			break;
		case MENUSTATE_OPTIONS:
			DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "Video settings");
			DrawTextWithShadow(font, data->selected==1 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, "Audio settings");
			DrawTextWithShadow(font, data->selected==3 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Back");
			break;
		case MENUSTATE_AUDIO:
			if (game->config.music) snprintf(text, 255, "Music volume: %d0%%", game->config.music);
			else sprintf(text, "Music disabled");
			DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, text);
			if (game->config.fx) snprintf(text, 255, "Effects volume: %d0%%", game->config.fx);
			else sprintf(text, "Effects disabled");
			DrawTextWithShadow(font, data->selected==1 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, text);
			DrawTextWithShadow(font, data->selected==3 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Back");
			break;
		case MENUSTATE_ABOUT:
			About(game, data);
			break;
		case MENUSTATE_VIDEO:
			if (data->options.fullscreen) {
				sprintf(text, "Fullscreen: yes");
				color = al_map_rgba(0,0,0,128);
			}
			else {
				sprintf(text, "Fullscreen: no");
				color = data->selected==1 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255);
			}
			DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, text);
			sprintf(text, "Resolution: %dx", data->options.resolution);
			DrawTextWithShadow(font, color, game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, text);
			DrawTextWithShadow(font, data->selected==3 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Back");
			break;
		case MENUSTATE_HIDDEN:
			break;
		case MENUSTATE_LOST:
			DrawTextWithShadow(font, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "You lost!");
			sprintf(text, "Score: %d", data->score);
			DrawTextWithShadow(font, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, text);
			DrawTextWithShadow(font, al_map_rgb(255,255,128), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Back to menu");
			break;
		case MENUSTATE_INTRO:
			DrawTextWithShadow(font, al_map_rgba(0,0,0,64), 46, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "Evi");
			DrawTextWithShadow(font, al_map_rgba(0,0,0,64), 51, game->viewport.height*0.5-1, ALLEGRO_ALIGN_CENTRE, "vi");
			DrawTextWithShadow(font, al_map_rgb(255,255,128), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "Evil record label representatives want");
			DrawTextWithShadow(font, al_map_rgba(0,0,0,64), 47, game->viewport.height*0.55, ALLEGRO_ALIGN_CENTRE, "tu");
			DrawTextWithShadow(font, al_map_rgba(0,0,0,64), 48, game->viewport.height*0.55 - 1, ALLEGRO_ALIGN_CENTRE, "tu");
			DrawTextWithShadow(font, al_map_rgb(255,255,128), game->viewport.width*0.5, game->viewport.height*0.55, ALLEGRO_ALIGN_CENTRE, "to turn your awesome single into radio edit.");
			DrawTextWithShadow(font, al_map_rgb(255,255,128), game->viewport.width*0.5, game->viewport.height*0.6, ALLEGRO_ALIGN_CENTRE, "Thankfully, with your facemelting guitar");
			DrawTextWithShadow(font, al_map_rgb(255,255,128), game->viewport.width*0.5, game->viewport.height*0.65, ALLEGRO_ALIGN_CENTRE, "skills you don't have to give up so easily!");
			DrawTextWithShadow(font, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.8, ALLEGRO_ALIGN_CENTRE, "Press ENTER to continue...");
			break;
		default:
			data->selected=0;
			DrawTextWithShadow(font, data->selected==0 ? al_map_rgb(255,255,128) : al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5, ALLEGRO_ALIGN_CENTRE, "Not implemented yet");
			break;
	}
	free(text);
}

void AnimateBadguys(struct Game *game, struct MenuResources *data, int i) {
	struct Badguy *tmp = data->badguys[i];
	while (tmp) {
		AnimateCharacter(game, tmp->character, tmp->melting ? 1 : tmp->speed * data->badguySpeed);
		tmp=tmp->next;
	}
}

void MoveBadguys(struct Game *game, struct MenuResources *data, int i, float dx) {
	struct Badguy *tmp = data->badguys[i];
	while (tmp) {

		if (!tmp->character->spritesheet->kill) {
			MoveCharacter(game, tmp->character, dx * tmp->speed * data->badguySpeed, 0, 0);
		}

		if (tmp->character->dead) {
			if (tmp->prev) {
				tmp->prev->next = tmp->next;
				if (tmp->next) tmp->next->prev = tmp->prev;
			} else {
				data->badguys[i] = tmp->next;
				if (tmp->next) tmp->next->prev = NULL;
			}
			struct Badguy *old = tmp;
			tmp = tmp->next;
			old->character->dead = true;
			old->prev = NULL;
			old->next = data->destroyQueue;
			if (data->destroyQueue) data->destroyQueue->prev = old;
			data->destroyQueue = old;
		} else {
			tmp = tmp->next;
		}

	}
}

void ChangeMenuState(struct Game *game, struct MenuResources* data, enum menustate_enum state) {
	data->menustate=state;
	data->selected=0;
	PrintConsole(game, "menu state changed %d", state);
}

void CheckForEnd(struct Game *game, struct MenuResources *data) {
	int i;
	bool lost = false;
	for (i=0; i<4; i++) {
		struct Badguy *tmp = data->badguys[i];
		while (tmp) {
			if (tmp->character->x <= (139-(i*10))-10) {
				lost = true;
				break;
			}
			tmp=tmp->next;
		}
		if (lost) break;
	}

	if (lost) {

		al_stop_sample_instance(data->solo);
		data->soloactive=false;
		data->soloanim=0;
		data->soloflash=0;
		data->soloready=0;

		al_stop_sample_instance(data->music);
		al_play_sample_instance(data->end);
		SelectSpritesheet(game, data->ego, "cry");
		ChangeMenuState(game, data, MENUSTATE_LOST);
	}
}

void DrawBadguys(struct Game *game, struct MenuResources *data, int i) {
	struct Badguy *tmp = data->badguys[i];
	while (tmp) {
		DrawCharacter(game, tmp->character, al_map_rgb(255,255,255), 0);
		tmp=tmp->next;
	}
}

void Gamestate_Draw(struct Game *game, struct MenuResources* data) {

	al_set_target_bitmap(al_get_backbuffer(game->display));

	al_clear_to_color(al_map_rgb(3, 213, 255));

	al_draw_bitmap(data->bg,0, 0,0);

	al_draw_bitmap(data->cloud,game->viewport.width*data->cloud_position/100, 10 ,0);

	al_draw_bitmap(data->forest,0, 0,0);

	al_draw_bitmap(data->grass,0, 0,0);

	DrawCharacter(game, data->cow, al_map_rgb(255,255,255), 0);

	al_draw_bitmap(data->speaker,104, 19,0);

	al_draw_bitmap(data->stage,0, 0,0);

	al_draw_bitmap(data->lines, 100, 136,0);

	al_draw_bitmap(data->cable,0,151,0);

	DrawCharacter(game, data->ego, al_map_rgb(255,255,255), 0);

	if (data->menustate == MENUSTATE_HIDDEN) {

		if (!data->soloactive) {
			if (data->marky == 0) {
				al_draw_bitmap(data->marksmall, data->markx, 128, 0);
			} else if (data->marky == 1) {
				al_draw_bitmap(data->marksmall, data->markx, 140, 0);
			} else if (data->marky == 2) {
				al_draw_bitmap(data->markbig, data->markx, 152, 0);
			} else if (data->marky == 3) {
				al_draw_bitmap(data->markbig, data->markx, 166, 0);
			}
		}

		if (data->lightanim) {
			int offset = -5;
			if (data->lighty == 1) offset = -3;
			if (data->lighty == 2) offset = 0;
			if (data->lighty == 3) offset = 4;
			al_draw_tinted_bitmap(data->light, al_map_rgba(255, 255, 255,rand() % 256 / 50 * 50) , data->lightx - 171 - (data->lighty < 2 ? 1 : 0), 109+(data->lighty*10) - 143 + offset, 0);
		}

	}

	DrawBadguys(game, data, 0);
	DrawBadguys(game, data, 1);
	DrawBadguys(game, data, 2);
	DrawBadguys(game, data, 3);

	if (data->menustate != MENUSTATE_HIDDEN) {
		DrawTextWithShadow(data->font_title, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.15, ALLEGRO_ALIGN_CENTRE, data->menustate == MENUSTATE_LOST ? "Radio Edited!" : "Radio Edit");
		DrawMenuState(game, data);
	} else {
		char score[255];
		snprintf(score, 255, "Score: %d", data->score);
		DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 2, game->viewport.height - 10, ALLEGRO_ALIGN_LEFT, score);

		if ((data->soloready >= SOLO_MIN) && (data->soloanim <= 30)) {
			DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.15, ALLEGRO_ALIGN_CENTRE, "Press ENTER to play a solo!");
		}
	}

	if (data->soloflash) {
		al_draw_filled_rectangle(0, 0, 320, 180, al_map_rgb(255,255,255));
	}
}

void AddBadguy(struct Game *game, struct MenuResources* data, int i) {
	struct Badguy *n = malloc(sizeof(struct Badguy));
	n->next = NULL;
	n->prev = NULL;
	n->speed = (rand() % 3) * 0.25 + 1;
	n->melting = false;
	n->character = CreateCharacter(game, "badguy");
	n->character->spritesheets = data->badguy->spritesheets;
	n->character->shared = true;
	SelectSpritesheet(game, n->character, "walk");
	SetCharacterPosition(game, n->character, 320, 108+(i*13), 0);

	if (data->badguys[i]) {
		struct Badguy *tmp = data->badguys[i];
		while (tmp->next) {
			tmp=tmp->next;
		}
		tmp->next = n;
		n->prev = tmp;
	} else {
		data->badguys[i] = n;
	}
}

void Fire(struct Game *game, struct MenuResources *data) {

	if (data->soloactive) return;

	data->lightx = data->markx;
	data->lighty = data->marky;
	data->lightanim=1;
	data->usage=30;

	int num = rand() % 3;
	if (((al_get_sample_instance_position(data->music) + 20000) / 44118) % 2 == 1) {
		num += 3;
	}
	al_stop_sample_instance(data->chords[num]);
	al_play_sample_instance(data->chords[num]);
	PrintConsole(game, "playing chord nr %d", num);

	struct Badguy *tmp = data->badguys[data->marky];
	while (tmp) {
		if (!tmp->melting) {
			if ((data->markx >= tmp->character->x - 9) && (data->markx <= tmp->character->x + 1)) {
				data->score += 100 * tmp->speed;
				SelectSpritesheet(game, tmp->character, "melt");
				data->soloready++;
				tmp->melting = true;
			}
		}
		tmp=tmp->next;
	}
}

void Gamestate_Logic(struct Game *game, struct MenuResources* data) {

	if (data->keys.lastkey == data->keys.key) {
		data->keys.delay = data->keys.lastdelay; // workaround for random bugus UP/DOWN events
	}

	data->cloud_position-=0.1;
	if (data->cloud_position<-40) { data->cloud_position=100; PrintConsole(game, "cloud_position"); }
	AnimateCharacter(game, data->ego, 1);
	AnimateCharacter(game, data->cow, 1);

	if (data->menustate == MENUSTATE_HIDDEN) {

		if ((data->keys.key) && (data->keys.delay < 3)) {

			if (data->keys.key==ALLEGRO_KEY_UP) {
				data->marky--;
				int min = 139-(data->marky*10);
				int step = 10 - (data->markx - min) / ((320-min)/10);
				data->markx+= step;
				if (data->marky < 0) {
					data->markx-=4*step;
					data->marky = 3;
				}
			}

			if (data->keys.key==ALLEGRO_KEY_DOWN) {
				data->marky++;
				int min = 139-(data->marky*10);
				int step = 10 - (data->markx - min) / ((320-min)/10);
				data->markx-= step;
				if (data->marky > 3) {
					data->markx+=4*step;
					data->marky = 0;
				}
			}

			if (data->keys.key==ALLEGRO_KEY_LEFT) {
				int min = 139-(data->marky*10);
				data->markx-= data->keys.shift ? 5 : 2;
				if (data->markx < min) data->markx=min;
			}

			if (data->keys.key==ALLEGRO_KEY_RIGHT) {
				int max = 320 - al_get_bitmap_width(data->markbig);
				if (data->marky < 2) max = 320 - al_get_bitmap_width(data->marksmall);
				data->markx+= data->keys.shift ? 5 : 2;
				if (data->markx > max) data->markx=max;
			}

			if ((data->keys.key==ALLEGRO_KEY_SPACE) && (data->usage==0)) {
				Fire(game, data);
			}

			if (data->keys.delay == INT_MIN) data->keys.delay = 45;
			else data->keys.delay += 4;

		} else if (data->keys.key) {
			data->keys.delay-=3;
		}

		AnimateBadguys(game, data, 0);
		AnimateBadguys(game, data, 1);
		AnimateBadguys(game, data, 2);
		AnimateBadguys(game, data, 3);

		MoveBadguys(game, data, 0, -0.17);
		MoveBadguys(game, data, 1, -0.18);
		MoveBadguys(game, data, 2, -0.19);
		MoveBadguys(game, data, 3, -0.2);

		data->timeTillNextBadguy--;
		if (data->timeTillNextBadguy <= 0) {
			data->timeTillNextBadguy = data->badguyRate;
			data->badguyRate -= data->badguyRate * 0.02;
			if (data->badguyRate < 20) {
				data->badguyRate = 20;
			}

			data->badguySpeed+= 0.001;
			AddBadguy(game, data, rand() % 4);
		}

		if (data->usage) { data->usage--; }
		if (data->lightanim) { data->lightanim++;}
		if (data->lightanim > 25) { data->lightanim = 0; }

		CheckForEnd(game, data);
	}

	data->soloanim++;
	if (data->soloanim >= 60) data->soloanim=0;

	if (data->soloactive) {
		if (al_get_sample_instance_position(data->solo) >= 163840) {
			PrintConsole(game, "BLAAAST");
			data->soloflash = 6;
			data->soloactive=false;
			data->badguySpeed+=0.5;
			data->badguyRate += 20;

			int i;
			for (i=0; i<4; i++) {
				struct Badguy *tmp = data->badguys[i];
				while (tmp) {
					if ((!tmp->melting) && (!tmp->character->dead)) {
						data->score += 100 * tmp->speed;
						SelectSpritesheet(game, tmp->character, "melt");
						tmp->melting = true;
					}
					tmp=tmp->next;
				}
			}

		}
	}

	if (data->soloflash) data->soloflash--;

	data->keys.lastkey = data->keys.key;
	data->keys.lastdelay = data->keys.delay;

	TM_Process(data->timeline);
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {

	struct MenuResources *data = malloc(sizeof(struct MenuResources));

	data->timeline = TM_Init(game, "main");
	(*progress)(game);

	data->options.fullscreen = game->config.fullscreen;
	data->options.fps = game->config.fps;
	data->options.width = game->config.width;
	data->options.height = game->config.height;
	data->options.resolution = game->config.width / 320;
	if (game->config.height / 180 < data->options.resolution) data->options.resolution = game->config.height / 180;

	data->bg = al_load_bitmap( GetDataFilePath(game, "bg.png") );
	data->forest = al_load_bitmap( GetDataFilePath(game, "forest.png") );
	data->grass = al_load_bitmap( GetDataFilePath(game, "grass.png") );
	data->speaker = al_load_bitmap( GetDataFilePath(game, "speaker.png") );
	data->stage = al_load_bitmap( GetDataFilePath(game, "stage.png") );
	data->cloud = al_load_bitmap( GetDataFilePath(game, "cloud.png") );
	data->lines = al_load_bitmap( GetDataFilePath(game, "lines.png") );
	data->cable = al_load_bitmap( GetDataFilePath(game, "cable.png") );
	data->marksmall = al_load_bitmap( GetDataFilePath(game, "mark-small.png") );
	data->markbig = al_load_bitmap( GetDataFilePath(game, "mark-big.png") );
	data->light = al_load_bitmap( GetDataFilePath(game, "light.png") );
	data->sample = al_load_sample( GetDataFilePath(game, "menu.flac") );
	data->click_sample = al_load_sample( GetDataFilePath(game, "click.flac") );
	data->quit_sample = al_load_sample( GetDataFilePath(game, "quit.flac") );
	data->end_sample = al_load_sample( GetDataFilePath(game, "end.flac") );
	data->solo_sample = al_load_sample( GetDataFilePath(game, "solo.flac") );
	(*progress)(game);

	data->music = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->music, game->audio.music);
	al_set_sample_instance_playmode(data->music, ALLEGRO_PLAYMODE_LOOP);

	data->click = al_create_sample_instance(data->click_sample);
	al_attach_sample_instance_to_mixer(data->click, game->audio.fx);
	al_set_sample_instance_playmode(data->click, ALLEGRO_PLAYMODE_ONCE);

	data->quit = al_create_sample_instance(data->quit_sample);
	al_attach_sample_instance_to_mixer(data->quit, game->audio.fx);
	al_set_sample_instance_playmode(data->quit, ALLEGRO_PLAYMODE_ONCE);

	data->solo = al_create_sample_instance(data->solo_sample);
	al_attach_sample_instance_to_mixer(data->solo, game->audio.fx);
	al_set_sample_instance_playmode(data->solo, ALLEGRO_PLAYMODE_ONCE);

	data->end = al_create_sample_instance(data->end_sample);
	al_attach_sample_instance_to_mixer(data->end, game->audio.fx);
	al_set_sample_instance_playmode(data->end, ALLEGRO_PLAYMODE_ONCE);

	int i;
	for (i=0; i<6; i++) {
		char name[] = "chords/0.flac";
		name[7] = '1' + i;
		data->chord_samples[i] = al_load_sample( GetDataFilePath(game, name) );

		data->chords[i] = al_create_sample_instance(data->chord_samples[i]);
		al_attach_sample_instance_to_mixer(data->chords[i], game->audio.fx);
		al_set_sample_instance_playmode(data->chords[i], ALLEGRO_PLAYMODE_ONCE);
	}

	if (!data->click_sample){
		fprintf(stderr, "Audio clip sample not loaded!\n" );
		exit(-1);
	}
	(*progress)(game);

	data->font_title = al_load_ttf_font(GetDataFilePath(game, "fonts/MonkeyIsland.ttf"),game->viewport.height*0.16,0 );
	data->font = al_load_ttf_font(GetDataFilePath(game, "fonts/MonkeyIsland.ttf"),game->viewport.height*0.05,0 );
	(*progress)(game);

	data->ego = CreateCharacter(game, "ego");
	RegisterSpritesheet(game, data->ego, "stand");
	RegisterSpritesheet(game, data->ego, "fix");
	RegisterSpritesheet(game, data->ego, "fix2");
	RegisterSpritesheet(game, data->ego, "fix3");
	RegisterSpritesheet(game, data->ego, "play");
	RegisterSpritesheet(game, data->ego, "cry");
	LoadSpritesheets(game, data->ego);

	data->cow = CreateCharacter(game, "cow");
	RegisterSpritesheet(game, data->cow, "stand");
	RegisterSpritesheet(game, data->cow, "chew");
	RegisterSpritesheet(game, data->cow, "look");
	LoadSpritesheets(game, data->cow);

	data->badguy = CreateCharacter(game, "badguy");
	RegisterSpritesheet(game, data->badguy, "walk");
	RegisterSpritesheet(game, data->badguy, "melt");
	LoadSpritesheets(game, data->badguy);
	(*progress)(game);

	al_set_target_backbuffer(game->display);
	return data;
}

void DestroyBadguys(struct Game *game, struct MenuResources* data, int i) {
	struct Badguy *tmp = data->badguys[i];
	if (!tmp) {
		tmp = data->destroyQueue;
		data->destroyQueue = NULL;
	}
	while (tmp) {
		DestroyCharacter(game, tmp->character);
		struct Badguy *old = tmp;
		tmp = tmp->next;
		free(old);
		if ((!tmp) && (data->destroyQueue)) {
			tmp = data->destroyQueue;
			data->destroyQueue = NULL;
		}
	}
	data->badguys[i] = NULL;
}

void Gamestate_Stop(struct Game *game, struct MenuResources* data) {
	al_stop_sample_instance(data->music);

	int i;
	for (i=0; i<4; i++) {
		DestroyBadguys(game, data, i);
	}
}

void Gamestate_Unload(struct Game *game, struct MenuResources* data) {
	if (game->config.fx) {
		al_clear_to_color(al_map_rgb(0,0,0));
		DrawConsole(game);
		al_flip_display();
		al_play_sample_instance(data->quit);
		al_rest(0.3);
		int i;
		for (i=0;i<50; i++) {
			al_rest(0.05);
			ALLEGRO_KEYBOARD_STATE kb;
			al_get_keyboard_state(&kb);
			if (al_key_down(&kb, ALLEGRO_KEY_ESCAPE)) return;
		}
	}

	al_destroy_bitmap(data->bg);
	al_destroy_bitmap(data->cloud);
	al_destroy_bitmap(data->grass);
	al_destroy_bitmap(data->forest);
	al_destroy_bitmap(data->stage);
	al_destroy_bitmap(data->speaker);
	al_destroy_bitmap(data->lines);
	al_destroy_bitmap(data->cable);
	al_destroy_bitmap(data->light);
	al_destroy_bitmap(data->marksmall);
	al_destroy_bitmap(data->markbig);
	al_destroy_font(data->font_title);
	al_destroy_font(data->font);
	al_destroy_sample_instance(data->music);
	al_destroy_sample_instance(data->click);
	al_destroy_sample_instance(data->end);
	al_destroy_sample_instance(data->quit);
	al_destroy_sample_instance(data->solo);
	al_destroy_sample(data->sample);
	al_destroy_sample(data->click_sample);
	al_destroy_sample(data->quit_sample);
	al_destroy_sample(data->end_sample);
	al_destroy_sample(data->solo_sample);
	int i;
	for (i=0; i<6; i++) {
		al_destroy_sample_instance(data->chords[i]);
		al_destroy_sample(data->chord_samples[i]);
	}
	DestroyCharacter(game, data->ego);
	DestroyCharacter(game, data->cow);
	DestroyCharacter(game, data->badguy);
	TM_Destroy(data->timeline);
}

// TODO: refactor to single Enqueue_Anim
bool Anim_CowLook(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct MenuResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_START) {
		ChangeSpritesheet(game, data->cow, "look");
		TM_AddQueuedBackgroundAction(data->timeline, &Anim_CowLook, TM_AddToArgs(NULL, 1, data), 54*1000, "cow_look");
	}
	return true;
}

void StartGame(struct Game *game, struct MenuResources *data) {
	TM_CleanQueue(data->timeline);
	TM_CleanBackgroundQueue(data->timeline);
	ChangeSpritesheet(game, data->ego, "play");
	ChangeSpritesheet(game, data->cow, "chew");
	ChangeMenuState(game,data,MENUSTATE_HIDDEN);
	al_play_sample_instance(data->chords[0]);
}

bool Anim_FixGuitar(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct MenuResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_START) {
		ChangeSpritesheet(game, data->ego, "fix");
		TM_AddQueuedBackgroundAction(data->timeline, &Anim_FixGuitar, TM_AddToArgs(NULL, 1, data), 30*1000, "fix_guitar");
	}
	return true;
}


void Gamestate_Start(struct Game *game, struct MenuResources* data) {
	data->cloud_position = 100;
	SetCharacterPosition(game, data->ego, 22, 107, 0);
	SetCharacterPosition(game, data->cow, 35, 88, 0);

	data->score = 0;

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

	data->badguySpeed = 1.2;

	data->usage = 0;

	SelectSpritesheet(game, data->ego, "stand");
	SelectSpritesheet(game, data->cow, "chew");
	ChangeMenuState(game,data,MENUSTATE_MAIN);
	TM_AddQueuedBackgroundAction(data->timeline, &Anim_FixGuitar, TM_AddToArgs(NULL, 1, data), 15*1000, "fix_guitar");
	TM_AddQueuedBackgroundAction(data->timeline, &Anim_CowLook, TM_AddToArgs(NULL, 1, data), 5*1000, "cow_look");
	al_play_sample_instance(data->music);
	al_rest(0.01); // poor man's synchronization

	data->badguys[0] = NULL;
	data->badguys[1] = NULL;
	data->badguys[2] = NULL;
	data->badguys[3] = NULL;
	data->destroyQueue = NULL;

	data->badguyRate = 100;
	data->timeTillNextBadguy = 0;
}

void Gamestate_ProcessEvent(struct Game *game, struct MenuResources* data, ALLEGRO_EVENT *ev) {
	TM_HandleEvent(data->timeline, ev);

	if ((data->menustate == MENUSTATE_ABOUT) && (ev->type == ALLEGRO_EVENT_KEY_DOWN)) {
		ChangeMenuState(game, data, MENUSTATE_MAIN);
		return;
	}

	if (data->menustate == MENUSTATE_HIDDEN) {

		if (ev->type == ALLEGRO_EVENT_KEY_DOWN) {

			switch (ev->keyboard.keycode) {
				case ALLEGRO_KEY_UP:
				case ALLEGRO_KEY_DOWN:
				case ALLEGRO_KEY_LEFT:
				case ALLEGRO_KEY_RIGHT:
				case ALLEGRO_KEY_SPACE:
					if (data->keys.key != ev->keyboard.keycode) {
						data->keys.key = ev->keyboard.keycode;
						data->keys.delay = INT_MIN;
					}
					break;
				case ALLEGRO_KEY_ESCAPE:
					Gamestate_Stop(game, data);
					Gamestate_Start(game, data);
					break;
				case ALLEGRO_KEY_LSHIFT:
				case ALLEGRO_KEY_RSHIFT:
					data->keys.shift = true;
					break;
				case ALLEGRO_KEY_ENTER:
					if ((!data->soloactive) && (data->soloready >= SOLO_MIN)) {
						data->soloready = 0;
						al_play_sample_instance(data->solo);
						data->soloactive = true;
						data->badguySpeed-=0.5;
					}
					break;
				default:
					data->keys.key = 0;
					break;
			}
		} else if (ev->type == ALLEGRO_EVENT_KEY_UP) {
			switch (ev->keyboard.keycode) {
				case ALLEGRO_KEY_LSHIFT:
				case ALLEGRO_KEY_RSHIFT:
					data->keys.shift = false;
					break;
				default:
					if (ev->keyboard.keycode == data->keys.key) {
						data->keys.key = 0;
					}
					break;
			}
		}

		return;
	}

	if (ev->type != ALLEGRO_EVENT_KEY_DOWN) return;

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
						ChangeMenuState(game,data,MENUSTATE_INTRO);
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
			case MENUSTATE_INTRO:
				StartGame(game, data);
				break;
			case MENUSTATE_LOST:
				Gamestate_Stop(game,data);
				Gamestate_Start(game,data);
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
				Gamestate_Stop(game,data);
				Gamestate_Start(game,data);
				break;
			case MENUSTATE_VIDEO:
				ChangeMenuState(game,data,MENUSTATE_OPTIONS);
				break;
			case MENUSTATE_AUDIO:
				ChangeMenuState(game,data,MENUSTATE_OPTIONS);
				break;
			case MENUSTATE_INTRO:
				ChangeMenuState(game,data,MENUSTATE_MAIN);
				break;
			case MENUSTATE_LOST:
				Gamestate_Stop(game,data);
				Gamestate_Start(game,data);
				break;
			default:
				UnloadGamestate(game, "menu");
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

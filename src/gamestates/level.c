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
#define FLOOR_OF_FUN 0.68
#define CEIL_OF_FUN 0.88

int Gamestate_ProgressCount = 8;

void SaveScore(struct Game *game, struct LevelResources *data) {

	if (((data->score / (double)(data->time / 10)) > (data->savedScore / (double)data->savedTime)) || ((data->score / (double)(data->time / 10)) == (data->savedScore / (double)data->savedTime) && (data->score > data->savedScore))) {
		char *text = malloc(255*sizeof(char));
		snprintf(text, 255, "%d", data->score);
		SetConfigOption(game, "TickleMonster", "score", text);
		snprintf(text, 255, "%d", data->time / 10);
		SetConfigOption(game, "TickleMonster", "time", text);
	}

}

void AnimateBadguys(struct Game *game, struct LevelResources *data, int i) {
	struct Kid *tmp = data->kids[i];
	while (tmp) {
		AnimateCharacter(game, tmp->character, tmp->tickled ? 1 : tmp->speed * data->kidSpeed);
		tmp=tmp->next;
	}
}

void MoveBadguys(struct Game *game, struct LevelResources *data, int i, float dx) {
	struct Kid *tmp = data->kids[i];
	while (tmp) {

		if (!tmp->grownup) {
			if ((!tmp->character->spritesheet->kill) && (!tmp->tickled)) {
				MoveCharacter(game, tmp->character, dx * tmp->speed * data->kidSpeed, 0, 0);
			}

			if (tmp->character->x < 30) {
				if (tmp->fun > FLOOR_OF_FUN * MAX_FUN && tmp->fun < CEIL_OF_FUN * MAX_FUN) {
					tmp->happy = true;
					al_set_sample_instance_playing(data->click, true);
					data->score++;
				} else {
					if (rand() % 3 == 0) {
						tmp->grownup = true;
						tmp->right = true;
						tmp->character->spritesheets = data->suit->spritesheets;
						SelectSpritesheet(game, tmp->character, "walk");
						MoveCharacter(game, tmp->character, 0, -8, 0);
					}
				}
			}
		} else {
			// grownup
			MoveCharacter(game, tmp->character, (tmp->right ? -1 : 1) * dx * tmp->speed * data->kidSpeed / 2, 0, 0);
			if (tmp->character->x > 270) {
				tmp->right = false;
			} else if (tmp->character->x < 42) {
				if (rand() % 2) { // 50% chance for getting rid
					tmp->right = true;
				} else {
					if (!tmp->right) {
						tmp->happy = true;
					}
				}
			}
		}

		if (tmp->grownup) {
			if ((tmp->character->x > data->monster->x) && (tmp->character->x + 10 < data->monster->x + 20) && (abs(tmp->character->y - data->monster->y) < 5)) {
				data->lost = true;
				al_stop_sample_instance(data->laughter);
				al_stop_timer(data->timer);
				SaveScore(game, data);
			}
		}

		if (tmp->happy) {
			if (tmp->prev) {
				tmp->prev->next = tmp->next;
				if (tmp->next) tmp->next->prev = tmp->prev;
			} else {
				data->kids[i] = tmp->next;
				if (tmp->next) tmp->next->prev = NULL;
			}
			struct Kid *old = tmp;
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

void CheckForEnd(struct Game *game, struct LevelResources *data) {
	return;

	int i;
	bool lost = false;
	for (i=0; i<6; i++) {
		struct Kid *tmp = data->kids[i];
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

		data->soloactive=false;
		data->soloanim=0;
		data->soloflash=0;
		data->soloready=0;

		SelectSpritesheet(game, data->monster, "cry");
	}
}

void DrawBadguys(struct Game *game, struct LevelResources *data, int i) {
	struct Kid *tmp = data->kids[i];
	while (tmp) {
		if (!tmp->happy) {
			DrawCharacter(game, tmp->character, al_map_rgb(255,255,255), (tmp->grownup && !tmp->right) ? ALLEGRO_FLIP_HORIZONTAL : 0);
		}
		tmp=tmp->next;
	}
}

void Gamestate_Draw(struct Game *game, struct LevelResources* data) {

	al_set_target_bitmap(al_get_backbuffer(game->display));

	al_clear_to_color(al_map_rgb(3, 213, 255));

	al_draw_bitmap(data->bg,0, 0,0);

	for (int i=0; i<6; i++) {
		DrawBadguys(game, data, i);
		if ((int)((data->monster->y + 18) / 20) > i) {
			DrawCharacter(game, data->monster, al_map_rgb(255,255,255), 0);
		}
	}

	al_draw_bitmap(data->buildings,0, 0,0);
	if (data->savedScore) {
		al_draw_bitmap(data->hid2,0, 0,0);
	} else {
		al_draw_bitmap(data->hid,0, 0,0);
	}

	if (data->tickling && data->haskid) {
		al_draw_bitmap(data->meter,0, 0,0);
		int length = (data->tickledKid->fun / MAX_FUN) * 151;
		al_draw_filled_rectangle(160, 163, 160 + ((length > 151) ? 151 : length), 173, al_map_rgb(255,255,255));
		if ((data->tickledKid->fun / MAX_FUN) > FLOOR_OF_FUN) {
			al_draw_filled_rectangle(160 + 151 * FLOOR_OF_FUN, 163, 160 + ((length > 151 * CEIL_OF_FUN) ? (151 * CEIL_OF_FUN) : length), 173, al_map_rgb(192, 255, 192));
		}
	}

	if (data->soloflash) {
		al_draw_filled_rectangle(0, 0, 320, 180, al_map_rgb(255,255,255));
	}

	char *text = malloc(255*sizeof(char));
	snprintf(text, 255, "%d", data->score);
	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 21, 162, 0, text);
	snprintf(text, 255, "%d", data->time / 10);
	DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 61, 162, 0, text);
	if (data->savedScore) {
		snprintf(text, 255, "%d / %d", data->savedScore, data->savedTime);
		DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 106, 162, 0, text);
	}
	free(text);

	if (data->lost) {
		al_draw_filled_rectangle(0, 0, 320, 180, al_map_rgba(0,0,0,128));
		al_draw_bitmap(data->busted,0, 0,0);

		char *text = malloc(255*sizeof(char));
		snprintf(text, 255, "Score: %d", data->score);
		DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 200, 118, 0, text);
		snprintf(text, 255, "Time: %d", data->time / 10);
		DrawTextWithShadow(data->font, al_map_rgb(255,255,255), 200, 128, 0, text);
		free(text);
	}

	if (data->paused) {
		al_draw_filled_rectangle(0, 0, 320, 180, al_map_rgba(0,0,0,128));
		DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5 - 25, ALLEGRO_ALIGN_CENTRE, "Game paused!");
		DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5 + 5, ALLEGRO_ALIGN_CENTRE, "SPACE to resume");
		DrawTextWithShadow(data->font, al_map_rgb(255,255,255), game->viewport.width*0.5, game->viewport.height*0.5 + 15, ALLEGRO_ALIGN_CENTRE, "ESCAPE to leave");
	}
}

void AddBadguy(struct Game *game, struct LevelResources* data, int i) {
	struct Kid *n = malloc(sizeof(struct Kid));
	n->next = NULL;
	n->prev = NULL;
	n->speed = (rand() % 3) * 0.25 + 1;
	n->tickled = false;
	n->grownup = false;
	n->happy = false;
	n->fun = 0;
	n->character = CreateCharacter(game, "kid");
	n->character->spritesheets = data->kid->spritesheets;
	n->character->shared = true;
	SelectSpritesheet(game, n->character, "walk");
	SetCharacterPosition(game, n->character, 280, 20+(i*TILE_SIZE), 0);

	if (data->kids[i]) {
		struct Kid *tmp = data->kids[i];
		while (tmp->next) {
			tmp=tmp->next;
		}
		tmp->next = n;
		n->prev = tmp;
	} else {
		data->kids[i] = n;
	}
}

void Fire(struct Game *game, struct LevelResources *data) {

	if (data->movedown || data->moveup) return;

	if (data->tickling) {

		if (data->haskid) {
			data->tickledKid->tickled = false;
			SelectSpritesheet(game, data->tickledKid->character, "walk");
			data->haskid = false;
			al_set_sample_instance_playing(data->laughter, false);
			MoveCharacter(game, data->tickledKid->character, 0, 3, 0);
			data->tickledKid = NULL;
		}

		SelectSpritesheet(game, data->monster, "stand");
		MoveCharacter(game, data->monster, 2, -2, 0);
		data->tickling = false;
		return;
	}

	SelectSpritesheet(game, data->monster, "ticklefail");
	MoveCharacter(game, data->monster, -2, 2, 0);

	data->tickling = true;

	//PrintConsole(game, "MONSTAH %f", data->monster->x);
}

void Gamestate_Logic(struct Game *game, struct LevelResources* data) {

	if ((data->lost) || (data->paused)) return;

	if (strcmp(data->monster->spritesheet->name, "fail") == 0) {
		data->tickling = false;
		MoveCharacter(game, data->monster, 2, -2, 0);
		SelectSpritesheet(game, data->monster, "stand");
	}

	if (data->tickling) {
		if (!data->haskid) {
			struct Kid *tmp = data->kids[(int)((data->monster->y - 15) / 20)];
			while (tmp) {
				if ((tmp->character->x > data->monster->x + 16) && (tmp->character->x < data->monster->x + 23)) {
					if (tmp->grownup) {
						data->lost = true;
						al_stop_sample_instance(data->laughter);
						al_stop_timer(data->timer);
						SaveScore(game, data);
					} else {
						tmp->tickled = true;
						SelectSpritesheet(game, data->monster, "tickle");
						SelectSpritesheet(game, tmp->character, "laugh");
						data->haskid = true;
						data->tickledKid = tmp;
						SetCharacterPosition(game, tmp->character, data->monster->x + 22, tmp->character->y - 3, 0);
						al_set_sample_instance_playing(data->laughter, true);
					}
					break;
				}
				tmp=tmp->next;
			}
		}

		if (data->haskid) {
			data->tickledKid->fun++;
		}
	}


	if (data->keys.lastkey == data->keys.key) {
		data->keys.delay = data->keys.lastdelay; // workaround for random bugus UP/DOWN events
	}

	if (data->moveup && data->monster->y < 14) {
		data->moveup = false;
	}
	if (data->movedown && data->monster->y > 112) {
		data->movedown = false;
	}

	if (data->moveup) {
		MoveCharacter(game, data->monster, 0, -1, 0);
	} else if (data->movedown) {
		MoveCharacter(game, data->monster, 0, 1, 0);
	}

	if ((int)(data->monster->y + 7) % TILE_SIZE == 0) {
		data->moveup = false;
		data->movedown = false;
	}

	data->cloud_position-=0.1;
	if (data->cloud_position<-40) { data->cloud_position=100; PrintConsole(game, "cloud_position"); }
	AnimateCharacter(game, data->monster, 1);

		if ((data->keys.key) && (data->keys.delay < 3)) {

			if (!data->tickling) {
				if (data->keys.key==ALLEGRO_KEY_LEFT) {
					if (data->monster->x > 42) {
						MoveCharacter(game, data->monster, -1, 0, 0);
					}
				}

				if (data->keys.key==ALLEGRO_KEY_RIGHT) {
					if (data->monster->x < 256) {
						MoveCharacter(game, data->monster, 1, 0, 0);
					}
				}
			}

			if (data->keys.delay == INT_MIN) data->keys.delay = 3;
			else data->keys.delay += 3;

		} else if (data->keys.key) {
			data->keys.delay-=3;
		}

		AnimateBadguys(game, data, 0);
		AnimateBadguys(game, data, 1);
		AnimateBadguys(game, data, 2);
		AnimateBadguys(game, data, 3);
		AnimateBadguys(game, data, 4);
		AnimateBadguys(game, data, 5);

		MoveBadguys(game, data, 0, -0.17);
		MoveBadguys(game, data, 1, -0.18);
		MoveBadguys(game, data, 2, -0.19);
		MoveBadguys(game, data, 3, -0.2);
		MoveBadguys(game, data, 4, -0.21);
		MoveBadguys(game, data, 5, -0.22);

		data->timeTillNextBadguy--;
		if (data->timeTillNextBadguy <= 0) {
			data->timeTillNextBadguy = data->kidRate * 2;
			data->kidRate -= data->kidRate * 0.005;
			if (data->kidRate < 50) {
				data->kidRate = 50;
			}

			data->kidSpeed+= 0.0005;
			AddBadguy(game, data, rand() % 6);
		}

		if (data->usage) { data->usage--; }
		if (data->lightanim) { data->lightanim++;}
		if (data->lightanim > 25) { data->lightanim = 0; }

		CheckForEnd(game, data);

	data->soloanim++;
	if (data->soloanim >= 60) data->soloanim=0;

	if (data->soloflash) data->soloflash--;

	data->keys.lastkey = data->keys.key;
	data->keys.lastdelay = data->keys.delay;

	TM_Process(data->timeline);
}

void* Gamestate_Load(struct Game *game, void (*progress)(struct Game*)) {

	struct LevelResources *data = malloc(sizeof(struct LevelResources));

	data->timer = al_create_timer(0.1);
	al_register_event_source(game->_priv.event_queue, al_get_timer_event_source(data->timer));

	data->timeline = TM_Init(game, "main");
	(*progress)(game);

	data->bg = al_load_bitmap( GetDataFilePath(game, "bg2.png") );
	data->buildings = al_load_bitmap( GetDataFilePath(game, "buildings.png") );
	data->hid = al_load_bitmap( GetDataFilePath(game, "hid.png") );
	data->hid2 = al_load_bitmap( GetDataFilePath(game, "hid2.png") );
	data->meter = al_load_bitmap( GetDataFilePath(game, "meter.png") );
	data->busted = al_load_bitmap( GetDataFilePath(game, "busted.png") );
	data->click_sample = al_load_sample( GetDataFilePath(game, "point.flac") );
	(*progress)(game);

	data->click = al_create_sample_instance(data->click_sample);
	al_attach_sample_instance_to_mixer(data->click, game->audio.fx);
	al_set_sample_instance_playmode(data->click, ALLEGRO_PLAYMODE_ONCE);
	(*progress)(game);


	data->sample = al_load_sample( GetDataFilePath(game, "laughter.flac") );
	(*progress)(game);

	data->laughter = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->laughter, game->audio.fx);
	al_set_sample_instance_playmode(data->laughter, ALLEGRO_PLAYMODE_LOOP);
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

	data->suit = CreateCharacter(game, "suit");
	RegisterSpritesheet(game, data->suit, "walk");
	LoadSpritesheets(game, data->suit);
	(*progress)(game);

	data->kid = CreateCharacter(game, "kid");
	RegisterSpritesheet(game, data->kid, "walk");
	RegisterSpritesheet(game, data->kid, "laugh");
	LoadSpritesheets(game, data->kid);

	al_set_target_backbuffer(game->display);
	return data;
}

void DestroyBadguys(struct Game *game, struct LevelResources* data, int i) {
	struct Kid *tmp = data->kids[i];
	if (!tmp) {
		tmp = data->destroyQueue;
		data->destroyQueue = NULL;
	}
	while (tmp) {
		DestroyCharacter(game, tmp->character);
		struct Kid *old = tmp;
		tmp = tmp->next;
		free(old);
		if ((!tmp) && (data->destroyQueue)) {
			tmp = data->destroyQueue;
			data->destroyQueue = NULL;
		}
	}
	data->kids[i] = NULL;
}

void Gamestate_Stop(struct Game *game, struct LevelResources* data) {
	int i;
	for (i=0; i<6; i++) {
		DestroyBadguys(game, data, i);
	}
	al_set_sample_instance_playing(data->laughter, false);
}

void Gamestate_Unload(struct Game *game, struct LevelResources* data) {
	al_destroy_bitmap(data->bg);
	al_destroy_bitmap(data->buildings);
	al_destroy_bitmap(data->hid);
	al_destroy_bitmap(data->hid2);
	al_destroy_bitmap(data->meter);
	al_destroy_bitmap(data->busted);
	al_destroy_font(data->font_title);
	al_destroy_font(data->font);
	al_destroy_sample_instance(data->laughter);
	al_destroy_sample_instance(data->click);
	al_destroy_sample(data->sample);
	al_destroy_sample(data->click_sample);
	DestroyCharacter(game, data->monster);
	DestroyCharacter(game, data->suit);
	DestroyCharacter(game, data->kid);
	TM_Destroy(data->timeline);
}

// TODO: refactor to single Enqueue_Anim
bool Anim_CowLook(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct LevelResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_START) {
		ChangeSpritesheet(game, data->suit, "look");
		TM_AddQueuedBackgroundAction(data->timeline, &Anim_CowLook, TM_AddToArgs(NULL, 1, data), 54*1000, "cow_look");
	}
	return true;
}

bool Anim_FixGuitar(struct Game *game, struct TM_Action *action, enum TM_ActionState state) {
	struct LevelResources *data = action->arguments->value;
	if (state == TM_ACTIONSTATE_START) {
		ChangeSpritesheet(game, data->monster, "fix");
		TM_AddQueuedBackgroundAction(data->timeline, &Anim_FixGuitar, TM_AddToArgs(NULL, 1, data), 30*1000, "fix_guitar");
	}
	return true;
}

void StartGame(struct Game *game, struct LevelResources *data) {
	TM_CleanQueue(data->timeline);
	TM_CleanBackgroundQueue(data->timeline);
	ChangeSpritesheet(game, data->monster, "stand");
 }

void Gamestate_Start(struct Game *game, struct LevelResources* data) {
	data->cloud_position = 100;
	SetCharacterPosition(game, data->monster, 150, 73, 0);
	SetCharacterPosition(game, data->suit, 65, 88, 0);

	al_start_timer(data->timer);

	data->score = 0;
	data->time = 0;
	data->paused = false;

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

	char *end = "";

	data->savedScore = strtol(GetConfigOptionDefault(game, "TickleMonster", "score", "0"), &end, 10);
	data->savedTime = strtol(GetConfigOptionDefault(game, "TickleMonster", "time", "-1"), &end, 10);

}

void Gamestate_ProcessEvent(struct Game *game, struct LevelResources* data, ALLEGRO_EVENT *ev) {
	TM_HandleEvent(data->timeline, ev);

	if (ev->type == ALLEGRO_EVENT_TIMER) {
		if (ev->timer.source == data->timer) {
			data->time++;
		}
	}

 if (ev->type == ALLEGRO_EVENT_KEY_DOWN) {
	 if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
		 if ((data->paused) || (data->lost)) {
			 SwitchGamestate(game, "level", "menu");
		 }
		 al_stop_timer(data->timer);
		 data->paused = true;
		 return;
	 }
	 if (data->lost && ev->keyboard.keycode == ALLEGRO_KEY_ENTER) {
		 SwitchGamestate(game, "level", "menu");
		 return;
	 }
 }
	if (data->lost) return;

		if (ev->type == ALLEGRO_EVENT_KEY_DOWN) {

			switch (ev->keyboard.keycode) {
				case ALLEGRO_KEY_LEFT:
				case ALLEGRO_KEY_RIGHT:
					if (!data->tickling) {
						if (data->keys.key != ev->keyboard.keycode) {
							data->keys.key = ev->keyboard.keycode;
							data->keys.delay = INT_MIN;
						}
					}
					break;
				case ALLEGRO_KEY_UP:
					if (!data->tickling) {
						if (!data->moveup && !data->movedown) {
							SelectSpritesheet(game, data->monster, "jump");
						}
						data->moveup = true;
						data->movedown = false;
					}
					break;
				case ALLEGRO_KEY_DOWN:
					if (!data->tickling) {
						if (!data->moveup && !data->movedown) {
							SelectSpritesheet(game, data->monster, "jump");
						}
						data->moveup = false;
						data->movedown = true;
					}
					break;
				case ALLEGRO_KEY_SPACE:
					if (data->paused) {
						al_start_timer(data->timer);
						data->paused = false;
					} else {
						Fire(game, data);
					}
					break;
				case ALLEGRO_KEY_LSHIFT:
				case ALLEGRO_KEY_RSHIFT:
					data->keys.shift = true;
					break;
				case ALLEGRO_KEY_ENTER:
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


}

void Gamestate_Pause(struct Game *game, struct LevelResources* data) {
	data->paused = true;
	TM_Pause(data->timeline);
}
void Gamestate_Resume(struct Game *game, struct LevelResources* data) {
	data->paused = false;
	TM_Resume(data->timeline);
}
void Gamestate_Reload(struct Game *game, struct LevelResources* data) {}

/*! \file character.c
 *  \brief Character and spritesheet functions.
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

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <string.h>
#include "internal.h"
#include "utils.h"

SYMBOL_EXPORT void SelectSpritesheet(struct Game *game, struct Character *character, char *name) {
	struct Spritesheet *tmp = character->spritesheets;
	PrintConsole(game, "Selecting spritesheet for %s: %s", character->name, name);
	if (!tmp) {
		PrintConsole(game, "ERROR: No spritesheets registered for %s!", character->name);
		return;
	}
	while (tmp) {
		if (!strcmp(tmp->name, name)) {
			character->spritesheet = tmp;
			if (character->successor) free(character->successor);
			if (tmp->successor) {
				character->successor = strdup(tmp->successor);
			} else {
				character->successor = NULL;
			}
			character->repeat = tmp->repeat;
			character->pos = 0;
			if (character->bitmap) {
				if ((al_get_bitmap_width(character->bitmap) != tmp->width / tmp->cols) || (al_get_bitmap_height(character->bitmap) != tmp->height / tmp->rows)) {
					al_destroy_bitmap(character->bitmap);
					character->bitmap = al_create_bitmap(tmp->width / tmp->cols, tmp->height / tmp->rows);
				}
			} else {
				character->bitmap = al_create_bitmap(tmp->width / tmp->cols, tmp->height / tmp->rows);
			}
			PrintConsole(game, "SUCCESS: Spritesheet for %s activated: %s (%dx%d)", character->name, character->spritesheet->name, al_get_bitmap_width(character->bitmap), al_get_bitmap_height(character->bitmap));
			return;
		}
		tmp = tmp->next;
	}
	PrintConsole(game, "ERROR: No spritesheets registered for %s with given name: %s", character->name, name);
	return;
}

SYMBOL_EXPORT void ChangeSpritesheet(struct Game *game, struct Character *character, char* name) {
	if (character->successor) free(character->successor);
	character->successor = strdup(name);
}

SYMBOL_EXPORT void LoadSpritesheets(struct Game *game, struct Character *character) {
	PrintConsole(game, "Loading spritesheets for character %s...", character->name);
	struct Spritesheet *tmp = character->spritesheets;
	while (tmp) {
		if (!tmp->bitmap) {
			char filename[255] = { };
			snprintf(filename, 255, "sprites/%s/%s.png", character->name, tmp->name);
			tmp->bitmap = al_load_bitmap(GetDataFilePath(game, filename));
			tmp->width = al_get_bitmap_width(tmp->bitmap);
			tmp->height = al_get_bitmap_height(tmp->bitmap);
		}
		tmp = tmp->next;
	}
}

SYMBOL_EXPORT void UnloadSpritesheets(struct Game *game, struct Character *character) {
	PrintConsole(game, "Unloading spritesheets for character %s...", character->name);
	struct Spritesheet *tmp = character->spritesheets;
	while (tmp) {
		if (tmp->bitmap) al_destroy_bitmap(tmp->bitmap);
		tmp->bitmap = NULL;
		tmp = tmp->next;
	}
}

SYMBOL_EXPORT void RegisterSpritesheet(struct Game *game, struct Character *character, char* name) {
	struct Spritesheet *s = character->spritesheets;
	while (s) {
		if (!strcmp(s->name, name)) {
			//PrintConsole(game, "%s spritesheet %s already registered!", character->name, name);
			return;
		}
		s = s->next;
	}
	PrintConsole(game, "Registering %s spritesheet: %s", character->name, name);
	char filename[255] = { };
	snprintf(filename, 255, "sprites/%s/%s.ini", character->name, name);
	ALLEGRO_CONFIG *config = al_load_config_file(GetDataFilePath(game, filename));
	s = malloc(sizeof(struct Spritesheet));
	s->name = strdup(name);
	s->bitmap = NULL;
	s->cols = atoi(al_get_config_value(config, "", "cols"));
	s->rows = atoi(al_get_config_value(config, "", "rows"));
	s->blanks = atoi(al_get_config_value(config, "", "blanks"));
	s->delay = atof(al_get_config_value(config, "", "delay"));
	const char* val = al_get_config_value(config, "", "repeat");
	if (val) {
		s->repeat = atof(val);
	} else {
		s->repeat = 0;
	}
	s->kill = false;
	const char *kill = al_get_config_value(config, "", "kill");
	if (kill)	s->kill = atoi(kill);
	s->successor=NULL;
	const char* successor = al_get_config_value(config, "", "successor");
	if (successor) {
		s->successor = malloc(255*sizeof(char));
		strncpy(s->successor, successor, 255);
	}
	s->next = character->spritesheets;
	character->spritesheets = s;
	al_destroy_config(config);
}

SYMBOL_EXPORT struct Character* CreateCharacter(struct Game *game, char* name) {
	PrintConsole(game, "Creating character %s...", name);
	struct Character *character = malloc(sizeof(struct Character));
	character->name = strdup(name);
	character->angle = 0;
	character->bitmap = NULL;
	character->data = NULL;
	character->pos = 0;
	character->pos_tmp = 0;
	character->x = -1;
	character->y = -1;
	character->spritesheets = NULL;
	character->spritesheet = NULL;
	character->successor = NULL;
	character->repeat = 0;
	character->shared = false;
	character->dead = false;
	return character;
}

SYMBOL_EXPORT void DestroyCharacter(struct Game *game, struct Character *character) {
	PrintConsole(game, "Destroying character %s...", character->name);
	if (!character->shared) {
		struct Spritesheet *tmp, *s = character->spritesheets;
		tmp = s;
		while (s) {
			tmp = s;
			s = s->next;
			if (tmp->bitmap) al_destroy_bitmap(tmp->bitmap);
			if (tmp->successor) free(tmp->successor);
			free(tmp->name);
			free(tmp);
		}
	}

	if (character->bitmap) al_destroy_bitmap(character->bitmap);
	if (character->successor) free(character->successor);
	free(character->name);
	free(character);
}

SYMBOL_EXPORT void AnimateCharacter(struct Game *game, struct Character *character, float speed_modifier) {
	if (character->dead) return;
	if (speed_modifier) {
		character->pos_tmp++;
		if (character->pos_tmp >= character->spritesheet->delay / speed_modifier) {
			character->pos_tmp = 0;
			character->pos++;
		}
		if (character->pos>=character->spritesheet->cols*character->spritesheet->rows-character->spritesheet->blanks) {
			character->pos=0;
			if (character->repeat) {
				character->repeat--;
			} else {
				if (character->spritesheet->kill) {
					character->dead = true;
				} else if (character->successor) {
					SelectSpritesheet(game, character, character->successor);
				}
			}
		}
	}
}

SYMBOL_EXPORT void MoveCharacter(struct Game *game, struct Character *character, float x, float y, float angle) {
	MoveCharacterF(game, character, x / (float)game->viewport.width, y / (float)game->viewport.height, angle);
}

SYMBOL_EXPORT void MoveCharacterF(struct Game *game, struct Character *character, float x, float y, float angle) {
	if (character->dead) return;
	character->x += x;
	character->y += y;
	character->angle += angle;
}

SYMBOL_EXPORT void SetCharacterPositionF(struct Game *game, struct Character *character, float x, float y, float angle) {
	if (character->dead) return;
	character->x = x;
	character->y = y;
	character->angle = angle;
}

SYMBOL_EXPORT void SetCharacterPosition(struct Game *game, struct Character *character, float x, float y, float angle) {
	SetCharacterPositionF(game, character, x / (float)game->viewport.width, y / (float)game->viewport.height, angle);
}


SYMBOL_EXPORT void DrawScaledCharacterF(struct Game *game, struct Character *character, ALLEGRO_COLOR tint, float scalex, float scaley, int flags) {
	if (character->dead) return;
	int spritesheetX = al_get_bitmap_width(character->bitmap)*(character->pos%character->spritesheet->cols);
	int spritesheetY = al_get_bitmap_height(character->bitmap)*(character->pos/character->spritesheet->cols);
	al_draw_tinted_scaled_rotated_bitmap_region(character->spritesheet->bitmap, spritesheetX, spritesheetY, al_get_bitmap_width(character->bitmap), al_get_bitmap_height(character->bitmap), tint, al_get_bitmap_width(character->bitmap)/2, al_get_bitmap_height(character->bitmap)/2, character->x*game->viewport.width + al_get_bitmap_width(character->bitmap)*scalex/2, character->y*game->viewport.height + al_get_bitmap_height(character->bitmap)*scaley/2, scalex, scaley, character->angle, flags);
}

SYMBOL_EXPORT void DrawCharacterF(struct Game *game, struct Character *character, ALLEGRO_COLOR tint, int flags) {
	DrawScaledCharacterF(game, character, tint, 1, 1, flags);
}

SYMBOL_EXPORT void DrawScaledCharacter(struct Game *game, struct Character *character, ALLEGRO_COLOR tint, float scalex, float scaley, int flags) {
	if (character->dead) return;
	int spritesheetX = al_get_bitmap_width(character->bitmap)*(character->pos%character->spritesheet->cols);
	int spritesheetY = al_get_bitmap_height(character->bitmap)*(character->pos/character->spritesheet->cols);
	al_draw_tinted_scaled_rotated_bitmap_region(character->spritesheet->bitmap, spritesheetX, spritesheetY, al_get_bitmap_width(character->bitmap), al_get_bitmap_height(character->bitmap), tint, al_get_bitmap_width(character->bitmap)/2, al_get_bitmap_height(character->bitmap)/2, (int)(character->x*game->viewport.width + al_get_bitmap_width(character->bitmap)*scalex/2), (int)(character->y*game->viewport.height + al_get_bitmap_height(character->bitmap)*scaley/2), scalex, scaley, character->angle, flags);
}

SYMBOL_EXPORT void DrawCharacter(struct Game *game, struct Character *character, ALLEGRO_COLOR tint, int flags) {
	DrawScaledCharacter(game, character, tint, 1, 1, flags);
}

SYMBOL_EXPORT int GetCharacterX(struct Game *game, struct Character *character) {
	return character->x * game->viewport.width;
}

SYMBOL_EXPORT int GetCharacterY(struct Game *game, struct Character *character) {
	return character->y * game->viewport.height;
}

SYMBOL_EXPORT float GetCharacterAngle(struct Game *game, struct Character *character) {
	return character->angle;
}

SYMBOL_EXPORT bool IsOnCharacter(struct Game *game, struct Character *character, int x, int y) {
	int x1 = GetCharacterX(game, character), y1 = GetCharacterY(game, character);
	int x2 = x1 + al_get_bitmap_width(character->bitmap), y2 = y1 + al_get_bitmap_height(character->bitmap);

	return ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2));
}

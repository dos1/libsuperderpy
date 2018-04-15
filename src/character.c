/*! \file character.c
 *  \brief Character and spritesheet functions.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "internal.h"
#include "utils.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

SYMBOL_EXPORT void SelectSpritesheet(struct Game* game, struct Character* character, char* name) {
	struct Spritesheet* tmp = character->spritesheets;
	bool reversed = false;
	if (name[0] == '-') {
		reversed = true;
		name++;
	}
	PrintConsole(game, "Selecting spritesheet for %s: %s", character->name, name);
	if (!tmp) {
		PrintConsole(game, "ERROR: No spritesheets registered for %s!", character->name);
		return;
	}
	while (tmp) {
		if (!strcmp(tmp->name, name)) {
			character->spritesheet = tmp;
			if (character->successor) {
				free(character->successor);
			}
			if (tmp->successor) {
				character->successor = strdup(tmp->successor);
			} else {
				character->successor = NULL;
			}
			if (character->predecessor) {
				free(character->predecessor);
			}
			if (tmp->predecessor) {
				character->predecessor = strdup(tmp->predecessor);
			} else {
				character->predecessor = NULL;
			}
			character->repeats = tmp->repeats;
			character->pos = reversed ? (tmp->frameCount - 1) : 0;
			character->reversed = reversed;
			character->reversing = tmp->reversed ^ reversed;
			character->frame = &tmp->frames[character->pos];
			//character->bitmap = tmp->frames[character->pos].bitmap;
			PrintConsole(game, "SUCCESS: Spritesheet for %s activated: %s (%dx%d)", character->name, character->spritesheet->name, character->spritesheet->width, character->spritesheet->height);
			return;
		}
		tmp = tmp->next;
	}
	PrintConsole(game, "ERROR: No spritesheets registered for %s with given name: %s", character->name, name);
}

SYMBOL_EXPORT void EnqueueSpritesheet(struct Game* game, struct Character* character, char* name) {
	if (character->successor) {
		free(character->successor);
	}
	character->successor = strdup(name);
}

SYMBOL_EXPORT struct Spritesheet* GetSpritesheet(struct Game* game, struct Character* character, char* name) {
	struct Spritesheet* tmp = character->spritesheets;
	while (tmp) {
		if (!strcmp(tmp->name, name)) {
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;
}

SYMBOL_EXPORT void LoadSpritesheets(struct Game* game, struct Character* character) {
	PrintConsole(game, "Loading spritesheets for character %s...", character->name);
	struct Spritesheet* tmp = character->spritesheets;
	while (tmp) {
		if ((!tmp->bitmap) && (tmp->file)) {
			char filename[255] = {0};
			snprintf(filename, 255, "sprites/%s/%s", character->name, tmp->file);
			tmp->bitmap = al_load_bitmap(GetDataFilePath(game, filename));
			tmp->width = al_get_bitmap_width(tmp->bitmap) / tmp->cols;
			tmp->height = al_get_bitmap_height(tmp->bitmap) / tmp->rows;
		}
		for (int i = 0; i < tmp->frameCount; i++) {
			if ((!tmp->frames[i].bitmap) && (tmp->frames[i].file)) {
				char filename[255] = {0};
				snprintf(filename, 255, "sprites/%s/%s", character->name, tmp->frames[i].file);
				tmp->frames[i].bitmap = al_load_bitmap(GetDataFilePath(game, filename));
				int width = al_get_bitmap_width(tmp->frames[i].bitmap);
				if (width > tmp->width) {
					tmp->width = width;
				}
				int height = al_get_bitmap_height(tmp->frames[i].bitmap);
				if (height > tmp->height) {
					tmp->height = height;
				}
			} else if (!tmp->frames[i].bitmap) {
				tmp->frames[i].bitmap = al_create_sub_bitmap(tmp->bitmap, tmp->frames[i].col * tmp->width, tmp->frames[i].row * tmp->height, tmp->width, tmp->height);
			}
		}
		tmp = tmp->next;
	}

	if ((!character->spritesheet) && (character->spritesheets)) {
		SelectSpritesheet(game, character, character->spritesheets->name);
	}
}

SYMBOL_EXPORT void UnloadSpritesheets(struct Game* game, struct Character* character) {
	PrintConsole(game, "Unloading spritesheets for character %s...", character->name);
	struct Spritesheet* tmp = character->spritesheets;
	while (tmp) {
		for (int i = 0; i < tmp->frameCount; i++) {
			if (tmp->frames[i].bitmap) {
				al_destroy_bitmap(tmp->frames[i].bitmap);
			}
		}
		if (tmp->bitmap) {
			al_destroy_bitmap(tmp->bitmap);
		}
		tmp->bitmap = NULL;
		tmp = tmp->next;
	}
}

static long strtolnull(const char* _nptr, long val) {
	if (_nptr == NULL) {
		return val;
	}
	return strtol(_nptr, NULL, 10);
}

static double strtodnull(const char* _nptr, double val) {
	if (_nptr == NULL) {
		return val;
	}
	return strtod(_nptr, NULL);
}

SYMBOL_EXPORT void RegisterSpritesheet(struct Game* game, struct Character* character, char* name) {
	struct Spritesheet* s = character->spritesheets;
	while (s) {
		if (!strcmp(s->name, name)) {
			//PrintConsole(game, "%s spritesheet %s already registered!", character->name, name);
			return;
		}
		s = s->next;
	}
	PrintConsole(game, "Registering %s spritesheet: %s", character->name, name);
	char filename[255] = {0};
	snprintf(filename, 255, "sprites/%s/%s.ini", character->name, name);
	ALLEGRO_CONFIG* config = al_load_config_file(GetDataFilePath(game, filename));
	s = malloc(sizeof(struct Spritesheet));
	s->name = strdup(name);
	s->bitmap = NULL;
	s->frameCount = strtolnull(al_get_config_value(config, "animation", "frames"), 0);
	s->rows = strtolnull(al_get_config_value(config, "animation", "rows"), 0);
	s->cols = strtolnull(al_get_config_value(config, "animation", "cols"), 0);
	s->flipX = strtolnull(al_get_config_value(config, "animation", "flipX"), 0);
	s->flipY = strtolnull(al_get_config_value(config, "animation", "flipY"), 0);
	int blanks = strtolnull(al_get_config_value(config, "animation", "blanks"), 0);
	if (s->frameCount == 0) {
		s->frameCount = s->rows * s->cols - blanks;
	} else {
		s->rows = floor(sqrt(s->frameCount));
		s->cols = ceil(s->frameCount / (double)s->rows);
	}

	s->bidir = strtolnull(al_get_config_value(config, "animation", "bidir"), 0);
	s->reversed = strtolnull(al_get_config_value(config, "animation", "reversed"), 0);

	s->duration = strtodnull(al_get_config_value(config, "animation", "duration"), 16.66);

	s->width = 0;
	s->height = 0;

	s->repeats = strtolnull(al_get_config_value(config, "animation", "repeats"), -1);

	s->successor = NULL;
	const char* successor = al_get_config_value(config, "animation", "successor");
	if (successor) {
		int len = strlen(successor) + 1;
		s->successor = malloc(len * sizeof(char));
		strncpy(s->successor, successor, len);
	}

	s->predecessor = NULL;
	const char* predecessor = al_get_config_value(config, "animation", "predecessor");
	if (predecessor) {
		int len = strlen(predecessor) + 1;
		s->predecessor = malloc(len * sizeof(char));
		strncpy(s->predecessor, predecessor, len);
	}

	{
		s->file = NULL;
		const char* file = al_get_config_value(config, "animation", "file");
		if (file) {
			int len = strlen(file) + 1;
			s->file = malloc(len * sizeof(char));
			strncpy(s->file, file, len);
		}
	}

	s->pivotX = strtodnull(al_get_config_value(config, "pivot", "x"), 0.5);
	s->pivotY = strtodnull(al_get_config_value(config, "pivot", "y"), 0.5);

	s->frames = malloc(sizeof(struct SpritesheetFrame) * s->frameCount);

	for (int i = 0; i < s->frameCount; i++) {
		char framename[255];
		snprintf(framename, 255, "frame%d", i);
		s->frames[i].duration = strtodnull(al_get_config_value(config, framename, "duration"), s->duration);

		s->frames[i].bitmap = NULL;
		s->frames[i].x = strtolnull(al_get_config_value(config, framename, "x"), 0);
		s->frames[i].y = strtolnull(al_get_config_value(config, framename, "y"), 0);
		s->frames[i].flipX = strtolnull(al_get_config_value(config, framename, "flipX"), 0);
		s->frames[i].flipY = strtolnull(al_get_config_value(config, framename, "flipY"), 0);

		s->frames[i].file = NULL;
		const char* file = al_get_config_value(config, framename, "file");
		if (file) {
			int len = strlen(file) + 1;
			s->frames[i].file = malloc(len * sizeof(char));
			strncpy(s->frames[i].file, file, len);
		}

		if (!file) {
			s->frames[i].col = i % s->cols;
			s->frames[i].row = i / s->cols;

			const char* col_str = al_get_config_value(config, filename, "col");
			if (col_str) {
				s->frames[i].col = strtol(col_str, NULL, 10);
			}
			const char* row_str = al_get_config_value(config, filename, "row");
			if (row_str) {
				s->frames[i].row = strtol(row_str, NULL, 10);
			}
		}
	}

	s->next = character->spritesheets;
	character->spritesheets = s;
	al_destroy_config(config);
}

SYMBOL_EXPORT struct Character* CreateCharacter(struct Game* game, char* name) {
	PrintConsole(game, "Creating character %s...", name);
	struct Character* character = malloc(sizeof(struct Character));
	character->name = strdup(name);
	character->frame = NULL;
	character->spritesheet = NULL;
	character->spritesheets = NULL;
	character->pos = 0;
	character->delta = 0.0;
	character->successor = NULL;
	character->predecessor = NULL;
	character->x = -1;
	character->y = -1;
	character->tint = al_map_rgb(255, 255, 255);
	//character->pivotX = 0.5;
	//character->pivotY = 0.5;
	character->scaleX = 1.0;
	character->scaleY = 1.0;
	character->angle = 0;
	character->confineX = -1;
	character->confineY = -1;
	character->flipX = false;
	character->flipY = false;
	character->shared = false;
	character->repeats = -1;
	character->reversing = false;
	character->parent = NULL;
	character->hidden = false;
	character->data = NULL;
	character->callback = NULL;
	character->callbackData = NULL;

	return character;
}

SYMBOL_EXPORT void DestroyCharacter(struct Game* game, struct Character* character) {
	PrintConsole(game, "Destroying %scharacter %s...", character->shared ? "shared " : "", character->name);
	if (!character->shared) {
		struct Spritesheet *tmp, *s = character->spritesheets;
		while (s) {
			tmp = s;
			s = s->next;
			if (tmp->successor) {
				free(tmp->successor);
			}
			if (tmp->predecessor) {
				free(tmp->predecessor);
			}
			if (tmp->file) {
				free(tmp->file);
			}
			for (int i = 0; i < tmp->frameCount; i++) {
				if (tmp->frames[i].bitmap) {
					al_destroy_bitmap(tmp->frames[i].bitmap);
				}
				if (tmp->frames[i].file) {
					free(tmp->frames[i].file);
				}
			}
			if (tmp->bitmap) {
				al_destroy_bitmap(tmp->bitmap);
			}
			free(tmp->frames);
			free(tmp->name);
			free(tmp);
		}
	}

	//if (character->bitmap) {
	//	al_destroy_bitmap(character->bitmap);
	//}
	if (character->successor) {
		free(character->successor);
	}
	free(character->name);
	free(character);
}

SYMBOL_EXPORT void AnimateCharacter(struct Game* game, struct Character* character, float delta, float speed_modifier) {
	if (character->hidden) {
		return;
	}

	delta *= speed_modifier;
	character->delta += delta * 1000;

	while (character->delta >= character->spritesheet->duration) {
		bool reachedEnd = false;
		character->delta -= character->spritesheet->duration;
		if (character->reversing) {
			character->pos--;
		} else {
			character->pos++;
		}
		if (character->pos >= character->spritesheet->frameCount) {
			if (character->spritesheet->bidir) {
				character->pos -= 2;
				character->reversing = true;
				if (character->reversed) {
					reachedEnd = true;
				}
			} else {
				character->pos = 0;
				reachedEnd = true;
			}
		}
		if (character->pos < 0) {
			if (character->spritesheet->bidir) {
				character->pos += 2;
				character->reversing = false;
				if (!character->reversed) {
					reachedEnd = true;
				}
			} else {
				character->pos = character->spritesheet->frameCount - 1;
				reachedEnd = true;
			}
		}

		if (reachedEnd) {
			if (character->repeats > 0) {
				character->repeats--;
				if (character->callback) {
					character->callback(game, character, NULL, character->spritesheet->name, character->callbackData);
				}
			} else {
				if ((!character->reversed) && (character->successor)) {
					char* old = character->spritesheet->name;
					SelectSpritesheet(game, character, character->successor);
					if (character->callback) {
						character->callback(game, character, character->spritesheet->name, old, character->callbackData);
					}
				} else if ((character->reversed) && (character->predecessor)) {
					char* old = character->spritesheet->name;
					SelectSpritesheet(game, character, character->predecessor);
					if (character->callback) {
						character->callback(game, character, character->spritesheet->name, old, character->callbackData);
					}
				} else {
					if (character->repeats == 0) {
						if (character->reversed) {
							character->pos = 1;
						} else {
							character->pos = character->spritesheet->frameCount - 1;
						}
						if (character->callback) {
							character->callback(game, character, NULL, character->spritesheet->name, character->callbackData);
						}
					}
				}
			}
		}

		if (character->spritesheet->frameCount == 1) {
			character->pos = 0;
		}
	}

	character->frame = &character->spritesheet->frames[character->pos];
}

SYMBOL_EXPORT void MoveCharacter(struct Game* game, struct Character* character, float x, float y, float angle) {
	MoveCharacterF(game, character, x / (float)GetCharacterConfineX(game, character), y / (float)GetCharacterConfineY(game, character), angle);
}

SYMBOL_EXPORT void MoveCharacterF(struct Game* game, struct Character* character, float x, float y, float angle) {
	character->x += x;
	character->y += y;
	character->angle += angle;
}

SYMBOL_EXPORT void SetCharacterPositionF(struct Game* game, struct Character* character, float x, float y, float angle) {
	character->x = x;
	character->y = y;
	character->angle = angle;
}

SYMBOL_EXPORT void SetCharacterPosition(struct Game* game, struct Character* character, float x, float y, float angle) {
	SetCharacterPositionF(game, character, x / (float)GetCharacterConfineX(game, character), y / (float)GetCharacterConfineY(game, character), angle);
}

SYMBOL_EXPORT void DrawCharacter(struct Game* game, struct Character* character) {
	if (character->hidden) {
		return;
	}
	int w = character->spritesheet->width, h = character->spritesheet->height;

	ALLEGRO_TRANSFORM current = *al_get_current_transform();
	// TODO: move to function, use in IsOnCharacter etc.
	al_identity_transform(&character->transform);
	al_translate_transform(&character->transform, -w / 2, -h / 2);
	al_scale_transform(&character->transform, ((character->flipX ^ character->spritesheet->flipX ^ character->frame->flipX) ? -1 : 1), ((character->flipY ^ character->spritesheet->flipY ^ character->frame->flipY) ? -1 : 1)); // flipping; FIXME: should it be here or later?
	al_translate_transform(&character->transform, w / 2, h / 2);

	al_translate_transform(&character->transform, character->spritesheet->frames[character->pos].x, character->spritesheet->frames[character->pos].y); // spritesheet frame offset
	al_translate_transform(&character->transform, -w * character->spritesheet->pivotX, -h * character->spritesheet->pivotY); // pivot
	al_scale_transform(&character->transform, character->scaleX, character->scaleY);
	al_rotate_transform(&character->transform, character->angle);
	al_translate_transform(&character->transform, GetCharacterX(game, character), GetCharacterY(game, character)); // position

	ALLEGRO_TRANSFORM transform;
	al_identity_transform(&transform);
	al_compose_transform(&transform, &character->transform);
	al_compose_transform(&transform, &current);
	al_use_transform(&transform);

	al_draw_tinted_bitmap(character->frame->bitmap, character->tint, 0, 0, 0);

	al_use_transform(&current);
}

SYMBOL_EXPORT void SetCharacterConfines(struct Game* game, struct Character* character, int x, int y) {
	character->confineX = x;
	character->confineY = y;
}

SYMBOL_EXPORT int GetCharacterConfineX(struct Game* game, struct Character* character) {
	return (character->confineX >= 0) ? character->confineX : game->viewport.width;
}

SYMBOL_EXPORT int GetCharacterConfineY(struct Game* game, struct Character* character) {
	return (character->confineY >= 0) ? character->confineY : game->viewport.height;
}

SYMBOL_EXPORT float GetCharacterX(struct Game* game, struct Character* character) {
	return character->x * GetCharacterConfineX(game, character);
}

SYMBOL_EXPORT float GetCharacterY(struct Game* game, struct Character* character) {
	return character->y * GetCharacterConfineY(game, character);
}

/*
static void SortTwoFloats(float* v1, float* v2) {
	float pom = *v1;
	if (v1 > v2) {
		*v1 = *v2;
		*v2 = pom;
	}
}
*/

SYMBOL_EXPORT bool IsOnCharacter(struct Game* game, struct Character* character, float x, float y, bool pixelperfect) {
	// TODO: fucking rework
	if (character->hidden) {
		return false;
	}

	int w = character->spritesheet->width, h = character->spritesheet->height;

	//float x1 = GetCharacterX(game, character), y1 = GetCharacterY(game, character);
	//x -= GetCharacterX(game, character);
	//y -= GetCharacterY(game, character);
	float x1 = GetCharacterX(game, character) - (w / 2) * fabs(character->scaleX), y1 = GetCharacterY(game, character) - (h / 2) * fabs(character->scaleY);
	float x2 = x1 + w * fabs(character->scaleX), y2 = y1 + h * fabs(character->scaleY);
	//PrintConsole(game, "character %s x %f y %f; %fx%f; %fx%f -> %fx%f", character->name, x, y, character->scaleX, character->scaleY, x1, y1, x2, y2);

	/*	float scalex = character->scaleX;
	float scaley = character->scaleY;
	if (character->flipX) {
		scalex *= -1;
	}
	if (character->flipY) {
		scaley *= -1;
	}
	ALLEGRO_TRANSFORM transform;
	al_identity_transform(&transform);
	al_translate_transform(&transform, -character->pivotX * w, -character->pivotY * h);
	al_scale_transform(&transform, scalex, scaley);
	al_translate_transform(&transform, character->pivotX * w, character->pivotY * h);
	al_transform_coordinates(&transform, &x1, &y1);
	al_transform_coordinates(&transform, &x2, &y2);
	SortTwoFloats(&x1, &x2);
	SortTwoFloats(&y1, &y2);
*/
	bool test = ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2));

	//al_transform_coordinates(&transform, &x, &y);
	if (test && pixelperfect) {
		x -= x1;
		y -= y1;
		x /= fabs(character->scaleX);
		y /= fabs(character->scaleY);
		if (character->flipX) {
			x = w - x;
		}
		if (character->flipY) {
			y = h - y;
		}
		ALLEGRO_COLOR color = al_get_pixel(character->frame->bitmap, x, y);
		return (color.a > 0.0);
	}

	return test;
}

void ShowCharacter(struct Game* game, struct Character* character) {
	character->hidden = false;
}
void HideCharacter(struct Game* game, struct Character* character) {
	character->hidden = true;
}

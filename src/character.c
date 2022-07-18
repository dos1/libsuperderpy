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

	if (character->spritesheet && character->spritesheet->stream && character->frame) {
		if (character->frame->owned) {
			al_destroy_bitmap(character->frame->bitmap);
		}
		al_destroy_bitmap(character->frame->_priv.image);
		free(character->frame);
		character->frame = NULL;
	}

	while (tmp) {
		if (!strcmp(tmp->name, name)) {
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
			character->pos = reversed ? (tmp->frame_count - 1) : 0;
			character->reversed = reversed;
			character->reversing = tmp->reversed ^ reversed;
			if (tmp->stream) {
				character->frame = calloc(1, sizeof(struct SpritesheetFrame));
				*(character->frame) = tmp->stream(game, 0.0, character->pos, tmp->stream_data);
				character->frame->_priv.image = al_create_sub_bitmap(character->frame->bitmap, character->frame->sx * tmp->scale, character->frame->sy * tmp->scale, (character->frame->sw > 0) ? (character->frame->sw * tmp->scale) : al_get_bitmap_width(character->frame->bitmap), (character->frame->sh > 0) ? (character->frame->sh * tmp->scale) : al_get_bitmap_height(character->frame->bitmap));

				tmp->width = al_get_bitmap_width(character->frame->bitmap);
				tmp->height = al_get_bitmap_height(character->frame->bitmap);

				if (character->frame->end) {
					tmp->frame_count = character->pos + 1;
				}
			} else {
				character->frame = &tmp->frames[character->pos];
			}
			character->finished = false;
			character->spritesheet = tmp;
			PrintConsole(game, "SUCCESS: Spritesheet for %s activated: %s (%dx%d)", character->name, character->spritesheet->name, character->spritesheet->width, character->spritesheet->height);
			return;
		}
		tmp = tmp->next;
	}
	PrintConsole(game, "ERROR: No spritesheets registered for %s with given name: %s", character->name, name);
}

SYMBOL_EXPORT void SwitchSpritesheet(struct Game* game, struct Character* character, char* name) {
	int pos = character->pos;
	struct Spritesheet* old = character->spritesheet;
	bool oldrev = character->reversing;
	if (old && strcmp(name, old->name) == 0) {
		return;
	}
	SelectSpritesheet(game, character, name);
	if (old && old->bidir && character->spritesheet->bidir && oldrev) {
		character->reversing = oldrev;
	}
	if (pos < character->spritesheet->frame_count && !character->spritesheet->stream) {
		character->pos = pos;
		character->frame = &character->spritesheet->frames[character->pos];
	}
}

SYMBOL_EXPORT void EnqueueSpritesheet(struct Game* game, struct Character* character, char* name) {
	if (character->successor) {
		free(character->successor);
	}
	character->successor = strdup(name);
}

SYMBOL_EXPORT void SetSpritesheetPosition(struct Game* game, struct Character* character, int frame) {
	struct Spritesheet* spritesheet = character->spritesheet;
	if (!spritesheet) {
		return;
	}
	if (spritesheet->stream) {
		PrintConsole(game, "%s: tried to set position of a streaming spritesheet %s!", character->name, spritesheet->name);
		return;
	}
	if (frame < spritesheet->frame_count) {
		character->pos = frame;
		character->frame = &character->spritesheet->frames[character->pos];
	}
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

SYMBOL_EXPORT void LoadSpritesheets(struct Game* game, struct Character* character, void (*progress)(struct Game*)) {
	PrintConsole(game, "Loading spritesheets for character %s...", character->name);
	struct Spritesheet* tmp = character->spritesheets;
	while (tmp) {
		PrintConsole(game, "- %s", tmp->name);
		if (!tmp->stream) {
			if ((!tmp->bitmap) && (tmp->file)) {
				char filename[255] = {0};
				if (strstr(tmp->file, "../") == tmp->file) {
					snprintf(filename, 255, "sprites/%s", tmp->file + 3);
				} else {
					snprintf(filename, 255, "sprites/%s/%s", character->name, tmp->file);
				}
				tmp->filepath = strdup(filename);
				tmp->bitmap = AddBitmap(game, filename);
				tmp->width = (al_get_bitmap_width(tmp->bitmap) / tmp->scale) / tmp->cols;
				tmp->height = (al_get_bitmap_height(tmp->bitmap) / tmp->scale) / tmp->rows;
			}
			for (int i = 0; i < tmp->frame_count; i++) {
				if ((!tmp->frames[i].bitmap) && (tmp->frames[i].file)) {
					if (game->config.debug.enabled) {
						PrintConsole(game, "  - %s", tmp->frames[i].file);
					}
					char filename[255] = {0};
					if (strstr(tmp->frames[i].file, "../") == tmp->frames[i].file) {
						snprintf(filename, 255, "sprites/%s", tmp->frames[i].file + 3);
					} else {
						snprintf(filename, 255, "sprites/%s/%s", character->name, tmp->frames[i].file);
					}
					tmp->frames[i].bitmap = AddBitmap(game, filename);
					tmp->frames[i]._priv.filepath = strdup(filename);
				} else if (!tmp->frames[i].bitmap) {
					tmp->frames[i].bitmap = al_create_sub_bitmap(tmp->bitmap, tmp->frames[i].col * tmp->width * tmp->scale, tmp->frames[i].row * tmp->height * tmp->scale, tmp->width * tmp->scale, tmp->height * tmp->scale);
				}
				tmp->frames[i]._priv.image = al_create_sub_bitmap(tmp->frames[i].bitmap, tmp->frames[i].sx * tmp->scale, tmp->frames[i].sy * tmp->scale, (tmp->frames[i].sw > 0) ? (tmp->frames[i].sw * tmp->scale) : al_get_bitmap_width(tmp->frames[i].bitmap), (tmp->frames[i].sh > 0) ? (tmp->frames[i].sh * tmp->scale) : al_get_bitmap_height(tmp->frames[i].bitmap));

				int width = al_get_bitmap_width(tmp->frames[i]._priv.image) / tmp->scale + MAX(0, tmp->frames[i].x) + MAX(0, tmp->offsetX);
				if (width > tmp->width) {
					tmp->width = width;
				}
				int height = al_get_bitmap_height(tmp->frames[i]._priv.image) / tmp->scale + MAX(0, tmp->frames[i].y) + MAX(0, tmp->offsetY);
				if (height > tmp->height) {
					tmp->height = height;
				}
				if (character->detailed_progress && progress) {
					progress(game);
				}
			}
		}
		if (progress) {
			progress(game);
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
		for (int i = 0; i < tmp->frame_count; i++) {
			if (tmp->frames[i]._priv.filepath) {
				RemoveBitmap(game, tmp->frames[i]._priv.filepath);
			} else {
				if (tmp->frames[i].owned) {
					al_destroy_bitmap(tmp->frames[i].bitmap);
				}
			}
			al_destroy_bitmap(tmp->frames[i]._priv.image);
		}
		if (tmp->bitmap) {
			RemoveBitmap(game, tmp->filepath);
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

SYMBOL_EXPORT void PreloadStreamedSpritesheet(struct Game* game, struct Character* character, char* name) {
	struct Spritesheet* spritesheet = GetSpritesheet(game, character, name);
	if (!spritesheet->stream) {
		PrintConsole(game, "%s: tried to preload non-streaming spritesheet %s!", character->name, name);
		return;
	}

	for (int i = 0; i < spritesheet->frame_count; i++) {
		if (spritesheet->frames[i].file) {
			free(spritesheet->frames[i].file);
		}
		if (spritesheet->frames[i]._priv.filepath) {
			free(spritesheet->frames[i]._priv.filepath);
		}
	}
	free(spritesheet->frames);

	PrintConsole(game, "Preloading %s streaming spritesheet: %s", character->name, name);
	int size = 255, i = 0;
	double delta = 0;
	spritesheet->frames = calloc(size, sizeof(struct SpritesheetFrame));
	while (true) {
		PrintConsole(game, " - frame %d", i);
		spritesheet->frames[i] = spritesheet->stream(game, delta, i, spritesheet->stream_data);

		if (!spritesheet->frames[i].owned) {
			spritesheet->frames[i].bitmap = al_clone_bitmap(spritesheet->frames[i].bitmap);
			spritesheet->frames[i].owned = true;
		}

		spritesheet->frames[i]._priv.image = al_create_sub_bitmap(spritesheet->frames[i].bitmap, spritesheet->frames[i].sx * spritesheet->scale, spritesheet->frames[i].sy * spritesheet->scale, (spritesheet->frames[i].sw > 0) ? (spritesheet->frames[i].sw * spritesheet->scale) : al_get_bitmap_width(spritesheet->frames[i].bitmap), (spritesheet->frames[i].sh > 0) ? (spritesheet->frames[i].sh * spritesheet->scale) : al_get_bitmap_height(spritesheet->frames[i].bitmap));

		int width = al_get_bitmap_width(spritesheet->frames[i]._priv.image) / spritesheet->scale + MAX(0, spritesheet->frames[i].x) + MAX(0, spritesheet->offsetX);
		if (width > spritesheet->width) {
			spritesheet->width = width;
		}
		int height = al_get_bitmap_height(spritesheet->frames[i]._priv.image) / spritesheet->scale + MAX(0, spritesheet->frames[i].y) + MAX(0, spritesheet->offsetY);
		if (height > spritesheet->height) {
			spritesheet->height = height;
		}

		if (spritesheet->frames[i].end) {
			spritesheet->frame_count = i + 1;
			break;
		}

		delta = spritesheet->frames[i].duration / 1000.0;
		i++;
		if (i == size) {
			size += 255;
			spritesheet->frames = realloc(spritesheet->frames, sizeof(struct SpritesheetFrame) * size);
		}
	}
	spritesheet->frames = realloc(spritesheet->frames, sizeof(struct SpritesheetFrame) * spritesheet->frame_count);
	if (spritesheet->stream_destructor) {
		spritesheet->stream_destructor(game, spritesheet->stream_data);
	}
	spritesheet->stream = NULL;
	spritesheet->stream_data = NULL;
	spritesheet->stream_destructor = NULL;
}

SYMBOL_EXPORT void RegisterSpritesheet(struct Game* game, struct Character* character, char* name) {
	struct Spritesheet* s = character->spritesheets;
	while (s) {
		if (!strcmp(s->name, name)) {
			PrintConsole(game, "%s: spritesheet %s already registered!", character->name, name);
			return;
		}
		s = s->next;
	}
	PrintConsole(game, "Registering %s spritesheet: %s", character->name, name);
	char filename[255] = {0};
	snprintf(filename, 255, "sprites/%s/%s.ini", character->name, name);
	ALLEGRO_CONFIG* config = al_load_config_file(GetDataFilePath(game, filename));
	s = calloc(1, sizeof(struct Spritesheet));
	s->shared = false;
	s->name = strdup(name);
	s->bitmap = NULL;
	s->frame_count = strtolnull(al_get_config_value(config, "animation", "frames"), 0);
	s->rows = strtolnull(al_get_config_value(config, "animation", "rows"), 0);
	s->cols = strtolnull(al_get_config_value(config, "animation", "cols"), 0);
	s->flipX = strtolnull(al_get_config_value(config, "animation", "flipX"), 0);
	s->flipY = strtolnull(al_get_config_value(config, "animation", "flipY"), 0);
	int blanks = strtolnull(al_get_config_value(config, "animation", "blanks"), 0);
	if (s->frame_count == 0) {
		s->frame_count = s->rows * s->cols - blanks;
	} else {
		s->rows = floor(sqrt(s->frame_count));
		s->cols = ceil(s->frame_count / (double)s->rows);
	}

	s->bidir = strtolnull(al_get_config_value(config, "animation", "bidir"), 0);
	s->reversed = strtolnull(al_get_config_value(config, "animation", "reversed"), 0);

	s->duration = strtodnull(al_get_config_value(config, "animation", "duration"), 16.66);

	s->width = strtolnull(al_get_config_value(config, "animation", "width"), 0);
	s->height = strtolnull(al_get_config_value(config, "animation", "height"), 0);

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

	s->filepath = NULL;

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

	s->offsetX = strtolnull(al_get_config_value(config, "offset", "x"), 0);
	s->offsetY = strtolnull(al_get_config_value(config, "offset", "y"), 0);

	s->hitbox.x1 = strtodnull(al_get_config_value(config, "hitbox", "x1"), 0.0);
	s->hitbox.y1 = strtodnull(al_get_config_value(config, "hitbox", "y1"), 0.0);
	s->hitbox.x2 = strtodnull(al_get_config_value(config, "hitbox", "x2"), 0.0);
	s->hitbox.y2 = strtodnull(al_get_config_value(config, "hitbox", "y2"), 0.0);

	s->frames = calloc(s->frame_count, sizeof(struct SpritesheetFrame));

	for (int i = 0; i < s->frame_count; i++) {
		char framename[255];
		snprintf(framename, 255, "frame%d", i);
		s->frames[i].duration = strtodnull(al_get_config_value(config, framename, "duration"), s->duration);

		s->frames[i].bitmap = NULL;
		s->frames[i]._priv.image = NULL;
		s->frames[i].x = strtolnull(al_get_config_value(config, framename, "x"), 0);
		s->frames[i].y = strtolnull(al_get_config_value(config, framename, "y"), 0);
		s->frames[i].sx = strtolnull(al_get_config_value(config, framename, "sx"), 0);
		s->frames[i].sy = strtolnull(al_get_config_value(config, framename, "sy"), 0);
		s->frames[i].sw = strtolnull(al_get_config_value(config, framename, "sw"), 0);
		s->frames[i].sh = strtolnull(al_get_config_value(config, framename, "sh"), 0);
		s->frames[i].flipX = strtolnull(al_get_config_value(config, framename, "flipX"), 0);
		s->frames[i].flipY = strtolnull(al_get_config_value(config, framename, "flipY"), 0);

		double r = strtodnull(al_get_config_value(config, framename, "r"), 1.0);
		double g = strtodnull(al_get_config_value(config, framename, "g"), 1.0);
		double b = strtodnull(al_get_config_value(config, framename, "b"), 1.0);
		double a = strtodnull(al_get_config_value(config, framename, "a"), 1.0);
		s->frames[i].tint = al_premul_rgba_f(r, g, b, a);

		s->frames[i].file = NULL;
		const char* file = al_get_config_value(config, framename, "file");
		if (file) {
			int len = strlen(file) + 1;
			s->frames[i].file = malloc(len * sizeof(char));
			strncpy(s->frames[i].file, file, len);
		}
		s->frames[i]._priv.filepath = NULL;

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
		s->frames[i].start = i == 0 ? true : false;
		s->frames[i].end = i == (s->frame_count - 1) ? true : false;
	}

	s->scale = LIBSUPERDERPY_IMAGE_SCALE;

	s->stream = NULL;
	s->stream_data = NULL;
	s->stream_destructor = NULL;

	s->next = character->spritesheets;
	character->spritesheets = s;
	al_destroy_config(config);
}

SYMBOL_EXPORT void RegisterStreamedSpritesheet(struct Game* game, struct Character* character, char* name, SpritesheetStream* callback, SpritesheetStreamDestructor* destructor, void* data) {
	RegisterSpritesheet(game, character, name);
	struct Spritesheet* spritesheet = GetSpritesheet(game, character, name);
	spritesheet->stream = callback;
	spritesheet->stream_data = data;
	spritesheet->stream_destructor = destructor;
	spritesheet->scale = 1.0;
}

SYMBOL_EXPORT void RegisterSpritesheetFromBitmap(struct Game* game, struct Character* character, char* name, ALLEGRO_BITMAP* bitmap) {
	struct Spritesheet* s = character->spritesheets;
	while (s) {
		if (!strcmp(s->name, name)) {
			PrintConsole(game, "%s: spritesheet %s already registered!", character->name, name);
			return;
		}
		s = s->next;
	}
	PrintConsole(game, "Registering %s spritesheet: %s (from bitmap)", character->name, name);
	s = calloc(1, sizeof(struct Spritesheet));
	s->name = strdup(name);
	s->bitmap = bitmap;
	s->frame_count = 1;
	s->rows = 1;
	s->cols = 1;
	s->flipX = false;
	s->flipY = false;
	s->bidir = false;
	s->reversed = false;
	s->duration = 16.66;
	s->width = al_get_bitmap_width(bitmap);
	s->height = al_get_bitmap_height(bitmap);
	s->repeats = -1;
	s->successor = NULL;
	s->predecessor = NULL;
	s->filepath = NULL;
	s->file = NULL;
	s->pivotX = 0.5;
	s->pivotY = 0.5;
	s->offsetX = 0;
	s->offsetY = 0;
	s->shared = true;
	s->scale = LIBSUPERDERPY_IMAGE_SCALE;
	s->stream = NULL;
	s->stream_data = NULL;
	s->stream_destructor = NULL;

	s->frames = calloc(1, sizeof(struct SpritesheetFrame));

	s->frames[0].duration = 16.66;
	s->frames[0].bitmap = NULL;
	s->frames[0]._priv.image = NULL;
	s->frames[0].x = 0;
	s->frames[0].y = 0;
	s->frames[0].sx = 0;
	s->frames[0].sy = 0;
	s->frames[0].sw = 0;
	s->frames[0].sh = 0;
	s->frames[0].flipX = false;
	s->frames[0].flipY = false;
	s->frames[0].tint = al_premul_rgba_f(1.0, 1.0, 1.0, 1.0);
	s->frames[0].file = NULL;
	s->frames[0]._priv.filepath = NULL;
	s->frames[0].col = 0;
	s->frames[0].row = 0;
	s->frames[0].start = true;
	s->frames[0].end = true;

	s->next = character->spritesheets;
	character->spritesheets = s;
}

SYMBOL_EXPORT struct Character* CreateCharacter(struct Game* game, char* name) {
	if (name) {
		PrintConsole(game, "Creating character %s...", name);
	}
	struct Character* character = calloc(1, sizeof(struct Character));
	character->name = name ? strdup(name) : NULL;
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
	character->parent_tint = true;
	character->hidden = false;
	character->data = NULL;
	character->callback = NULL;
	character->callback_data = NULL;
	character->destructor = NULL;
	character->detailed_progress = false;
	character->bounds.enabled = false;

	return character;
}

SYMBOL_EXPORT void DestroyCharacter(struct Game* game, struct Character* character) {
	if (!character->shared) {
		PrintConsole(game, "Destroying character %s...", character->name);
	}

	if (character->destructor) {
		character->destructor(game, character);
	}

	if (character->spritesheet && character->spritesheet->stream) {
		if (character->frame) {
			if (character->frame->owned) {
				al_destroy_bitmap(character->frame->bitmap);
			}
			al_destroy_bitmap(character->frame->_priv.image);
			free(character->frame);
		}
	}

	if (!character->shared) {
		struct Spritesheet* s = character->spritesheets;
		while (s) {
			struct Spritesheet* tmp = s;
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
			for (int i = 0; i < tmp->frame_count; i++) {
				if (tmp->frames[i]._priv.filepath && !tmp->shared) {
					RemoveBitmap(game, tmp->frames[i]._priv.filepath);
				} else {
					al_destroy_bitmap(tmp->frames[i]._priv.image);
				}
				if (tmp->frames[i].file) {
					free(tmp->frames[i].file);
				}
				if (tmp->frames[i]._priv.filepath) {
					free(tmp->frames[i]._priv.filepath);
				}
			}
			if (tmp->stream && tmp->stream_destructor) {
				tmp->stream_destructor(game, tmp->stream_data);
			}
			if (tmp->bitmap && !tmp->shared) {
				RemoveBitmap(game, tmp->filepath);
			}
			if (tmp->filepath) {
				free(tmp->filepath);
			}
			free(tmp->frames);
			free(tmp->name);
			free(tmp);
		}
	}

	if (character->successor) {
		free(character->successor);
	}
	if (character->predecessor) {
		free(character->predecessor);
	}
	if (character->name) {
		free(character->name);
	}
	free(character);
}

SYMBOL_EXPORT void AnimateCharacter(struct Game* game, struct Character* character, float delta, float speed_modifier) {
	if (IsCharacterHidden(game, character)) {
		return;
	}

	if (character->finished) {
		return;
	}

	delta *= speed_modifier;
	character->delta += delta * 1000;

	int pos = character->pos;

	while (character->delta >= character->frame->duration) {
		bool reachedEnd = false;
		character->delta -= character->frame->duration;

		if (character->reversing) {
			if (character->spritesheet->stream) {
				FatalError(game, true, "Tried to animate streaming spritesheet '%s' of character '%s' in reverse", character->spritesheet->name, character->name);
				QuitGame(game, false);
				return;
			}

			if (character->frame->start) {
				if (character->spritesheet->bidir) {
					if (!character->frame->end) {
						character->pos++;
					}
					character->reversing = false;
					if (!character->reversed) {
						reachedEnd = true;
					}
				} else {
					character->pos = character->spritesheet->frame_count - 1;
					reachedEnd = true;
				}
			} else {
				character->pos--;
			}
		} else {
			if (character->frame->end) {
				if (character->spritesheet->bidir) {
					if (character->spritesheet->stream) {
						FatalError(game, true, "Tried to animate streaming spritesheet '%s' of character '%s' in bidir", character->spritesheet->name, character->name);
						QuitGame(game, false);
						return;
					}
					if (!character->frame->start) {
						character->pos--;
					}
					character->reversing = true;
					if (character->reversed) {
						reachedEnd = true;
					}
				} else {
					character->pos = 0;
					reachedEnd = true;
				}
			} else {
				character->pos++;
			}
		}

		if (reachedEnd) {
			if (character->repeats > 0) {
				character->repeats--;
				if (character->callback) {
					character->callback(game, character, NULL, character->spritesheet, character->callback_data);
				}
			} else {
				if ((!character->reversed) && (character->successor)) {
					struct Spritesheet* old = character->spritesheet;
					SelectSpritesheet(game, character, character->successor);
					if (character->callback) {
						character->callback(game, character, character->spritesheet, old, character->callback_data);
					}
				} else if ((character->reversed) && (character->predecessor)) {
					struct Spritesheet* old = character->spritesheet;
					SelectSpritesheet(game, character, character->predecessor);
					if (character->callback) {
						character->callback(game, character, character->spritesheet, old, character->callback_data);
					}
				} else {
					if (character->repeats == 0) {
						if (character->reversed) {
							character->pos = 0;
						} else {
							character->pos = character->spritesheet->frame_count - 1;
						}
						character->finished = true;
						if (character->callback) {
							character->callback(game, character, NULL, character->spritesheet, character->callback_data);
						}
					}
				}
			}
		}

		if (character->spritesheet->stream) {
			if (!reachedEnd && pos != character->pos) {
				pos = character->pos;
				double duration = character->frame->duration;
				ALLEGRO_BITMAP* image = character->frame->_priv.image;
				if (character->frame->owned) {
					al_destroy_bitmap(character->frame->bitmap);
				}
				*(character->frame) = character->spritesheet->stream(game, duration, pos, character->spritesheet->stream_data);
				character->frame->_priv.image = image;
				al_reparent_bitmap(character->frame->_priv.image, character->frame->bitmap, character->frame->sx * character->spritesheet->scale, character->frame->sy * character->spritesheet->scale, (character->frame->sw > 0) ? (character->frame->sw * character->spritesheet->scale) : al_get_bitmap_width(character->frame->bitmap), (character->frame->sh > 0) ? (character->frame->sh * character->spritesheet->scale) : al_get_bitmap_height(character->frame->bitmap));
			}
		} else {
			character->frame = &character->spritesheet->frames[character->pos];
		}
	}
}

static void BoundCharacter(struct Game* game, struct Character* character) {
	if (!character->bounds.enabled) {
		return;
	}

	if (character->x < character->bounds.x1) {
		character->x = character->bounds.x1;
	}
	if (character->y < character->bounds.y1) {
		character->y = character->bounds.y1;
	}
	if (character->x > character->bounds.x2) {
		character->x = character->bounds.x2;
	}
	if (character->y > character->bounds.y2) {
		character->y = character->bounds.y2;
	}
}

SYMBOL_EXPORT void MoveCharacter(struct Game* game, struct Character* character, float x, float y, float angle) {
	MoveCharacterF(game, character, x / (float)GetCharacterConfineX(game, character), y / (float)GetCharacterConfineY(game, character), angle);
}

SYMBOL_EXPORT void MoveCharacterF(struct Game* game, struct Character* character, float x, float y, float angle) {
	character->x += x;
	character->y += y;
	character->angle += angle;
	BoundCharacter(game, character);
}

SYMBOL_EXPORT void SetCharacterPositionF(struct Game* game, struct Character* character, float x, float y, float angle) {
	character->x = x;
	character->y = y;
	character->angle = angle;
	BoundCharacter(game, character);
}

SYMBOL_EXPORT void SetCharacterPosition(struct Game* game, struct Character* character, float x, float y, float angle) {
	SetCharacterPositionF(game, character, x / (float)GetCharacterConfineX(game, character), y / (float)GetCharacterConfineY(game, character), angle);
}

SYMBOL_EXPORT ALLEGRO_TRANSFORM GetCharacterTransform(struct Game* game, struct Character* character) {
	ALLEGRO_TRANSFORM transform;
	int w = character->spritesheet->width, h = character->spritesheet->height;
	al_identity_transform(&transform);

	al_translate_transform(&transform, -w * character->spritesheet->pivotX, -h * character->spritesheet->pivotY);
	al_scale_transform(&transform, character->flipX ? -1 : 1, character->flipY ? -1 : 1);
	al_scale_transform(&transform, character->scaleX, character->scaleY);
	al_rotate_transform(&transform, character->angle);
	al_translate_transform(&transform, GetCharacterX(game, character), GetCharacterY(game, character));

	if (character->parent) {
		ALLEGRO_TRANSFORM parent = GetCharacterTransform(game, character->parent);
		struct Spritesheet* parent_spritesheet = character->parent->spritesheet;
		al_translate_transform(&transform, parent_spritesheet->width * parent_spritesheet->pivotX, parent_spritesheet->height * parent_spritesheet->pivotY);
		al_compose_transform(&transform, &parent);
	}

	return transform;
}

SYMBOL_EXPORT ALLEGRO_COLOR GetCharacterTint(struct Game* game, struct Character* character) {
	ALLEGRO_COLOR color;
	if (character->parent && character->parent_tint) {
		float r = 0, g = 0, b = 0, a = 0;
		al_unmap_rgba_f(character->tint, &r, &g, &b, &a);
		float r2 = 0, g2 = 0, b2 = 0, a2 = 0;
		al_unmap_rgba_f(GetCharacterTint(game, character->parent), &r2, &g2, &b2, &a2);

		color = al_map_rgba_f(r * r2, g * g2, b * b2, a * a2);
	} else {
		color = character->tint;
	}

	float r = 0, g = 0, b = 0, a = 0, r2 = 0, g2 = 0, b2 = 0, a2 = 0;
	al_unmap_rgba_f(color, &r, &g, &b, &a);
	al_unmap_rgba_f(character->frame->tint, &r2, &g2, &b2, &a2);
	return al_map_rgba_f(r * r2, g * g2, b * b2, a * a2);
}

static bool HasValidHitbox(struct Spritesheet* spritesheet) {
	return spritesheet->hitbox.x1 != 0.0 && spritesheet->hitbox.y1 != 0.0 && spritesheet->hitbox.x2 != 0.0 && spritesheet->hitbox.y2 != 0.0;
}

SYMBOL_EXPORT void DrawCharacter(struct Game* game, struct Character* character) {
	if (IsCharacterHidden(game, character)) {
		return;
	}

	ALLEGRO_TRANSFORM current = *al_get_current_transform();

	ALLEGRO_TRANSFORM transform = GetCharacterTransform(game, character);
	al_compose_transform(&transform, &current);
	al_use_transform(&transform);

	al_draw_tinted_scaled_bitmap(character->frame->_priv.image, GetCharacterTint(game, character),
		0, 0,
		al_get_bitmap_width(character->frame->_priv.image), al_get_bitmap_height(character->frame->_priv.image),
		character->frame->x + character->spritesheet->offsetX, character->frame->y + character->spritesheet->offsetY,
		al_get_bitmap_width(character->frame->_priv.image) / character->spritesheet->scale, al_get_bitmap_height(character->frame->_priv.image) / character->spritesheet->scale,
		((character->spritesheet->flipX ^ character->frame->flipX) ? ALLEGRO_FLIP_HORIZONTAL : 0) | ((character->spritesheet->flipY ^ character->frame->flipY) ? ALLEGRO_FLIP_VERTICAL : 0));

	al_use_transform(&current);
}

SYMBOL_EXPORT void DrawDebugCharacter(struct Game* game, struct Character* character) {
	if (!game->config.debug.enabled || !game->show_console || IsCharacterHidden(game, character)) {
		return;
	}

	ALLEGRO_TRANSFORM current = *al_get_current_transform();

	ALLEGRO_TRANSFORM transform = GetCharacterTransform(game, character);
	al_compose_transform(&transform, &current);
	al_use_transform(&transform);

	al_draw_rectangle(0, 0, character->spritesheet->width, character->spritesheet->height, al_map_rgb(0, 255, 255), 5);

	if (HasValidHitbox(character->spritesheet)) {
		al_draw_rectangle(character->spritesheet->hitbox.x1 * character->spritesheet->width, character->spritesheet->hitbox.y1 * character->spritesheet->height,
			character->spritesheet->hitbox.x2 * character->spritesheet->width, character->spritesheet->hitbox.y2 * character->spritesheet->height,
			al_map_rgb(255, 255, 0), 3);
	}

	al_draw_filled_rectangle(character->spritesheet->width * character->spritesheet->pivotX - 5,
		character->spritesheet->height * character->spritesheet->pivotY - 5,
		character->spritesheet->width * character->spritesheet->pivotX + 5,
		character->spritesheet->height * character->spritesheet->pivotY + 5,
		al_map_rgb(255, 0, 255));

	al_use_transform(&current);
}

SYMBOL_EXPORT void CopyCharacter(struct Game* game, struct Character* from, struct Character* to) {
	to->shared = true;
	if (to->name) {
		free(to->name);
	}
	to->name = from->name ? strdup(from->name) : NULL;
	to->spritesheets = from->spritesheets;
	to->spritesheet = from->spritesheet;
	to->frame = from->frame;
	to->delta = from->delta;
	to->pos = from->pos;
	to->predecessor = from->predecessor ? strdup(from->predecessor) : NULL;
	to->repeats = from->repeats;
	to->reversed = from->reversed;
	to->reversing = from->reversing;
	to->successor = from->successor ? strdup(from->successor) : NULL;
	to->frame = &to->spritesheet->frames[to->pos];
}

SYMBOL_EXPORT void SetParentCharacter(struct Game* game, struct Character* character, struct Character* parent) {
	character->parent = parent;
}

SYMBOL_EXPORT void SetCharacterConfines(struct Game* game, struct Character* character, int x, int y) {
	character->confineX = x;
	character->confineY = y;
}

SYMBOL_EXPORT void SetCharacterBounds(struct Game* game, struct Character* character, float x1, float y1, float x2, float y2) {
	SetCharacterBoundsF(game, character,
		x1 / GetCharacterConfineX(game, character), y1 / GetCharacterConfineY(game, character),
		x2 / GetCharacterConfineX(game, character), y2 / GetCharacterConfineY(game, character));
}

SYMBOL_EXPORT void SetCharacterBoundsF(struct Game* game, struct Character* character, float x1, float y1, float x2, float y2) {
	character->bounds.x1 = x1;
	character->bounds.y1 = y1;
	character->bounds.x2 = x2;
	character->bounds.y2 = y2;
	character->bounds.enabled = true;
}

SYMBOL_EXPORT void SetCharacterUnbounded(struct Game* game, struct Character* character) {
	character->bounds.enabled = false;
}

SYMBOL_EXPORT int GetCharacterConfineX(struct Game* game, struct Character* character) {
	return (character->confineX >= 0) ? character->confineX : (character->parent ? GetCharacterConfineX(game, character->parent) : game->viewport.width);
}

SYMBOL_EXPORT int GetCharacterConfineY(struct Game* game, struct Character* character) {
	return (character->confineY >= 0) ? character->confineY : (character->parent ? GetCharacterConfineY(game, character->parent) : game->viewport.height);
}

SYMBOL_EXPORT float GetCharacterX(struct Game* game, struct Character* character) {
	return character->x * GetCharacterConfineX(game, character);
}

SYMBOL_EXPORT float GetCharacterY(struct Game* game, struct Character* character) {
	return character->y * GetCharacterConfineY(game, character);
}

static void SortTwoFloats(float* v1, float* v2) {
	float pom = *v1;
	if (*v1 > *v2) {
		*v1 = *v2;
		*v2 = pom;
	}
}

SYMBOL_EXPORT bool IsOnCharacter(struct Game* game, struct Character* character, float x, float y, bool pixelperfect) {
	if (IsCharacterHidden(game, character)) {
		return false;
	}

	float x1 = MIN(0.0, character->spritesheet->offsetX) + MIN(0.0, character->frame->x), y1 = MIN(0.0, character->spritesheet->offsetY) + MIN(0.0, character->frame->y);
	float x2 = character->spritesheet->width, y2 = character->spritesheet->height;

	if (HasValidHitbox(character->spritesheet)) {
		PrintConsole(game, "valid %f %d", character->spritesheet->hitbox.x1, isnan(character->spritesheet->hitbox.x1));
		x1 = character->spritesheet->hitbox.x1 * character->spritesheet->width;
		y1 = character->spritesheet->hitbox.y1 * character->spritesheet->height;
		x2 = character->spritesheet->hitbox.x2 * character->spritesheet->width;
		y2 = character->spritesheet->hitbox.y2 * character->spritesheet->height;
	}

	ALLEGRO_TRANSFORM transform = GetCharacterTransform(game, character);
	al_transform_coordinates(&transform, &x1, &y1);
	al_transform_coordinates(&transform, &x2, &y2);
	SortTwoFloats(&x1, &x2);
	SortTwoFloats(&y1, &y2);

	bool test = ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2));

	if (test && pixelperfect) {
		al_invert_transform(&transform);
		al_transform_coordinates(&transform, &x, &y);
		int xpos = (x - character->frame->x - character->spritesheet->offsetX) * character->spritesheet->scale;
		int ypos = (y - character->frame->y - character->spritesheet->offsetY) * character->spritesheet->scale;
		if (xpos < 0 || ypos < 0 || xpos >= al_get_bitmap_width(character->frame->_priv.image) || ypos >= al_get_bitmap_height(character->frame->_priv.image)) {
			return false;
		}
		ALLEGRO_COLOR color = al_get_pixel(character->frame->_priv.image, xpos, ypos);
		return (color.a > 0.0);
	}

	return test;
}

SYMBOL_EXPORT void ShowCharacter(struct Game* game, struct Character* character) {
	character->hidden = false;
}

SYMBOL_EXPORT void HideCharacter(struct Game* game, struct Character* character) {
	character->hidden = true;
}

SYMBOL_EXPORT bool IsCharacterHidden(struct Game* game, struct Character* character) {
	if (character->hidden) {
		return true;
	}
	if (character->parent) {
		return IsCharacterHidden(game, character->parent);
	}
	return false;
}

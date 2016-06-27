/*! \file utils.c
 *  \brief Helper functions.
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
#include "stdio.h"
#include "config.h"
#include "string.h"
#include "math.h"
#include "utils.h"

char* strdup(const char *str) {
	int n = strlen(str) + 1;
	char *dup = malloc(n);
	if (dup) { strcpy(dup, str); }
	return dup;
}

void DrawConsole(struct Game *game) {
	if (game->_priv.showconsole) {
		ALLEGRO_TRANSFORM trans;
		al_identity_transform(&trans);
		int clipX, clipY, clipWidth, clipHeight;
		al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
		al_use_transform(&trans);

		al_draw_bitmap(game->_priv.console, clipX, clipY, 0);
		double game_time = al_get_time();
		if(game_time - game->_priv.fps_count.old_time >= 1.0) {
			game->_priv.fps_count.fps = game->_priv.fps_count.frames_done / (game_time - game->_priv.fps_count.old_time);
			game->_priv.fps_count.frames_done = 0;
			game->_priv.fps_count.old_time = game_time;
		}
		char sfps[6] = { };
		snprintf(sfps, 6, "%.0f", game->_priv.fps_count.fps);
		al_use_transform(&game->projection);

		DrawTextWithShadow(game->_priv.font, al_map_rgb(255,255,255), game->viewport.width*0.99, 0, ALLEGRO_ALIGN_RIGHT, sfps);

	}
	game->_priv.fps_count.frames_done++;
}

void Console_Load(struct Game *game) {
	game->_priv.font_console = NULL;
	game->_priv.console = NULL;
	game->_priv.font_console = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"),al_get_display_height(game->display)*0.025,0 );
	if (al_get_display_height(game->display)*0.025 >= 16) {
        game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/PerfectDOSVGA437.ttf"),16 * ((al_get_display_height(game->display) > 1080) ? 2 : 1) ,0 );
	} else {
		game->_priv.font_bsod = al_load_ttf_font(GetDataFilePath(game, "fonts/DejaVuSansMono.ttf"), al_get_display_height(game->display)*0.025,0 );
	}
	game->_priv.console = al_create_bitmap((al_get_display_width(game->display) / 320) * 320, al_get_font_line_height(game->_priv.font_console)*5);
	game->_priv.font = al_load_ttf_font(GetDataFilePath(game, "fonts/MonkeyIsland.ttf"), 0 ,0 );
	al_set_target_bitmap(game->_priv.console);
	al_clear_to_color(al_map_rgba(0,0,0,80));
	al_set_target_bitmap(al_get_backbuffer(game->display));
}

void Console_Unload(struct Game *game) {
	al_destroy_font(game->_priv.font);
	al_destroy_font(game->_priv.font_console);
	al_destroy_bitmap(game->_priv.console);
}


void SetupViewport(struct Game *game) {
	game->viewport.width = 320;
	game->viewport.height = 180;

	al_clear_to_color(al_map_rgb(0,0,0));

	int resolution = al_get_display_width(game->display) / 320;
	if (al_get_display_height(game->display) / 180 < resolution) resolution = al_get_display_height(game->display) / 180;
	if (resolution < 1) resolution = 1;

	if (atoi(GetConfigOptionDefault(game, "SuperDerpy", "letterbox", "1"))) {
		int clipWidth = 320 * resolution, clipHeight = 180 * resolution;
		int clipX = (al_get_display_width(game->display) - clipWidth) / 2, clipY = (al_get_display_height(game->display) - clipHeight) / 2;
		al_set_clipping_rectangle(clipX, clipY, clipWidth, clipHeight);

		al_build_transform(&game->projection, clipX, clipY, resolution, resolution, 0.0f);
		al_use_transform(&game->projection);

	} else if ((atoi(GetConfigOptionDefault(game, "SuperDerpy", "rotate", "1"))) && (game->viewport.height > game->viewport.width)) {
		al_identity_transform(&game->projection);
		al_rotate_transform(&game->projection, 0.5*ALLEGRO_PI);
		al_translate_transform(&game->projection, game->viewport.width, 0);
		al_scale_transform(&game->projection, resolution, resolution);
		al_use_transform(&game->projection);
		int temp = game->viewport.height;
		game->viewport.height = game->viewport.width;
		game->viewport.width = temp;
	}
	if (game->_priv.console) Console_Unload(game);
	Console_Load(game);
}

void DrawVerticalGradientRect(float x, float y, float w, float h, ALLEGRO_COLOR top, ALLEGRO_COLOR bottom) {
	ALLEGRO_VERTEX v[] = {
		{.x = x, .y = y, .z = 0, .color = top},
		{.x = x + w, .y = y, .z = 0, .color = top},
		{.x = x, .y = y + h, .z = 0, .color = bottom},
		{.x = x + w, .y = y + h, .z = 0, .color = bottom}};
	al_draw_prim(v, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP);
}

void DrawHorizontalGradientRect(float x, float y, float w, float h, ALLEGRO_COLOR left, ALLEGRO_COLOR right) {
	ALLEGRO_VERTEX v[] = {
		{.x = x, .y = y, .z = 0, .color = left},
		{.x = x + w, .y = y, .z = 0, .color = right},
		{.x = x, .y = y + h, .z = 0, .color = left},
		{.x = x + w, .y = y + h, .z = 0, .color = right}};
	al_draw_prim(v, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP);
}

void DrawTextWithShadow(ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int flags, char const *text) {
	al_draw_text(font, al_map_rgba(0,0,0,128), (int)x+1, (int)y+1, flags, text);
	al_draw_text(font, color, (int)x, (int)y, flags, text);
}

/* linear filtering code written by SiegeLord */
ALLEGRO_COLOR interpolate(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float frac) {
	return al_map_rgba_f(c1.r + frac * (c2.r - c1.r),
											 c1.g + frac * (c2.g - c1.g),
											 c1.b + frac * (c2.b - c1.b),
											 c1.a + frac * (c2.a - c1.a));
}

/*! \brief Scales bitmap using software linear filtering method to current target. */
void ScaleBitmap(ALLEGRO_BITMAP* source, int width, int height) {
	if ((al_get_bitmap_width(source)==width) && (al_get_bitmap_height(source)==height)) {
		al_draw_bitmap(source, 0, 0, 0);
		return;
	}
	int x, y;
	al_lock_bitmap(al_get_target_bitmap(), ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
	al_lock_bitmap(source, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

	for (y = 0; y < height; y++) {
		float pixy = ((float)y / height) * ((float)al_get_bitmap_height(source) - 1);
		float pixy_f = floor(pixy);
		for (x = 0; x < width; x++) {
			float pixx = ((float)x / width) * ((float)al_get_bitmap_width(source) - 1);
			float pixx_f = floor(pixx);

			ALLEGRO_COLOR a = al_get_pixel(source, pixx_f, pixy_f);
			ALLEGRO_COLOR b = al_get_pixel(source, pixx_f + 1, pixy_f);
			ALLEGRO_COLOR c = al_get_pixel(source, pixx_f, pixy_f + 1);
			ALLEGRO_COLOR d = al_get_pixel(source, pixx_f + 1, pixy_f + 1);

			ALLEGRO_COLOR ab = interpolate(a, b, pixx - pixx_f);
			ALLEGRO_COLOR cd = interpolate(c, d, pixx - pixx_f);
			ALLEGRO_COLOR result = interpolate(ab, cd, pixy - pixy_f);

			al_put_pixel(x, y, result);
		}
	}
	al_unlock_bitmap(al_get_target_bitmap());
	al_unlock_bitmap(source);
}

ALLEGRO_BITMAP* LoadScaledBitmap(struct Game *game, char* filename, int width, int height) {
	bool memoryscale = !atoi(GetConfigOptionDefault(game, "SuperDerpy", "GPU_scaling", "1"));
	ALLEGRO_BITMAP *source, *target = al_create_bitmap(width, height);
	al_set_target_bitmap(target);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	char* origfn = GetDataFilePath(game, filename);

	if (memoryscale) al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

	source = al_load_bitmap( origfn );
	if (memoryscale) {
		al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR);
		ScaleBitmap(source, width, height);
	}
	else {
		al_draw_scaled_bitmap(source, 0, 0, al_get_bitmap_width(source), al_get_bitmap_height(source), 0, 0, width, height, 0);
	}

	al_destroy_bitmap(source);

	free(origfn);
	return target;

}

void FatalError(struct Game *game, bool fatal, char* format, ...) {
	char text[1024] = {};
	if (!game->_priv.console) {
		va_list vl;
		va_start(vl, format);
		vsnprintf(text, 1024, format, vl);
		va_end(vl);
		printf("%s\n", text);
	} else {
		PrintConsole(game, "Fatal Error, displaying Blue Screen of Derp...");
		va_list vl;
		va_start(vl, format);
		vsnprintf(text, 1024, format, vl);
		va_end(vl);
		PrintConsole(game, text);
	}

	ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	al_use_transform(&trans);

	if (!game->_priv.font_bsod) {
		game->_priv.font_bsod = al_create_builtin_font();
	}

	al_set_target_backbuffer(game->display);
	al_clear_to_color(al_map_rgb(0,0,170));
	al_flip_display();
	al_rest(0.6);

	bool done = false;
	while (!done) {

		al_set_target_backbuffer(game->display);
		al_clear_to_color(al_map_rgb(0,0,170));

        char *header = "MEDIATOR";

		al_draw_filled_rectangle(al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header)/2 - 4, (int)(al_get_display_height(game->display) * 0.32), 4 + al_get_display_width(game->display)/2 + al_get_text_width(game->_priv.font_bsod, header)/2, (int)(al_get_display_height(game->display) * 0.32) + al_get_font_line_height(game->_priv.font_bsod), al_map_rgb(170,170,170));

		al_draw_text(game->_priv.font_bsod, al_map_rgb(0, 0, 170), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32), ALLEGRO_ALIGN_CENTRE, header);

		char *header2 = "A fatal exception 0xD3RP has occured at 0028:M00F11NZ in GST SD(01) +";

		al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+2*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, header2);
		al_draw_textf(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+3*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "%p and system just doesn't know what went wrong.", game);

		al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+5*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, text);

		al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+7*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Press any key to terminate this error.");
		al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+8*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Press any key to destroy all muffins in the world.");
		al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+9*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "* Just kidding, please press any key anyway.");


		if (fatal) {
			al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+11*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "This is fatal error. My bad.");

			al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+13*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, "Press any key to quit _");
		} else {
			al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header2)/2, (int)(al_get_display_height(game->display) * 0.32+11*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_LEFT, "Anything I can do to help?");

			al_draw_text(game->_priv.font_bsod, al_map_rgb(255,255,255), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32+13*al_get_font_line_height(game->_priv.font_bsod)*1.25), ALLEGRO_ALIGN_CENTRE, "Press any key to continue _");
		}

		al_flip_display();

		ALLEGRO_KEYBOARD_STATE kb;
		al_get_keyboard_state(&kb);

		int i;
		for (i=0; i<ALLEGRO_KEY_PAUSE; i++) {
			if (al_key_down(&kb, i)) {
				done = true;
				break;
			}
		}
	}
	al_use_transform(&game->projection);
}

void TestPath(char* filename, char* subpath, char** result) {
	if (*result) return; //already found
	ALLEGRO_PATH *tail = al_create_path(filename);
	ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	ALLEGRO_PATH *data = al_create_path(subpath);
	al_join_paths(path, data);
	al_join_paths(path, tail);
	//printf("Testing for %s\n", al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
	if (al_filename_exists(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP))) {
		*result = strdup(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
	}
	al_destroy_path(tail);
	al_destroy_path(data);
	al_destroy_path(path);
}

char* GetDataFilePath(struct Game *game, char* filename) {

	//TODO: support for current game

	//FIXME: strdups result in memory leaks!

	char *result = 0;

	if (al_filename_exists(filename)) {
		return strdup(filename);
	}

	char origfn[255] = "data/";
	strcat(origfn, filename);

	if (al_filename_exists(origfn)) {
		return strdup(origfn);
	}

	TestPath(filename, "data/", &result);
    TestPath(filename, "../share/mediator/data/", &result);
	TestPath(filename, "../data/", &result);
#ifdef ALLEGRO_MACOSX
	TestPath(filename, "../Resources/data/", &result);
	TestPath(filename, "../Resources/gamestates/", &result);
#endif

	if (!result) {
		FatalError(game, true, "Could not find data file: %s!", filename);
		exit(1);
	}
	return result;
}

void PrintConsole(struct Game *game, char* format, ...) {
	va_list vl;
	va_start(vl, format);
	char text[1024] = {};
	vsnprintf(text, 1024, format, vl);
	va_end(vl);
	if (game->config.debug) { printf("%s\n", text); fflush(stdout); }
	if (!game->_priv.console) return;
	if ((!game->config.debug) && (!game->_priv.showconsole)) return;
	ALLEGRO_BITMAP *con = al_create_bitmap(al_get_bitmap_width(game->_priv.console), al_get_bitmap_height(game->_priv.console));
	al_set_target_bitmap(con);
	al_clear_to_color(al_map_rgba(0,0,0,80));
	al_draw_bitmap_region(game->_priv.console, 0, (int)(al_get_bitmap_height(game->_priv.console)*0.2), al_get_bitmap_width(game->_priv.console), (int)(al_get_bitmap_height(game->_priv.console)*0.8), 0, 0, 0);
	al_draw_text(game->_priv.font_console, al_map_rgb(255,255,255), (int)(game->viewport.width*0.005), (int)(al_get_bitmap_height(game->_priv.console)*0.81), ALLEGRO_ALIGN_LEFT, text);
	al_set_target_bitmap(game->_priv.console);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	al_draw_bitmap(con, 0, 0, 0);
	al_set_target_bitmap(al_get_backbuffer(game->display));
	al_destroy_bitmap(con);
}


void SelectSpritesheet(struct Game *game, struct Character *character, char *name) {
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

void ChangeSpritesheet(struct Game *game, struct Character *character, char* name) {
	if (character->successor) free(character->successor);
	character->successor = strdup(name);
}

void LoadSpritesheets(struct Game *game, struct Character *character) {
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

void UnloadSpritesheets(struct Game *game, struct Character *character) {
	PrintConsole(game, "Unloading spritesheets for character %s...", character->name);
	struct Spritesheet *tmp = character->spritesheets;
	while (tmp) {
		if (tmp->bitmap) al_destroy_bitmap(tmp->bitmap);
		tmp->bitmap = NULL;
		tmp = tmp->next;
	}
}

void RegisterSpritesheet(struct Game *game, struct Character *character, char* name) {
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

struct Character* CreateCharacter(struct Game *game, char* name) {
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
	character->shared = false;
	character->dead = false;
	return character;
}

void DestroyCharacter(struct Game *game, struct Character *character) {
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

void AnimateCharacter(struct Game *game, struct Character *character, float speed_modifier) {
	if (character->dead) return;
	if (speed_modifier) {
		character->pos_tmp++;
		if (character->pos_tmp >= character->spritesheet->delay / speed_modifier) {
			character->pos_tmp = 0;
			character->pos++;
		}
        if (character->pos>=character->spritesheet->cols*character->spritesheet->rows-character->spritesheet->blanks) {
            character->pos=0;
            if (character->spritesheet->kill) {
				character->dead = true;
			} else if (character->successor) {
                SelectSpritesheet(game, character, character->successor);
			}
        }
	}
}

void MoveCharacter(struct Game *game, struct Character *character, float x, float y, float angle) {
	if (character->dead) return;
	character->x += x;
	character->y += y;
	character->angle += angle;
}

void SetCharacterPosition(struct Game *game, struct Character *character, int x, int y, float angle) {
	if (character->dead) return;
	character->x = x;
	character->y = y;
	character->angle = angle;
}

bool GetAbstractIsItBonusLevelTimeNowFactoryProvider(struct Game *game) {
	return game->mediator.strike && (game->mediator.strike % 5 == 0);
}

void DrawCharacter(struct Game *game, struct Character *character, ALLEGRO_COLOR tint, int flags) {
	if (character->dead) return;
    int spritesheetX = al_get_bitmap_width(character->bitmap)*(character->pos%character->spritesheet->cols);
    int spritesheetY = al_get_bitmap_height(character->bitmap)*(character->pos/character->spritesheet->cols);
    al_draw_tinted_scaled_rotated_bitmap_region(character->spritesheet->bitmap, spritesheetX, spritesheetY, al_get_bitmap_width(character->bitmap), al_get_bitmap_height(character->bitmap), tint, al_get_bitmap_width(character->bitmap)/2, al_get_bitmap_height(character->bitmap)/2, character->x + al_get_bitmap_width(character->bitmap)/2, character->y + al_get_bitmap_height(character->bitmap)/2, 1, 1, character->angle, flags);
}

void AdvanceLevel(struct Game *game, bool won) {
    if (won) {
        game->mediator.score++;
				game->mediator.strike++;
    } else {
        game->mediator.lives--;
				game->mediator.strike = 0;
    }
		game->mediator.modificator *= 1.025;
    SelectSpritesheet(game, game->mediator.heart, "heart");
}

void ShowLevelStatistics(struct Game *game) {
    // show as many bitmaps as there are lives
    // show additional one as a animated character

    al_draw_filled_rectangle(0, 0, 320, 240, al_map_rgba(0, 0, 0, 192));

    int x = 75;

    int pos = game->mediator.heart->pos;
    struct Spritesheet *a = game->mediator.heart->spritesheet;

    for (int i = 0; i < game->mediator.lives; i++) {
        SetCharacterPosition(game, game->mediator.heart, x, 50, 0);
        SelectSpritesheet(game, game->mediator.heart, "heart");
        DrawCharacter(game, game->mediator.heart, al_map_rgb(255, 255, 255), 0);
        x += 48;
    }
    game->mediator.heart->pos = pos;
    game->mediator.heart->spritesheet = a;

    if (game->mediator.lives >= 0) {
        SetCharacterPosition(game, game->mediator.heart, x, 50, 0);
        DrawCharacter(game, game->mediator.heart, al_map_rgb(255, 255, 255), 0);
    }

    //DrawTextWithShadow(game->_priv.font, al_map_rgb(255,255,255), 50, 50, 0, text);
}

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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "internal.h"
#include "config.h"
#include "utils.h"

SYMBOL_EXPORT void DrawVerticalGradientRect(float x, float y, float w, float h, ALLEGRO_COLOR top, ALLEGRO_COLOR bottom) {
	ALLEGRO_VERTEX v[] = {
	  {.x = x, .y = y, .z = 0, .color = top},
	  {.x = x + w, .y = y, .z = 0, .color = top},
	  {.x = x, .y = y + h, .z = 0, .color = bottom},
	  {.x = x + w, .y = y + h, .z = 0, .color = bottom}};
	al_draw_prim(v, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP);
}

SYMBOL_EXPORT void DrawHorizontalGradientRect(float x, float y, float w, float h, ALLEGRO_COLOR left, ALLEGRO_COLOR right) {
	ALLEGRO_VERTEX v[] = {
	  {.x = x, .y = y, .z = 0, .color = left},
	  {.x = x + w, .y = y, .z = 0, .color = right},
	  {.x = x, .y = y + h, .z = 0, .color = left},
	  {.x = x + w, .y = y + h, .z = 0, .color = right}};
	al_draw_prim(v, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP);
}

SYMBOL_EXPORT void DrawTextWithShadow(ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int flags, char const *text) {
	al_draw_text(font, al_map_rgba(0,0,0,128), x+1, y+1, flags, text);
	al_draw_text(font, color, x, y, flags, text);
}

SYMBOL_EXPORT int DrawWrappedText(ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int width, int flags, char const* text) {

	char stext[1024]; // Copy of the passed text.
	char *pch; // A pointer to each word.
	char word[255]; // A string containing the word (for convienence)
	char *breakchar = "\n";
	char lines[40][1024]; // A lovely array of strings to hold all the lines (40 max atm)
	char temp[1024]; // Holds the string data of the current line only.
	int line = 0; // Counts which line we are currently using.
	int height = al_get_font_line_height(font) + 1;

	// Setup our strings
	strcpy(stext, text);
	strcpy(temp, "");
	for (int i = 0; i < 40; i+=1) {
		strcpy(lines[i], "");
	}
	//-------------------- Code Begins

	char *context;

	pch = strtok_r(stext," ", &context);                               // Get the first word.
	do {
		strcpy(word, "");                                  // Truncate the string, to ensure there's no crazy stuff in there from memory.
		sprintf(word,"%s ", pch);
		strcat(temp, word);														// Append the word to the end of TempLine
		// This code checks for the new line character.
		if (strcmp(word, breakchar) == 0) {
			line += 1;                                 // Move down a Line
			strcpy(temp, "");                            // Clear the tempstring
		} else {
			if (al_get_text_width(font, temp) >= (width)) {  // Check if text is larger than the area.
				strcpy(temp, word);                      // clear the templine and add the word to it.
				line +=1;                             // Move to the next line.
			}
			if (line < 40) {
				strcat(lines[line], word);                // Append the word to whatever line we are currently on.
			}
		}
		pch = strtok_r (NULL, " ", &context);                           // Get the next word.
	} while (pch != NULL);
	// ---------------------------------- Time to draw.

	for (int i = 0; i<=line; i+=1) {                    // Move through each line and draw according to the passed flags.
		switch (flags) {
			case ALLEGRO_ALIGN_CENTER:
				al_draw_text(font, color, x + (width/2), y + (i * height), ALLEGRO_ALIGN_CENTER, lines[i]);
				break;
			case ALLEGRO_ALIGN_RIGHT:
				al_draw_text(font, color, x + width, y + (i * height), ALLEGRO_ALIGN_RIGHT, lines[i]);
				break;
			case ALLEGRO_ALIGN_LEFT:
			default:
				al_draw_text(font, color, x, y + (i * height), ALLEGRO_ALIGN_LEFT, lines[i]);
				break;
		}
	}
	return ((line+1) * height);  // Return the actual height of the text in pixels.
}

SYMBOL_EXPORT int DrawWrappedTextWithShadow(ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int width, int flags, char const *text) {
	DrawWrappedText(font, al_map_rgba(0,0,0,128), x+1, y+1, width, flags, text);
	return DrawWrappedText(font, color, x, y, width, flags, text);
}

/* linear filtering code written by SiegeLord */
SYMBOL_EXPORT ALLEGRO_COLOR InterpolateColor(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float frac) {
	return al_map_rgba_f(c1.r + frac * (c2.r - c1.r),
	                     c1.g + frac * (c2.g - c1.g),
	                     c1.b + frac * (c2.b - c1.b),
	                     c1.a + frac * (c2.a - c1.a));
}

/*! \brief Scales bitmap using software linear filtering method to current target. */
SYMBOL_EXPORT void ScaleBitmap(ALLEGRO_BITMAP* source, int width, int height) {
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

			ALLEGRO_COLOR ab = InterpolateColor(a, b, pixx - pixx_f);
			ALLEGRO_COLOR cd = InterpolateColor(c, d, pixx - pixx_f);
			ALLEGRO_COLOR result = InterpolateColor(ab, cd, pixy - pixy_f);

			al_put_pixel(x, y, result);
		}
	}
	al_unlock_bitmap(al_get_target_bitmap());
	al_unlock_bitmap(source);
}

SYMBOL_EXPORT ALLEGRO_BITMAP* LoadScaledBitmap(struct Game *game, char* filename, int width, int height) {
	bool memoryscale = !atoi(GetConfigOptionDefault(game, "SuperDerpy", "GPU_scaling", "1"));
	ALLEGRO_BITMAP *source, *target = al_create_bitmap(width, height);
	al_set_target_bitmap(target);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	char* origfn = GetDataFilePath(game, filename);

	int flags = al_get_new_bitmap_flags();
	if (memoryscale) {
		al_add_new_bitmap_flag(ALLEGRO_MEMORY_BITMAP);
	}

	source = al_load_bitmap( origfn );
	if (memoryscale) {
		al_set_new_bitmap_flags(flags);
		ScaleBitmap(source, width, height);
	}
	else {
		al_draw_scaled_bitmap(source, 0, 0, al_get_bitmap_width(source), al_get_bitmap_height(source), 0, 0, width, height, 0);
	}

	al_destroy_bitmap(source);

	free(origfn);
	return target;

}

SYMBOL_EXPORT void FatalError(struct Game *game, bool fatal, char* format, ...) {
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

		const char *header = game->name;

		al_draw_filled_rectangle(al_get_display_width(game->display)/2 - al_get_text_width(game->_priv.font_bsod, header)/2 - 4, (int)(al_get_display_height(game->display) * 0.32), 4 + al_get_display_width(game->display)/2 + al_get_text_width(game->_priv.font_bsod, header)/2, (int)(al_get_display_height(game->display) * 0.32) + al_get_font_line_height(game->_priv.font_bsod), al_map_rgb(170,170,170));

		al_draw_text(game->_priv.font_bsod, al_map_rgb(0, 0, 170), al_get_display_width(game->display)/2, (int)(al_get_display_height(game->display) * 0.32), ALLEGRO_ALIGN_CENTRE, header);

		const char *header2 = "A fatal exception 0xD3RP has occured at 0028:M00F11NZ in GST SD(01) +";

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

SYMBOL_INTERNAL void TestPath(char* filename, char* subpath, char** result) {
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

SYMBOL_EXPORT char* GetGameName(struct Game *game, char* format) {
	char *result = malloc(sizeof(char)*255);
	snprintf(result, 255, format, game->name);
	return AddGarbage(game, result);
}

SYMBOL_EXPORT char* GetDataFilePath(struct Game *game, char* filename) {

	char *result = 0;

	if (al_filename_exists(filename)) {
		return AddGarbage(game, strdup(filename));
	}

	{
		char origfn[255] = "data/";
		strcat(origfn, filename);

		if (al_filename_exists(origfn)) {
			return AddGarbage(game, strdup(origfn));
		}
	}

	TestPath(filename, "data/", &result);
	TestPath(filename, GetGameName(game, "../share/%s/data/"), &result);

	TestPath(filename, "../data/", &result);
#ifdef ALLEGRO_MACOSX
	TestPath(filename, "../Resources/data/", &result);
	TestPath(filename, "../Resources/gamestates/", &result);
#endif

	if (!result) {
		FatalError(game, true, "Could not find data file: %s!", filename);
		exit(1);
	}
	return AddGarbage(game, result);
}

SYMBOL_EXPORT char* GetDataFilePathNull(struct Game *game, char* filename) {

	char *result = 0;

	if (al_filename_exists(filename)) {
		return AddGarbage(game, strdup(filename));
	}

	{
		char origfn[255] = "data/";
		strcat(origfn, filename);

		if (al_filename_exists(origfn)) {
			return AddGarbage(game, strdup(origfn));
		}
	}

	TestPath(filename, "data/", &result);
	TestPath(filename, GetGameName(game, "../share/%s/data/"), &result);

	TestPath(filename, "../data/", &result);
#ifdef ALLEGRO_MACOSX
	TestPath(filename, "../Resources/data/", &result);
	TestPath(filename, "../Resources/gamestates/", &result);
#endif

	if (!result) {
		return filename;
	}
	return AddGarbage(game, result);
}

ALLEGRO_DEBUG_CHANNEL("libsuperderpy")

SYMBOL_EXPORT void PrintConsole(struct Game *game, char* format, ...) {
	va_list vl;
	va_start(vl, format);
	char text[1024] = {};
	vsnprintf(text, 1024, format, vl);
	va_end(vl);
	ALLEGRO_DEBUG("%s", text);
	if (game->config.debug) { printf("%s\n", text); fflush(stdout); }
	if (!game->_priv.draw) return;
	if (!game->_priv.console) return;
	if ((!game->config.debug) && (!game->_priv.showconsole)) return;
	al_set_target_bitmap(game->_priv.console_tmp);
	al_clear_to_color(al_map_rgba(0,0,0,80));
	al_draw_bitmap_region(game->_priv.console, 0, (int)(al_get_bitmap_height(game->_priv.console)*0.2), al_get_bitmap_width(game->_priv.console), (int)(al_get_bitmap_height(game->_priv.console)*0.8), 0, 0, 0);
	al_draw_text(game->_priv.font_console, al_map_rgb(255,255,255), (int)(game->viewport.width*0.005), (int)(al_get_bitmap_height(game->_priv.console)*0.81), ALLEGRO_ALIGN_LEFT, text);
	al_set_target_bitmap(game->_priv.console);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	al_draw_bitmap(game->_priv.console_tmp, 0, 0, 0);
	al_set_target_bitmap(al_get_backbuffer(game->display));
}

SYMBOL_EXPORT void SetupViewport(struct Game *game, struct Viewport config) {

	game->viewport = config;

	if ((game->viewport.width == 0) || (game->viewport.height == 0)) {
		game->viewport.height = al_get_display_height(game->display);
		game->viewport.width = game->viewport.aspect * game->viewport.height;
		if (game->viewport.width > al_get_display_width(game->display)) {
			game->viewport.width = al_get_display_width(game->display);
			game->viewport.height = game->viewport.width / (float)game->viewport.aspect;
		}
	}
	game->viewport.aspect = game->viewport.width / (float)game->viewport.height;

	al_set_target_backbuffer(game->display);
	al_identity_transform(&game->projection);
	al_use_transform(&game->projection);
	al_set_clipping_rectangle(0, 0, al_get_display_width(game->display), al_get_display_height(game->display));

	float resolution = al_get_display_height(game->display) / (float)game->viewport.height;
	if (al_get_display_width(game->display) / (float)game->viewport.width < resolution) {
		resolution = al_get_display_width(game->display) / (float)game->viewport.width;
	}
	if (game->viewport.integer_scaling) {
		resolution = floor(resolution);
	}
	if ((!atoi(GetConfigOptionDefault(game, "SuperDerpy", "downscale", "1"))) && (resolution < 1)) {
		resolution = 1;
	}
	if (!atoi(GetConfigOptionDefault(game, "SuperDerpy", "scaling", "1"))) {
		resolution = 1;
	}
	if (resolution == 0) {
		resolution = 1;
	}

	int clipWidth = game->viewport.width * resolution;
	int clipHeight = game->viewport.height * resolution;
	if (atoi(GetConfigOptionDefault(game, "SuperDerpy", "letterbox", "1"))) {
		int clipX = (al_get_display_width(game->display) - clipWidth) / 2;
		int clipY = (al_get_display_height(game->display) - clipHeight) / 2;
		al_build_transform(&game->projection, clipX, clipY, resolution, resolution, 0.0f);
		al_set_clipping_rectangle(clipX, clipY, clipWidth, clipHeight);
	} else if (atoi(GetConfigOptionDefault(game, "SuperDerpy", "scaling", "1"))) {
		al_build_transform(&game->projection, 0, 0, al_get_display_width(game->display) / (float)game->viewport.width, al_get_display_height(game->display) / (float)game->viewport.height, 0.0f);
	}
	al_use_transform(&game->projection);
	if (game->_priv.console) Console_Unload(game);
	Console_Load(game);
}

SYMBOL_EXPORT void WindowCoordsToViewport(struct Game *game, int *x, int *y) {
	int clipX, clipY, clipWidth, clipHeight;
	al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
	*x -= clipX;
	*y -= clipY;
	*x /= clipWidth / (float)game->viewport.width;
	*y /= clipHeight / (float)game->viewport.height;
}

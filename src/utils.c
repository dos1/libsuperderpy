/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This file is part of libsuperderpy.
 *
 * libsuperderpy is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * libsuperderpy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libsuperderpy. If not, see <http://www.gnu.org/licenses/>.
 *
 * Also, ponies.
 */

#include "internal.h"
#include <ctype.h>
#ifdef ALLEGRO_ANDROID
#include <android/log.h>
#endif

// TODO: split to separate files

ALLEGRO_DEBUG_CHANNEL("libsuperderpy")

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

SYMBOL_EXPORT void DrawTextWithShadow(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags, char const* text) {
	// TODO: consider using a set of shaders
	al_draw_text(font, al_map_rgba(0, 0, 0, 128), x + 1, y + 1, flags, text);
	al_draw_text(font, color, x, y, flags, text);
}

SYMBOL_EXPORT int DrawWrappedText(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int width, int flags, char const* text) {
	// TODO: use al_do_multiline_text; and switch to al_draw_multiline_text once it returns number of lines

	char stext[1024]; // Copy of the passed text.
	char* pch = NULL; // A pointer to each word.
	char word[255]; // A string containing the word (for convienence)
	char* breakchar = "\n";
	char lines[40][1024]; // A lovely array of strings to hold all the lines (40 max atm)
	char temp[1024]; // Holds the string data of the current line only.
	int line = 0; // Counts which line we are currently using.
	int height = al_get_font_line_height(font) + 1;

	// Setup our strings
	strncpy(stext, text, 1023);
	strncpy(temp, "", 1023);
	for (int i = 0; i < 40; i += 1) {
		strncpy(lines[i], "", 1024);
	}
	//-------------------- Code Begins

	char* context = NULL;

	pch = strtok_r(stext, " ", &context); // Get the first word.
	do {
		snprintf(word, 255, "%s ", pch);
		strncat(temp, word, 255); // Append the word to the end of TempLine
		// This code checks for the new line character.
		if (strncmp(word, breakchar, 1) == 0) {
			line += 1; // Move down a Line
			strncpy(temp, "", 1023); // Clear the tempstring
		} else {
			if (al_get_text_width(font, temp) >= (width)) { // Check if text is larger than the area.
				strncpy(temp, word, 255); // clear the templine and add the word to it.
				line += 1; // Move to the next line.
			}
			if (line < 40) {
				strncat(lines[line], word, 255); // Append the word to whatever line we are currently on.
			}
		}
		pch = strtok_r(NULL, " ", &context); // Get the next word.
	} while (pch != NULL);
	// ---------------------------------- Time to draw.

	for (int i = 0; i <= line; i += 1) { // Move through each line and draw according to the passed flags.
		switch (flags) {
			case ALLEGRO_ALIGN_CENTER:
				al_draw_text(font, color, x + (width / 2.0), y + (i * height), ALLEGRO_ALIGN_CENTER, lines[i]);
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
	return ((line + 1) * height); // Return the actual height of the text in pixels.
}

SYMBOL_EXPORT int DrawWrappedTextWithShadow(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int width, int flags, char const* text) {
	DrawWrappedText(font, al_map_rgba(0, 0, 0, 128), x + 1, y + 1, width, flags, text);
	return DrawWrappedText(font, color, x, y, width, flags, text);
}

SYMBOL_EXPORT void DrawCentered(ALLEGRO_BITMAP* bitmap, float x, float y, int flags) {
	al_draw_bitmap(bitmap, x - al_get_bitmap_width(bitmap) / 2.0, y - al_get_bitmap_height(bitmap) / 2.0, flags);
}

SYMBOL_EXPORT void DrawCenteredScaled(ALLEGRO_BITMAP* bitmap, float x, float y, double sx, double sy, int flags) {
	DrawCenteredTintedScaled(bitmap, al_map_rgb(255, 255, 255), x, y, sx, sy, flags);
}

SYMBOL_EXPORT void DrawCenteredTintedScaled(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float x, float y, double sx, double sy, int flags) {
	al_draw_tinted_scaled_rotated_bitmap(bitmap, tint, al_get_bitmap_width(bitmap) / 2.0, al_get_bitmap_height(bitmap) / 2.0,
		x, y, sx, sy, 0, flags);
}

SYMBOL_EXPORT void ClearToColor(struct Game* game, ALLEGRO_COLOR color) {
	ALLEGRO_BITMAP* target = al_get_target_bitmap();
	if (game->_priv.current_gamestate && GetFramebuffer(game) == target && al_get_parent_bitmap(target) == al_get_backbuffer(game->display)) {
		al_set_target_backbuffer(game->display);
	}
	int x = 0, y = 0, w = 0, h = 0;
	al_get_clipping_rectangle(&x, &y, &w, &h);
	al_reset_clipping_rectangle();
	al_clear_to_color(color);
	al_set_clipping_rectangle(x, y, w, h);
	if (al_get_target_bitmap() != target) {
		al_set_target_bitmap(target);
	}
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
	if ((al_get_bitmap_width(source) == width) && (al_get_bitmap_height(source) == height)) {
		al_draw_bitmap(source, 0, 0, 0);
		return;
	}
	int x = 0, y = 0;
	al_lock_bitmap(al_get_target_bitmap(), ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
	al_lock_bitmap(source, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

	for (y = 0; y < height; y++) {
		float pixy = ((float)y / height) * ((float)al_get_bitmap_height(source) - 1);
		int pixy_f = (int)floorf(pixy);
		for (x = 0; x < width; x++) {
			float pixx = ((float)x / width) * ((float)al_get_bitmap_width(source) - 1);
			int pixx_f = (int)floorf(pixx);

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

SYMBOL_EXPORT ALLEGRO_BITMAP* LoadScaledBitmap(struct Game* game, char* filename, int width, int height) {
	bool memoryscale = !strtol(GetConfigOptionDefault(game, "SuperDerpy", "GPU_scaling", "1"), NULL, 10);
	ALLEGRO_BITMAP *source = NULL, *target = al_create_bitmap(width, height);
	al_set_target_bitmap(target);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));

	int flags = al_get_new_bitmap_flags();
	if (memoryscale) {
		al_add_new_bitmap_flag(ALLEGRO_MEMORY_BITMAP);
	}

	source = al_load_bitmap(GetDataFilePath(game, filename));
	if (memoryscale) {
		al_set_new_bitmap_flags(flags);
		ScaleBitmap(source, width, height);
	} else {
		al_draw_scaled_bitmap(source, 0, 0, al_get_bitmap_width(source), al_get_bitmap_height(source), 0, 0, width, height, 0);
	}

	al_destroy_bitmap(source);

	return target;
}

SYMBOL_EXPORT ALLEGRO_BITMAP* LoadMemoryBitmap(const char* filename) {
	int flags = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags((flags | ALLEGRO_MEMORY_BITMAP) & ~(ALLEGRO_VIDEO_BITMAP | ALLEGRO_CONVERT_BITMAP));
	ALLEGRO_BITMAP* bitmap = al_load_bitmap(filename);
	al_set_new_bitmap_flags(flags);
	return bitmap;
}

SYMBOL_EXPORT ALLEGRO_BITMAP* CreateMemoryBitmap(int width, int height) {
	int flags = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags((flags | ALLEGRO_MEMORY_BITMAP) & ~(ALLEGRO_VIDEO_BITMAP | ALLEGRO_CONVERT_BITMAP));
	ALLEGRO_BITMAP* bitmap = al_create_bitmap(width, height);
	al_set_new_bitmap_flags(flags);
	return bitmap;
}

SYMBOL_EXPORT void FatalErrorWithContext(struct Game* game, int line, const char* file, const char* func, bool exit, char* format, ...) {
	char text[1024] = {0};
	PrintConsole(game, "Fatal Error, displaying Blue Screen of Derp...");
	va_list vl;
	va_start(vl, format);
	vsnprintf(text, 1024, format, vl);
	va_end(vl);
	fprintf(stderr, "%s:%d [%s]\n%s\n", file, line, func, text);

#ifndef LIBSUPERDERPY_SINGLE_THREAD
	if (game->_priv.loading.in_progress) {
		al_lock_mutex(game->_priv.bsod_mutex);
		game->_priv.in_bsod = true;
		game->_priv.bsod_sync = true;
		while (game->_priv.bsod_sync) {
			al_wait_cond(game->_priv.bsod_cond, game->_priv.bsod_mutex);
		}
		al_unlock_mutex(game->_priv.bsod_mutex);
	}
#endif

	al_set_target_backbuffer(game->display);

	ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	al_use_transform(&trans);

	if (!game->_priv.font_bsod) {
		game->_priv.font_bsod = al_create_builtin_font();
	}

	al_clear_to_color(al_map_rgb(0, 0, 170));
	al_flip_display();
	al_rest(0.6);

	const int offsetx = al_get_display_width(game->display) / 2;
	const int offsety = (int)(al_get_display_height(game->display) * 0.30);
	const int fonth = al_get_font_line_height(game->_priv.font_bsod);

	bool done = false;
	while (!done) {
		al_set_target_backbuffer(game->display);
		al_clear_to_color(al_map_rgb(0, 0, 170));

		const char* header = game->_priv.name;
		const int headw = al_get_text_width(game->_priv.font_bsod, header);

		al_draw_filled_rectangle(offsetx - headw / 2.0 - 4, offsety, 4 + offsetx + headw / 2.0, offsety + fonth, al_map_rgb(170, 170, 170));

		al_draw_text(game->_priv.font_bsod, al_map_rgb(0, 0, 170), offsetx, offsety, ALLEGRO_ALIGN_CENTRE, header);

		const char* header2 = "A fatal exception 0xD3RP has occured at 0028:M00F11NZ in GST SD(01) +";
		const int head2w = al_get_text_width(game->_priv.font_bsod, header2);

		al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx, (int)(offsety + 2 * fonth * 1.25), ALLEGRO_ALIGN_CENTRE, header2);
		al_draw_textf(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx - head2w / 2.0, (int)(offsety + 3 * fonth * 1.25), ALLEGRO_ALIGN_LEFT, "%p and system just doesn't know what went wrong.", game);

		const int error_len = strlen(text);
		const int error_w = al_get_text_width(game->_priv.font_bsod, text);
		const int lines = ceil(error_w / (al_get_display_width(game->display) * 0.8));
		const int letters_per_line = (error_len / lines) + 1;

		int row = 5;
		for (int l = 0; l < lines; ++l) {
			int start = l * letters_per_line;
			unsigned int end = (l + 1) * letters_per_line;
			if (end >= sizeof(text)) {
				end = sizeof(text) - 1;
			}

			const char save_char = text[end];
			text[end] = '\0';

			al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx - (error_w / (float)lines) / 2.0, (int)(offsety + row++ * fonth * 1.25), ALLEGRO_ALIGN_LEFT, text + start);
			text[end] = save_char;
		}
		++row;

		al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx - head2w / 2.0, (int)(offsety + row++ * fonth * 1.25), ALLEGRO_ALIGN_LEFT, "* Press any key to terminate this error.");
		al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx - head2w / 2.0, (int)(offsety + row++ * fonth * 1.25), ALLEGRO_ALIGN_LEFT, "* Press any key to destroy all muffins in the world.");
		al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx - head2w / 2.0, (int)(offsety + row++ * fonth * 1.25), ALLEGRO_ALIGN_LEFT, "* Just kidding, please press any key anyway.");

		++row;
		if (exit) {
			al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx - head2w / 2.0, (int)(offsety + row * fonth * 1.25), ALLEGRO_ALIGN_LEFT, "This is a fatal error. My bad.");

			al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx, (int)(offsety + (row + 2) * fonth * 1.25), ALLEGRO_ALIGN_CENTRE, "Press any key to quit _");
		} else {
			al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx - head2w / 2.0, (int)(offsety + row * fonth * 1.25), ALLEGRO_ALIGN_LEFT, "Anything I can do to help?");

			al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx, (int)(offsety + (row + 2) * fonth * 1.25), ALLEGRO_ALIGN_CENTRE, "Press any key to continue _");
		}

		al_flip_display();

		ALLEGRO_KEYBOARD_STATE kb;
		al_get_keyboard_state(&kb);

// FIXME: implement proper event loop there
#ifndef __EMSCRIPTEN__
		for (int i = 0; i < ALLEGRO_KEY_PAUSE; i++) {
			if (al_key_down(&kb, i)) {
				done = true;
				break;
			}
		}
#else
		done = true;
#endif
	}
	al_use_transform(&game->_priv.projection);
#ifndef LIBSUPERDERPY_SINGLE_THREAD
	if (game->_priv.loading.in_progress) {
		PrintConsole(game, "Resuming the main thread...");
		game->_priv.in_bsod = false;
		al_signal_cond(game->_priv.bsod_cond);
	}
#endif
}

static void TestPath(const char* filename, const char* subpath, char** result) {
	if (*result) { return; } //already found
	ALLEGRO_PATH* tail = al_create_path(filename);
	ALLEGRO_PATH* path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	ALLEGRO_PATH* data = al_create_path(subpath);
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

static char* TestDataFilePath(struct Game* game, const char* filename) {
	char* result = NULL;

	TestPath(filename, "data/", &result);
	TestPath(filename, GetGameName(game, "../share/%s/data/"), &result);
	TestPath(filename, GetGameName(game, "../share/games/%s/data/"), &result);

#ifdef ALLEGRO_MACOSX
	TestPath(filename, "../Resources/data/", &result);
#endif

	// build directories
	TestPath(filename, "../../data/", &result);
	TestPath(filename, "../../../data/", &result);

	if (result) {
		return result;
	}

	// try current working directory if everything else fails
	char origfn[255] = "data/";
	strncat(origfn, filename, 249);

	if (al_filename_exists(origfn)) {
		return strdup(origfn);
	}

	return NULL;
}

SYMBOL_EXPORT const char* FindDataFilePath(struct Game* game, const char* filename) {
	char* result = 0;

#ifdef ALLEGRO_ANDROID
	char origfn[255] = "android/";
	strncat(origfn, filename, 246);

	result = TestDataFilePath(game, origfn);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

#ifdef MAEMO5
	char origfn[255] = "maemo5/";
	strncat(origfn, filename, 247);

	result = TestDataFilePath(game, origfn);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

#ifdef POCKETCHIP
	char origfn[255] = "pocketchip/";
	strncat(origfn, filename, 243);

	result = TestDataFilePath(game, origfn);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

#ifdef RASPBERRYPI
	char origfn[255] = "raspberrypi/";
	strncat(origfn, filename, 242);

	result = TestDataFilePath(game, origfn);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

#ifdef STEAMLINK
	char origfn[255] = "steamlink/";
	strncat(origfn, filename, 244);

	result = TestDataFilePath(game, origfn);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

#ifdef __EMSCRIPTEN__
	char origfn[255] = "emscripten/";
	strncat(origfn, filename, 243);

	result = TestDataFilePath(game, origfn);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

#ifdef __vita__
	char origfn[255] = "vita/";
	strncat(origfn, filename, 249);

	result = TestDataFilePath(game, origfn);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

#ifdef LIBSUPERDERPY_FLACTOLOSSY_EXT
	{
		char* file = AddGarbage(game, strdup(filename));
		char* sub = strstr(file, ".flac");
		if (sub) {
			sub[0] = '.';
			sub[1] = LIBSUPERDERPY_FLACTOLOSSY_EXT[0];
			sub[2] = LIBSUPERDERPY_FLACTOLOSSY_EXT[1];
			sub[3] = LIBSUPERDERPY_FLACTOLOSSY_EXT[2];
			sub[4] = LIBSUPERDERPY_FLACTOLOSSY_EXT[3];
			sub[5] = 0;
		}
		result = TestDataFilePath(game, file);
		if (result) {
			return AddGarbage(game, result);
		}
	}
#endif

#ifdef LIBSUPERDERPY_IMGTOWEBP
	{
		char* file = AddGarbage(game, strdup(filename));
		char* sub = strstr(file, ".png");
		if (sub) {
			sub[0] = '.';
			sub[1] = 'w';
			sub[2] = 'b';
			sub[3] = 'p';
			sub[4] = 0;
		}
		result = TestDataFilePath(game, file);
		if (result) {
			return AddGarbage(game, result);
		}

		sub = strstr(file, ".jpg");
		if (sub) {
			sub[0] = '.';
			sub[1] = 'w';
			sub[2] = 'b';
			sub[3] = 'p';
			sub[4] = 0;
		}
		result = TestDataFilePath(game, file);
		if (result) {
			return AddGarbage(game, result);
		}

		sub = strstr(file, ".JPG");
		if (sub) {
			sub[0] = '.';
			sub[1] = 'w';
			sub[2] = 'b';
			sub[3] = 'p';
			sub[4] = 0;
		}
		result = TestDataFilePath(game, file);
		if (result) {
			return AddGarbage(game, result);
		}

		sub = strstr(file, ".webp");
		if (sub) {
			sub[0] = '.';
			sub[1] = 'w';
			sub[2] = 'b';
			sub[3] = 'p';
			sub[4] = 0;
		}
		result = TestDataFilePath(game, file);
		if (result) {
			return AddGarbage(game, result);
		}
	}
#endif

	result = TestDataFilePath(game, filename);
	if (result) {
		return AddGarbage(game, result);
	}

	return NULL;
}

SYMBOL_EXPORT const char* GetDataFilePath(struct Game* game, const char* filename) {
	const char* result = FindDataFilePath(game, filename);

	if (result) {
		return result;
	}

	FatalError(game, true, "Could not find data file: %s!", filename);
#ifdef __EMSCRIPTEN__
	emscripten_exit_with_live_runtime();
#endif
	exit(1);
}

SYMBOL_EXPORT void PrintConsoleWithContext(struct Game* game, int line, const char* file, const char* func, char* format, ...) {
	al_lock_mutex(game->_priv.mutex);
	va_list vl;
	va_start(vl, format);
	char* text = game->_priv.console[game->_priv.console_pos];
	vsnprintf(text, (sizeof(game->_priv.console[0]) / sizeof(game->_priv.console[0][0])), format, vl);
	va_end(vl);

	SUPPRESS_WARNING("-Wused-but-marked-unused")
	ALLEGRO_DEBUG("%s\n", text);
	SUPPRESS_END

#if !defined(__EMSCRIPTEN__) && !defined(ALLEGRO_ANDROID)
	if (game->config.debug.enabled)
#endif
	{
#ifdef ALLEGRO_ANDROID
		if (game->config.debug.verbose) {
			__android_log_print(ANDROID_LOG_DEBUG, al_get_app_name(), "%f %s:%d [%s] %s", al_get_time(), file, line, func, text);
		} else {
			__android_log_print(ANDROID_LOG_DEBUG, al_get_app_name(), "[%s] %s", func, text);
		}
#elif defined(__EMSCRIPTEN__)
		if (game->config.debug.verbose) {
			emscripten_log(EM_LOG_CONSOLE, "%f %s:%d [%s] %s", al_get_time(), file, line, func, text);
		} else {
			emscripten_log(EM_LOG_CONSOLE, "[%s] %s", func, text);
		}
#else
		if (game->config.debug.verbose) {
			printf("%f %s:%d ", al_get_time(), file, line);
		}
		printf("[%s] %s\n", func, text);
		fflush(stdout);
#endif
	}

	game->_priv.console_pos++;
	if (game->_priv.console_pos >= (sizeof(game->_priv.console) / sizeof(game->_priv.console[0]))) {
		game->_priv.console_pos = 0;
	}
	al_unlock_mutex(game->_priv.mutex);
}

SYMBOL_EXPORT void WindowCoordsToViewport(struct Game* game, int* x, int* y) {
	int clipX = 0, clipY = 0, clipWidth = 0, clipHeight = 0;
	al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
	*x -= clipX;
	*y -= clipY;
	*x /= (int)(clipWidth / (float)game->viewport.width);
	*y /= (int)(clipHeight / (float)game->viewport.height);
}

SYMBOL_EXPORT ALLEGRO_BITMAP* GetFramebuffer(struct Game* game) {
	return game->_priv.current_gamestate->fb;
}

SYMBOL_EXPORT void SetFramebufferAsTarget(struct Game* game) {
	ALLEGRO_BITMAP* framebuffer = GetFramebuffer(game);
	if (al_get_target_bitmap() != framebuffer) {
		al_set_target_bitmap(framebuffer);
	}
	if (framebuffer != al_get_backbuffer(game->display)) {
		double x = al_get_bitmap_width(framebuffer) / (double)game->viewport.width;
		double y = al_get_bitmap_height(framebuffer) / (double)game->viewport.height;
		ALLEGRO_TRANSFORM t;
		al_identity_transform(&t);
		al_scale_transform(&t, x, y);
		al_use_transform(&t);
	}
}

SYMBOL_EXPORT void SetClippingRectangle(int x, int y, int width, int height) {
	ALLEGRO_TRANSFORM transform = *al_get_current_transform();
	float nx = x, ny = y, nx2 = x + width, ny2 = y + height;
	al_transform_coordinates(&transform, &nx, &ny);
	al_transform_coordinates(&transform, &nx2, &ny2);
	al_set_clipping_rectangle((int)nx, (int)ny, (int)(nx2 - nx), (int)(ny2 - ny));
}

SYMBOL_EXPORT void ResetClippingRectangle(void) {
	al_reset_clipping_rectangle();
}

SYMBOL_EXPORT void PushTransform(struct Game* game, ALLEGRO_TRANSFORM* t) {
	ALLEGRO_TRANSFORM transform = *t;

	if (game->_priv.transforms_no == game->_priv.transforms_alloc) {
		game->_priv.transforms = realloc(game->_priv.transforms, sizeof(ALLEGRO_TRANSFORM) * ++game->_priv.transforms_alloc);
	}

	game->_priv.transforms[game->_priv.transforms_no++] = *al_get_current_transform();

	al_compose_transform(&transform, al_get_current_transform());
	al_use_transform(&transform);
}

SYMBOL_EXPORT void PopTransform(struct Game* game) {
	al_use_transform(&game->_priv.transforms[--game->_priv.transforms_no]);
}

SYMBOL_EXPORT ALLEGRO_BITMAP* CreateNotPreservedBitmap(int width, int height) {
	int flags = al_get_new_bitmap_flags();
	//al_set_new_bitmap_depth(24);
	al_add_new_bitmap_flag(ALLEGRO_NO_PRESERVE_TEXTURE);
	ALLEGRO_BITMAP* bitmap = al_create_bitmap(width, height);
	al_set_new_bitmap_flags(flags);
	//al_set_new_bitmap_depth(0);
	return bitmap;
}

SYMBOL_EXPORT void EnableCompositor(struct Game* game, void compositor(struct Game* game)) {
	PrintConsole(game, "Compositor enabled.");
	game->_priv.params.handlers.compositor = compositor ? compositor : SimpleCompositor;
	ResizeGamestates(game);
}

SYMBOL_EXPORT void DisableCompositor(struct Game* game) {
	PrintConsole(game, "Compositor disabled.");
	game->_priv.params.handlers.compositor = NULL;
	ResizeGamestates(game);
}

SYMBOL_EXPORT char* StrToLower(struct Game* game, const char* text) {
	// FIXME: UTF-8
	char *res = strdup(text), *iter = res;
	while (*iter) {
		*iter = tolower((unsigned char)*iter);
		iter++;
	}
	return AddGarbage(game, res);
}

SYMBOL_EXPORT char* StrToUpper(struct Game* game, const char* text) {
	// FIXME: UTF-8
	char *res = strdup(text), *iter = res;
	while (*iter) {
		*iter = toupper((unsigned char)*iter);
		iter++;
	}
	return AddGarbage(game, res);
}

SYMBOL_EXPORT char* PunchNumber(struct Game* game, const char* text, char ch, int number) {
	char* txt = strdup(text);
	char* tmp = txt;
	while (*tmp) {
		tmp++;
	}
	int num = 1;
	while (tmp != txt) {
		tmp--;
		if (*tmp == ch) {
			*tmp = '0' + (int)floorf(number / (float)num) % 10;
			num *= 10;
		}
	}
	return AddGarbage(game, txt);
}

SYMBOL_EXPORT void QuitGame(struct Game* game, bool allow_pausing) {
#ifdef ALLEGRO_ANDROID
	if (allow_pausing) {
		JNIEnv* env = al_android_get_jni_env();
		jclass class_id = (*env)->GetObjectClass(env, al_android_get_activity());
		jmethodID method_id = (*env)->GetMethodID(env, class_id, "moveTaskToBack",
			"(Z)Z");
		jvalue jdata;
		jdata.z = JNI_TRUE;
		(*env)->CallBooleanMethodA(env, al_android_get_activity(), method_id, &jdata);
		return;
	}
#endif
	UnloadAllGamestates(game);
}

SYMBOL_EXPORT bool ToggleFullscreen(struct Game* game) {
	if (IS_EMSCRIPTEN && game->_priv.params.fixed_size) {
		al_set_display_flag(game->display, ALLEGRO_FULLSCREEN_WINDOW, true);
		SetupViewport(game);
		PrintConsole(game, "Fullscreen toggled");
		return true;
	}
	game->config.fullscreen = !game->config.fullscreen;
	if (game->config.fullscreen) {
		SetConfigOption(game, "SuperDerpy", "fullscreen", "1");
	} else {
		SetConfigOption(game, "SuperDerpy", "fullscreen", "0");
	}
#ifdef ALLEGRO_ANDROID
	al_set_display_flag(game->display, ALLEGRO_FRAMELESS, game->config.fullscreen);
#endif
	al_set_display_flag(game->display, ALLEGRO_FULLSCREEN_WINDOW, game->config.fullscreen);
	SetupViewport(game);
	PrintConsole(game, "Fullscreen toggled: %s", game->config.fullscreen ? "on" : "off");
	return game->config.fullscreen;
}

SYMBOL_EXPORT bool ToggleMute(struct Game* game) {
	game->config.mute = !game->config.mute;
	al_set_mixer_gain(game->audio.mixer, game->config.mute ? 0.0 : 1.0);
	SetConfigOption(game, "SuperDerpy", "mute", game->config.mute ? "1" : "0");
	PrintConsole(game, "Mute: %d", game->config.mute);
	return game->config.mute;
}

SYMBOL_EXPORT double GetGameSpeed(struct Game* game) {
	return game->_priv.speed;
}

SYMBOL_EXPORT void SetBackgroundColor(struct Game* game, ALLEGRO_COLOR bg) {
	game->_priv.bg = bg;
}

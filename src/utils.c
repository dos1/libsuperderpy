/*! \file utils.c
 *  \brief Helper functions.
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
#include <ctype.h>
#ifdef ALLEGRO_ANDROID
#include <android/log.h>
#endif

// TODO: split to separate files

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
	char* pch; // A pointer to each word.
	char word[255]; // A string containing the word (for convienence)
	char* breakchar = "\n";
	char lines[40][1024]; // A lovely array of strings to hold all the lines (40 max atm)
	char temp[1024]; // Holds the string data of the current line only.
	int line = 0; // Counts which line we are currently using.
	int height = al_get_font_line_height(font) + 1;

	// Setup our strings
	strncpy(stext, text, 1023);
	strncpy(temp, "", 1024);
	for (int i = 0; i < 40; i += 1) {
		strncpy(lines[i], "", 1024);
	}
	//-------------------- Code Begins

	char* context = NULL;

	pch = strtok_r(stext, " ", &context); // Get the first word.
	do {
		strncpy(word, "", 255); // Truncate the string, to ensure there's no crazy stuff in there from memory.
		snprintf(word, 255, "%s ", pch);
		strncat(temp, word, 255); // Append the word to the end of TempLine
		// This code checks for the new line character.
		if (strncmp(word, breakchar, 255) == 0) {
			line += 1; // Move down a Line
			strncpy(temp, "", 1024); // Clear the tempstring
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

SYMBOL_EXPORT void DrawCentered(ALLEGRO_BITMAP* bitmap, int x, int y, int flags) {
	al_draw_bitmap(bitmap, x - al_get_bitmap_width(bitmap) / 2.0, y - al_get_bitmap_height(bitmap) / 2.0, flags);
}

SYMBOL_EXPORT void DrawCenteredScaled(ALLEGRO_BITMAP* bitmap, int x, int y, double sx, double sy, int flags) {
	DrawCenteredTintedScaled(bitmap, al_map_rgb(255, 255, 255), x, y, sx, sy, flags);
}

SYMBOL_EXPORT void DrawCenteredTintedScaled(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, int x, int y, double sx, double sy, int flags) {
	al_draw_tinted_scaled_rotated_bitmap(bitmap, tint, al_get_bitmap_width(bitmap) / 2.0, al_get_bitmap_height(bitmap) / 2.0,
		x, y, sx, sy, 0, flags);
}

SYMBOL_EXPORT void ClearToColor(struct Game* game, ALLEGRO_COLOR color) {
	ALLEGRO_BITMAP* target = al_get_target_bitmap();
	if (GetFramebuffer(game) == target && al_get_parent_bitmap(target) == al_get_backbuffer(game->display)) {
		al_set_target_backbuffer(game->display);
	}
	int x, y, w, h;
	al_get_clipping_rectangle(&x, &y, &w, &h);
	al_reset_clipping_rectangle();
	al_clear_to_color(color);
	al_set_clipping_rectangle(x, y, w, h);
	al_set_target_bitmap(target);
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
	int x, y;
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
	ALLEGRO_BITMAP *source, *target = al_create_bitmap(width, height);
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

SYMBOL_EXPORT void FatalErrorWithContext(struct Game* game, int line, const char* file, const char* func, bool exit, char* format, ...) {
	// TODO: interrupt and take over loading thread when it happens
	char text[1024] = {0};
	PrintConsole(game, "Fatal Error, displaying Blue Screen of Derp...");
	va_list vl;
	va_start(vl, format);
	vsnprintf(text, 1024, format, vl);
	va_end(vl);
	fprintf(stderr, "%s:%d [%s]\n%s\n", file, line, func, text);

#ifndef LIBSUPERDERPY_SINGLE_THREAD
	if (game->_priv.loading.inProgress) {
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

		const char* header = game->name;
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
			al_draw_text(game->_priv.font_bsod, al_map_rgb(255, 255, 255), offsetx - head2w / 2.0, (int)(offsety + row * fonth * 1.25), ALLEGRO_ALIGN_LEFT, "This is fatal error. My bad.");

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
		int i;
		for (i = 0; i < ALLEGRO_KEY_PAUSE; i++) {
			if (al_key_down(&kb, i)) {
				done = true;
				break;
			}
		}
#else
		done = true;
#endif
	}
	al_use_transform(&game->projection);
#ifndef LIBSUPERDERPY_SINGLE_THREAD
	if (game->_priv.loading.inProgress) {
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

#ifdef ALLEGRO_MACOSX
	TestPath(filename, "../Resources/data/", &result);
	TestPath(filename, "../Resources/gamestates/", &result);
#endif

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

SYMBOL_EXPORT char* GetDataFilePath(struct Game* game, const char* filename) {
	char* result = 0;

#ifdef ALLEGRO_ANDROID
	char origfn[255] = "android/";
	strncat(origfn, filename, 246);

	result = TestDataFilePath(game, origfn);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

#if (defined __EMSCRIPTEN__) || (defined ALLEGRO_ANDROID)
	char* file = AddGarbage(game, strdup(filename));
	char* sub = strstr(file, ".flac");
	if (sub) {
		sub[0] = '.';
		sub[1] = 'o';
		sub[2] = 'g';
		sub[3] = 'g';
		sub[4] = 0;
	}
	result = TestDataFilePath(game, file);
	if (result) {
		return AddGarbage(game, result);
	}
#endif

	result = TestDataFilePath(game, filename);
	if (result) {
		return AddGarbage(game, result);
	}

	FatalError(game, true, "Could not find data file: %s!", filename);
#ifdef __EMSCRIPTEN__
	emscripten_exit_with_live_runtime();
#endif
	exit(1);
}

ALLEGRO_DEBUG_CHANNEL("libsuperderpy")

SYMBOL_EXPORT void PrintConsoleWithContext(struct Game* game, int line, const char* file, const char* func, char* format, ...) {
	va_list vl;
	va_start(vl, format);
	char* text = game->_priv.console[game->_priv.console_pos];
	vsnprintf(text, (sizeof(game->_priv.console[0]) / sizeof(game->_priv.console[0][0])), format, vl);
	va_end(vl);

	SUPPRESS_WARNING("-Wused-but-marked-unused")
	ALLEGRO_DEBUG("%s\n", text);
	SUPPRESS_END

#if !defined(__EMSCRIPTEN__) && !defined(ALLEGRO_ANDROID)
	if (game->config.debug)
#endif
	{
		if (game->_priv.debug.verbose) {
			printf("%s:%d ", file, line);
		}
		printf("[%s] %s\n", func, text);
#ifdef ALLEGRO_ANDROID
		__android_log_print(ANDROID_LOG_DEBUG, al_get_app_name(), "[%s] %s", func, text);
#endif
		fflush(stdout);
	}
	game->_priv.console_pos++;
	if (game->_priv.console_pos >= (sizeof(game->_priv.console) / sizeof(game->_priv.console[0]))) {
		game->_priv.console_pos = 0;
	}
}

SYMBOL_EXPORT void SetupViewport(struct Game* game, struct Viewport config) {
	game->viewport = config;

	if ((game->viewport.width == 0) || (game->viewport.height == 0)) {
		game->viewport.height = al_get_display_height(game->display);
		game->viewport.width = (int)(game->viewport.aspect * game->viewport.height);
		if (game->viewport.width > al_get_display_width(game->display)) {
			game->viewport.width = al_get_display_width(game->display);
			game->viewport.height = (int)(game->viewport.width / game->viewport.aspect);
		}
	}
	game->viewport.aspect = game->viewport.width / (float)game->viewport.height;

	al_set_target_backbuffer(game->display);
	al_identity_transform(&game->projection);
	al_use_transform(&game->projection);
	al_reset_clipping_rectangle();

	float resolution = al_get_display_height(game->display) / (float)game->viewport.height;
	if (al_get_display_width(game->display) / (float)game->viewport.width < resolution) {
		resolution = al_get_display_width(game->display) / (float)game->viewport.width;
	}
	if (game->viewport.integer_scaling) {
		resolution = floorf(resolution);
		if (floorf(resolution) == 0) {
			resolution = 1;
		}
	}
	if ((!strtol(GetConfigOptionDefault(game, "SuperDerpy", "downscale", "1"), NULL, 10)) && (resolution < 1)) {
		resolution = 1;
	}
	if (!strtol(GetConfigOptionDefault(game, "SuperDerpy", "scaling", "1"), NULL, 10)) {
		resolution = 1;
	}

	int clipWidth = (int)(game->viewport.width * resolution);
	int clipHeight = (int)(game->viewport.height * resolution);
	if (strtol(GetConfigOptionDefault(game, "SuperDerpy", "letterbox", "1"), NULL, 10)) {
		int clipX = (al_get_display_width(game->display) - clipWidth) / 2;
		int clipY = (al_get_display_height(game->display) - clipHeight) / 2;
		al_build_transform(&game->projection, clipX, clipY, resolution, resolution, 0.0f);
		al_set_clipping_rectangle(clipX, clipY, clipWidth, clipHeight);
		game->_priv.clip_rect.x = clipX;
		game->_priv.clip_rect.y = clipY;
		game->_priv.clip_rect.w = clipWidth;
		game->_priv.clip_rect.h = clipHeight;
	} else if (strtol(GetConfigOptionDefault(game, "SuperDerpy", "scaling", "1"), NULL, 10)) {
		al_build_transform(&game->projection, 0, 0, al_get_display_width(game->display) / (float)game->viewport.width, al_get_display_height(game->display) / (float)game->viewport.height, 0.0f);
	}
	al_use_transform(&game->projection);
	Console_Unload(game);
	Console_Load(game);
	ResizeGamestates(game);

	PrintConsole(game, "Viewport %dx%d; display %dx%d", game->viewport.width, game->viewport.height, al_get_display_width(game->display), al_get_display_height(game->display));
}

SYMBOL_EXPORT void WindowCoordsToViewport(struct Game* game, int* x, int* y) {
	int clipX, clipY, clipWidth, clipHeight;
	al_get_clipping_rectangle(&clipX, &clipY, &clipWidth, &clipHeight);
	*x -= clipX;
	*y -= clipY;
	*x /= (int)(clipWidth / (float)game->viewport.width);
	*y /= (int)(clipHeight / (float)game->viewport.height);
}

SYMBOL_EXPORT ALLEGRO_BITMAP* GetFramebuffer(struct Game* game) {
	if (!game->_priv.current_gamestate) {
		return game->loading_fb;
	}
	return game->_priv.current_gamestate->fb;
}

SYMBOL_EXPORT void SetFramebufferAsTarget(struct Game* game) {
	ALLEGRO_BITMAP* framebuffer = GetFramebuffer(game);
	al_set_target_bitmap(framebuffer);
	if (framebuffer != al_get_backbuffer(game->display)) {
		double x = al_get_bitmap_width(framebuffer) / (double)game->viewport.width;
		double y = al_get_bitmap_height(framebuffer) / (double)game->viewport.height;
		ALLEGRO_TRANSFORM t;
		al_identity_transform(&t);
		al_scale_transform(&t, x, y);
		al_use_transform(&t);
	}
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

SYMBOL_EXPORT void EnableCompositor(struct Game* game, void compositor(struct Game* game, struct Gamestate* gamestates)) {
	PrintConsole(game, "Compositor enabled.");
	game->handlers.compositor = compositor ? compositor : SimpleCompositor;
	ResizeGamestates(game);
}

SYMBOL_EXPORT void DisableCompositor(struct Game* game) {
	PrintConsole(game, "Compositor disabled.");
	game->handlers.compositor = NULL;
	ResizeGamestates(game);
}

SYMBOL_EXPORT char* StrToLower(struct Game* game, char* text) {
	char *res = strdup(text), *iter = res;
	while (*iter) {
		*iter = tolower(*iter);
		iter++;
	}
	return AddGarbage(game, res);
}

SYMBOL_EXPORT char* StrToUpper(struct Game* game, char* text) {
	char *res = strdup(text), *iter = res;
	while (*iter) {
		*iter = toupper(*iter);
		iter++;
	}
	return AddGarbage(game, res);
}

SYMBOL_EXPORT char* PunchNumber(struct Game* game, char* text, char ch, int number) {
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
	};
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

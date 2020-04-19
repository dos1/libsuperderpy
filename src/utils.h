/*! \file utils.h
 *  \brief Headers of helper functions.
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

#ifndef LIBSUPERDERPY_UTILS_H
#define LIBSUPERDERPY_UTILS_H

#include "libsuperderpy.h"

struct Gamestate;

/*! \brief Draws rectangle filled with vertical gradient. */
void DrawVerticalGradientRect(float x, float y, float w, float h, ALLEGRO_COLOR top, ALLEGRO_COLOR bottom);
/*! \brief Draws rectangle filled with horizontal gradient. */
void DrawHorizontalGradientRect(float x, float y, float w, float h, ALLEGRO_COLOR left, ALLEGRO_COLOR right);
/*! \brief Draws text with shadow.
 *
 * Draws given text two times: once with color (0,0,0,128) and 1px off in both x and y axis,
 * and second time with actual given color and position.
 */
void DrawTextWithShadow(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int flags, char const* text);

int DrawWrappedText(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int width, int flags, char const* text);
int DrawWrappedTextWithShadow(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y, int width, int flags, char const* text);

void DrawCentered(ALLEGRO_BITMAP* bitmap, float x, float y, int flags);
void DrawCenteredScaled(ALLEGRO_BITMAP* bitmap, float x, float y, double sx, double sy, int flags);
void DrawCenteredTintedScaled(ALLEGRO_BITMAP* bitmap, ALLEGRO_COLOR tint, float x, float y, double sx, double sy, int flags);

/*! \brief Clears the current target completely, without taking current clipping rectangle into account. */
void ClearToColor(struct Game* game, ALLEGRO_COLOR color);

ALLEGRO_COLOR InterpolateColor(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float frac);
void ScaleBitmap(ALLEGRO_BITMAP* source, int width, int height);

/*! \brief Loads bitmap into memory and scales it with software linear filtering. */
ALLEGRO_BITMAP* LoadScaledBitmap(struct Game* game, char* filename, int width, int height);

/*! \brief Finds the path for data file. Returns NULL when the file can't be found, or ephemeral string otherwise. */
const char* FindDataFilePath(struct Game* game, const char* filename);
/*! \brief Finds the path for data file. Triggers BSOD and quits when the file can't be found, returns ephemeral string otherwise. */
const char* GetDataFilePath(struct Game* game, const char* filename);

__attribute__((__format__(__printf__, 5, 6))) void PrintConsoleWithContext(struct Game* game, int line, const char* file, const char* func, char* format, ...);
/*! \brief Print some message on game console.
 *
 * Draws message on console bitmap, so it'll be displayed when calling DrawConsole.
 * If game->debug is true, then it also prints given message on stdout.
 * It needs to be called in printf style.
 */
#define PrintConsole(game, format, ...) PrintConsoleWithContext(game, __LINE__, __FILE__, __func__, format, ##__VA_ARGS__)

__attribute__((__format__(__printf__, 6, 7))) void FatalErrorWithContext(struct Game* game, int line, const char* file, const char* func, bool exit, char* format, ...);
#define FatalError(game, exit, format, ...) FatalErrorWithContext(game, __LINE__, __FILE__, __func__, exit, format, ##__VA_ARGS__)

void WindowCoordsToViewport(struct Game* game, int* x, int* y);

ALLEGRO_BITMAP* GetFramebuffer(struct Game* game);
void SetFramebufferAsTarget(struct Game* game);

void SetClippingRectangle(int x, int y, int width, int height);
void ResetClippingRectangle(void);

void PushTransform(struct Game* game, ALLEGRO_TRANSFORM* transform);
void PopTransform(struct Game* game);

ALLEGRO_BITMAP* CreateNotPreservedBitmap(int width, int height);

void EnableCompositor(struct Game* game, void compositor(struct Game* game));
void DisableCompositor(struct Game* game);

/*! \brief Converts given string to lowercase. Returns ephemeral string. */
char* StrToLower(struct Game* game, const char* text);
/*! \brief Converts given string to uppercase. Returns ephemeral string. */
char* StrToUpper(struct Game* game, const char* text);

/*! \brief Puts a given number into places in given string marked by given character. Returns ephemeral string. */
char* PunchNumber(struct Game* game, const char* text, char ch, int number);

/*! \brief Quits the game. On platforms that allow it, brings the game to the background without quiting if <allow_pausing> is true. */
void QuitGame(struct Game* game, bool allow_pausing);

bool ToggleFullscreen(struct Game* game);
bool ToggleMute(struct Game* game);

double GetGameSpeed(struct Game* game);

#endif /* LIBSUPERDERPY_UTILS_H */

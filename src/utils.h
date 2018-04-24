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
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#ifdef ALLEGRO_WINDOWS
#define LIBRARY_EXTENSION ".dll"
#elif defined(ALLEGRO_MACOSX)
#define LIBRARY_EXTENSION ".dylib"
#elif defined(__EMSCRIPTEN__)
#define LIBRARY_EXTENSION ".js"
#else
#define LIBRARY_EXTENSION ".so"
#endif

struct Viewport;
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

void DrawCentered(ALLEGRO_BITMAP* bitmap, int x, int y, int flags);
void DrawCenteredScaled(ALLEGRO_BITMAP* bitmap, int x, int y, double sx, double sy, int flags);

double DotProduct(const double v[], const double u[], int n);
double VectorLength(double x, double y, double z);
double Wrap(double left, double right, double val);
double Clamp(double left, double right, double val);
double Lerp(double left, double right, double pos);

ALLEGRO_COLOR InterpolateColor(ALLEGRO_COLOR c1, ALLEGRO_COLOR c2, float frac);
void ScaleBitmap(ALLEGRO_BITMAP* source, int width, int height);

/*! \brief Loads bitmap into memory and scales it with software linear filtering. */
ALLEGRO_BITMAP* LoadScaledBitmap(struct Game* game, char* filename, int width, int height);

/*! \brief Finds path for data file. */
char* GetDataFilePath(struct Game* game, const char* filename);

char* GetGameName(struct Game* game, const char* format);

/*! \brief Print some message on game console.
 *
 * Draws message on console bitmap, so it'll be displayed when calling DrawConsole.
 * If game->debug is true, then it also prints given message on stdout.
 * It needs to be called in printf style.
 */
void PrintConsole(struct Game* game, char* format, ...);

void FatalError(struct Game* game, bool exit, char* format, ...);

void SetupViewport(struct Game* game, struct Viewport config);

void WindowCoordsToViewport(struct Game* game, int* x, int* y);

ALLEGRO_BITMAP* GetFramebuffer(struct Game* game);

void SetFramebufferAsTarget(struct Game* game);

ALLEGRO_BITMAP* CreateNotPreservedBitmap(int width, int height);

void EnableCompositor(struct Game* game, void compositor(struct Game* game, struct Gamestate* gamestates));
void DisableCompositor(struct Game* game);

#endif

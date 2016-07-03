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

#ifndef LIBSUPERDERPY_INTERNAL_H
#define LIBSUPERDERPY_INTERNAL_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "libsuperderpy.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#  define SYMBOL_INTERNAL
#  define SYMBOL_EXPORT     __declspec(dllexport)
#  define SYMBOL_IMPORT     __declspec(dllimport)
#else
#  if defined(__GNUC__) && (__GNUC__ >= 4)
#    define SYMBOL_INTERNAL __attribute__ ((visibility ("hidden")))
#    define SYMBOL_EXPORT   __attribute__ ((visibility ("default")))
#  else
#    define SYMBOL_INTERNAL
#    define SYMBOL_EXPORT
#  endif
#  define SYMBOL_IMPORT     extern
#endif

void DrawGamestates(struct Game *game);
void LogicGamestates(struct Game *game);
void EventGamestates(struct Game *game, ALLEGRO_EVENT *ev);
void PauseGamestates(struct Game *game);
void ResumeGamestates(struct Game *game);
void DrawConsole(struct Game *game);
void Console_Load(struct Game *game);
void Console_Unload(struct Game *game);
void SetupViewport(struct Game *game);
void GamestateProgress(struct Game *game);

#endif

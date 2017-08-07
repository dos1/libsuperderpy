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

struct libsuperderpy_list {
		void *data;
		struct libsuperderpy_list *next;
};

void DrawGamestates(struct Game *game);
void LogicGamestates(struct Game *game);
void EventGamestates(struct Game *game, ALLEGRO_EVENT *ev);
void ReloadGamestates(struct Game *game);
void FreezeGamestates(struct Game *game);
void UnfreezeGamestates(struct Game *game);
void DrawConsole(struct Game *game);
void Console_Load(struct Game *game);
void Console_Unload(struct Game *game);
void GamestateProgress(struct Game *game);
void* AddGarbage(struct Game *game, void* data);
void ClearGarbage(struct Game *game);
void ClearScreen(struct Game *game);
void AddTimeline(struct Game *game, struct Timeline *timeline);
void RemoveTimeline(struct Game *game, struct Timeline *timeline);
void DrawTimelines(struct Game *game);

#endif

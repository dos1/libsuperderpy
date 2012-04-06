/*! \file level.c
 *  \brief Playable Level code.
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
#include <stdio.h>
#include "moonwalk.h"
#include "level.h"

void Level_Draw(struct Game *game) {
	Moonwalk_Draw(game);
}

void Level_Load(struct Game *game) {
	Moonwalk_Load(game);
}

int Level_Keydown(struct Game *game, ALLEGRO_EVENT *ev) {
	return Moonwalk_Keydown(game, ev);
}

void Level_Preload(struct Game *game) {
	Moonwalk_Preload(game);
}

void Level_Unload(struct Game *game) {
	Moonwalk_Unload(game);
}

void Level_UnloadBitmaps(struct Game *game) {
	Moonwalk_UnloadBitmaps(game);
}

void Level_PreloadBitmaps(struct Game *game) {
	Moonwalk_PreloadBitmaps(game);
}

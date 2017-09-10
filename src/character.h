/*! \file character.h
 *  \brief Headers of character and spritesheet functions.
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

#ifndef LIBSUPERDERPY_CHARACTER_H
#define LIBSUPERDERPY_CHARACTER_H

#include "libsuperderpy.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

/*! \brief Structure representing one spritesheet for character animation. */
struct Spritesheet {
	char* name; /*!< Name of the spritesheet (used in file paths). */
	ALLEGRO_BITMAP* bitmap; /*!< Spritesheet bitmap. */
	int rows; /*!< Number of rows in the spritesheet. */
	int cols; /*!< Number of columns in the spritesheet. */
	int blanks; /*!< Number of blank frames at the end of the spritesheet. */
	int width;
	int height;
	int delay;
	bool kill;
	int repeat;
	float scale; /*!< Scale modifier of the frame. */
	char* successor; /*!< Name of animation successor. If it's not blank, then animation will be played only once. */
	struct Spritesheet* next; /*!< Next spritesheet in the queue. */
};

/*! \brief Structure representing one visible character. */
struct Character {
	char* name; /*!< Name of the character (used in file paths). */
	struct Spritesheet* spritesheet; /*!< Current spritesheet used by character. */
	struct Spritesheet* spritesheets; /*!< List of all spritesheets registered to character. */
	char* successor;
	ALLEGRO_BITMAP* bitmap;
	int pos; /*!< Current spritesheet position. */
	int pos_tmp; /*!< A counter used to slow down spritesheet animation. */
	float x; /*!< Horizontal position of character. */
	float y; /*!< Vertical position of character. */
	float angle; /*!< Characters display angle (radians). */
	void* data; /*!< Additional, custom character data (HP etc.). */
	int repeat;
	bool shared;
	bool dead;
};

void SelectSpritesheet(struct Game* game, struct Character* character, char* name);
void ChangeSpritesheet(struct Game* game, struct Character* character, char* name);
void RegisterSpritesheet(struct Game* game, struct Character* character, char* name);

void DrawCharacterF(struct Game* game, struct Character* character, ALLEGRO_COLOR tint, int flags);
void DrawCharacter(struct Game* game, struct Character* character, ALLEGRO_COLOR tint, int flags);
void DrawScaledCharacterF(struct Game* game, struct Character* character, ALLEGRO_COLOR tint, float scalex, float scaley, int flags);
void DrawScaledCharacter(struct Game* game, struct Character* character, ALLEGRO_COLOR tint, float scalex, float scaley, int flags);

struct Character* CreateCharacter(struct Game* game, char* name);
void DestroyCharacter(struct Game* game, struct Character* character);

void LoadSpritesheets(struct Game* game, struct Character* character);
void UnloadSpritesheets(struct Game* game, struct Character* character);

void AnimateCharacter(struct Game* game, struct Character* character, float speed_modifier);
void MoveCharacter(struct Game* game, struct Character* character, float x, float y, float angle);
void MoveCharacterF(struct Game* game, struct Character* character, float x, float y, float angle);
void SetCharacterPosition(struct Game* game, struct Character* character, float x, float y, float angle);
void SetCharacterPositionF(struct Game* game, struct Character* character, float x, float y, float angle);

int GetCharacterX(struct Game* game, struct Character* character);
int GetCharacterY(struct Game* game, struct Character* character);
float GetCharacterAngle(struct Game* game, struct Character* character);

bool IsOnCharacter(struct Game* game, struct Character* character, int x, int y);

#endif

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

struct SpritesheetFrame {
	char* file;
	ALLEGRO_BITMAP* bitmap;
	double duration;
	int row;
	int col;
	int x;
	int y;
	bool flipX;
	bool flipY;
};

/*! \brief Structure representing one spritesheet for character animation. */
struct Spritesheet {
	char* name; /*!< Name of the spritesheet (used in file paths). */
	ALLEGRO_BITMAP* bitmap; /*!< Spritesheet bitmap. */
	int frameCount;
	int rows; /*!< Number of rows in the spritesheet. */
	int cols; /*!< Number of columns in the spritesheet. */
	double duration;
	char* file;
	int repeats; /*!< Number of repeats to make before the spritesheet is changed to its successor. */
	char* successor; /*!< Name of animation successor. If it's not blank, then animation will be played only once. */
	bool bidir;
	bool reversed;
	double pivotX;
	double pivotY;
	bool flipX;
	bool flipY;
	struct SpritesheetFrame* frames;

	int width;
	int height;

	struct Spritesheet* next; /*!< Next spritesheet in the queue. */

	// TODO: missing docs
};

/*! \brief Structure representing one visible character. */
struct Character {
	char* name; /*!< Name of the character (used in file paths). */
	struct Character* parent; /*!< Parent character. NULL is no parent. */
	//ALLEGRO_BITMAP* bitmap; /*!< Subbitmap with character's current frame. */
	struct SpritesheetFrame* frame; /*!< Current frame. */
	struct Spritesheet* spritesheet; /*!< Current spritesheet used by character. */
	struct Spritesheet* spritesheets; /*!< List of all spritesheets registered to character. */
	int pos; /*!< Current spritesheet position. */
	double delta; /*!< A counter used internally to slow down spritesheet animation. */ // TODO: change to delta
	char* successor; /*!< Name of the next spritesheet to be played when the current one finishes. */
	float x; /*!< Horizontal position of character. */
	float y; /*!< Vertical position of character. */
	ALLEGRO_COLOR tint; /*!< Color with which the character's pixels will be multiplied (tinted). White for no effect. */
	//float pivotX; /*!< Pivot point's X, for scaling and rotating, relative of character's size. */
	//float pivotY; /*!< Pivot point's Y, for scaling and rotating, relative of character's size. */
	float scaleX; /*!< Scale factor for X axis. */
	float scaleY; /*!< Scale factor for Y axis. */
	float angle; /*!< Character's rotation angle (radians). */
	int confineX; /*!< Width of the canvas being drawn to, for correct position calculation; when -1, uses parent's confines or viewport size */
	int confineY; /*!< Height of the canvas being drawn to, for correct position calculation; when -1, uses parent's confines or viewport size */
	bool flipX; /*!< Flips the character's sprite vertically. */
	bool flipY; /*!< Flips the character's sprite horizontally. */
	int repeats; /*!< Number of repeats left before the spritesheet is changed to its successor. */
	bool reversing;
	bool hidden;
	void* data; /*!< Additional, custom character data (HP etc.). */
	void (*callback)(struct Game*, struct Character*, char* newAnim, char* oldAnim, void*);
	void* callbackData;
	bool shared; /*!< Marks the list of spritesheets as shared, so it won't be freed together with the character. */

	// TODO: parents
};

// TODO: document functions

void SelectSpritesheet(struct Game* game, struct Character* character, char* name);
void EnqueueSpritesheet(struct Game* game, struct Character* character, char* name);
void RegisterSpritesheet(struct Game* game, struct Character* character, char* name);

void DrawCharacter(struct Game* game, struct Character* character);
void DrawScaledCharacterF(struct Game* game, struct Character* character, ALLEGRO_COLOR tint, float scalex, float scaley, int flags);
void DrawScaledCharacter(struct Game* game, struct Character* character, ALLEGRO_COLOR tint, float scalex, float scaley, int flags);

struct Character* CreateCharacter(struct Game* game, char* name);
void DestroyCharacter(struct Game* game, struct Character* character);

void LoadSpritesheets(struct Game* game, struct Character* character);
void UnloadSpritesheets(struct Game* game, struct Character* character);

void AnimateCharacter(struct Game* game, struct Character* character, float delta, float speed_modifier);
void MoveCharacter(struct Game* game, struct Character* character, float x, float y, float angle);
void MoveCharacterF(struct Game* game, struct Character* character, float x, float y, float angle);
void SetCharacterPosition(struct Game* game, struct Character* character, float x, float y, float angle);
void SetCharacterPositionF(struct Game* game, struct Character* character, float x, float y, float angle);
void SetCharacterPivotPoint(struct Game* game, struct Character* character, float x, float y);
void SetCharacterConfines(struct Game* game, struct Character* character, int x, int y);

float GetCharacterX(struct Game* game, struct Character* character);
float GetCharacterY(struct Game* game, struct Character* character);
int GetCharacterConfineX(struct Game* game, struct Character* character);
int GetCharacterConfineY(struct Game* game, struct Character* character);

bool IsOnCharacter(struct Game* game, struct Character* character, float x, float y, bool pixelperfect);
void ShowCharacter(struct Game* game, struct Character* character);
void HideCharacter(struct Game* game, struct Character* character);

#endif

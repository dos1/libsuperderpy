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

struct SpritesheetFrame {
	ALLEGRO_BITMAP* bitmap;
	char* file;
	double duration;
	ALLEGRO_COLOR tint;
	int row;
	int col;
	int x;
	int y;
	int sx;
	int sy;
	int sw;
	int sh;
	bool flipX;
	bool flipY;
	bool start;
	bool end;
	bool shared;
	bool owned;

	struct {
		ALLEGRO_BITMAP* image;
		char* filepath;
	} _priv;
};

typedef struct SpritesheetFrame SpritesheetStream(struct Game*, double, int, void*);
#define SPRITESHEET_STREAM(x) struct SpritesheetFrame x(struct Game* game, double delta, int frame, void* data)

typedef void SpritesheetStreamDestructor(struct Game*, void*);
#define SPRITESHEET_STREAM_DESCTRUCTOR(x) void x(struct Game* game, void* data)

/*! \brief Structure representing one spritesheet for character animation. */
struct Spritesheet {
	char* name; /*!< Name of the spritesheet (used in file paths). */
	ALLEGRO_BITMAP* bitmap; /*!< Spritesheet bitmap. */
	int frame_count;
	int rows; /*!< Number of rows in the spritesheet. */
	int cols; /*!< Number of columns in the spritesheet. */
	double duration;
	char* file;
	char* filepath;
	int repeats; /*!< Number of repeats to make before the spritesheet is changed to its successor. */
	char* successor; /*!< Name of animation successor. If it's not blank, then animation will be played only once. */
	char* predecessor;
	bool bidir;
	bool reversed;
	double pivotX;
	double pivotY;
	int offsetX;
	int offsetY;
	bool flipX;
	bool flipY;
	double scale;
	struct SpritesheetFrame* frames;
	bool shared; /*!< Marks the spritesheet bitmaps as shared, so they won't be freed together with the spritesheet. */
	SpritesheetStream* stream;
	SpritesheetStreamDestructor* stream_destructor;
	void* stream_data;

	int width;
	int height;

	struct Spritesheet* next; /*!< Next spritesheet in the queue. */

	// TODO: missing docs
};

struct Character;
typedef void CharacterCallback(struct Game*, struct Character*, struct Spritesheet* newAnim, struct Spritesheet* oldAnim, void*);
#define CHARACTER_CALLBACK(x) void x(struct Game* game, struct Character* character, struct Spritesheet* new, struct Spritesheet* old, void* data)
typedef void CharacterDestructor(struct Game*, struct Character*);

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
	char* predecessor; /*!< Name of the next spritesheet to be played when the current one finishes when in reverse mode. */
	float x; /*!< Horizontal position of character. */
	float y; /*!< Vertical position of character. */
	ALLEGRO_COLOR tint; /*!< Color with which the character's pixels will be multiplied (tinted). White for no effect. */
	bool parent_tint; /*!< When true, the character tint is multiplied by its parent tint. */
	//float pivotX; /*!< Pivot point's X, for scaling and rotating, relative of character's size. */
	//float pivotY; /*!< Pivot point's Y, for scaling and rotating, relative of character's size. */
	float scaleX; /*!< Scale factor for X axis. */
	float scaleY; /*!< Scale factor for Y axis. */
	float angle; /*!< Character's rotation angle (radians). */
	int confineX; /*!< Width of the canvas being drawn to, for correct position calculation; when -1, uses parent's confines or viewport size */
	int confineY; /*!< Height of the canvas being drawn to, for correct position calculation; when -1, uses parent's confines or viewport size */
	bool flipX; /*!< Flips the character's sprite vertically. */
	bool flipY; /*!< Flips the character's sprite horizontally. */
	int repeats; /*!< Number of repeats left before the spritesheet is changed to its successor or stopped. */
	bool reversing; /*!< Whether the animation is currently played backwards. */
	bool reversed; /*!< Whether the current animation has been requested as reversed. */
	bool hidden;
	bool finished;
	void* data; /*!< Additional, custom character data (HP etc.). */
	CharacterCallback* callback;
	void* callback_data;
	CharacterDestructor* destructor;
	bool shared; /*!< Marks the list of spritesheets as shared, so it won't be freed together with the character. */
	bool detailed_progress; /*!< Reports progress of loading individual frames. */
};

// TODO: document functions

void SelectSpritesheet(struct Game* game, struct Character* character, char* name);
void SwitchSpritesheet(struct Game* game, struct Character* character, char* name);
void EnqueueSpritesheet(struct Game* game, struct Character* character, char* name);
void RegisterSpritesheet(struct Game* game, struct Character* character, char* name);
void RegisterStreamedSpritesheet(struct Game* game, struct Character* character, char* name, SpritesheetStream* callback, SpritesheetStreamDestructor* destructor, void* data);
void RegisterSpritesheetFromBitmap(struct Game* game, struct Character* character, char* name, ALLEGRO_BITMAP* bitmap);
struct Spritesheet* GetSpritesheet(struct Game* game, struct Character* character, char* name);
void SetSpritesheetPosition(struct Game* game, struct Character* character, int frame);

ALLEGRO_TRANSFORM GetCharacterTransform(struct Game* game, struct Character* character);
ALLEGRO_COLOR GetCharacterTint(struct Game* game, struct Character* character);

void DrawCharacter(struct Game* game, struct Character* character);
void DrawDebugCharacter(struct Game* game, struct Character* character);

struct Character* CreateCharacter(struct Game* game, char* name);
void DestroyCharacter(struct Game* game, struct Character* character);

void LoadSpritesheets(struct Game* game, struct Character* character, void (*progress)(struct Game*));
void UnloadSpritesheets(struct Game* game, struct Character* character);
void PreloadStreamedSpritesheet(struct Game* game, struct Character* character, char* name);

void AnimateCharacter(struct Game* game, struct Character* character, float delta, float speed_modifier);
void MoveCharacter(struct Game* game, struct Character* character, float x, float y, float angle);
void MoveCharacterF(struct Game* game, struct Character* character, float x, float y, float angle);
void SetCharacterPosition(struct Game* game, struct Character* character, float x, float y, float angle);
void SetCharacterPositionF(struct Game* game, struct Character* character, float x, float y, float angle);
void SetCharacterConfines(struct Game* game, struct Character* character, int x, int y);

void SetParentCharacter(struct Game* game, struct Character* character, struct Character* parent);

void CopyCharacter(struct Game* game, struct Character* from, struct Character* to);

float GetCharacterX(struct Game* game, struct Character* character);
float GetCharacterY(struct Game* game, struct Character* character);
int GetCharacterConfineX(struct Game* game, struct Character* character);
int GetCharacterConfineY(struct Game* game, struct Character* character);

bool IsOnCharacter(struct Game* game, struct Character* character, float x, float y, bool pixelperfect);
void ShowCharacter(struct Game* game, struct Character* character);
void HideCharacter(struct Game* game, struct Character* character);
bool IsCharacterHidden(struct Game* game, struct Character* character);

#endif /* LIBSUPERDERPY_CHARACTER_H */

/*! \file gamestate.h
 *  \brief Gamestate management.
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
#ifndef LIBSUPERDERPY_GAMESTATE_H
#define LIBSUPERDERPY_GAMESTATE_H

#include "libsuperderpy.h"
#include <allegro5/allegro.h>

struct Gamestate_API {
	void (*Gamestate_Draw)(struct Game* game, void* data);
	void (*Gamestate_Logic)(struct Game* game, void* data);

	void* (*Gamestate_Load)(struct Game* game, void (*progress)(struct Game* game));
	void (*Gamestate_Start)(struct Game* game, void* data);
	void (*Gamestate_Pause)(struct Game* game, void* data);
	void (*Gamestate_Resume)(struct Game* game, void* data);
	void (*Gamestate_Stop)(struct Game* game, void* data);
	void (*Gamestate_Unload)(struct Game* game, void* data);

	void (*Gamestate_ProcessEvent)(struct Game* game, void* data, ALLEGRO_EVENT* ev);
	void (*Gamestate_Reload)(struct Game* game, void* data);

	int* Gamestate_ProgressCount;
};

struct Gamestate {
	char* name;
	void* handle;
	bool loaded, pending_load, pending_unload;
	bool started, pending_start, pending_stop;
	bool frozen;
	bool showLoading;
	bool paused;
	struct Gamestate* next;
	void* data;
	struct Gamestate_API* api;
};

void LoadGamestate(struct Game* game, const char* name);
void UnloadGamestate(struct Game* game, const char* name);
void RegisterGamestate(struct Game* game, const char* name, struct Gamestate_API* api);
void StartGamestate(struct Game* game, const char* name);
void StopGamestate(struct Game* game, const char* name);
void PauseGamestate(struct Game* game, const char* name);
void ResumeGamestate(struct Game* game, const char* name);
void PauseAllGamestates(struct Game* game);
void ResumeAllGamestates(struct Game* game);
void UnloadAllGamestates(struct Game* game);
void SwitchGamestate(struct Game* game, const char* current, const char* n);
void SwitchCurrentGamestate(struct Game* game, const char* n);
void ChangeGamestate(struct Game* game, const char* current, const char* n);
void ChangeCurrentGamestate(struct Game* game, const char* n);
void StopCurrentGamestate(struct Game* game);
void PauseCurrentGamestate(struct Game* game);
void UnloadCurrentGamestate(struct Game* game);

#endif

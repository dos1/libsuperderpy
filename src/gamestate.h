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

struct GamestateAPI {
	void (*draw)(struct Game* game, void* data);
	void (*logic)(struct Game* game, void* data, double delta);
	void (*tick)(struct Game* game, void* data);

	void* (*load)(struct Game* game, void (*progress)(struct Game* game));
	void (*post_load)(struct Game* game, void* data);
	void (*start)(struct Game* game, void* data);
	void (*pause)(struct Game* game, void* data);
	void (*resume)(struct Game* game, void* data);
	void (*stop)(struct Game* game, void* data);
	void (*unload)(struct Game* game, void* data);

	void (*process_event)(struct Game* game, void* data, ALLEGRO_EVENT* ev);
	void (*reload)(struct Game* game, void* data);

	int* progress_count;
};

struct Gamestate;

void LoadGamestate(struct Game* game, const char* name);
void UnloadGamestate(struct Game* game, const char* name);
void RegisterGamestate(struct Game* game, const char* name, struct GamestateAPI* api);
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
struct Gamestate* GetCurrentGamestate(struct Game* game);
struct Gamestate* GetGamestate(struct Game* game, const char* name);
ALLEGRO_BITMAP* GetGamestateFramebuffer(struct Game* game, struct Gamestate* gamestate);
struct Gamestate* GetNextGamestate(struct Game* game, struct Gamestate* gamestate);
bool IsGamestateVisible(struct Game* game, struct Gamestate* gamestate);

// Gamestate API

extern int Gamestate_ProgressCount;
__attribute__((used)) void Gamestate_Draw(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta);
__attribute__((used)) void Gamestate_Tick(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*));
__attribute__((used)) void Gamestate_PostLoad(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Start(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Pause(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Resume(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Stop(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Unload(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev);
__attribute__((used)) void Gamestate_Reload(struct Game* game, struct GamestateResources* data);

#endif /* LIBSUPERDERPY_GAMESTATE_H */

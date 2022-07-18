/*! \file gamestate.h
 *  \brief Gamestate management.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This file is part of libsuperderpy.
 *
 * libsuperderpy is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * libsuperderpy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libsuperderpy. If not, see <http://www.gnu.org/licenses/>.
 *
 * Also, ponies.
 */

#ifndef LIBSUPERDERPY_GAMESTATE_H
#define LIBSUPERDERPY_GAMESTATE_H

#include "libsuperderpy.h"

struct GamestateAPI {
	void (*draw)(struct Game* game, void* data);
	void (*logic)(struct Game* game, void* data, double delta);
	void (*tick)(struct Game* game, void* data);
	void (*predraw)(struct Game* game, void* data);
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

#if defined(LIBSUPERDERPY_STATIC_GAMESTATES) && defined(LIBSUPERDERPY_GAMESTATE)

#define GAMESTATE_CONCAT_STR(x, y) x##y
#define GAMESTATE_CONCAT(x, y) GAMESTATE_CONCAT_STR(x, y)

#define Gamestate_ProgressCount GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_ProgressCount)
#define Gamestate_Draw GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Draw)
#define Gamestate_Logic GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Logic)
#define Gamestate_Tick GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Tick)
#define Gamestate_PreDraw GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_PreDraw)
#define Gamestate_Load GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Load)
#define Gamestate_PostLoad GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_PostLoad)
#define Gamestate_Start GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Start)
#define Gamestate_Pause GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Pause)
#define Gamestate_Resume GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Resume)
#define Gamestate_Stop GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Stop)
#define Gamestate_Unload GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Unload)
#define Gamestate_ProcessEvent GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, Gamestate_ProcessEvent)
#define Gamestate_Reload GAMESTATE_CONCAT(LIBSUPERDERPY_GAMESTATE, _Gamestate_Reload)

#endif

extern int Gamestate_ProgressCount;
__attribute__((used)) void Gamestate_Draw(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta);
__attribute__((used)) void Gamestate_Tick(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_PreDraw(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*));
__attribute__((used)) void Gamestate_PostLoad(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Start(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Pause(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Resume(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Stop(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_Unload(struct Game* game, struct GamestateResources* data);
__attribute__((used)) void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev);
__attribute__((used)) void Gamestate_Reload(struct Game* game, struct GamestateResources* data);

#if defined(LIBSUPERDERPY_STATIC_GAMESTATES) && defined(LIBSUPERDERPY_GAMESTATE)

#define GAMESTATE_INIT_NAME_STR(x) __libsuperderpy_init_##x##_gamestate
#define GAMESTATE_INIT_NAME(x) GAMESTATE_INIT_NAME_STR(x)
#define GAMESTATE_STR(b) #b
#define GAMESTATE_STRINGIFY(a) GAMESTATE_STR(a)

void __libsuperderpy_register_gamestate(const char*, struct GamestateAPI*, struct Game*);

void GAMESTATE_INIT_NAME(LIBSUPERDERPY_GAMESTATE)(void) {
	struct GamestateAPI api = {
		.draw = (void*)Gamestate_Draw,
		.logic = (void*)Gamestate_Logic,
		.tick = (void*)Gamestate_Tick,
		.predraw = (void*)Gamestate_PreDraw,
		.load = (void*)Gamestate_Load,
		.post_load = (void*)Gamestate_PostLoad,
		.start = (void*)Gamestate_Start,
		.pause = (void*)Gamestate_Pause,
		.resume = (void*)Gamestate_Resume,
		.stop = (void*)Gamestate_Stop,
		.unload = (void*)Gamestate_Unload,
		.process_event = (void*)Gamestate_ProcessEvent,
		.reload = (void*)Gamestate_Reload,
		.progress_count = &Gamestate_ProgressCount,
	};
	__libsuperderpy_register_gamestate(GAMESTATE_STRINGIFY(LIBSUPERDERPY_GAMESTATE), &api, NULL);
}

#undef GAMESTATE_INIT_NAME_STR
#undef GAMESTATE_INIT_NAME
#undef GAMESTATE_STR
#undef GAMESTATE_STRINGIFY

#endif

#endif /* LIBSUPERDERPY_GAMESTATE_H */

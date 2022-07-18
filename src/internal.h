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

/// \privatesection

#ifndef LIBSUPERDERPY_INTERNAL_H
#define LIBSUPERDERPY_INTERNAL_H

#define LIBSUPERDERPY_PRIV_ACCESS

#include "libsuperderpy.h"
#ifdef LIBSUPERDERPY_IMGUI
#include "imgui/imgui_impl_allegro5.h"
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#define SYMBOL_INTERNAL
#define SYMBOL_EXPORT __declspec(dllexport)
#define SYMBOL_IMPORT __declspec(dllimport)
#else
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define SYMBOL_INTERNAL __attribute__((visibility("hidden")))
#define SYMBOL_EXPORT __attribute__((visibility("default")))
#else
#define SYMBOL_INTERNAL
#define SYMBOL_EXPORT
#endif
#define SYMBOL_IMPORT extern
#endif

#ifdef __EMSCRIPTEN__
#define IS_EMSCRIPTEN true
#else
#define IS_EMSCRIPTEN false
#endif

#ifdef POCKETCHIP
#define IS_POCKETCHIP true
#else
#define IS_POCKETCHIP false
#endif

#ifdef ALLEGRO_WINDOWS
#define LIBRARY_EXTENSION ".dll"
#elif defined(__EMSCRIPTEN__)
#if defined(LIBSUPERDERPY_WASM)
#define LIBRARY_EXTENSION ".wasm.so"
#else
#define LIBRARY_EXTENSION ".js"
#endif
#else
#define LIBRARY_EXTENSION ".so"
#endif

struct RefCount {
	int counter;
	char* id;
	void* data;
};

struct List {
	void* data;
	struct List* next;
};

struct GamestateLoadingThreadData {
	struct Game* game;
	struct Gamestate* gamestate;
	ALLEGRO_STATE state;
};

struct ScreenshotThreadData {
	struct Game* game;
	ALLEGRO_BITMAP* bitmap;
};

struct Gamestate {
	char* name;
	void* handle;
	bool loaded, pending_load, pending_unload;
	bool started, pending_start, pending_stop;
	bool frozen;
	bool show_loading;
	bool paused;
	bool fromlib;
	bool open;
	struct Gamestate* next;
	struct GamestateAPI* api;
	ALLEGRO_BITMAP* fb;
	int progress_count;
	void* data;
};

void SimpleCompositor(struct Game* game);
void DrawGamestates(struct Game* game);
void LogicGamestates(struct Game* game, double delta);
void EventGamestates(struct Game* game, ALLEGRO_EVENT* ev);
void ReloadGamestates(struct Game* game);
void FreezeGamestates(struct Game* game);
void UnfreezeGamestates(struct Game* game);
void ResizeGamestates(struct Game* game);
int SetupAudio(struct Game* game);
void StopAudio(struct Game* game);
void DrawConsole(struct Game* game);
void Console_Load(struct Game* game);
void Console_Unload(struct Game* game);
void* GamestateLoadingThread(void* arg);
void* ScreenshotThread(void* arg);
void CalculateProgress(struct Game* game);
void GamestateProgress(struct Game* game);
void* AddGarbage(struct Game* game, void* data);
void ClearGarbage(struct Game* game);
void ClearScreen(struct Game* game);
struct List* AddToList(struct List* list, void* data);
struct List* FindInList(struct List* list, void* data, bool (*identity)(struct List* elem, void* data));
void* RemoveFromList(struct List** list, void* data, bool (*identity)(struct List* elem, void* data));
void AddTimeline(struct Game* game, struct Timeline* timeline);
void RemoveTimeline(struct Game* game, struct Timeline* timeline);
void DrawTimelines(struct Game* game);
bool OpenGamestate(struct Game* game, struct Gamestate* gamestate, bool required);
bool LinkGamestate(struct Game* game, struct Gamestate* gamestate);
void CloseGamestate(struct Game* game, struct Gamestate* gamestate);
struct Gamestate* AllocateGamestate(struct Game* game, const char* name);
char* GetLibraryPath(struct Game* game, char* filename);
void PauseExecution(struct Game* game);
void ReloadCode(struct Game* game);
void ResumeExecution(struct Game* game);
void ReloadShaders(struct Game* game, bool force);
void DestroyShaders(struct Game* game);
__attribute__((__format__(__printf__, 2, 0))) char* GetGameName(struct Game* game, const char* format);
ALLEGRO_BITMAP* AddBitmap(struct Game* game, char* filename);
void RemoveBitmap(struct Game* game, char* filename);
void SetupViewport(struct Game* game);
void RedrawScreen(struct Game* game);

#endif /* LIBSUPERDERPY_INTERNAL_H */

/*! \file gamestate.c
 *  \brief Gamestate management.
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

#include "internal.h"
#include "utils.h"
#include "gamestate.h"

SYMBOL_INTERNAL struct Gamestate* AddNewGamestate(struct Game *game, const char* name) {
	struct Gamestate *tmp = game->_priv.gamestates;
	if (!tmp) {
		game->_priv.gamestates = malloc(sizeof(struct Gamestate));
		tmp = game->_priv.gamestates;
	} else {
		while (tmp->next) {
			tmp = tmp->next;
		}
		tmp->next = malloc(sizeof(struct Gamestate));
		tmp = tmp->next;
	}
	tmp->name = strdup(name);
	tmp->handle = NULL;
	tmp->loaded = false;
	tmp->paused = false;
	tmp->frozen = false;
	tmp->started = false;
	tmp->pending_load = false;
	tmp->pending_start = false;
	tmp->pending_stop = false;
	tmp->pending_unload = false;
	tmp->next = NULL;
	tmp->api = NULL;
	return tmp;
}

SYMBOL_INTERNAL struct Gamestate* FindGamestate(struct Game *game, const char* name) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if (!strcmp(name, tmp->name)) {
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;
}

SYMBOL_EXPORT void RegisterGamestate(struct Game *game, const char* name, struct Gamestate_API *api) {
	struct Gamestate *gs = FindGamestate(game, name);
	if (!gs) {
		gs = AddNewGamestate(game, name);
	}
	if (gs->api) {
		PrintConsole(game, "Trying to register already registered gamestate \"%s\"!", name);
		return;
	}
	gs->api = api;
	PrintConsole(game, "Gamestate \"%s\" registered.", name);
}

SYMBOL_EXPORT void LoadGamestate(struct Game *game, const char* name) {
	struct Gamestate *gs = FindGamestate(game, name);
	if (gs) {
		if (gs->loaded && !gs->pending_unload) {
			PrintConsole(game, "Gamestate \"%s\" already loaded.", name);
			return;
		}
		gs->pending_load = true;
	} else {
		gs = AddNewGamestate(game, name);
		gs->pending_load = true;
		gs->showLoading = true;
	}
	PrintConsole(game, "Gamestate \"%s\" marked to be LOADED.", name);
	game->_priv.gamestate_scheduled = true;
}

SYMBOL_EXPORT void UnloadGamestate(struct Game *game, const char* name) {
	struct Gamestate *gs = FindGamestate(game, name);
	if (gs) {
		if (gs->pending_load) {
			gs->pending_load = false;
			PrintConsole(game, "Canceling loading of gamestate \"%s\".", name);
			return;
		}
		if (!gs->loaded) {
			PrintConsole(game, "Gamestate \"%s\" already unloaded.", name);
			return;
		}
		if (gs->started) gs->pending_stop=true;
		gs->pending_unload = true;
		PrintConsole(game, "Gamestate \"%s\" marked to be UNLOADED.", name);
	} else {
		PrintConsole(game, "Tried to unload nonexisitent gamestate \"%s\"", name);
	}
	game->_priv.gamestate_scheduled = true;
}

SYMBOL_EXPORT void StartGamestate(struct Game *game, const char* name) {
	struct Gamestate *gs = FindGamestate(game, name);
	if (gs) {
		if (gs->started && !gs->pending_stop) {
			PrintConsole(game, "Gamestate \"%s\" already started.", name);
			return;
		}
		gs->pending_start = true;
		PrintConsole(game, "Gamestate \"%s\" marked to be STARTED.", name);
	} else {
		PrintConsole(game, "Tried to start nonexisitent gamestate \"%s\"", name);
	}
	game->_priv.gamestate_scheduled = true;
}

SYMBOL_EXPORT void StopGamestate(struct Game *game, const char* name) {
	struct Gamestate *gs = FindGamestate(game, name);
	if (gs) {
		if (gs->pending_start) {
			gs->pending_start = false;
			PrintConsole(game, "Canceling starting of gamestate \"%s\".", name);
			return;
		}
		if (!gs->started) {
			PrintConsole(game, "Gamestate \"%s\" already stopped.", name);
			return;
		}
		gs->pending_stop = true;
		PrintConsole(game, "Gamestate \"%s\" marked to be STOPPED.", name);
	} else {
		PrintConsole(game, "Tried to stop nonexisitent gamestate \"%s\"", name);
	}
	game->_priv.gamestate_scheduled = true;
}

SYMBOL_EXPORT void PauseGamestate(struct Game *game, const char* name) {
	struct Gamestate *gs = FindGamestate(game, name);
	if (gs) {
		if (!gs->started) {
			PrintConsole(game, "Tried to pause gamestate \"%s\" which is not started.", name);
			return;
		}
		if (gs->paused) {
			PrintConsole(game, "Gamestate \"%s\" already paused.", name);
			return;
		}
		gs->paused = true;
		game->_priv.current_gamestate = gs;
		(*gs->api->Gamestate_Pause)(game, gs->data);
		PrintConsole(game, "Gamestate \"%s\" paused.", name);
	} else {
		PrintConsole(game, "Tried to pause nonexisitent gamestate \"%s\"", name);
	}
}

SYMBOL_EXPORT void ResumeGamestate(struct Game *game, const char* name) {
	struct Gamestate *gs = FindGamestate(game, name);
	if (gs) {
		if (!gs->started) {
			PrintConsole(game, "Tried to resume gamestate \"%s\" which is not started.", name);
			return;
		}
		if (!gs->paused) {
			PrintConsole(game, "Gamestate \"%s\" already resumed.", name);
			return;
		}
		gs->paused = false;
		game->_priv.current_gamestate = gs;
		(*gs->api->Gamestate_Resume)(game, gs->data);
		PrintConsole(game, "Gamestate \"%s\" resumed.", name);
	} else {
		PrintConsole(game, "Tried to resume nonexisitent gamestate \"%s\"", name);
	}
}

SYMBOL_EXPORT void UnloadAllGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		UnloadGamestate(game, tmp->name);
		tmp = tmp->next;
	}
}

SYMBOL_EXPORT void PauseAllGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->started || !tmp->paused) {
			PauseGamestate(game, tmp->name);
		}
		tmp = tmp->next;
	}
}

SYMBOL_EXPORT void ResumeAllGamestates(struct Game *game) {
	struct Gamestate *tmp = game->_priv.gamestates;
	while (tmp) {
		if (tmp->paused) {
			ResumeGamestate(game, tmp->name);
		}
		tmp = tmp->next;
	}
}

SYMBOL_EXPORT void SwitchGamestate(struct Game *game, const char* current, const char* n) {
	StopGamestate(game, current);
	UnloadGamestate(game, current);
	LoadGamestate(game, n);
	StartGamestate(game, n);
}

SYMBOL_EXPORT void ChangeGamestate(struct Game *game, const char* current, const char* n) {
	StopGamestate(game, current);
	LoadGamestate(game, n);
	StartGamestate(game, n);
}

SYMBOL_EXPORT void SwitchCurrentGamestate(struct Game *game, const char* n) {
	SwitchGamestate(game, game->_priv.current_gamestate->name, n);
}

SYMBOL_EXPORT void ChangeCurrentGamestate(struct Game *game, const char* n) {
	ChangeGamestate(game, game->_priv.current_gamestate->name, n);
}

SYMBOL_EXPORT void StopCurrentGamestate(struct Game *game) {
	StopGamestate(game, game->_priv.current_gamestate->name);
}

SYMBOL_EXPORT void PauseCurrentGamestate(struct Game *game) {
	PauseGamestate(game, game->_priv.current_gamestate->name);
}

SYMBOL_EXPORT void UnloadCurrentGamestate(struct Game *game) {
	UnloadGamestate(game, game->_priv.current_gamestate->name);
}

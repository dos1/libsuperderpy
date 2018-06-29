/*! \file timeline.c
 *  \brief Timeline Manager framework code.
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
#include "timeline.h"
#include "internal.h"
#include "utils.h"
#include <allegro5/allegro.h>

static void DestroyArgs(struct TM_Arguments* args) {
	struct TM_Arguments* pom;
	while (args) {
		pom = args->next;
		free(args);
		args = pom;
	}
}

SYMBOL_EXPORT struct Timeline* TM_Init(struct Game* game, char* name) {
	PrintConsole(game, "Timeline Manager[%s]: init", name);
	struct Timeline* timeline = malloc(sizeof(struct Timeline));
	timeline->game = game;
	timeline->lastid = 0;
	timeline->queue = NULL;
	timeline->background = NULL;
	timeline->name = strdup(name);
	AddTimeline(game, timeline);
	return timeline;
}

SYMBOL_EXPORT void TM_Process(struct Timeline* timeline, double delta) {
	// NOTICE: current implementation has no way to know how much time
	// an action has "eaten". This means that if you pass a huge delta
	// that spans across multiple actions, the end result will most likely
	// differ from what you would get from calling TM_Process repeatively
	// with smaller deltas that sum up to the first value. Be aware!

	/* process first element from queue.
		 if returns true, delete it and repeat for the next one */
	delta *= 1000;
	bool next = true;
	while (next) {
		if (timeline->queue) {
			timeline->queue->delta = delta / 1000.0;

			if (timeline->queue->active && timeline->queue->delay > 0.0) {
				timeline->queue->delay -= delta;
				if (timeline->queue->delay <= 0.0) {
					timeline->queue->started = true;
					if (timeline->queue->function) {
						PrintConsole(timeline->game, "Timeline Manager[%s]: queue: run action (%d - %s)", timeline->name, timeline->queue->id, timeline->queue->name);
						(*timeline->queue->function)(timeline->game, timeline->queue, TM_ACTIONSTATE_START);
					} else {
						PrintConsole(timeline->game, "Timeline Manager[%s]: queue: delay reached (%d - %s)", timeline->name, timeline->queue->id, timeline->queue->name);
					}
					timeline->queue->delay = 0.0;
				}
			}

			if (timeline->queue->function) {
				if (!timeline->queue->started) {
					PrintConsole(timeline->game, "Timeline Manager[%s]: queue: run action (%d - %s)", timeline->name, timeline->queue->id, timeline->queue->name);
					(*timeline->queue->function)(timeline->game, timeline->queue, TM_ACTIONSTATE_START);
					timeline->queue->started = true;
				}
				if ((*timeline->queue->function)(timeline->game, timeline->queue, TM_ACTIONSTATE_RUNNING)) {
					PrintConsole(timeline->game, "Timeline Manager[%s]: queue: destroy action (%d - %s)", timeline->name, timeline->queue->id, timeline->queue->name);
					struct TM_Action* tmp = timeline->queue;
					timeline->queue = timeline->queue->next;
					(*tmp->function)(timeline->game, tmp, TM_ACTIONSTATE_DESTROY);
					DestroyArgs(tmp->arguments);
					free(tmp->name);
					free(tmp);
				} else {
					next = false;
				}
			} else {
				/* delay handling */
				if (timeline->queue->started) {
					struct TM_Action* tmp = timeline->queue;
					timeline->queue = timeline->queue->next;
					free(tmp->name);
					free(tmp);
				} else {
					if (!timeline->queue->active) {
						PrintConsole(timeline->game, "Timeline Manager[%s]: queue: delay started %d ms (%d - %s)", timeline->name, (int)timeline->queue->delay, timeline->queue->id, timeline->queue->name);
						timeline->queue->active = true;
					}
					next = false;
				}
			}
		} else {
			next = false;
		}
	}

	/* process all elements from background queue */
	struct TM_Action *tmp, *tmp2, *pom = timeline->background;
	tmp = NULL;
	while (pom != NULL) {
		bool destroy = false;
		pom->delta = delta / 1000.0;
		if (pom->started) {
			if (pom->function) {
				if ((pom->function)(timeline->game, pom, TM_ACTIONSTATE_RUNNING)) {
					PrintConsole(timeline->game, "Timeline Manager[%s]: background: destroy action (%d - %s)", timeline->name, pom->id, pom->name);
					(pom->function)(timeline->game, pom, TM_ACTIONSTATE_DESTROY);
					if (tmp) {
						tmp->next = pom->next;
					} else {
						timeline->background = pom->next;
					}
					destroy = true;
				}
			} else {
				/* delay handling */
				if (tmp) {
					tmp->next = pom->next;
				} else {
					timeline->background = pom->next;
				}
				destroy = true;
			}
		} else {
			pom->delay -= delta;
			if (pom->delay <= 0.0) {
				PrintConsole(timeline->game, "Timeline Manager[%s]: background: delay reached, run action (%d - %s)", timeline->name, pom->id, pom->name);
				pom->delay = 0.0;
				if (pom->function) {
					pom->function(timeline->game, pom, TM_ACTIONSTATE_START);
				}
				pom->started = true;
			}
		}

		if (!destroy) {
			tmp = pom;
			pom = pom->next;
		} else {
			DestroyArgs(pom->arguments);
			free(pom->name);
			free(pom);
			tmp2 = tmp;
			if (!tmp) {
				if (timeline->background) {
					pom = timeline->background->next;
				} else {
					pom = NULL;
				}
			} else {
				pom = tmp->next;
			}
			tmp = tmp2;
		}
	}
}

SYMBOL_EXPORT struct TM_Action* TM_AddAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, char* name) {
	struct TM_Action* action = malloc(sizeof(struct TM_Action));
	if (timeline->queue) {
		struct TM_Action* pom = timeline->queue;
		while (pom->next != NULL) {
			pom = pom->next;
		}
		pom->next = action;
	} else {
		timeline->queue = action;
	}
	action->next = NULL;
	action->function = func;
	action->arguments = args;
	action->name = strdup(name);
	action->active = false;
	action->started = false;
	action->delay = 0.0;
	action->id = ++timeline->lastid;
	if (action->function) {
		PrintConsole(timeline->game, "Timeline Manager[%s]: queue: init action (%d - %s)", timeline->name, action->id, action->name);
		action->function(timeline->game, action, TM_ACTIONSTATE_INIT);
	}
	return action;
}

SYMBOL_EXPORT struct TM_Action* TM_AddBackgroundAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, int delay, char* name) {
	struct TM_Action* action = malloc(sizeof(struct TM_Action));
	if (timeline->background) {
		struct TM_Action* pom = timeline->background;
		while (pom->next != NULL) {
			pom = pom->next;
		}
		pom->next = action;
	} else {
		timeline->background = action;
	}
	action->next = NULL;
	action->function = func;
	action->arguments = args;
	action->name = strdup(name);
	action->delay = delay;
	action->id = ++timeline->lastid;
	action->active = true;
	action->started = false;
	PrintConsole(timeline->game, "Timeline Manager[%s]: background: init action with delay %d ms (%d - %s)", timeline->name, delay, action->id, action->name);
	(*action->function)(timeline->game, action, TM_ACTIONSTATE_INIT);
	return action;
}

/*! \brief Predefined action used by TM_AddQueuedBackgroundAction */
static TM_Action(RunInBackground) {
	int* delay = (int*)TM_GetArg(action->arguments, 1);
	char* name = (char*)TM_GetArg(action->arguments, 2);
	struct Timeline* timeline = (struct Timeline*)TM_GetArg(action->arguments, 3);
	struct TM_Arguments* arguments = (struct TM_Arguments*)TM_GetArg(action->arguments, 4);
	bool* used = (bool*)TM_GetArg(action->arguments, 5);
	if (state == TM_ACTIONSTATE_START) {
		TM_AddBackgroundAction(timeline, TM_GetArg(action->arguments, 0), arguments, *delay, name);
		*used = true;
	}
	if (state == TM_ACTIONSTATE_DESTROY) {
		free(name);
		free(delay);
		if (!(*used)) {
			DestroyArgs(arguments);
		}
		free(used);
	}
	return true;
}

SYMBOL_EXPORT struct TM_Action* TM_AddQueuedBackgroundAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, int delay, char* name) {
	TM_WrapArg(int, del, delay);
	TM_WrapArg(bool, used, false);
	struct TM_Arguments* arguments = TM_AddToArgs(NULL, 6, (void*)func, del, strdup(name), (void*)timeline, args, used);
	return TM_AddAction(timeline, RunInBackground, arguments, "TM_BackgroundAction");
}

SYMBOL_EXPORT void TM_AddDelay(struct Timeline* timeline, int delay) {
	struct TM_Action* tmp = TM_AddAction(timeline, NULL, NULL, "TM_Delay");
	PrintConsole(timeline->game, "Timeline Manager[%s]: queue: adding delay %d ms (%d)", timeline->name, delay, tmp->id);
	tmp->delay = delay;
}

SYMBOL_EXPORT void TM_CleanQueue(struct Timeline* timeline) {
	PrintConsole(timeline->game, "Timeline Manager[%s]: cleaning queue", timeline->name);
	struct TM_Action *tmp, *pom = timeline->queue;
	while (pom != NULL) {
		if (*pom->function) {
			(*pom->function)(timeline->game, pom, TM_ACTIONSTATE_DESTROY);
		}
		DestroyArgs(pom->arguments);
		tmp = pom->next;
		free(pom->name);
		free(pom);
		pom = tmp;
		timeline->queue = pom;
	}
}

SYMBOL_EXPORT void TM_CleanBackgroundQueue(struct Timeline* timeline) {
	PrintConsole(timeline->game, "Timeline Manager[%s]: cleaning background queue", timeline->name);
	struct TM_Action *tmp, *pom = timeline->background;
	while (pom != NULL) {
		if (*pom->function) {
			(*pom->function)(timeline->game, pom, TM_ACTIONSTATE_DESTROY);
		}
		DestroyArgs(pom->arguments);
		tmp = pom->next;
		free(pom->name);
		free(pom);
		pom = tmp;
		timeline->background = pom;
	}
}

SYMBOL_EXPORT void TM_SkipDelay(struct Timeline* timeline) {
	if (timeline->queue && timeline->queue->delay) {
		timeline->queue->delay = 0.0;
	}
}

SYMBOL_EXPORT bool TM_IsEmpty(struct Timeline* timeline) {
	return !timeline->queue;
}
SYMBOL_EXPORT bool TM_IsBackgroundEmpty(struct Timeline* timeline) {
	return !timeline->background;
}

SYMBOL_EXPORT void TM_Destroy(struct Timeline* timeline) {
	RemoveTimeline(timeline->game, timeline);
	TM_CleanQueue(timeline);
	TM_CleanBackgroundQueue(timeline);
	PrintConsole(timeline->game, "Timeline Manager[%s]: destroy", timeline->name);
	free(timeline->name);
	free(timeline);
}

SYMBOL_EXPORT struct TM_Arguments* TM_AddToArgs(struct TM_Arguments* args, int num, ...) {
	va_list ap;
	int i;
	va_start(ap, num);
	struct TM_Arguments* tmp = args;
	for (i = 0; i < num; i++) {
		if (!tmp) {
			tmp = malloc(sizeof(struct TM_Arguments));
			tmp->value = va_arg(ap, void*);
			tmp->next = NULL;
			args = tmp;
		} else {
			while (tmp->next) {
				tmp = tmp->next;
			}
			tmp->next = malloc(sizeof(struct TM_Arguments));
			tmp->next->value = va_arg(ap, void*);
			tmp->next->next = NULL;
		}
	}
	va_end(ap);
	return args;
}

SYMBOL_EXPORT void* TM_GetArg(struct TM_Arguments* args, int num) {
	for (int i = 0; i < num; i++) {
		args = args->next;
	}
	return args->value;
}

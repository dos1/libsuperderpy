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

#include "internal.h"

static void DestroyArgs(struct TM_Arguments* args) {
	struct TM_Arguments* pom;
	while (args) {
		pom = args->next;
		free(args);
		args = pom;
	}
}

SYMBOL_EXPORT struct Timeline* TM_Init(struct Game* game, struct GamestateResources* data, char* name) {
	PrintConsole(game, "Timeline Manager[%s]: init", name);
	struct Timeline* timeline = malloc(sizeof(struct Timeline));
	timeline->game = game;
	timeline->lastid = 0;
	timeline->queue = NULL;
	timeline->background = NULL;
	timeline->name = strdup(name);
	timeline->data = data;
	AddTimeline(game, timeline);
	return timeline;
}

SYMBOL_EXPORT void TM_Process(struct Timeline* timeline, double delta) {
	// NOTICE: if you create background actions from running action,
	// newly created ones have no way to know how much time has been
	// "eaten" by the one that created it, which may result in incorrect
	// result when calling TM_Process with huge delta. This behaviour
	// may change in the future.

	// TODO: make sure new action STARTS in the same tick as the old one
	// DESTROYS, but make it RUNNING only in the next tick (or when there's
	// some remaining delta).

	/* process first element from queue.
		 if returns true, delete it and repeat for the next one */
	double origDelta = delta;
	while (delta > 0.0) {
		if (timeline->queue) {
			timeline->queue->delta = delta;

			if (timeline->queue->active && timeline->queue->delay > 0.0) {
				timeline->queue->delay -= delta;
				if (timeline->queue->delay <= 0.0) {
					timeline->queue->started = true;
					if (timeline->queue->function) {
						PrintConsole(timeline->game, "Timeline Manager[%s]: queue: run action (%d - %s)", timeline->name, timeline->queue->id, timeline->queue->name);
						timeline->queue->state = TM_ACTIONSTATE_START;
						(*timeline->queue->function)(timeline->game, timeline->data, timeline->queue);
					} else {
						PrintConsole(timeline->game, "Timeline Manager[%s]: queue: delay reached (%d - %s)", timeline->name, timeline->queue->id, timeline->queue->name);
					}
					timeline->queue->delay = 0.0;
				}
			}

			if (timeline->queue->function) {
				if (!timeline->queue->started) {
					PrintConsole(timeline->game, "Timeline Manager[%s]: queue: run action (%d - %s)", timeline->name, timeline->queue->id, timeline->queue->name);
					timeline->queue->state = TM_ACTIONSTATE_START;
					(*timeline->queue->function)(timeline->game, timeline->data, timeline->queue);
					timeline->queue->started = true;
				}
				timeline->queue->state = TM_ACTIONSTATE_RUNNING;
				if ((*timeline->queue->function)(timeline->game, timeline->data, timeline->queue)) {
					PrintConsole(timeline->game, "Timeline Manager[%s]: queue: destroy action (%d - %s)", timeline->name, timeline->queue->id, timeline->queue->name);
					delta -= timeline->queue->delta;
					struct TM_Action* tmp = timeline->queue;
					timeline->queue = timeline->queue->next;
					tmp->state = TM_ACTIONSTATE_DESTROY;
					(*tmp->function)(timeline->game, timeline->data, tmp);
					DestroyArgs(tmp->arguments);
					free(tmp->name);
					free(tmp);
				} else {
					delta = 0.0;
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
						PrintConsole(timeline->game, "Timeline Manager[%s]: queue: delay started %d ms (%d - %s)", timeline->name, (int)(timeline->queue->delay * 1000), timeline->queue->id, timeline->queue->name);
						timeline->queue->active = true;
					}
					delta = 0.0;
				}
			}
		} else {
			delta = 0.0;
		}
	}
	delta = origDelta;

	/* process all elements from background queue */
	struct TM_Action *tmp, *tmp2, *pom = timeline->background;
	tmp = NULL;
	while (pom != NULL) {
		bool destroy = false;
		pom->delta = delta;
		if (pom->started) {
			if (pom->function) {
				pom->state = TM_ACTIONSTATE_RUNNING;
				if ((pom->function)(timeline->game, timeline->data, pom)) {
					PrintConsole(timeline->game, "Timeline Manager[%s]: background: destroy action (%d - %s)", timeline->name, pom->id, pom->name);
					pom->state = TM_ACTIONSTATE_DESTROY;
					(pom->function)(timeline->game, timeline->data, pom);
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
					pom->state = TM_ACTIONSTATE_START;
					pom->function(timeline->game, timeline->data, pom);
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

static struct TM_Action* CreateAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, char* name) {
	struct TM_Action* action = malloc(sizeof(struct TM_Action));
	action->next = NULL;
	action->function = func;
	action->arguments = args;
	action->name = strdup(name);
	action->active = false;
	action->started = false;
	action->delay = 0.0;
	action->id = ++timeline->lastid;
	action->timeline = timeline;
	if (action->function) {
		PrintConsole(timeline->game, "Timeline Manager[%s]: queue: init action (%d - %s)", timeline->name, action->id, action->name);
		action->state = TM_ACTIONSTATE_INIT;
		action->function(timeline->game, timeline->data, action);
	}
	return action;
}

SYMBOL_EXPORT struct TM_Action* TM_AddNamedAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, char* name) {
	struct TM_Action* action = CreateAction(timeline, func, args, name);
	if (timeline->queue) {
		struct TM_Action* pom = timeline->queue;
		while (pom->next != NULL) {
			pom = pom->next;
		}
		pom->next = action;
	} else {
		timeline->queue = action;
	}
	return action;
}

SYMBOL_EXPORT struct TM_Action* TM_AddNamedActionAfter(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, struct TM_Action* after, char* name) {
	struct TM_Action* action = CreateAction(timeline, func, args, name);
	action->next = after->next;
	after->next = action;
	return action;
}

SYMBOL_EXPORT struct TM_Action* TM_AddNamedBackgroundAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, int delay, char* name) {
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
	action->delay = delay / 1000.0;
	action->id = ++timeline->lastid;
	action->active = true;
	action->started = false;
	action->timeline = timeline;
	PrintConsole(timeline->game, "Timeline Manager[%s]: background: init action with delay %d ms (%d - %s)", timeline->name, delay, action->id, action->name);
	action->state = TM_ACTIONSTATE_INIT;
	(*action->function)(timeline->game, timeline->data, action);
	return action;
}

/*! \brief Predefined action used by TM_AddQueuedBackgroundAction */
static TM_ACTION(TM_RunInBackground) {
	int* delay = TM_Arg(1);
	char* name = TM_Arg(2);
	struct TM_Arguments* arguments = TM_Arg(3);
	bool* used = TM_Arg(4);
	if (action->state == TM_ACTIONSTATE_START) {
		TM_AddNamedBackgroundAction(action->timeline, TM_Arg(0), arguments, *delay, name);
		*used = true;
	}
	if (action->state == TM_ACTIONSTATE_DESTROY) {
		free(name);
		free(delay);
		if (!(*used)) {
			DestroyArgs(arguments);
		}
		free(used);
	}
	return true;
}

SYMBOL_EXPORT struct TM_Action* TM_AddQueuedNamedBackgroundAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, int delay, char* name) {
	TM_WrapArg(int, del, delay);
	TM_WrapArg(bool, used, false);
	struct TM_Arguments* arguments = TM_Args(func, del, strdup(name), args, used);
	return TM_AddAction(timeline, TM_RunInBackground, arguments);
}

SYMBOL_EXPORT void TM_AddDelay(struct Timeline* timeline, int delay) {
	struct TM_Action* tmp = TM_AddNamedAction(timeline, NULL, NULL, "TM_Delay");
	PrintConsole(timeline->game, "Timeline Manager[%s]: queue: adding delay %d ms (%d)", timeline->name, delay, tmp->id);
	tmp->delay = delay / 1000.0;
}

SYMBOL_EXPORT void TM_CleanQueue(struct Timeline* timeline) {
	PrintConsole(timeline->game, "Timeline Manager[%s]: cleaning queue", timeline->name);
	struct TM_Action *tmp, *pom = timeline->queue;
	while (pom != NULL) {
		if (*pom->function) {
			pom->state = TM_ACTIONSTATE_DESTROY;
			(*pom->function)(timeline->game, timeline->data, pom);
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
			pom->state = TM_ACTIONSTATE_DESTROY;
			(*pom->function)(timeline->game, timeline->data, pom);
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

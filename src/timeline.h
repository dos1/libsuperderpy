/*! \file timeline.h
 *  \brief Timeline Manager framework.
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

#ifndef LIBSUPERDERPY_TIMELINE_H
#define LIBSUPERDERPY_TIMELINE_H

#include "libsuperderpy.h"

struct TM_Action;
typedef bool TM_ActionCallback(struct Game*, struct GamestateResources*, struct TM_Action*);
#define TM_NUMARGS(...) (sizeof((void*[]){__VA_ARGS__}) / sizeof(void*))

/*! \brief State of the TM_Action. */
enum TM_ActionState {
	TM_ACTIONSTATE_INIT,
	TM_ACTIONSTATE_START,
	TM_ACTIONSTATE_RUNNING,
	TM_ACTIONSTATE_DESTROY
};

/*! \brief Timeline structure. */
struct Timeline {
	struct TM_Action* queue; /*!< Main timeline queue. */
	struct TM_Action* background; /*!< Background queue. */
	char* name; /*!< Name of the timeline. */
	unsigned int lastid; /*!< Last ID given to timeline action. */
	struct Game* game; /*!< Reference to the game object. */
	struct GamestateResources* data; /*!< User data pointer for use in actions. */
};

/*! \brief Arguments for TM_Action. */
struct TM_Arguments {
	void* value; /*!< Value of argument. */
	struct TM_Arguments* next; /*!< Pointer to next argument. */
};

/*! \brief Timeline action. */
struct TM_Action {
	TM_ActionCallback* function; /*!< Function callback of the action. */
	struct TM_Arguments* arguments; /*!< Arguments of the action. */
	bool active; /*!< Whether this action is being processed by the queue right now. */
	bool started; /*!< If false, then the action is waiting for its delay to finish. */
	double delay; /*!< Number of miliseconds to delay before action is started. */
	double delta; /*!< Number of miliseconds since the last TM_Process invocation. */
	unsigned int id; /*!< ID of the action. */
	char* name; /*!< "User friendly" name of the action. */
	struct Timeline* timeline; /*!< A pointer to the timeline where this action is used. */
	enum TM_ActionState state; /*!< Current state of the action. */
	struct TM_Action* next; /*!< Pointer to next action in queue. */
};

/*! \brief Init timeline. */
struct Timeline* TM_Init(struct Game* game, struct GamestateResources* data, char* name);

/*! \brief Process current timeline actions. */
void TM_Process(struct Timeline*, double delta);

/*! \brief Add new action to main queue, with specified name. */
struct TM_Action* TM_AddNamedAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, char* name);

/*! \brief Add new action to main queue, with specified name, placed after specified action. */
struct TM_Action* TM_AddNamedActionAfter(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, struct TM_Action* after, char* name);

/*! \brief Add new action to background queue, with specified name. */
struct TM_Action* TM_AddNamedBackgroundAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, int delay, char* name);

/*! \brief Add new action to main queue, which adds specified action into background queue, with specified name. */
struct TM_Action* TM_AddQueuedNamedBackgroundAction(struct Timeline* timeline, TM_ActionCallback* func, struct TM_Arguments* args, int delay, char* name);

/*! \brief Add new action to main queue. */
#define TM_AddAction(timeline, func, args) TM_AddNamedAction(timeline, func, args, #func)

/*! \brief Add new action to main queue placed after specified action.. */
#define TM_AddActionAfter(timeline, func, args, after) TM_AddNamedAction(timeline, func, args, after, #func)

/*! \brief Add new action to background queue. */
#define TM_AddBackgroundAction(timeline, func, args, delay) TM_AddNamedBackgroundAction(timeline, func, args, delay, #func)

/*! \brief Add new action to main queue, which adds specified action into background queue. */
#define TM_AddQueuedBackgroundAction(timeline, func, args, delay) TM_AddQueuedNamedBackgroundAction(timeline, func, args, delay, #func)

/*! \brief Add delay to main queue. */
void TM_AddDelay(struct Timeline*, int delay);

/*! \brief Remove all actions from main queue. */
void TM_CleanQueue(struct Timeline*);

/*! \brief Remove all actions from background queue. */
void TM_CleanBackgroundQueue(struct Timeline*);

/*! \brief Destroy given timeline. */
void TM_Destroy(struct Timeline*);

/*! \brief Add data to TM_Arguments queue (or create if NULL). */
struct TM_Arguments* TM_AddToArgs(struct TM_Arguments* args, int num, ...);

/*! \brief Get nth argument from TM_Arguments queue (counted from 0). */
void* TM_GetArg(struct TM_Arguments* args, int num);

/*! \brief Skip delay of the first action in the queue */
void TM_SkipDelay(struct Timeline*);

/*! \brief Checks if the main queue is empty */
bool TM_IsEmpty(struct Timeline* timeline);

/*! \brief Checks if the background queue is empty */
bool TM_IsBackgroundEmpty(struct Timeline* timeline);

/*! \brief Allocates memory and sets given value. */
#define TM_WrapArg(type, result, val)  \
	type* result = malloc(sizeof(type)); \
	*result = val;

/*! \brief Indicates that the action handles only TM_ACTIONSTATE_RUNNING state. */
#define TM_RunningOnly \
	if (action->state != TM_ACTIONSTATE_RUNNING) return false;

/*! \brief Shorthand for creating list of arguments for action. */
#define TM_Args(...) TM_AddToArgs(NULL, TM_NUMARGS(__VA_ARGS__), __VA_ARGS__)

/*! \brief Shorthand for accessing the nth argument of current action. */
#define TM_Arg(n) TM_GetArg(action->arguments, n)

/*! \brief Macro for easy timeline action definition. */
#define TM_ACTION(name) bool name(struct Game* game, struct GamestateResources* data, struct TM_Action* action)

#endif /* LIBSUPERDERPY_TIMELINE_H */

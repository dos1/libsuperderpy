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

#define TM_WrapArg(type, result, val) type* result = malloc(sizeof(type)); *result = val;

/*! \brief State of the TM_Action. */
enum TM_ActionState {
	TM_ACTIONSTATE_INIT,
	TM_ACTIONSTATE_START,
	TM_ACTIONSTATE_RUNNING,
	TM_ACTIONSTATE_DRAW,
	TM_ACTIONSTATE_DESTROY,
	TM_ACTIONSTATE_PAUSE,
	TM_ACTIONSTATE_RESUME
};

/*! \brief Timeline structure. */
struct Timeline {
		struct TM_Action *queue; /*!< Main timeline queue. */
		struct TM_Action *background; /*!< Background queue. */
		char* name; /*!< Name of the timeline. */
		unsigned int lastid; /*!< Last ID given to timeline action. */
		struct Game* game; /*!< Reference to the game object. */
};

/*! \brief Arguments for TM_Action. */
struct TM_Arguments {
		void *value; /*!< Value of argument. */
		struct TM_Arguments *next; /*!< Pointer to next argument. */
};

/*! \brief Timeline action. */
struct TM_Action {
		bool (*function)(struct Game*, struct TM_Action*, enum TM_ActionState); /*!< Function callback of the action. */
		struct TM_Arguments *arguments; /*!< Arguments of the action. */
		ALLEGRO_TIMER *timer; /*!< Delay timer. */
		bool active; /*!< If false, then this action is waiting for it's delay to finish. */
		int delay; /*!< Number of miliseconds to delay before action is started. */
		struct TM_Action *next; /*!< Pointer to next action in queue. */
		unsigned int id; /*!< ID of the action. */
		char* name; /*!< "User friendly" name of the action. */
};

/*! \brief Init timeline. */
struct Timeline* TM_Init(struct Game* game, char* name);
/*! \brief Process current timeline actions. */
void TM_Process(struct Timeline*);
/*! \brief Ask current timeline actions to draw. */
void TM_Draw(struct Timeline*);
/*! \brief Pauses the timeline. */
void TM_Pause(struct Timeline*);
/*! \brief Resumes the timeline. */
void TM_Resume(struct Timeline*);
/*! \brief Handle timer events. */
void TM_HandleEvent(struct Timeline*, ALLEGRO_EVENT *ev);
/*! \brief Add new action to main queue. */
struct TM_Action* TM_AddAction(struct Timeline*, bool (*func)(struct Game*, struct TM_Action*, enum TM_ActionState), struct TM_Arguments* args, char* name);
/*! \brief Add new action to background queue. */
struct TM_Action* TM_AddBackgroundAction(struct Timeline*, bool (*func)(struct Game*, struct TM_Action*, enum TM_ActionState), struct TM_Arguments* args, int delay, char* name);
/*! \brief Add new action to main queue, which adds specified action into background queue. */
struct TM_Action* TM_AddQueuedBackgroundAction(struct Timeline*, bool (*func)(struct Game*, struct TM_Action*, enum TM_ActionState), struct TM_Arguments* args, int delay, char* name);
/*! \brief Add delay to main queue. */
void TM_AddDelay(struct Timeline*, int delay);
/*! \brief Remove all actions from main queue. */
void TM_CleanQueue(struct Timeline*);
/*! \brief Remove all actions from background queue. */
void TM_CleanBackgroundQueue(struct Timeline*);
/*! \brief Destroy timeline. */
void TM_Destroy(struct Timeline*);
/*! \brief Add data to TM_Arguments queue. */
struct TM_Arguments* TM_AddToArgs(struct TM_Arguments* args, int num, ...);
/*! \brief Get nth argument from TM_Arguments queue (counted from 0). */
void* TM_GetArg(struct TM_Arguments *args, int num);
/*! \brief Destroy TM_Arguments queue. */
void TM_DestroyArgs(struct TM_Arguments* args);
/*! \brief Skip delay if it's currently the first action in the queue */
void TM_SkipDelay(struct Timeline*);
/*! \brief Checks if the main queue is empty */
bool TM_IsEmpty(struct Timeline* timeline);
/*! \brief Checks if the background queue is empty */
bool TM_IsBackgroundEmpty(struct Timeline* timeline);

#endif

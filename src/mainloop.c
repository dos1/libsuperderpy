/*! \file mainloop.c
 *  \brief Mainloop handling.
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
 *
 * Also, ponies.
 */

#include "internal.h"
#include "libsuperderpy.h"

static inline void HandleEvent(struct Game* game, ALLEGRO_EVENT* ev) {
	switch (ev->type) {
		case ALLEGRO_EVENT_TIMER:
			if (ev->timer.source == game->_priv.timer) {
				TickGamestates(game);
			}
			break;

		case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
			PauseExecution(game);
			al_acknowledge_drawing_halt(game->display);
			break;

		case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
			al_acknowledge_drawing_resume(game->display);
			ReloadGamestates(game);
			ResumeExecution(game);
			break;

		case ALLEGRO_EVENT_DISPLAY_RESIZE:
			al_acknowledge_resize(game->display);
			SetupViewport(game, game->viewport_config);
			break;

		case ALLEGRO_EVENT_KEY_DOWN:
#ifdef ALLEGRO_ANDROID
			if ((ev->keyboard.keycode == ALLEGRO_KEY_MENU) || (ev->keyboard.keycode == ALLEGRO_KEY_TILDE) || (ev->keyboard.keycode == ALLEGRO_KEY_BACKQUOTE)) {
#else
			if ((ev->keyboard.keycode == ALLEGRO_KEY_TILDE) || (ev->keyboard.keycode == ALLEGRO_KEY_BACKQUOTE)) {
#endif
				game->_priv.showconsole = !game->_priv.showconsole;
				if ((ev->keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) && (game->config.debug)) {
					game->_priv.showtimeline = game->_priv.showconsole;
				}
			}

			if (ev->keyboard.keycode == ALLEGRO_KEY_F12) {
				DrawGamestates(game);
				int flags = al_get_new_bitmap_flags();
				al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
				ALLEGRO_BITMAP* bitmap = al_create_bitmap(al_get_display_width(game->display), al_get_display_height(game->display));
				al_set_new_bitmap_flags(flags);
				ALLEGRO_BITMAP* target = al_get_target_bitmap();
				al_set_target_bitmap(bitmap);
				al_draw_bitmap(al_get_backbuffer(game->display), 0, 0, 0);
				al_set_target_bitmap(target);
				PrintConsole(game, "Screenshot made! Storing...");

				struct ScreenshotThreadData* data = malloc(sizeof(struct ScreenshotThreadData));
				data->game = game;
				data->bitmap = bitmap;
#ifndef LIBSUPERDERPY_SINGLE_THREAD
				al_run_detached_thread(ScreenshotThread, data);
#else
				ScreenshotThread(data);
#endif
			}

			break;
		default:
			break;
	}

#ifdef MAEMO5
	// on Maemo we get mouse events instead of touch ones, so we'll rewrite them by ourselves

	if ((ev->type == ALLEGRO_EVENT_MOUSE_AXES) || (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) || (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)) {
		switch (ev->type) {
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
				ev->type = ALLEGRO_EVENT_TOUCH_BEGIN;
				break;
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
				ev->type = ALLEGRO_EVENT_TOUCH_END;
				break;
			case ALLEGRO_EVENT_MOUSE_AXES:
				ev->type = ALLEGRO_EVENT_TOUCH_MOVE;
				break;
			default:
				break;
		}
		ALLEGRO_DISPLAY* display = ev->mouse.display;
		float dx = ev->mouse.dx;
		float dy = ev->mouse.dy;
		float x = ev->mouse.x;
		float y = ev->mouse.y;
		double timestamp = ev->mouse.timestamp;

		ev->touch.display = display;
		ev->touch.dx = dx;
		ev->touch.dy = dy;
		ev->touch.id = 0;
		ev->touch.primary = true;
		ev->touch.source = (ALLEGRO_TOUCH_INPUT*)al_get_touch_input_event_source();
		ev->touch.timestamp = timestamp;
		ev->touch.x = x;
		ev->touch.y = y;
	}
#endif
}

static inline void HandleDebugEvent(struct Game* game, ALLEGRO_EVENT* ev) {
	switch (ev->type) {
		case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
			if (game->_priv.debug.autopause) {
				PrintConsole(game, "DEBUG: autopause");
				PauseExecution(game);
			}
			break;

		case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
			if (game->_priv.debug.autopause) {
				if (game->_priv.debug.livereload) {
					ReloadCode(game);
				}
				ResumeExecution(game);
			}
			break;

		case ALLEGRO_EVENT_KEY_DOWN:

			switch (ev->keyboard.keycode) {
				case ALLEGRO_KEY_F1:
					if (!game->_priv.paused) {
						PauseExecution(game);
					} else {
						ReloadCode(game);
						ResumeExecution(game);
					}
					break;
				case ALLEGRO_KEY_F9:
					al_set_timer_speed(game->_priv.timer, ALLEGRO_BPS_TO_SECS(60.0));
					game->_priv.showconsole = true;
					PrintConsole(game, "DEBUG: Gameplay speed: 1.00x");
					break;
				case ALLEGRO_KEY_F10: {
					double speed = ALLEGRO_BPS_TO_SECS(al_get_timer_speed(game->_priv.timer)); // inverting
					speed -= 10;
					if (speed < 10) { speed = 10; }
					al_set_timer_speed(game->_priv.timer, ALLEGRO_BPS_TO_SECS(speed));
					game->_priv.showconsole = true;
					PrintConsole(game, "DEBUG: Gameplay speed: %.2fx", speed / 60.0);
				} break;
				case ALLEGRO_KEY_F11: {
					double speed = ALLEGRO_BPS_TO_SECS(al_get_timer_speed(game->_priv.timer)); // inverting
					speed += 10;
					if (speed > 600) { speed = 600; }
					al_set_timer_speed(game->_priv.timer, ALLEGRO_BPS_TO_SECS(speed));
					game->_priv.showconsole = true;
					PrintConsole(game, "DEBUG: Gameplay speed: %.2fx", speed / 60.0);
				} break;
			}

			break;
		default:
			break;
	}
}

static inline bool MainloopTick(struct Game* game) {
	if (game->_priv.paused) {
		return true;
	}

	struct Gamestate* tmp = game->_priv.gamestates;

	game->_priv.loading.toLoad = 0;
	game->_priv.loading.loaded = 0;
	game->loading_progress = 0;

	// TODO: support gamestate dependences/ordering
	while (tmp) {
		if (tmp->pending_stop) {
			PrintConsole(game, "Stopping gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Stop)(game, tmp->data);
			tmp->started = false;
			tmp->pending_stop = false;
			PrintConsole(game, "Gamestate \"%s\" stopped successfully.", tmp->name);
		}

		if (tmp->pending_load) { game->_priv.loading.toLoad++; }
		tmp = tmp->next;
	}

	tmp = game->_priv.gamestates;

	while (tmp) {
		if (tmp->pending_unload) {
			PrintConsole(game, "Unloading gamestate \"%s\"...", tmp->name);
			al_stop_timer(game->_priv.timer);
			tmp->loaded = false;
			tmp->pending_unload = false;
			game->_priv.current_gamestate = tmp;
			(*tmp->api->Gamestate_Unload)(game, tmp->data);
			al_resume_timer(game->_priv.timer);
			PrintConsole(game, "Gamestate \"%s\" unloaded successfully.", tmp->name);
		}
		if (tmp->pending_load) {
			al_stop_timer(game->_priv.timer);
			if (tmp->showLoading) {
				(*game->_priv.loading.gamestate->api->Gamestate_Start)(game, game->_priv.loading.gamestate->data);
			}

			if (!tmp->api) {
				if (!OpenGamestate(game, tmp) || !LinkGamestate(game, tmp)) {
					tmp->pending_load = false;
					tmp->pending_start = false;
					tmp->next = tmp;
					continue;
				}
			}
			if (tmp->api) {
				PrintConsole(game, "Loading gamestate \"%s\"...", tmp->name);
				game->_priv.loading.progress = 0;

				game->_priv.loading.current = tmp;
				game->_priv.current_gamestate = tmp;

				struct GamestateLoadingThreadData data = {.game = game, .gamestate = tmp, .bitmap_flags = al_get_new_bitmap_flags()};
				game->_priv.loading.inProgress = true;
				double time = al_get_time();
				game->_priv.loading.time = time;

				CalculateProgress(game);
#ifndef LIBSUPERDERPY_SINGLE_THREAD
				al_run_detached_thread(GamestateLoadingThread, &data);
				while (game->_priv.loading.inProgress) {
					double delta = al_get_time() - game->_priv.loading.time;
					if (tmp->showLoading) {
						(*game->_priv.loading.gamestate->api->Gamestate_Logic)(game, game->_priv.loading.gamestate->data, delta);
						DrawGamestates(game);
					}
					game->_priv.loading.time += delta;
					game->time += delta; // TODO: ability to disable passing time during loading
					if (game->_priv.texture_sync) {
						al_convert_memory_bitmaps();
						game->_priv.texture_sync = false;
						al_signal_cond(game->_priv.texture_sync_cond);
						game->_priv.loading.time = al_get_time();
					}
					DrawConsole(game);
					al_flip_display();
				}
#else
				GamestateLoadingThread(&data);
				al_convert_memory_bitmaps();
#endif

				al_set_new_bitmap_flags(data.bitmap_flags);

				if (tmp->api->Gamestate_PostLoad) {
					PrintConsole(game, "[%s] Post-loading...", tmp->name);
					tmp->api->Gamestate_PostLoad(game, tmp->data);
				}

				game->_priv.loading.progress++;
				CalculateProgress(game);
				PrintConsole(game, "Gamestate \"%s\" loaded successfully in %f seconds.", tmp->name, al_get_time() - time);
				game->_priv.loading.loaded++;

				tmp->loaded = true;
				tmp->pending_load = false;
			}
			if (tmp->showLoading) {
				(*game->_priv.loading.gamestate->api->Gamestate_Stop)(game, game->_priv.loading.gamestate->data);
			}
			tmp->showLoading = true;
			al_resume_timer(game->_priv.timer);
			game->_priv.timestamp = al_get_time();
		}

		tmp = tmp->next;
	}

	if (game->_priv.loading.loaded) {
		ReloadShaders(game, false);
	}

	bool gameActive = false;
	tmp = game->_priv.gamestates;

	while (tmp) {
		if ((tmp->pending_start) && (tmp->loaded)) {
			PrintConsole(game, "Starting gamestate \"%s\"...", tmp->name);
			al_stop_timer(game->_priv.timer);
			game->_priv.current_gamestate = tmp;
			tmp->started = true;
			tmp->pending_start = false;
			(*tmp->api->Gamestate_Start)(game, tmp->data);
			al_resume_timer(game->_priv.timer);
			game->_priv.timestamp = al_get_time();
			PrintConsole(game, "Gamestate \"%s\" started successfully.", tmp->name);
		}

		if ((tmp->started) || (tmp->pending_start) || (tmp->pending_load)) {
			gameActive = true;
		}
		tmp = tmp->next;
	}

	if (!gameActive) {
		PrintConsole(game, "No gamestates left, exiting...");
		return false;
	}

	al_convert_memory_bitmaps();

	double delta = al_get_time() - game->_priv.timestamp;
	game->_priv.timestamp += delta;
	delta *= ALLEGRO_BPS_TO_SECS(al_get_timer_speed(game->_priv.timer) / (1 / 60.f));
	game->time += delta;

	LogicGamestates(game, delta);
	DrawGamestates(game);

	DrawConsole(game);
	al_flip_display();
	return true;
}

static inline bool MainloopEvents(struct Game* game) {
	do {
		ALLEGRO_EVENT ev;

		if (game->_priv.paused) {
			// there's no frame flipping when paused, so avoid pointless busylooping
			al_wait_for_event(game->_priv.event_queue, &ev);
		} else if (!al_get_next_event(game->_priv.event_queue, &ev)) {
			break;
		}

		if (game->handlers.event) {
			if ((*game->handlers.event)(game, &ev)) {
				continue;
			}
		}

		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			EventGamestates(game, &ev);
			return false;
		}

		HandleEvent(game, &ev);

		if (game->config.debug) {
			HandleDebugEvent(game, &ev);
		}

		EventGamestates(game, &ev);

	} while (!al_is_event_queue_empty(game->_priv.event_queue));
	return true;
}

SYMBOL_EXPORT bool libsuperderpy_mainloop(struct Game* game) {
	ClearGarbage(game);
	return MainloopEvents(game) && MainloopTick(game);
}

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

static inline bool HandleEvent(struct Game* game, ALLEGRO_EVENT* ev) {
	switch (ev->type) {
		case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
			PauseExecution(game);
			al_acknowledge_drawing_halt(game->display);
			break;

		case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
			al_acknowledge_drawing_resume(game->display);
			ReloadGamestates(game);
			ResumeExecution(game);
			break;

		case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
			if (game->config.autopause) {
				PrintConsole(game, "Focus lost, autopausing...");
				PauseExecution(game);
			}
			break;

		case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
			if (game->config.autopause) {
				if (game->config.debug.enabled && game->config.debug.livereload) {
					ReloadCode(game);
				}
				ResumeExecution(game);
			}
			break;

		case ALLEGRO_EVENT_DISPLAY_RESIZE:
			PrintConsole(game, "Resize event: %dx%d", ev->display.width, ev->display.height);

#ifdef LIBSUPERDERPY_IMGUI
			ImGui_ImplAllegro5_InvalidateDeviceObjects();
#endif
			al_acknowledge_resize(game->display);
#ifdef LIBSUPERDERPY_IMGUI
			ImGui_ImplAllegro5_CreateDeviceObjects();
#endif

			// SetupViewport can be expensive, so don't do it when the resize event is already outdated or doesn't change anything
			if (((ev->display.width != game->_priv.window_width) || (ev->display.height != game->_priv.window_height)) &&
				(ev->display.width == al_get_display_width(game->display)) && (ev->display.height == al_get_display_height(game->display))) {
				SetupViewport(game);
			}

			break;
		case ALLEGRO_EVENT_KEY_DOWN:
#ifdef ALLEGRO_ANDROID
			if ((ev->keyboard.keycode == ALLEGRO_KEY_MENU) || (ev->keyboard.keycode == ALLEGRO_KEY_TILDE) || (ev->keyboard.keycode == ALLEGRO_KEY_BACKQUOTE)) {
#else
			if ((ev->keyboard.keycode == ALLEGRO_KEY_TILDE) || (ev->keyboard.keycode == ALLEGRO_KEY_BACKQUOTE)) {
#endif
				game->_priv.showconsole = !game->_priv.showconsole;
				if ((ev->keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) && (game->config.debug.enabled)) {
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
		case ALLEGRO_EVENT_JOYSTICK_CONFIGURATION:
			al_reconfigure_joysticks();
			break;
		case ALLEGRO_EVENT_JOYSTICK_AXIS:
#ifdef ALLEGRO_WITH_XWINDOWS
			// XBox pads on GNU/Linux have messed up stick/axis ordering
			if (ev->joystick.axis == 0) {
				if (ev->joystick.stick == 2) {
					ev->joystick.stick = 1;
				} else if (ev->joystick.stick == 1) {
					ev->joystick.stick = 2;
				}
			}
			if (ev->joystick.stick == 1) {
				ev->joystick.axis = 1 - ev->joystick.axis;
			}
#endif
			break;
#ifdef __SWITCH__
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
			// ignore button events coming form analog movement
			if (ev->joystick.button > 15) {
				return true;
			}
			break;
#endif
		default:
			break;
	}

#ifdef LIBSUPERDERPY_EMULATE_TOUCH
	// on some platforms (like Maemo or Pocket C.H.I.P) we get mouse events instead of touch ones, so we'll rewrite them by ourselves

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
	return false;
}

static inline void HandleDebugEvent(struct Game* game, ALLEGRO_EVENT* ev) {
	switch (ev->type) {
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
					game->_priv.speed = ALLEGRO_BPS_TO_SECS(60.0);
					game->_priv.showconsole = true;
					PrintConsole(game, "DEBUG: Gameplay speed: 1.00x");
					break;
				case ALLEGRO_KEY_F10: {
					double speed = ALLEGRO_BPS_TO_SECS(game->_priv.speed); // inverting
					speed -= 10;
					if (speed < 10) { speed = 10; }
					game->_priv.speed = ALLEGRO_BPS_TO_SECS(speed);
					game->_priv.showconsole = true;
					PrintConsole(game, "DEBUG: Gameplay speed: %.2fx", speed / 60.0);
				} break;
				case ALLEGRO_KEY_F11: {
					double speed = ALLEGRO_BPS_TO_SECS(game->_priv.speed); // inverting
					speed += 10;
					if (speed > 600) { speed = 600; }
					game->_priv.speed = ALLEGRO_BPS_TO_SECS(speed);
					game->_priv.showconsole = true;
					PrintConsole(game, "DEBUG: Gameplay speed: %.2fx", speed / 60.0);
				} break;
			}

			break;
		default:
			break;
	}
}

static inline bool MainloopEvents(struct Game* game) {
	do {
		ALLEGRO_EVENT ev;

		if (game->_priv.paused && !IS_EMSCRIPTEN) {
			// there's no frame flipping when paused, so avoid pointless busylooping
			al_wait_for_event(game->_priv.event_queue, &ev);
		} else if (!al_get_next_event(game->_priv.event_queue, &ev)) {
			break;
		}

#ifdef LIBSUPERDERPY_IMGUI
		ImGui_ImplAllegro5_ProcessEvent(&ev);
		switch (ev.type) {
			case ALLEGRO_EVENT_KEY_CHAR:
			case ALLEGRO_EVENT_KEY_DOWN:
			case ALLEGRO_EVENT_KEY_UP:
				if (igGetIO()->WantCaptureKeyboard) {
					continue;
				}
				break;
			case ALLEGRO_EVENT_MOUSE_AXES:
			case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			case ALLEGRO_EVENT_TOUCH_BEGIN:
			case ALLEGRO_EVENT_TOUCH_CANCEL:
			case ALLEGRO_EVENT_TOUCH_END:
			case ALLEGRO_EVENT_TOUCH_MOVE:
				if (igGetIO()->WantCaptureMouse) {
					continue;
				}
				break;
			default:
				break;
		}
#endif

		if (game->_priv.params.handlers.event) {
			if ((*game->_priv.params.handlers.event)(game, &ev)) {
				continue;
			}
		}

		if (HandleEvent(game, &ev)) {
			continue;
		}

		if (game->config.debug.enabled) {
			HandleDebugEvent(game, &ev);
		}

		EventGamestates(game, &ev);

		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			return false;
		}

		if (ALLEGRO_EVENT_TYPE_IS_USER(ev.type)) {
			al_unref_user_event(&ev.user);
		}

	} while (!al_is_event_queue_empty(game->_priv.event_queue));
	return true;
}

static inline bool MainloopTick(struct Game* game) {
	if (game->_priv.paused) {
		return true;
	}

	struct Gamestate* tmp = game->_priv.gamestates;

#ifdef __EMSCRIPTEN__
	emscripten_pause_main_loop();
#endif

	game->_priv.loading.to_load = 0;
	game->_priv.loading.loaded = 0;
	game->_priv.loading.lock = true;
	game->loading.progress = 0;

	// TODO: support gamestate dependences/ordering
	while (tmp) {
		if (tmp->pending_stop) {
			PrintConsole(game, "Stopping gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			(*tmp->api->stop)(game, tmp->data);
			tmp->started = false;
			tmp->pending_stop = false;
			al_destroy_bitmap(tmp->fb);
			tmp->fb = NULL;
			PrintConsole(game, "Gamestate \"%s\" stopped successfully.", tmp->name);
		}

		if (tmp->pending_load) { game->_priv.loading.to_load++; }
		tmp = tmp->next;
	}

	tmp = game->_priv.gamestates;

	while (tmp) {
		if (tmp->pending_unload) {
#ifdef __EMSCRIPTEN__
			al_detach_voice(game->audio.v);
#endif
			PrintConsole(game, "Unloading gamestate \"%s\"...", tmp->name);
			tmp->loaded = false;
			tmp->pending_unload = false;
			game->_priv.current_gamestate = tmp;
			(*tmp->api->unload)(game, tmp->data);
			PrintConsole(game, "Gamestate \"%s\" unloaded successfully.", tmp->name);
#ifdef __EMSCRIPTEN__
			al_attach_mixer_to_voice(game->audio.mixer, game->audio.v);
#endif
		}
		if (tmp->pending_load) {
#ifdef __EMSCRIPTEN__
			al_detach_voice(game->audio.v);
#endif
			if (tmp->show_loading && game->_priv.loading.gamestate->open) {
				(*game->_priv.loading.gamestate->api->start)(game, game->_priv.loading.gamestate->data);
			}

			if (!tmp->open) {
				if (!OpenGamestate(game, tmp, true) || !LinkGamestate(game, tmp)) {
					tmp->pending_load = false;
					tmp->pending_start = false;
					continue;
				}
			}
			if (tmp->api) {
				PrintConsole(game, "Loading gamestate \"%s\"...", tmp->name);
				game->_priv.loading.progress = 0;

				game->_priv.loading.current = tmp;
				game->_priv.current_gamestate = tmp;

				struct GamestateLoadingThreadData data = {.game = game, .gamestate = tmp, .bitmap_flags = al_get_new_bitmap_flags()};
				game->_priv.loading.in_progress = true;
				double time = al_get_time();
				game->_priv.loading.time = time;

				CalculateProgress(game);
				if (tmp->show_loading) {
					game->loading.shown = true;
					DrawGamestates(game);
					DrawConsole(game);
					al_flip_display();
#ifdef __EMSCRIPTEN__
					emscripten_sleep(0);
#endif
				}
#ifndef LIBSUPERDERPY_SINGLE_THREAD
				al_run_detached_thread(GamestateLoadingThread, &data);
				while (game->_priv.loading.in_progress) {
					double delta = al_get_time() - game->_priv.loading.time;
					game->time += delta; // TODO: ability to disable passing time during loading
					game->_priv.loading.time += delta;
					if (game->loading.shown && game->_priv.loading.gamestate->open) {
						(*game->_priv.loading.gamestate->api->logic)(game, game->_priv.loading.gamestate->data, delta);
					}
					DrawGamestates(game);
					if (game->_priv.texture_sync) {
						al_convert_memory_bitmaps();
						game->_priv.texture_sync = false;
						al_signal_cond(game->_priv.texture_sync_cond);
						game->_priv.loading.time = al_get_time(); // TODO: rethink time management during loading
					}
					DrawConsole(game);
					al_flip_display();

					if (game->_priv.bsod_sync) {
						al_set_target_bitmap(NULL);
						game->_priv.bsod_sync = false;
						al_signal_cond(game->_priv.bsod_cond);
					}

					al_lock_mutex(game->_priv.bsod_mutex);
					while (game->_priv.in_bsod) {
						al_wait_cond(game->_priv.bsod_cond, game->_priv.bsod_mutex);
					}
					al_unlock_mutex(game->_priv.bsod_mutex);
				}
#else
				GamestateLoadingThread(&data);
				DrawGamestates(game);
				DrawConsole(game);
				al_flip_display();
#ifdef __EMSCRIPTEN__
				emscripten_sleep(0);
#endif
#endif
				al_convert_memory_bitmaps();

				al_set_new_bitmap_flags(data.bitmap_flags);

				ReloadShaders(game, false);

				if (tmp->api->post_load) {
					PrintConsole(game, "[%s] Post-loading...", tmp->name);
					tmp->api->post_load(game, tmp->data);
				}

				game->_priv.loading.progress++;
				CalculateProgress(game);
				PrintConsole(game, "Gamestate \"%s\" loaded successfully in %f seconds.", tmp->name, al_get_time() - time);
				game->_priv.loading.loaded++;

				DrawGamestates(game);
				DrawConsole(game);
				al_flip_display();
#ifdef __EMSCRIPTEN__
				emscripten_sleep(0);
#endif

				tmp->loaded = true;
				tmp->pending_load = false;
			}
			if (tmp->show_loading && game->_priv.loading.gamestate->open) {
				(*game->_priv.loading.gamestate->api->stop)(game, game->_priv.loading.gamestate->data);
			}
			tmp->show_loading = true;
			game->loading.shown = false;
			game->_priv.timestamp = al_get_time();
#ifdef __EMSCRIPTEN__
			al_attach_mixer_to_voice(game->audio.mixer, game->audio.v);
#endif
		}

		tmp = tmp->next;
	}

	if (game->_priv.loading.loaded) {
		MainloopEvents(game); // consume queued events
#ifdef __EMSCRIPTEN__
		DrawGamestates(game);
		DrawConsole(game);
		al_flip_display();
		emscripten_sleep(0);
#endif
	}

	bool gameActive = false;
	tmp = game->_priv.gamestates;

	while (tmp) {
		if ((tmp->pending_start) && (tmp->loaded)) {
			PrintConsole(game, "Starting gamestate \"%s\"...", tmp->name);
			game->_priv.current_gamestate = tmp;
			tmp->started = true;
			tmp->pending_start = false;
			if (game->_priv.params.handlers.compositor) {
				tmp->fb = CreateNotPreservedBitmap(game->clip_rect.w, game->clip_rect.h);
			} else {
				tmp->fb = al_create_sub_bitmap(al_get_backbuffer(game->display), game->clip_rect.x, game->clip_rect.y, game->clip_rect.w, game->clip_rect.h);
			}

			(*tmp->api->start)(game, tmp->data);
			game->_priv.timestamp = al_get_time();
			PrintConsole(game, "Gamestate \"%s\" started successfully.", tmp->name);
		}

		if ((tmp->started) || (tmp->pending_start) || (tmp->pending_load)) {
			gameActive = true;
		}
		tmp = tmp->next;
	}

	game->_priv.loading.lock = false;
#ifdef __EMSCRIPTEN__
	emscripten_resume_main_loop();
#endif

	if (!gameActive) {
		PrintConsole(game, "No gamestates left, exiting...");
		ClearScreen(game);
		DrawConsole(game);
		al_flip_display();
		return false;
	}

	al_convert_memory_bitmaps();

	double delta = al_get_time() - game->_priv.timestamp;
	game->_priv.timestamp += delta;
	delta *= ALLEGRO_BPS_TO_SECS(game->_priv.speed / (1 / 60.0));

#ifdef LIBSUPERDERPY_IMGUI
	ImGui_ImplAllegro5_NewFrame();
	igNewFrame();
#endif

	LogicGamestates(game, delta);
	DrawGamestates(game);

#ifdef LIBSUPERDERPY_IMGUI
	igRender();
	ImGui_ImplAllegro5_RenderDrawData(igGetDrawData());
#endif

	DrawConsole(game);

	al_flip_display();
	return true;
}

SYMBOL_EXPORT bool libsuperderpy_mainloop(struct Game* game) {
	if (game->_priv.loading.lock) {
		return true;
	}
	ClearGarbage(game);
	return MainloopEvents(game) && MainloopTick(game) && MainloopEvents(game);
}

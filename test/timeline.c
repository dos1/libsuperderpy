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

#include "tests.h"

// -----------------------------------------

#define EPSILON 0.001

#define HandleTest(index)                                                      \
	{                                                                            \
		float* del = TM_Arg(index);                                                \
		if (del) {                                                                 \
			if (*del) {                                                              \
				PrintConsole(game, "Eating %f delta (from %f).", *del, action->delta); \
				action->delta -= *del;                                                 \
			}                                                                        \
			if (action->delta < 0 || !*del) {                                        \
				action->delta = 0;                                                     \
			}                                                                        \
			PrintConsole(game, "Delta left: %f", action->delta);                     \
		}                                                                          \
		function_called();                                                         \
	}                                                                            \
	(void)0

static TM_ACTION(DoNothing) {
	TM_RunningOnly;
	PrintConsole(game, "Doing nothing.");
	HandleTest(0);
	return true;
}

static void ActionInitialized(void) {
	function_called();
}

static void ActionDestroyed(void) {
	function_called();
}

static void ActionStarted(void) {
	function_called();
}

static void ActionStopped(void) {
	function_called();
}

static TM_ACTION(SetState) {
	bool* quit = TM_Arg(0);

	int* val = TM_Arg(1);
	*val = action->state;

	SUPPRESS_WARNING("-Wcovered-switch-default")

	switch (action->state) {
		case TM_ACTIONSTATE_INIT:
			PrintConsole(game, "Init");
			ActionInitialized();
			break;
		case TM_ACTIONSTATE_START:
			PrintConsole(game, "Started");
			ActionStarted();
			break;
		case TM_ACTIONSTATE_STOP:
			PrintConsole(game, "Stopped");
			ActionStopped();
			break;
		case TM_ACTIONSTATE_RUNNING:
			PrintConsole(game, "Running");
			HandleTest(2);
			return *quit;
		case TM_ACTIONSTATE_DESTROY:
			PrintConsole(game, "Destroyed");
			ActionDestroyed();
			break;
		default:
			PrintConsole(game, "Unknown!");
			assert_true(false);
			break;
	}

	SUPPRESS_END

	return true;
}

static TM_ACTION(AssertDelta) {
	TM_RunningOnly;
	float* delta = TM_Arg(0);
	assert_float_equal(action->delta, *delta, EPSILON);
	PrintConsole(game, "Delta given: %f", action->delta);
	HandleTest(1);
	return true;
}

static TM_ACTION(Loop) {
	TM_RunningOnly;
	float* duration = TM_Arg(0);
	float* delta = TM_Arg(1);
	float d = action->delta;
	if (delta) {
		d = *delta;
	}
	*duration -= d;
	PrintConsole(game, "Looping (for %f) with duration left: %f", action->delta, *duration);
	HandleTest(2);
	return *duration <= 0.0;
}

// -----------------------------------------

static int timeline_setup(void** state) {
	return engine_setup(state);
}

static int timeline_teardown(void** state) {
	return engine_teardown(state);
}

// -----------------------------------------

static void timeline_action(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	TM_AddAction(timeline, DoNothing, NULL);

	expect_function_call(DoNothing);
	TM_Process(timeline, 1);

	TM_Destroy(timeline);
}

static void timeline_action_lifecycle(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	int val = -1;
	bool quit = true;

	expect_function_call(ActionInitialized);
	TM_AddAction(timeline, SetState, TM_Args(&quit, &val));
	assert_int_equal(val, TM_ACTIONSTATE_INIT);

	expect_function_call(ActionStarted);
	expect_function_call(SetState);
	expect_function_call(ActionStopped);
	expect_function_call(ActionDestroyed);
	TM_Process(timeline, 1);

	assert_int_equal(val, TM_ACTIONSTATE_DESTROY);

	TM_Destroy(timeline);
}

static void timeline_action_lifecycle_repeated(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	int val = -1;
	bool quit = false;

	expect_function_call(ActionInitialized);
	TM_AddAction(timeline, SetState, TM_Args(&quit, &val));
	assert_int_equal(val, TM_ACTIONSTATE_INIT);

	expect_function_call(ActionStarted);
	expect_function_call(SetState);
	TM_Process(timeline, 1);

	assert_int_equal(val, TM_ACTIONSTATE_RUNNING);

	quit = true;
	expect_function_call(SetState);
	expect_function_call(ActionStopped);
	expect_function_call(ActionDestroyed);
	TM_Process(timeline, 1);

	assert_int_equal(val, TM_ACTIONSTATE_DESTROY);

	TM_Destroy(timeline);
}

static void timeline_action_queue(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float delta = 1;

	TM_AddAction(timeline, DoNothing, TM_Args(&delta));
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));

	expect_function_call(DoNothing);
	TM_Process(timeline, delta);

	expect_function_call(DoNothing);
	TM_Process(timeline, delta);

	TM_Destroy(timeline);
}

static void timeline_multiple_actions_in_one_process(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	TM_AddAction(timeline, DoNothing, NULL);
	TM_AddAction(timeline, DoNothing, NULL);

	expect_function_calls(DoNothing, 2);
	TM_Process(timeline, 1);

	TM_Destroy(timeline);
}

static void timeline_multiple_delays_in_one_process(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	TM_AddDelay(timeline, 1);
	TM_AddDelay(timeline, 1);
	TM_AddAction(timeline, DoNothing, NULL);

	expect_function_call(DoNothing);
	TM_Process(timeline, 3);

	TM_Destroy(timeline);
}

static void timeline_delta_less_than_process(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float delta = 0.2;
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));

	expect_function_calls(DoNothing, 2);
	TM_Process(timeline, 2);

	TM_Destroy(timeline);
}

static void timeline_delta_equals_process(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float delta = 1;
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));

	expect_function_call(DoNothing);
	TM_Process(timeline, 1);

	TM_Destroy(timeline);
}

static void timeline_stops_when_delta_eaten(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float delta = 0.2;
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));

	expect_function_calls(DoNothing, 5);
	TM_Process(timeline, 1);

	TM_Destroy(timeline);
}

static void timeline_delay_less_than_process(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float delta = 1, val = 0.4;
	TM_AddDelay(timeline, 0.6);
	TM_AddAction(timeline, AssertDelta, TM_Args(&val, &delta));

	expect_function_call(AssertDelta);
	TM_Process(timeline, 1);

	TM_Destroy(timeline);
}

static void timeline_delay_equals_process(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float delta = 1;
	TM_AddDelay(timeline, 1);
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));

	TM_Process(timeline, 1);

	TM_Destroy(timeline);
}

static void timeline_delay_more_than_process(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float delta = 1;
	TM_AddDelay(timeline, 2);
	TM_AddAction(timeline, DoNothing, TM_Args(&delta));

	TM_Process(timeline, 1);

	TM_Destroy(timeline);
}

static void timeline_delay_eats_delta(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float delta = 2;
	TM_AddDelay(timeline, 1);
	TM_AddAction(timeline, AssertDelta, TM_Args(&delta));

	expect_function_call(AssertDelta);
	TM_Process(timeline, 3);

	TM_Destroy(timeline);
}

static void timeline_delay_with_multiple_processes(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float val = 0.4;
	TM_AddDelay(timeline, 1);
	TM_AddAction(timeline, AssertDelta, TM_Args(&val));

	TM_Process(timeline, 0.4);

	expect_function_call(AssertDelta);
	TM_Process(timeline, 1);

	TM_Destroy(timeline);
}

static void timeline_repeat_when_delta_left(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	int repeats = 8;

	float delta = 1, time = delta * repeats;
	TM_AddAction(timeline, Loop, TM_Args(&time, &delta, &delta));

	expect_function_calls(Loop, repeats);

	TM_Process(timeline, time);

	TM_Destroy(timeline);
}

static void timeline_no_repeating_when_eating_zero_delta(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	float time = 42, delta = 0;
	TM_AddAction(timeline, Loop, TM_Args(&time, &delta));
	TM_AddAction(timeline, DoNothing, NULL);

	expect_function_call(Loop);

	TM_Process(timeline, 69);

	TM_Destroy(timeline);
}

static void timeline_destroyed(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	int val = -1;
	bool quit = false;

	expect_function_call(ActionInitialized);
	TM_AddAction(timeline, SetState, TM_Args(&quit, &val));
	assert_int_equal(val, TM_ACTIONSTATE_INIT);

	expect_function_call(ActionDestroyed);
	TM_Destroy(timeline);
	assert_int_equal(val, TM_ACTIONSTATE_DESTROY);
}

static void timeline_destroyed_while_running(void** state) {
	struct Timeline* timeline = TM_Init(*state, NULL, __func__);

	int val = -1;
	bool quit = false;

	expect_function_call(ActionInitialized);
	TM_AddAction(timeline, SetState, TM_Args(&quit, &val));
	assert_int_equal(val, TM_ACTIONSTATE_INIT);

	expect_function_call(ActionStarted);
	expect_function_call(SetState);
	TM_Process(timeline, 1);

	assert_int_equal(val, TM_ACTIONSTATE_RUNNING);

	expect_function_call(ActionStopped);
	expect_function_call(ActionDestroyed);
	TM_Destroy(timeline);
	assert_int_equal(val, TM_ACTIONSTATE_DESTROY);
}

int test_timeline(void) {
	const struct CMUnitTest timeline_tests[] = {
		cmocka_unit_test(timeline_action),
		cmocka_unit_test(timeline_action_lifecycle),
		cmocka_unit_test(timeline_action_lifecycle_repeated),
		cmocka_unit_test(timeline_action_queue),
		cmocka_unit_test(timeline_multiple_actions_in_one_process),
		cmocka_unit_test(timeline_multiple_delays_in_one_process),
		cmocka_unit_test(timeline_delta_less_than_process),
		cmocka_unit_test(timeline_delta_equals_process),
		cmocka_unit_test(timeline_delay_less_than_process),
		cmocka_unit_test(timeline_delay_equals_process),
		cmocka_unit_test(timeline_stops_when_delta_eaten),
		cmocka_unit_test(timeline_delay_more_than_process),
		cmocka_unit_test(timeline_delay_eats_delta),
		cmocka_unit_test(timeline_repeat_when_delta_left),
		cmocka_unit_test(timeline_no_repeating_when_eating_zero_delta),
		cmocka_unit_test(timeline_delay_with_multiple_processes),
		cmocka_unit_test(timeline_destroyed),
		cmocka_unit_test(timeline_destroyed_while_running),
		// TODO: delayed actions, background queue
	};
	return cmocka_run_group_tests(timeline_tests, timeline_setup, timeline_teardown);
}

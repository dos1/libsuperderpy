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

#include "tests.h"
#include <stdlib.h>
#include <sys/stat.h>

// -----------------------------------------

static char* anim = "[animation]\nduration=100\nframes=3\nrepeats=0\nfile=dummy\n";
static char* tmpdir = NULL;

static int character_setup(void** state) {
	char dir[255] = "libsuperderpy-XXXXXX";
	if (!mkdtemp(dir)) {
		exit(1);
	}
	if (chdir(dir)) {
		exit(2);
	}
	mkdir("data", 0700);
	mkdir("data/sprites", 0700);
	mkdir("data/sprites/test", 0700);
	FILE* file = fopen("data/sprites/test/animation.ini", "we");
	fputs(anim, file);
	fclose(file);
	tmpdir = strdup(dir);
	return engine_setup(state);
}

static int character_teardown(void** state) {
	unlink("data/sprites/test/animation.ini");
	rmdir("data/sprites/test");
	rmdir("data/sprites");
	rmdir("data");
	if (chdir("..")) {
		exit(3);
	}
	rmdir(tmpdir);
	free(tmpdir);
	return engine_teardown(state);
}

// -----------------------------------------

static void character_spritesheet_stops(void** state) {
	struct Game* game = *state;
	struct Character* character = CreateCharacter(game, "test");
	RegisterSpritesheet(game, character, "animation");
	SelectSpritesheet(game, character, "animation");

	assert_int_equal(character->pos, 0);
	AnimateCharacter(game, character, 0.1, 1.0);
	assert_int_equal(character->pos, 1);
	AnimateCharacter(game, character, 0.1, 1.0);
	assert_int_equal(character->pos, 2);
	AnimateCharacter(game, character, 0.1, 1.0);
	assert_int_equal(character->pos, 2);
	AnimateCharacter(game, character, 0.1, 1.0);
	assert_int_equal(character->pos, 2);
}

static void character_spritesheet_reversed_stops(void** state) {
	struct Game* game = *state;
	struct Character* character = CreateCharacter(game, "test");
	RegisterSpritesheet(game, character, "animation");
	SelectSpritesheet(game, character, "-animation");

	assert_int_equal(character->pos, 2);
	AnimateCharacter(game, character, 0.1, 1.0);
	assert_int_equal(character->pos, 1);
	AnimateCharacter(game, character, 0.1, 1.0);
	assert_int_equal(character->pos, 0);
	AnimateCharacter(game, character, 0.1, 1.0);
	assert_int_equal(character->pos, 0);
	AnimateCharacter(game, character, 0.1, 1.0);
	assert_int_equal(character->pos, 0);
}

int test_character(void) {
	const struct CMUnitTest character_tests[] = {
		cmocka_unit_test(character_spritesheet_stops),
		cmocka_unit_test(character_spritesheet_reversed_stops),
	};
	return cmocka_run_group_tests(character_tests, character_setup, character_teardown);
}

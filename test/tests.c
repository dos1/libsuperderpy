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

static struct Game* game = NULL;

int engine_setup(void** state) {
	*state = game;
	return 0;
}

int engine_teardown(void** state) {
	return 0;
}

int main(int argc, char** argv) {
	al_set_app_name("libsuperderpy");
	char* args[2] = {"", "--debug"};
	game = libsuperderpy_init(2, args, "test", (struct Params){});
	libsuperderpy_start(game);
	int ret = test_timeline() || test_character();
	libsuperderpy_destroy(game);
	return ret;
}

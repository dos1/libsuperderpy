/*! \file shader.c
 *  \brief GPU shaders abstraction.
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
#include "shader.h"
#include "internal.h"
#include <allegro5/allegro.h>
#include <math.h>

ALLEGRO_SHADER* CreateShader(struct Game* game, const char* vertex, const char* fragment) {
	const char* log;

	PrintConsole(game, "Creating shader V:%s F:%s...", vertex, fragment);

	ALLEGRO_SHADER* shader = al_create_shader(ALLEGRO_SHADER_GLSL);
	if (!al_attach_shader_source_file(shader, ALLEGRO_VERTEX_SHADER, vertex)) {
		log = al_get_shader_log(shader);
		if (log) {
			PrintConsole(game, "%s", log);
		}
	}
	if (!al_attach_shader_source_file(shader, ALLEGRO_PIXEL_SHADER, fragment)) {
		log = al_get_shader_log(shader);
		if (log) {
			PrintConsole(game, "%s", log);
		}
	}
	if (!al_build_shader(shader)) {
		log = al_get_shader_log(shader);
		if (log) {
			PrintConsole(game, "%s", log);
		}
	}

	game->_priv.shaders = AddToList(game->_priv.shaders, shader);

	PrintConsole(game, "Shader compiled successfully.");

	return shader;
}

void DestroyShader(struct Game* game, ALLEGRO_SHADER* shader) {
	RemoveFromList(&game->_priv.shaders, shader, NULL);
	al_destroy_shader(shader);
}

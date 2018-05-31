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

static bool AttachToShader(struct Game* game, ALLEGRO_SHADER* shader, ALLEGRO_SHADER_TYPE type, const char* filename) {
	bool ret;
	if (filename) {
		ret = al_attach_shader_source_file(shader, type, filename);
	} else {
		ret = al_attach_shader_source(shader, type, al_get_default_shader_source(al_get_shader_platform(shader), type));
	}
	if (!ret) {
		const char* log;
		log = al_get_shader_log(shader);
		if (log) {
			FatalError(game, false, "%s", log);
		}
	}
	return ret;
}

struct ShaderListItem {
	ALLEGRO_SHADER* shader;
	char* vertex;
	char* fragment;
	bool loaded;
};

SYMBOL_EXPORT ALLEGRO_SHADER* CreateShader(struct Game* game, const char* vertex, const char* fragment) {
	PrintConsole(game, "Creating shader V:%s F:%s...", vertex, fragment);

	ALLEGRO_SHADER* shader = al_create_shader(ALLEGRO_SHADER_GLSL);

	struct ShaderListItem* item = malloc(sizeof(struct ShaderListItem));
	item->shader = shader;
	item->vertex = vertex ? strdup(vertex) : NULL;
	item->fragment = fragment ? strdup(fragment) : NULL;
	item->loaded = false;

	game->_priv.shaders = AddToList(game->_priv.shaders, item);

	return shader;
}

static bool ShaderIdentity(struct libsuperderpy_list* item, void* shader) {
	return ((struct ShaderListItem*)item->data)->shader == shader;
}

SYMBOL_EXPORT void DestroyShader(struct Game* game, ALLEGRO_SHADER* shader) {
	struct ShaderListItem* item = RemoveFromList(&game->_priv.shaders, shader, ShaderIdentity);
	if (!item) {
		PrintConsole(game, "Tried to destroy a unregistered shader!");
		al_destroy_shader(shader);
		return;
	}
	al_destroy_shader(item->shader);
	if (item->vertex) {
		free(item->vertex);
	}
	if (item->fragment) {
		free(item->fragment);
	}
}

SYMBOL_INTERNAL void ReloadShaders(struct Game* game, bool force) {
	struct libsuperderpy_list* list = game->_priv.shaders;
	PrintConsole(game, "Reloading shaders...");
	while (list) {
		struct ShaderListItem* item = list->data;
		if (!item->loaded || force) {
			PrintConsole(game, "V:%s, F:%s", item->vertex, item->fragment);
			AttachToShader(game, item->shader, ALLEGRO_VERTEX_SHADER, item->vertex);
			AttachToShader(game, item->shader, ALLEGRO_PIXEL_SHADER, item->fragment);

			if (!al_build_shader(item->shader)) {
				const char* log = al_get_shader_log(item->shader);
				if (log) {
					FatalError(game, false, "%s", log);
				}
			}
		}
		list = list->next;
	}
	PrintConsole(game, "Shaders reloaded.");
}

SYMBOL_INTERNAL void DestroyShaders(struct Game* game) {
	PrintConsole(game, "Destroying shaders...");
	while (game->_priv.shaders) {
		struct ShaderListItem* item = game->_priv.shaders->data;
		al_destroy_shader(item->shader);
		if (item->vertex) {
			free(item->vertex);
		}
		if (item->fragment) {
			free(item->fragment);
		}
		struct libsuperderpy_list* prev = game->_priv.shaders;
		game->_priv.shaders = game->_priv.shaders->next;
		free(prev);
	}
}

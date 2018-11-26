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

#include "internal.h"

static ALLEGRO_USTR* GetShaderSource(struct Game* game, const char* filename) {
	ALLEGRO_FILE* fp = al_fopen(filename, "r");
	if (!fp) {
		FatalError(game, false, "Failed to open shader file %s", filename);
		return NULL;
	}

	// Use GLSL 1.20 (GL 2.1) and GLSL ES 1.00 (GLES 2.0)
	// We need to use 120 for GL because non-core GL profile on macOS is limited to 2.1
	// Even when ignoring macOS, the highest possible option right now is GLSL 1.30, because
	// most Mesa drivers implement only OpenGL 3.0 on compatibility profile.
	// TODO: upgrade to GLSL 1.50 (GL 3.2, highest possible on macOS) once Allegro works on core profiles
	ALLEGRO_USTR* str = al_ustr_new(al_get_opengl_variant() == ALLEGRO_OPENGL_ES ? "#version 100\n" : "#version 120\n");

	while (true) {
		char buf[512];
		size_t n;
		ALLEGRO_USTR_INFO info;

		n = al_fread(fp, buf, sizeof(buf));
		if (n <= 0) {
			break;
		}
		al_ustr_append(str, al_ref_buffer(&info, buf, n));
	}
	al_fclose(fp);
	return str;
}

static bool AttachToShader(struct Game* game, ALLEGRO_SHADER* shader, ALLEGRO_SHADER_TYPE type, const char* filename) {
	bool ret;
	if (filename) {
		ALLEGRO_USTR* src = GetShaderSource(game, filename);
		if (!src) {
			return false;
		}
		ret = al_attach_shader_source(shader, type, al_cstr(src));
		al_ustr_free(src);
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

static bool ShaderIdentity(struct List* item, void* shader) {
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
	free(item);
}

SYMBOL_INTERNAL void ReloadShaders(struct Game* game, bool force) {
	struct List* list = game->_priv.shaders;
	PrintConsole(game, "Reloading shaders...");
	while (list) {
		struct ShaderListItem* item = list->data;
		if (!item->loaded || force) {
			PrintConsole(game, "V:%s, F:%s", item->vertex, item->fragment);
			bool ret = AttachToShader(game, item->shader, ALLEGRO_VERTEX_SHADER, item->vertex);
			ret = ret && AttachToShader(game, item->shader, ALLEGRO_PIXEL_SHADER, item->fragment);

			if (ret && !al_build_shader(item->shader)) {
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
		struct List* prev = game->_priv.shaders;
		game->_priv.shaders = game->_priv.shaders->next;
		free(prev);
	}
}

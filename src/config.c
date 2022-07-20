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

#include "internal.h"

SYMBOL_EXPORT void InitConfig(struct Game* game) {
	const ALLEGRO_FILE_INTERFACE* iface = al_get_new_file_interface();
	al_set_standard_file_interface();
	ALLEGRO_PATH* path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	ALLEGRO_PATH* data = al_create_path("SuperDerpy.ini");
	al_join_paths(path, data);
	game->_priv.config = al_load_config_file(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
	if (!game->_priv.config) {
		game->_priv.config = al_create_config();
	}
	al_destroy_path(path);
	al_destroy_path(data);
	al_set_new_file_interface(iface);
}

static void StoreConfig(struct Game* game) {
	const ALLEGRO_FILE_INTERFACE* iface = al_get_new_file_interface();
	al_set_standard_file_interface();
	ALLEGRO_PATH* path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	ALLEGRO_PATH* data = al_create_path("SuperDerpy.ini");
	al_make_directory(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
	al_join_paths(path, data);
	al_save_config_file(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP), game->_priv.config);
	al_destroy_path(path);
	al_destroy_path(data);
	al_set_new_file_interface(iface);
}

SYMBOL_EXPORT void SetConfigOption(struct Game* game, char* section, char* name, char* value) {
	al_set_config_value(game->_priv.config, section, name, value);
#ifdef __EMSCRIPTEN__
	EM_ASM({
		var key = '[' + UTF8ToString($0) + '] ' + UTF8ToString($1);
		try {
			window.localStorage.setItem(key, UTF8ToString($2));
		} catch (e) {
			console.error(e);
		}
	},
		section, name, value);
#endif
	StoreConfig(game);
}

SYMBOL_EXPORT const char* GetConfigOption(struct Game* game, char* section, char* name) {
#ifdef __EMSCRIPTEN__
	char* buf = (char*)EM_ASM_INT({
		var key = '[' + UTF8ToString($0) + '] ' + UTF8ToString($1);
		var value;
		try {
			value = window.localStorage.getItem(key);
		} catch (e) {
			console.error(e);
		}
		if (value) {
			var buf = _malloc(255);
			stringToUTF8(value, buf, 255);
			return buf;
		}
		return 0;
	},
		section, name);
	return buf ? AddGarbage(game, buf) : NULL;
#endif
	return al_get_config_value(game->_priv.config, section, name);
}

SYMBOL_EXPORT const char* GetConfigOptionDefault(struct Game* game, char* section, char* name, const char* def) {
	const char* ret = GetConfigOption(game, section, name);
	if (!ret) {
		return def;
	}
	return ret;
}

SYMBOL_EXPORT void DeleteConfigOption(struct Game* game, char* section, char* name) {
	al_remove_config_key(game->_priv.config, section, name);
#ifdef __EMSCRIPTEN__
	EM_ASM({
		var key = '[' + UTF8ToString($0) + '] ' + UTF8ToString($1);
		try {
			window.localStorage.removeItem(key);
		} catch (e) {
			console.error(e);
		}
	},
		section, name);
#endif
	StoreConfig(game);
}

SYMBOL_EXPORT void DeinitConfig(struct Game* game) {
	al_destroy_config(game->_priv.config);
}

/*! \file config.h
 *  \brief Configuration manager headers.
 */
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

#ifndef LIBSUPERDERPY_CONFIG_H
#define LIBSUPERDERPY_CONFIG_H

#include "libsuperderpy.h"

/*! \brief Reads config from file into memory. */
void InitConfig(struct Game* game);
/*! \brief Returns value of requested config entry. */
const char* GetConfigOption(struct Game* game, char* section, char* name);
/*! \brief Returns value of requested config entry, or `def` if no such entry exists. */
const char* GetConfigOptionDefault(struct Game* game, char* section, char* name, const char* def);
/*! \brief Sets a new value of requested config entry, or creates a new one if no such entry exists. */
void SetConfigOption(struct Game* game, char* section, char* name, char* value);
/*! \brief Deletes an existing config entry. */
void DeleteConfigOption(struct Game* game, char* section, char* name);
/*! \brief Writes config from memory to file. */
void DeinitConfig(struct Game* game);

#endif /* LIBSUPERDERPY_CONFIG_H */

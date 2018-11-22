/*! \file shader.h
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

#ifndef LIBSUPERDERPY_SHADER_H
#define LIBSUPERDERPY_SHADER_H

#include "libsuperderpy.h"

ALLEGRO_SHADER* CreateShader(struct Game* game, const char* vertex, const char* fragment);
void DestroyShader(struct Game* game, ALLEGRO_SHADER* shader);

#endif

/*! \file tests.h
 *  \brief Headers for test files.
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

#ifndef LIBSUPERDERPY_TESTS_H
#define LIBSUPERDERPY_TESTS_H

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#include "internal.h"

int engine_setup(void** state);
int engine_teardown(void** state);
int test_timeline(void);
int test_character(void);

#endif

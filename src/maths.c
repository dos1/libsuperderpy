/*! \file maths.c
 *  \brief Math helper functions.
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

#include "maths.h"
#include "config.h"
#include "internal.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

SYMBOL_EXPORT double DotProduct(const double v[], const double u[], int n) {
	float result = 0.0;
	for (int i = 0; i < n; i++) {
		result += v[i] * u[i];
	}
	return result;
}

SYMBOL_EXPORT double VectorLength(double x, double y, double z) {
	return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

SYMBOL_EXPORT double Wrap(double left, double right, double val) {
	return left + fmod(val - left, right - left);
}

SYMBOL_EXPORT double Clamp(double left, double right, double val) {
	if (val > right) {
		return right;
	}
	if (val < left) {
		return left;
	}
	return val;
}

SYMBOL_EXPORT double Lerp(double left, double right, double pos) {
	return left + (right - left) * pos;
}

SYMBOL_EXPORT double Sign(double val) {
	return val / fabs(val);
}

SYMBOL_EXPORT double Fract(double val) {
	return val - floor(val);
}

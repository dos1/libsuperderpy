/*! \file maths.h
 *  \brief Headers of math helper functions.
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

#ifndef LIBSUPERDERPY_MATHS_H
#define LIBSUPERDERPY_MATHS_H

#include "libsuperderpy.h"

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

double DotProduct(const double v[], const double u[], int n);
double VectorLength(double x, double y, double z);
double Wrap(double left, double right, double val);
double Clamp(double left, double right, double val);
double Lerp(double left, double right, double pos);
int Sign(double val);
double Fract(double val);
double Distance(double x1, double y1, double x2, double y2);

#endif /* LIBSUPERDERPY_MATHS_H */

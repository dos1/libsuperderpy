/*! \file tween.h
 *  \brief Tweening engine.
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

#ifndef LIBSUPERDERPY_TWEEN_H
#define LIBSUPERDERPY_TWEEN_H

#include "libsuperderpy.h"

typedef enum TWEEN_STYLE {
	// Linear
	TWEEN_STYLE_LINEAR,
	// Quadratic easing; p^2
	TWEEN_STYLE_QUADRATIC_IN,
	TWEEN_STYLE_QUADRATIC_OUT,
	TWEEN_STYLE_QUADRATIC_IN_OUT,
	// Cubic easing; p^3
	TWEEN_STYLE_CUBIC_IN,
	TWEEN_STYLE_CUBIC_OUT,
	TWEEN_STYLE_CUBIC_IN_OUT,
	// Quartic easing; p^4
	TWEEN_STYLE_QUARTIC_IN,
	TWEEN_STYLE_QUARTIC_OUT,
	TWEEN_STYLE_QUARTIC_IN_OUT,
	// Quintic easing; p^5
	TWEEN_STYLE_QUINTIC_IN,
	TWEEN_STYLE_QUINTIC_OUT,
	TWEEN_STYLE_QUINTIC_IN_OUT,
	// Sine wave easing; sin(p * PI/2)
	TWEEN_STYLE_SINE_IN,
	TWEEN_STYLE_SINE_OUT,
	TWEEN_STYLE_SINE_IN_OUT,
	// Circular easing; sqrt(1 - p^2)
	TWEEN_STYLE_CIRCULAR_IN,
	TWEEN_STYLE_CIRCULAR_OUT,
	TWEEN_STYLE_CIRCULAR_IN_OUT,
	// Exponential easing, base 2
	TWEEN_STYLE_EXPONENTIAL_IN,
	TWEEN_STYLE_EXPONENTIAL_OUT,
	TWEEN_STYLE_EXPONENTIAL_IN_OUT,
	// Exponentially-damped sine wave easing
	TWEEN_STYLE_ELASTIC_IN,
	TWEEN_STYLE_ELASTIC_OUT,
	TWEEN_STYLE_ELASTIC_IN_OUT,
	// Overshooting cubic easing;
	TWEEN_STYLE_BACK_IN,
	TWEEN_STYLE_BACK_OUT,
	TWEEN_STYLE_BACK_IN_OUT,
	// Exponentially-decaying bounce easing
	TWEEN_STYLE_BOUNCE_IN,
	TWEEN_STYLE_BOUNCE_OUT,
	TWEEN_STYLE_BOUNCE_IN_OUT
} TWEEN_STYLE; // Cheet sheet at http://easings.net/ :)

struct Tween {
	double start, stop;
	double duration, pos;
	double predelay, postdelay;
	TWEEN_STYLE style;
	bool paused;
	bool done;
	struct Game* game;
	void (*callback)(struct Game* game, struct Tween* tween, void* data);
	void* data;
};

struct Tween Tween(struct Game* game, double start, double stop, TWEEN_STYLE style, double duration);
struct Tween StaticTween(struct Game* game, double value);
double GetTweenPosition(struct Tween* tween);
double GetTweenInterpolation(struct Tween* tween);
double GetTweenValue(struct Tween* tween);
void UpdateTween(struct Tween* tween, double delta);
double Interpolate(double pos, TWEEN_STYLE style);

#endif

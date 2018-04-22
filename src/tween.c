/*! \file tween.c
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
#include "tween.h"
#include "internal.h"
#include <allegro5/allegro.h>
#include <math.h>

// Easing formulas (c) 2011, Auerhaus Development, LLC
// Originally licensed under WTFPL 2.0
// https://github.com/warrenm/AHEasing

// Modeled after the line y = x
static double LinearInterpolation(double p) {
	return p;
}

// Modeled after the parabola y = x^2
static double QuadraticEaseIn(double p) {
	return p * p;
}

// Modeled after the parabola y = -x^2 + 2x
static double QuadraticEaseOut(double p) {
	return -(p * (p - 2));
}

// Modeled after the piecewise quadratic
// y = (1/2)((2x)^2)             ; [0, 0.5)
// y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
static double QuadraticEaseInOut(double p) {
	if (p < 0.5) {
		return 2 * p * p;
	}
	return (-2 * p * p) + (4 * p) - 1;
}

// Modeled after the cubic y = x^3
static double CubicEaseIn(double p) {
	return p * p * p;
}

// Modeled after the cubic y = (x - 1)^3 + 1
static double CubicEaseOut(double p) {
	double f = (p - 1);
	return f * f * f + 1;
}

// Modeled after the piecewise cubic
// y = (1/2)((2x)^3)       ; [0, 0.5)
// y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
static double CubicEaseInOut(double p) {
	if (p < 0.5) {
		return 4 * p * p * p;
	}
	double f = ((2 * p) - 2);
	return 0.5 * f * f * f + 1;
}

// Modeled after the quartic x^4
static double QuarticEaseIn(double p) {
	return p * p * p * p;
}

// Modeled after the quartic y = 1 - (x - 1)^4
static double QuarticEaseOut(double p) {
	double f = (p - 1);
	return f * f * f * (1 - p) + 1;
}

// Modeled after the piecewise quartic
// y = (1/2)((2x)^4)        ; [0, 0.5)
// y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
static double QuarticEaseInOut(double p) {
	if (p < 0.5) {
		return 8 * p * p * p * p;
	}
	double f = (p - 1);
	return -8 * f * f * f * f + 1;
}

// Modeled after the quintic y = x^5
static double QuinticEaseIn(double p) {
	return p * p * p * p * p;
}

// Modeled after the quintic y = (x - 1)^5 + 1
static double QuinticEaseOut(double p) {
	double f = (p - 1);
	return f * f * f * f * f + 1;
}

// Modeled after the piecewise quintic
// y = (1/2)((2x)^5)       ; [0, 0.5)
// y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
static double QuinticEaseInOut(double p) {
	if (p < 0.5) {
		return 16 * p * p * p * p * p;
	}
	double f = ((2 * p) - 2);
	return 0.5 * f * f * f * f * f + 1;
}

// Modeled after quarter-cycle of sine wave
static double SineEaseIn(double p) {
	return sin((p - 1) * (ALLEGRO_PI / 2.0)) + 1;
}

// Modeled after quarter-cycle of sine wave (different phase)
static double SineEaseOut(double p) {
	return sin(p * (ALLEGRO_PI / 2.0));
}

// Modeled after half sine wave
static double SineEaseInOut(double p) {
	return 0.5 * (1 - cos(p * ALLEGRO_PI));
}

// Modeled after shifted quadrant IV of unit circle
static double CircularEaseIn(double p) {
	return 1 - sqrt(1 - (p * p));
}

// Modeled after shifted quadrant II of unit circle
static double CircularEaseOut(double p) {
	return sqrt((2 - p) * p);
}

// Modeled after the piecewise circular function
// y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)
// y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
static double CircularEaseInOut(double p) {
	if (p < 0.5) {
		return 0.5 * (1 - sqrt(1 - 4 * (p * p)));
	}
	return 0.5 * (sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
}

// Modeled after the exponential function y = 2^(10(x - 1))
static double ExponentialEaseIn(double p) {
	return (p == 0.0) ? p : pow(2, 10 * (p - 1));
}

// Modeled after the exponential function y = -2^(-10x) + 1
static double ExponentialEaseOut(double p) {
	return (p == 1.0) ? p : 1 - pow(2, -10 * p);
}

// Modeled after the piecewise exponential
// y = (1/2)2^(10(2x - 1))         ; [0,0.5)
// y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
static double ExponentialEaseInOut(double p) {
	if (p == 0.0 || p == 1.0) { return p; }

	if (p < 0.5) {
		return 0.5 * pow(2, (20 * p) - 10);
	}
	return -0.5 * pow(2, (-20 * p) + 10) + 1;
}

// Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
static double ElasticEaseIn(double p) {
	return sin(13 * (ALLEGRO_PI / 2.0) * p) * pow(2, 10 * (p - 1));
}

// Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1
static double ElasticEaseOut(double p) {
	return sin(-13 * (ALLEGRO_PI / 2.0) * (p + 1)) * pow(2, -10 * p) + 1;
}

// Modeled after the piecewise exponentially-damped sine wave:
// y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)
// y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
static double ElasticEaseInOut(double p) {
	if (p < 0.5) {
		return 0.5 * sin(13 * (ALLEGRO_PI / 2.0) * (2 * p)) * pow(2, 10 * ((2 * p) - 1));
	}
	return 0.5 * (sin(-13 * (ALLEGRO_PI / 2.0) * ((2 * p - 1) + 1)) * pow(2, -10 * (2 * p - 1)) + 2);
}

// Modeled after the overshooting cubic y = x^3-x*sin(x*pi)
static double BackEaseIn(double p) {
	return p * p * p - p * sin(p * ALLEGRO_PI);
}

// Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
static double BackEaseOut(double p) {
	double f = (1 - p);
	return 1 - (f * f * f - f * sin(f * ALLEGRO_PI));
}

// Modeled after the piecewise overshooting cubic function:
// y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)
// y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
static double BackEaseInOut(double p) {
	if (p < 0.5) {
		double f = 2 * p;
		return 0.5 * (f * f * f - f * sin(f * ALLEGRO_PI));
	}
	double f = (1 - (2 * p - 1));
	return 0.5 * (1 - (f * f * f - f * sin(f * ALLEGRO_PI))) + 0.5;
}

static double BounceEaseOut(double p) {
	if (p < 4 / 11.0) {
		return (121 * p * p) / 16.0;
	}
	if (p < 8 / 11.0) {
		return (363 / 40.0 * p * p) - (99 / 10.0 * p) + 17 / 5.0;
	}
	if (p < 9 / 10.0) {
		return (4356 / 361.0 * p * p) - (35442 / 1805.0 * p) + 16061 / 1805.0;
	}
	return (54 / 5.0 * p * p) - (513 / 25.0 * p) + 268 / 25.0;
}

static double BounceEaseIn(double p) {
	return 1 - BounceEaseOut(1 - p);
}

static double BounceEaseInOut(double p) {
	if (p < 0.5) {
		return 0.5 * BounceEaseIn(p * 2);
	}
	return 0.5 * BounceEaseOut(p * 2 - 1) + 0.5;
}

// ------------------------------------------------------------------------------

struct Tween Tween(struct Game* game, double start, double stop, double duration, TWEEN_STYLE style) {
	return (struct Tween){
		.start = start,
		.stop = stop,
		.duration = duration,
		.style = style,
		.pos = 0,
		.paused = false,
		.game = game,
		.done = false,
		.callback = NULL,
		.data = NULL};
}

double GetTweenPosition(struct Tween* tween) {
	if (tween->duration == 0.0) {
		return 1.0;
	}
	return tween->pos / tween->duration;
}

double GetTweenInterpolation(struct Tween* tween) {
	double pos = GetTweenPosition(tween);
	switch (tween->style) {
		case TWEEN_STYLE_LINEAR:
			return LinearInterpolation(pos);
		case TWEEN_STYLE_QUADRATIC_IN:
			return QuadraticEaseIn(pos);
		case TWEEN_STYLE_QUADRATIC_OUT:
			return QuadraticEaseOut(pos);
		case TWEEN_STYLE_QUADRATIC_IN_OUT:
			return QuadraticEaseInOut(pos);
		case TWEEN_STYLE_CUBIC_IN:
			return CubicEaseIn(pos);
		case TWEEN_STYLE_CUBIC_OUT:
			return CubicEaseOut(pos);
		case TWEEN_STYLE_CUBIC_IN_OUT:
			return CubicEaseInOut(pos);
		case TWEEN_STYLE_QUARTIC_IN:
			return QuarticEaseIn(pos);
		case TWEEN_STYLE_QUARTIC_OUT:
			return QuarticEaseOut(pos);
		case TWEEN_STYLE_QUARTIC_IN_OUT:
			return QuarticEaseInOut(pos);
		case TWEEN_STYLE_QUINTIC_IN:
			return QuinticEaseIn(pos);
		case TWEEN_STYLE_QUINTIC_OUT:
			return QuinticEaseOut(pos);
		case TWEEN_STYLE_QUINTIC_IN_OUT:
			return QuinticEaseInOut(pos);
		case TWEEN_STYLE_SINE_IN:
			return SineEaseIn(pos);
		case TWEEN_STYLE_SINE_OUT:
			return SineEaseOut(pos);
		case TWEEN_STYLE_SINE_IN_OUT:
			return SineEaseInOut(pos);
		case TWEEN_STYLE_CIRCULAR_IN:
			return CircularEaseIn(pos);
		case TWEEN_STYLE_CIRCULAR_OUT:
			return CircularEaseOut(pos);
		case TWEEN_STYLE_CIRCULAR_IN_OUT:
			return CircularEaseInOut(pos);
		case TWEEN_STYLE_EXPONENTIAL_IN:
			return ExponentialEaseIn(pos);
		case TWEEN_STYLE_EXPONENTIAL_OUT:
			return ExponentialEaseOut(pos);
		case TWEEN_STYLE_EXPONENTIAL_IN_OUT:
			return ExponentialEaseInOut(pos);
		case TWEEN_STYLE_ELASTIC_IN:
			return ElasticEaseIn(pos);
		case TWEEN_STYLE_ELASTIC_OUT:
			return ElasticEaseOut(pos);
		case TWEEN_STYLE_ELASTIC_IN_OUT:
			return ElasticEaseInOut(pos);
		case TWEEN_STYLE_BACK_IN:
			return BackEaseIn(pos);
		case TWEEN_STYLE_BACK_OUT:
			return BackEaseOut(pos);
		case TWEEN_STYLE_BACK_IN_OUT:
			return BackEaseInOut(pos);
		case TWEEN_STYLE_BOUNCE_IN:
			return BounceEaseIn(pos);
		case TWEEN_STYLE_BOUNCE_OUT:
			return BounceEaseOut(pos);
		case TWEEN_STYLE_BOUNCE_IN_OUT:
			return BounceEaseInOut(pos);
	}
	return pos;
}

double GetTweenValue(struct Tween* tween) {
	return tween->start + GetTweenInterpolation(tween) * (tween->stop - tween->start);
}

void UpdateTween(struct Tween* tween, double delta) {
	if (tween->paused) { return; }
	tween->pos += delta;
	if (tween->pos > tween->duration) {
		tween->pos = tween->duration;
		if (!tween->done) {
			tween->done = true;
			if (tween->callback) {
				tween->callback(tween->game, tween, tween->data);
			}
		}
	}
}

// TODO: smooth update of the tween target
// TODO: Rumina-style movement mode

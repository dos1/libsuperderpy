/*! \file particle.h
 *  \brief Particle engine.
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

#ifndef LIBSUPERDERPY_PARTICLE_H
#define LIBSUPERDERPY_PARTICLE_H

#include "libsuperderpy.h"

struct Particle;

struct ParticleState {
	double x, y;
	double scaleX, scaleY;
	double angle;
	ALLEGRO_COLOR tint;
};

typedef bool ParticleFunc(struct Game* game, struct ParticleState* particle, double delta, void* data);

struct Particle {
	struct Character* character;
	bool active;
	ParticleFunc* func;
	struct ParticleState state;
	void* data;
};

struct ParticleBucket {
	int size;
	int last;
	int active;
	bool growing;
	struct Particle* particles;
};

struct GravityParticleData {
	double dx, dy;
	double gravity;
	double friction;
};

struct GravityParticleData* GravityParticleData(double dx, double dy, double gravity, double friction);
bool GravityParticle(struct Game* game, struct ParticleState* particle, double delta, void* d);

struct LinearParticleData {
	double dx, dy;
};

struct LinearParticleData* LinearParticleData(double dx, double dy);
bool LinearParticle(struct Game* game, struct ParticleState* particle, double delta, void* d);

struct FaderParticleData {
	ParticleFunc* func;
	void* data;
	double delay;
	double speed;
	double time;
	double fade;
};

struct FaderParticleData* FaderParticleData(double delay, double speed, ParticleFunc* func, void* d);
bool FaderParticle(struct Game* game, struct ParticleState* particle, double delta, void* d);

struct ParticleState SpawnParticleIn(float x, float y);
struct ParticleState SpawnParticleBetween(float x1, float y1, float x2, float y2);

struct ParticleBucket* CreateParticleBucket(struct Game* game, int size, bool growing);
void UpdateParticles(struct Game* game, struct ParticleBucket* bucket, double delta);
void DrawParticles(struct Game* game, struct ParticleBucket* bucket);
void EmitParticle(struct Game* game, struct ParticleBucket* bucket, struct Character* archetype, ParticleFunc* func, struct ParticleState state, void* data);
void DestroyParticleBucket(struct Game* game, struct ParticleBucket* bucket);

#endif

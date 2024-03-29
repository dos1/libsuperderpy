/*! \file particle.h
 *  \brief Particle engine.
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

struct ParticleBucket {
	int size;
	int last;
	int active;
	bool growing;
	struct Particle* particles;
};

void* GravityParticleData(double dx, double dy, double gravity, double friction);
bool GravityParticle(struct Game* game, struct ParticleState* particle, double delta, void* d);

void* LinearParticleData(double dx, double dy);
bool LinearParticle(struct Game* game, struct ParticleState* particle, double delta, void* d);

void* FaderParticleData(double delay, double speed, ParticleFunc* func, void* d);
bool FaderParticle(struct Game* game, struct ParticleState* particle, double delta, void* d);

struct ParticleState SpawnParticleIn(float x, float y);
struct ParticleState SpawnParticleBetween(float x1, float y1, float x2, float y2);

struct ParticleBucket* CreateParticleBucket(struct Game* game, int size, bool growing);
void UpdateParticles(struct Game* game, struct ParticleBucket* bucket, double delta);
void DrawParticles(struct Game* game, struct ParticleBucket* bucket);
void EmitParticle(struct Game* game, struct ParticleBucket* bucket, struct Character* archetype, ParticleFunc* func, struct ParticleState state, void* data);
void DestroyParticleBucket(struct Game* game, struct ParticleBucket* bucket);

#endif /* LIBSUPERDERPY_PARTICLE_H */

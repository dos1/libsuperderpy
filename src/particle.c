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

#include "internal.h"

// TODO: think about pre-allocating particle data

SYMBOL_EXPORT struct GravityParticleData* GravityParticleData(double dx, double dy, double gravity, double friction) {
	struct GravityParticleData* data = calloc(1, sizeof(struct GravityParticleData));
	data->dx = dx;
	data->dy = dy;
	data->gravity = gravity;
	data->friction = friction;
	return data;
}

SYMBOL_EXPORT bool GravityParticle(struct Game* game, struct ParticleState* particle, double delta, void* d) {
	struct GravityParticleData* data = d;
	if (!particle) {
		free(data);
		return false;
	}
	data->dx *= (1.0 - (data->friction * delta / (1 / 60.0)));
	data->dy += data->gravity * delta / (1 / 60.0);
	particle->x += data->dx * delta / (1 / 60.0);
	particle->y += data->dy * delta / (1 / 60.0);
	return true;
}

SYMBOL_EXPORT struct LinearParticleData* LinearParticleData(double dx, double dy) {
	struct LinearParticleData* data = calloc(1, sizeof(struct LinearParticleData));
	data->dx = dx;
	data->dy = dy;
	return data;
}

SYMBOL_EXPORT bool LinearParticle(struct Game* game, struct ParticleState* particle, double delta, void* d) {
	struct LinearParticleData* data = d;
	if (!particle) {
		free(data);
		return false;
	}
	particle->x += data->dx * delta / (1 / 60.0);
	particle->y += data->dy * delta / (1 / 60.0);
	return true;
}

SYMBOL_EXPORT struct FaderParticleData* FaderParticleData(double delay, double speed, ParticleFunc* func, void* d) {
	struct FaderParticleData* data = calloc(1, sizeof(struct FaderParticleData));
	data->delay = delay;
	data->data = d;
	data->fade = 0.0;
	data->func = func;
	data->speed = speed;
	data->time = 0.0;
	return data;
}

SYMBOL_EXPORT bool FaderParticle(struct Game* game, struct ParticleState* particle, double delta, void* d) {
	struct FaderParticleData* data = d;

	if (!particle) {
		data->func(game, particle, delta, data->data);
		free(data);
		return false;
	}

	data->time += delta;

	float r, g, b, a;
	al_unmap_rgba_f(particle->tint, &r, &g, &b, &a);
	r /= 1.0 - data->fade;
	g /= 1.0 - data->fade;
	b /= 1.0 - data->fade;
	a /= 1.0 - data->fade;
	particle->tint = al_map_rgba_f(r, g, b, a);

	if (!data->func(game, particle, delta, data->data)) {
		data->fade = 1.0;
	}
	if (data->time > data->delay) {
		data->fade += data->speed * delta / (1 / 60.0);
	}

	al_unmap_rgba_f(particle->tint, &r, &g, &b, &a);
	r *= 1.0 - data->fade;
	g *= 1.0 - data->fade;
	b *= 1.0 - data->fade;
	a *= 1.0 - data->fade;
	particle->tint = al_map_rgba_f(r, g, b, a);

	if (data->fade >= 1.0) {
		data->func(game, NULL, delta, data->data);
		free(data);
		return false;
	}

	return true;
}

SYMBOL_EXPORT struct ParticleState SpawnParticleIn(float x, float y) {
	return (struct ParticleState){.x = x, .y = y, .scaleX = 1.0, .scaleY = 1.0, .angle = 0.0, .tint = al_map_rgba(255, 255, 255, 255)};
}

SYMBOL_EXPORT struct ParticleState SpawnParticleBetween(float x1, float y1, float x2, float y2) {
	float x = x1 + (x2 - x1) * rand() / RAND_MAX, y = y1 + (y2 - y1) * rand() / RAND_MAX;
	return SpawnParticleIn(x, y);
}

SYMBOL_EXPORT struct ParticleBucket* CreateParticleBucket(struct Game* game, int size, bool growing) {
	struct ParticleBucket* bucket = calloc(1, sizeof(struct ParticleBucket));
	bucket->growing = growing;
	bucket->size = size;
	bucket->particles = calloc(size, sizeof(struct Particle));
	for (int i = 0; i < size; i++) {
		bucket->particles[i].character = CreateCharacter(game, NULL);
		bucket->particles[i].character->shared = true;
	}
	return bucket;
}

SYMBOL_EXPORT void UpdateParticles(struct Game* game, struct ParticleBucket* bucket, double delta) {
	int actives = 0;
	for (int i = 0; i < bucket->size; i++) {
		if (bucket->particles[i].active) {
			if (bucket->particles[i].func(game, &bucket->particles[i].state, delta, bucket->particles[i].data)) {
				actives++;

				SetCharacterPositionF(game, bucket->particles[i].character, bucket->particles[i].state.x, bucket->particles[i].state.y, bucket->particles[i].state.angle);
				AnimateCharacter(game, bucket->particles[i].character, delta, 1.0);
				bucket->particles[i].character->scaleX = bucket->particles[i].state.scaleX;
				bucket->particles[i].character->scaleY = bucket->particles[i].state.scaleY;
				bucket->particles[i].character->tint = bucket->particles[i].state.tint;
			} else {
				bucket->particles[i].active = false;
				bucket->active--;
			}
		}
		if (actives >= bucket->active) {
			return;
		}
	}
}

SYMBOL_EXPORT void DrawParticles(struct Game* game, struct ParticleBucket* bucket) {
	bool was_held = al_is_bitmap_drawing_held();
	al_hold_bitmap_drawing(true);
	for (int i = 0; i < bucket->size; i++) {
		if (bucket->particles[i].active) {
			DrawCharacter(game, bucket->particles[i].character);
		}
	}
	al_hold_bitmap_drawing(was_held);
}

SYMBOL_EXPORT void EmitParticle(struct Game* game, struct ParticleBucket* bucket, struct Character* archetype, ParticleFunc* func, struct ParticleState state, void* data) {
	if (bucket->size == bucket->active) {
		if (bucket->growing) {
			PrintConsole(game, "ERROR: Growing ParticleBucket is not implemented yet! Increase its size (current: %d)", bucket->size);
		} else {
			PrintConsole(game, "ERROR: ParticleBucket is full, increase its size (current: %d)", bucket->size);
		}
		return;
	}
	while (bucket->particles[bucket->last].active) {
		bucket->last++;
		if (bucket->last == bucket->size) {
			bucket->last = 0;
		}
	}

	bucket->particles[bucket->last].active = true;
	bucket->particles[bucket->last].func = func;
	bucket->particles[bucket->last].state = state;
	bucket->particles[bucket->last].data = data;

	CopyCharacter(game, archetype, bucket->particles[bucket->last].character);

	bucket->active++;
	bucket->last++;
	if (bucket->last == bucket->size) {
		bucket->last = 0;
	}
}

SYMBOL_EXPORT void DestroyParticleBucket(struct Game* game, struct ParticleBucket* bucket) {
	for (int i = 0; i < bucket->size; i++) {
		if (bucket->particles[i].active) {
			bucket->particles[i].func(game, NULL, 0.0, bucket->particles[i].data);
		}
		DestroyCharacter(game, bucket->particles[i].character);
	}
	free(bucket->particles);
	free(bucket);
}

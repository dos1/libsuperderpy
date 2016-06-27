/*! \file dosowisko.h
 *  \brief Init animation with dosowisko.net logo.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

struct RocketsResources {
        ALLEGRO_FONT *font;
				ALLEGRO_BITMAP *bg, *earth, *earth2, *pixelator, *loli, *loli2, *loli3;
        struct Character *rocket_template, *usa_flag, *ru_flag, *cursor, *rainbow, *riot, *euro;

        struct Rocket {
            struct Character *character;
            float dx, dy, modifier;
            bool blown, bumped;
            struct Rocket *next, *prev;
        } *rockets_left, *rockets_right;

				int counter, hearts, tick;
        float cloud_rotation;

        struct {
            bool top, right, left, bottom;
        } mousemove;

        ALLEGRO_SAMPLE *rocket_sample, *boom_sample, *rainbow_sample, *jump_sample, *wuwu_sample, *riot_sample;
        ALLEGRO_SAMPLE_INSTANCE *rocket_sound, *boom_sound, *rainbow_sound, *jump_sound, *wuwu_sound, *riot_sound;

        bool lost, won;

        int flash, zadyma;

        int timelimit, spawnspeed, currentspawn, spawncounter;

        struct Timeline *timeline;

				int color;
				int lizakpowa;
};

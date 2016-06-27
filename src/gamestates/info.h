/*! \file menu.h
 *  \brief Main Menu view headers.
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

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>

/*! \brief Resources used by Menu state. */
struct LevelResources {
		ALLEGRO_BITMAP *bg; /*!< Bitmap with lower portion of menu landscape. */
		ALLEGRO_BITMAP *buildings;
		ALLEGRO_BITMAP *hid;
		ALLEGRO_BITMAP *meter;
		ALLEGRO_BITMAP *busted;

		ALLEGRO_TIMER *timer;

		float kidSpeed;

		int markx, marky;

		int usage;
		int lightx, lighty, lightanim;

		int soloready, soloanim, soloflash;
		bool soloactive;

		bool tickling, moveup, movedown, haskid, lost;

		struct Kid {
				struct Character *character;
				struct Kid *next, *prev;
				float speed;
				bool tickled;
				bool grownup;
				int fun;
				bool happy;
				bool right;
		} *kids[6], *destroyQueue, *tickledKid;

		int timeTillNextBadguy, kidRate;

		struct Character *monster;
		struct Character *suit;
		struct Character *kid;
		struct Timeline *timeline;
		float cloud_position; /*!< Position of bigger cloud. */
		ALLEGRO_SAMPLE *sample; /*!< Music sample. */
		ALLEGRO_SAMPLE *click_sample; /*!< Click sound sample. */
		ALLEGRO_SAMPLE_INSTANCE *laughter; /*!< Sample instance with music sound. */
		ALLEGRO_SAMPLE_INSTANCE *click; /*!< Sample instance with click sound. */
		ALLEGRO_FONT *font_title; /*!< Font of "Super Derpy" text. */
		ALLEGRO_FONT *font; /*!< Font of standard menu item. */
		int selected; /*!< Number of selected menu item. */

		struct {
				int key;
				bool shift;
				int delay;
				// workaround for random bogus UP/DOWN events
				int lastkey;
				int lastdelay;
		} keys;

		int score, time;
};

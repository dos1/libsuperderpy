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

/*! \brief Enum of menu states in Menu and Pause game states. */
enum menustate_enum {
	MENUSTATE_MAIN,
	MENUSTATE_OPTIONS,
	MENUSTATE_VIDEO,
	MENUSTATE_AUDIO,
	MENUSTATE_HIDDEN,
	MENUSTATE_ABOUT,
	MENUSTATE_LOST,
	MENUSTATE_INTRO,
	// FIXME: menustate abuse eeeeew
};

/*! \brief Resources used by Menu state. */
struct MenuResources {
		ALLEGRO_BITMAP *bg; /*!< Bitmap with lower portion of menu landscape. */
		ALLEGRO_BITMAP *cloud; /*!< Bitmap with bigger cloud. */
		ALLEGRO_BITMAP *grass;
		ALLEGRO_BITMAP *forest;
		ALLEGRO_BITMAP *stage;
		ALLEGRO_BITMAP *speaker;
		ALLEGRO_BITMAP *lines;
		ALLEGRO_BITMAP *cable;
		ALLEGRO_BITMAP *light;

		ALLEGRO_BITMAP *marksmall;
		ALLEGRO_BITMAP *markbig;
		int markx, marky;

		float badguySpeed;

		ALLEGRO_SAMPLE *chord_samples[6];
		ALLEGRO_SAMPLE_INSTANCE *chords[6];
		// 0-2: low; 3-5: high

		int usage;
		int lightx, lighty, lightanim;

		int soloready, soloanim, soloflash;
		bool soloactive;

		struct Badguy {
				struct Character *character;
				struct Badguy *next, *prev;
				float speed;
				bool melting;
		} *badguys[4], *destroyQueue;

		int timeTillNextBadguy, badguyRate;

		struct Character *ego;
		struct Character *cow;
		struct Character *badguy;
		struct Timeline *timeline;
		float cloud_position; /*!< Position of bigger cloud. */
		ALLEGRO_SAMPLE *sample; /*!< Music sample. */
		ALLEGRO_SAMPLE *click_sample; /*!< Click sound sample. */
		ALLEGRO_SAMPLE *quit_sample;
		ALLEGRO_SAMPLE *end_sample;
		ALLEGRO_SAMPLE *solo_sample;
		ALLEGRO_SAMPLE_INSTANCE *music; /*!< Sample instance with music sound. */
		ALLEGRO_SAMPLE_INSTANCE *click; /*!< Sample instance with click sound. */
		ALLEGRO_SAMPLE_INSTANCE *quit;
		ALLEGRO_SAMPLE_INSTANCE *solo;
		ALLEGRO_SAMPLE_INSTANCE *end;
		ALLEGRO_FONT *font_title; /*!< Font of "Super Derpy" text. */
		ALLEGRO_FONT *font; /*!< Font of standard menu item. */
		int selected; /*!< Number of selected menu item. */
		enum menustate_enum menustate; /*!< Current menu page. */
		struct {
				bool fullscreen;
				int fps;
				int width;
				int height;
				int resolution;
		} options; /*!< Options which can be changed in menu. */

		struct {
				int key;
				bool shift;
				int delay;
				// workaround for random bogus UP/DOWN events
				int lastkey;
				int lastdelay;
		} keys;

		int score;
};

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
	MENUSTATE_HIDDEN,
	MENUSTATE_MAIN,
	MENUSTATE_OPTIONS,
	MENUSTATE_VIDEO,
	MENUSTATE_AUDIO,
	MENUSTATE_ABOUT
};

/*! \brief Resources used by Menu state. */
struct MenuResources {
		ALLEGRO_BITMAP *bg;
		ALLEGRO_BITMAP *monster; /*!< Bitmap with bigger cloud. */
		ALLEGRO_BITMAP *title;

		double title_pos;
		int screen_pos;
		bool invisible;
		int monster_pos;
		bool starting;

		ALLEGRO_SAMPLE *sample; /*!< Music sample. */
		ALLEGRO_SAMPLE *click_sample; /*!< Click sound sample. */
		ALLEGRO_SAMPLE_INSTANCE *music; /*!< Sample instance with music sound. */
		ALLEGRO_SAMPLE_INSTANCE *click; /*!< Sample instance with click sound. */
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

};

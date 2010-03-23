/*
 * $Id$
 *
 * Salasaga: Adds a mouse click to a movie clip
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
 *
 * This file is part of Salasaga.
 *
 * Salasaga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

// Turn on C99 compatibility
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../global_functions.h"
#include "../../dialog/display_warning.h"


gboolean swf_add_mouse_click(SWFMovie this_movie, gint click_type)
{
	// Local variables
	gfloat				click_duration;
	guint				click_frames;
	GString				*click_name;				// The name of the mouse click sound we'll need to play
	guint				i;							// Counter
	GString				*message;					// Used to construct message strings
	SWFDisplayItem 		sound_display_item;
	FILE				*sound_file;				// The file we load the sound from
	SWFMovieClip		sound_movie_clip;			// Movie clip specifically to hold a sound
	SWFSoundStream 		sound_stream;				// The sound we use gets loaded into this
	gchar				*sound_pathname = NULL;		// Full pathname to a sound file to load is constructed in this
	SWFAction			swf_action;					// Used when constructing action script


	// Initialisation
	click_name = g_string_new(NULL);

	// Determine the sound file to load
	switch (click_type)
	{
		case MOUSE_LEFT_ONE:
		case MOUSE_RIGHT_ONE:
		case MOUSE_MIDDLE_ONE:

			// If we've already added the sound to the movie, we don't need to add it again
			if (FALSE == get_mouse_click_single_added())
			{
				// Single click mouse sound
				sound_pathname = g_build_path(G_DIR_SEPARATOR_S, SOUND_DIR, "mouse_single_click.mp3", NULL);

				// Make sure we only add this click sound once
				set_mouse_click_single_added(TRUE);
			}

			click_name = g_string_assign(click_name, "mousesingleclick");
			click_duration = 0.5;
			break;

		case MOUSE_LEFT_DOUBLE:
		case MOUSE_RIGHT_DOUBLE:
		case MOUSE_MIDDLE_DOUBLE:

			// If we've already added the sound to the movie, we don't need to add it again
			if (FALSE == get_mouse_click_double_added())
			{
				// Double click mouse sound
				sound_pathname = g_build_path(G_DIR_SEPARATOR_S, SOUND_DIR, "mouse_double_click.mp3", NULL);

				// Make sure we only add this click sound once
				set_mouse_click_double_added(TRUE);
			}

			click_name = g_string_assign(click_name, "mousedoubleclick");
			click_duration = 0.5;
			break;

		case MOUSE_LEFT_TRIPLE:
		case MOUSE_RIGHT_TRIPLE:
		case MOUSE_MIDDLE_TRIPLE:

			// If we've already added the sound to the movie, we don't need to add it again
			if (FALSE == get_mouse_click_triple_added())
			{
				// Triple click mouse sound
				sound_pathname = g_build_path(G_DIR_SEPARATOR_S, SOUND_DIR, "mouse_triple_click.mp3", NULL);

				// Make sure we only add this click sound once
				set_mouse_click_triple_added(TRUE);
			}

			click_name = g_string_assign(click_name, "mousetripleclick");
			click_duration = 0.5;
			break;
	}

	if (NULL != sound_pathname)
	{
		// Create the sound object we'll be using
		if (get_debug_level()) printf("%s: '%s'\n", _("Full path name to sound file is"), sound_pathname);

		// Load the sound file
		sound_file = fopen(sound_pathname, "rb");
		if (NULL == sound_file)
		{
			// Something went wrong when loading the sound file, so return
			message = g_string_new(NULL);
			g_string_printf(message, "%s ED412: %s", _("Error"), _("Something went wrong when opening a mouse click sound file."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}
		sound_stream = newSWFSoundStream(sound_file);

		// Create a new movie clip file, containing only the sound
		sound_movie_clip = newSWFMovieClip();

		// Ensure the clip doesn't start out playing nor keep looping
		swf_action = compileSWFActionCode("this.stop(); ");
		SWFMovieClip_add(sound_movie_clip, (SWFBlock) swf_action);
		SWFMovieClip_nextFrame(sound_movie_clip);

		// Add the sound stream to the sound movie clip
		SWFMovieClip_setSoundStream(sound_movie_clip, sound_stream, 1);
		SWFMovieClip_nextFrame(sound_movie_clip);

		// Add extra frames to allow the click to play it's full length
		click_frames = roundf(click_duration * get_frames_per_second());
		for (i = 0; i < click_frames; i++)
		{
			SWFMovieClip_nextFrame(sound_movie_clip);
		}

		// Add the sound movie clip to the swf movie
		sound_display_item = SWFMovie_add(this_movie, (SWFBlock) sound_movie_clip);

		// Name the display item
		SWFDisplayItem_setName(sound_display_item, click_name->str);

		// Ensure the object is at the correct display depth
		SWFDisplayItem_setDepth(sound_display_item, 1000 + click_type); // Trying to pick a non conflicting depth
	}

	// Free the memory allocated in this functions
	g_string_free(click_name, TRUE);

	return TRUE;
}

/*
 * $Id$
 *
 * Salasaga: Adds a mouse click to a movie clip
 *
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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
#include "../../../externs.h"
#include "../../display_warning.h"


SWFMovieClip swf_add_mouse_click(SWFMovieClip movie_clip, layer *this_layer_data, gint click_type)
{
	// Local variables
	SWFDisplayItem 		sound_display_item;
	FILE				*sound_file;				// The file we load the sound from
	guint				sound_loop;
	SWFMovieClip		sound_movie_clip;			// Movie clip specifically to hold a sound
	SWFSoundStream 		sound_stream;				// The sound we use gets loaded into this
	gfloat				sound_start;
	guint				sound_start_frame;
	gchar				*sound_pathname = NULL;		// Full pathname to a sound file to load is constructed in this
	SWFAction			swf_action;					// Used when constructing action script


	// Determine the sound file to load
	switch (click_type)
	{
		case MOUSE_LEFT_ONE:
		case MOUSE_RIGHT_ONE:
		case MOUSE_MIDDLE_ONE:

			// Single click mouse sound
			sound_pathname = g_build_path(G_DIR_SEPARATOR_S, sound_path, "mouse_single_click.mp3", NULL);
			break;

		case MOUSE_LEFT_DOUBLE:
		case MOUSE_RIGHT_DOUBLE:
		case MOUSE_MIDDLE_DOUBLE:

			// Double click mouse sound
			sound_pathname = g_build_path(G_DIR_SEPARATOR_S, sound_path, "mouse_double_click.mp3", NULL);
			break;

		case MOUSE_LEFT_TRIPLE:
		case MOUSE_RIGHT_TRIPLE:
		case MOUSE_MIDDLE_TRIPLE:

			// Triple click mouse sound
			sound_pathname = g_build_path(G_DIR_SEPARATOR_S, sound_path, "mouse_triple_click.mp3", NULL);
			break;
	}

	// Create the sound object we'll be using
	if (debug_level) printf("Full path name to sound file is: %s\n", sound_pathname);

	// Load the sound file
	sound_file = fopen(sound_pathname, "rb");
	if (NULL == sound_file)
	{
		// Something went wrong when loading the sound file, so return
		display_warning("Error ED412: Something went wrong when opening a mouse click sound file");
		return FALSE;
	}
	sound_stream = newSWFSoundStream(sound_file);

	// Pad the movie clip by enough frames to start the sound at the correct place
	sound_start = 0.0;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		sound_start += this_layer_data->transition_in_duration;
	sound_start += this_layer_data->duration;
	sound_start_frame = CLAMP(floorf(sound_start * frames_per_second) - 2, 0, 1000); // 1000 was picked from the air
	for (sound_loop = 0; sound_loop < sound_start_frame; sound_loop++)
	{
		SWFMovieClip_nextFrame(movie_clip);
	}

	// Create a new movie clip file, containing only the sound
	sound_movie_clip = newSWFMovieClip();

	// Ensure the movie clip sound starts out playing
	swf_action = compileSWFActionCode("this.play();");
	SWFMovieClip_add(sound_movie_clip, (SWFBlock) swf_action);

	// Add the sound stream to the sound movie clip
	SWFMovieClip_setSoundStream(sound_movie_clip, sound_stream, 1);
	SWFMovieClip_nextFrame(sound_movie_clip);

	// Add the sound movie clip to the object movie clip
	sound_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) sound_movie_clip);

	// Advance the movie clip one frame, else it doesn't get displayed
	SWFMovieClip_nextFrame(movie_clip);

	// Stop the sound from looping
	swf_action = compileSWFActionCode("this.stop();");
	SWFMovieClip_add(movie_clip, (SWFBlock) swf_action);

	return movie_clip;
}

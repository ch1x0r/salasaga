/*
 * $Id$
 *
 * Salasaga: Creates a swf control bar for the given swf movie
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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
#include "../../dialog/display_warning.h"
#include "../../preference/project_preferences.h"
#include "swf_shape_from_image_file.h"


gboolean export_swf_control_bar(SWFMovie main_movie, guint cb_index, guint depth_number)
{
	// Create local variables
	gfloat				button_height;
	gfloat				button_spacing;
	gfloat				button_width;
	gfloat				button_x;
	gfloat				button_y;
	SWFDisplayItem		buttons_display_item;
	gfloat				current_ming_scale;			// Used when creating text swf output
	GString				*file_name_full;
	gchar				*image_path;
	gint				i;
	SWFAction			main_movie_action;
	GString				*message;					// Used to construct message strings
	SWFDisplayItem		mc_display_item;
	SWFMovieClip		movie_clip;
	gint				num_slides;
	gint				num_text_lines = 0;			// Number of text lines in a particular text layer
	gfloat				scaled_font_size;			// Display height of a font in swf, when scaled to the desired output size
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	slide				*slide_data;
	GString				*slide_name_tmp;
	GString				*slide_names_gstring;
	GtkTextIter			text_end;					// End position of text buffer
	gint				text_lines_counter;			// Counter used when processing text
	GtkTextIter			text_start;					// Start position of text buffer
	gfloat				this_text_string_width;		// Used when calculating how wide to draw the text background box
	gchar				*visible_string;			// Text string is retrieved into this
	gfloat				widest_text_string_width = 0.0;  // Used when calculating how wide to draw the text background box

	// Variables used creating the control bar background
	SWFShape			cb_background;

	// Variables used in working out control bar dimensions
	gfloat				control_bar_height;
	gfloat				control_bar_width;
	gfloat				control_bar_x;
	gfloat				control_bar_y;

	// Variables used for the finish button
	SWFAction			finish_action;
	SWFButton			finish_button;
	SWFShape			finish_shape_down;
	SWFShape			finish_shape_over;
	SWFShape			finish_shape_up;

	// Variables used for the forward button
	SWFAction			forward_action;
	SWFButton			forward_button = NULL;
	SWFShape			forward_shape_down;
	SWFShape			forward_shape_over;
	SWFShape			forward_shape_up;

	// Variables used for the pause button
	SWFAction			pause_action;
	SWFButton			pause_button;
	SWFShape			pause_shape_down;
	SWFShape			pause_shape_over;
	SWFShape			pause_shape_up;

	// Variables used for the play button
	SWFAction			play_action;
	SWFButton			play_button;
	SWFShape			play_shape_down;
	SWFShape			play_shape_over;
	SWFShape			play_shape_up;

	// Variables used for the restart button
	SWFAction			restart_action;
	SWFButton			restart_button;
	SWFShape			restart_shape_down;
	SWFShape			restart_shape_over;
	SWFShape			restart_shape_up;

	// Variables used for the rewind button
	SWFAction			rewind_action;
	SWFButton			rewind_button = NULL;
	SWFShape			rewind_shape_down;
	SWFShape			rewind_shape_over;
	SWFShape			rewind_shape_up;

	// Variables used for the information button
	GString				*as_gstring = NULL;			// Used for constructing action script statements
	SWFAction			info_action;
	SWFDisplayItem		info_bg_display_item;
	gfloat				info_bg_box_height;			// Used while generating swf output for text boxes
	gfloat				info_bg_box_width;			// Used while generating swf output for text boxes
	SWFButton			info_button = NULL;
	SWFMovieClip		info_button_movie_clip = NULL;  // The movie clip that contains the information text background and text
	gfloat				info_leading;				// Spacing to use at the edges of the font
	SWFMovieClip		info_movie_clip;			// The movie clip that contains the information text background and text
	SWFText				info_object;				// The information button text object we're working on goes in this
	gfloat				info_real_font_size = 0.0;
	SWFShape			info_shape_down;
	SWFShape			info_shape_over;
	SWFShape			info_shape_up;
	SWFShape			info_text_bg;
	SWFButton			info_text_button;
	SWFFillStyle		info_text_fill_style;

	// Control bar and button resolutions
	control_bar_elements	cb_size_array[] =
	{
		{ 1920, 1200,	// 1920 x 1200	=	0
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   883,		// control bar start x
			  1152,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 1920, 1080,	// 1920 x 1080	=	1
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   883,		// control bar start x
			  1032,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 1600, 1200,	// 1600 x 1200	=	2
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   723,		// control bar start x
			  1152,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 1280, 1024,	// 1280 x 1024	=	3
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   563,		// control bar start x
			   976,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 1280, 720,	// 1280 x 720	=	4
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   563,		// control bar start x
			   672,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 1024, 768,	// 1024 x 768	=	5
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   435,		// control bar start x
			   720,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 800, 600,		// 800 x 600	=	6
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   323,		// control bar start x
			   556,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 720, 480,		// 720 x 480	=	7
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   283,		// control bar start x
			   430,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 640, 480,		// 640 x 480	=	8
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   243,		// control bar start x
			   430,		// control bar start y
			    34,		// control bar height
			   184 },	// control bar width

		{ 600, 400,		// 600 x 400	=	9
				26,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				26,		// button width
			   236,		// control bar start x
			   356,		// control bar start y
				30,		// control bar height
			   160 },	// control bar width

		{ 528, 396,		// 528 x 396	=	10
				26,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				26,		// button width
			   200,		// control bar start x
			   356,		// control bar start y
			    30,		// control bar height
			   160 },	// control bar width

		{ 480, 120,		// 480 x 120	=	11
				14,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				14,		// button width
			   200,		// control bar start x
			    98,		// control bar start y
			    18,		// control bar height
			    88 },	// control bar width

		{ 352, 288,		// 352 x 288	=	12
				15,		// button height
				 1,		// button spacing
				 1,		// button start x
				 1,		// button start y
				15,		// button width
			   135,		// control bar start x
			   260,		// control bar start y
			    18,		// control bar height
				98 },	// control bar width

		{ 320, 240,		// 320 x 240	=	13
				15,		// button height
				 1,		// button spacing
				 1,		// button start x
				 1,		// button start y
				15,		// button width
			   119,		// control bar start x
			   215,		// control bar start y
			    17,		// control bar height
			    98 },	// control bar width

		{ 176, 144,		// 176 x 144	=	14
				11,		// button height
				 0,		// button spacing
				 1,		// button start x
				 1,		// button start y
				11,		// button width
			  59.5,		// control bar start x
			   129,		// control bar start y
			    13,		// control bar height
			    68 },	// control bar width

		{ 160, 120,		// 160 x 120	=	15
				11,		// button height
				 0,		// button spacing
				 1,		// button start x
				 1,		// button start y
				11,		// button width
			  51.5,		// control bar start x
			   105,		// control bar start y
			    13,		// control bar height
			    68 },	// control bar width

		{ 128,  96,		// 128 x 96		=	16
				11,		// button height
				 0,		// button spacing
				 1,		// button start x
				 1,		// button start y
				11,		// button width
			  35.5,		// control bar start x
			    81,		// control bar start y
			    13,		// control bar height
			    68 }	// control bar width
	};


	// Initialise various things
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);

	as_gstring = g_string_new(NULL);
	file_name_full = g_string_new(NULL);
	message = g_string_new(NULL);
	slide_name_tmp = g_string_new(NULL);
	slide_names_gstring = g_string_new(NULL);

	// Retrieve the control bar element positions
	button_x = cb_size_array[cb_index].button_start_x;
	button_y = cb_size_array[cb_index].button_start_y;
	button_height = cb_size_array[cb_index].button_height;
	button_spacing = cb_size_array[cb_index].button_spacing;
	button_width = cb_size_array[cb_index].button_width;
	control_bar_height = cb_size_array[cb_index].cb_height;
	control_bar_width = cb_size_array[cb_index].cb_width;
	control_bar_x = cb_size_array[cb_index].cb_start_x;
	control_bar_y = cb_size_array[cb_index].cb_start_y;

	// If there is only a single slide, the control bar background is not as wide
	if (1 == num_slides)
	{
		control_bar_width -= (button_width + button_spacing) * 2;
		control_bar_x += button_width;
	}

	// If the information button should be displayed, we add that to the control bar background width
	if (TRUE == get_info_display())
	{
		control_bar_width += button_width + button_spacing;
		control_bar_x -= (button_width + button_spacing) / 2;
	}

	// Ensure the swf output starts out in the correct play state and the play button is correct
	if (START_BEHAVIOUR_PLAY == get_start_behaviour())
	{
		g_string_printf(slide_names_gstring, " var playing = true; cb_main.cb_play._visible = false;");
	} else
	{
		g_string_printf(slide_names_gstring, " var playing = false; cb_main.cb_play._visible = true;");
	}

	// Create an action script list of slide names in the project
	g_string_append_printf(slide_names_gstring,
			" _root.info_text._visible = false;"
			" var num_slides = %u;"
			" var this_slide = 0;"
			" var reversing = false;"
			" var slide_names = [", num_slides);
	for (i = 0; i < (num_slides - 1); i++)
	{
		// Add a slide name to the list
		slide_data = g_list_nth_data(slides, i);
		if (NULL == slide_data->name)
		{
			// The slide doesn't have a name, so we create a temporary one
			g_string_printf(slide_name_tmp, "%s%u", _("Slide"), i);
			g_string_append_printf(slide_names_gstring, "\"%s\", ", slide_name_tmp->str);
		} else
		{
			g_string_append_printf(slide_names_gstring, "\"%s\", ", slide_data->name->str);
		}
	}
	if (i == (num_slides - 1))
	{
		// Add the last slide name to the list
		slide_data = g_list_nth_data(slides, i);
		if (NULL == slide_data->name)
		{
			// The slide doesn't have a name, so we create a temporary one
			g_string_printf(slide_name_tmp, "%s%u", _("Slide"), i);
			g_string_append_printf(slide_names_gstring, "\"%s\"]; ", slide_name_tmp->str);  // The lack of comma and closing square bracket are on purpose
		} else
		{
			g_string_append_printf(slide_names_gstring, "\"%s\"]; ", slide_data->name->str);  // The lack of comma and closing square bracket are on purpose
		}
	}

	// Add the action script to keep the animation paused if that's what is requested
	slide_names_gstring = g_string_append(slide_names_gstring, " if (false == _root.playing) { _root.stop(); };");

	// Displaying debugging info if requested
	if (get_debug_level())
	{
		printf(_("Slide name array Action Script: %s\n"), slide_names_gstring->str);
	}

	// Add the initialisation action to the movie
	main_movie_action = compileSWFActionCode(slide_names_gstring->str);
	SWFMovie_add(main_movie, (SWFBlock) main_movie_action);


	// *** Create the control bar background ***

	// Work out which background image to use
	g_string_printf(file_name_full, "background");  // We use the file_name_full variable here as a temporary variable, purely for efficiency
	if (TRUE == get_info_display())  // Check if the background image should be wide enough for the info button
	{
		g_string_append(file_name_full, "_with_info");
	} else
	{
		g_string_append(file_name_full, "_no_info");
	}
	if (1 == num_slides) // Check if the background image should be wide enough for the fast forward and reverse buttons
	{
		g_string_append(file_name_full, "_no_ffrv");
	} else
	{
		g_string_append(file_name_full, "_with_ffrv");
	}

	g_string_printf(message, "%ux%u", get_output_width(), get_output_height());  // We use the message variable here as a temporary variable, for efficiency
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", message->str, file_name_full->str, NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	cb_background = swf_shape_from_image_file(file_name_full->str, -1, -1);
	if (NULL == cb_background)
	{
		// Loading images isn't working.
		g_string_printf(message, "%s. Couldn't load the control bar background: %s", _("Error"), file_name_full->str);
		display_warning(message->str);
		return FALSE;
	}


	// *** Create the Restart button ***

	// Load restart button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "3leftarrow_up", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	restart_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == restart_shape_up)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		return FALSE;
	}

	// Load restart button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "3leftarrow_over", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	restart_shape_over = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == restart_shape_over)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		destroySWFShape(restart_shape_up);
		return FALSE;
	}

	// Load restart button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "3leftarrow_down", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	restart_shape_down = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == restart_shape_down)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		destroySWFShape(restart_shape_up);
		destroySWFShape(restart_shape_over);
		return FALSE;
	}

	// Create an empty button object we can use
	restart_button = newSWFButton();

	// Add the shapes to the button for its various states
	SWFButton_addShape(restart_button, (SWFCharacter) restart_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	SWFButton_addShape(restart_button, (SWFCharacter) restart_shape_over, SWFBUTTON_OVER);
	SWFButton_addShape(restart_button, (SWFCharacter) restart_shape_down, SWFBUTTON_DOWN);

	// Add the restart action to the restart button
	if (get_debug_level())
	{
		// If we're debugging, then generate debugging swf's too
		g_string_printf(message,

				// Format string, grouped as per the strings directly below
				"%s %s%s%s %s%s%s %s",

				// The grouped strings
				" _root.this_slide = 0;",										// %s

				" trace(\"",													// %s
				_("Restart button pressed, slide counter has been set to:"),	// %s
				" \" + _root.this_slide + \".\");",								// %s

				" trace(\"",													// %s
				_("We should now jump to the slide named"),						// %s
				" '\" + _root.slide_names[_root.this_slide] + \"'.\");",		// %s

				" if (true == _root.playing) {"									// %s
				" _root.gotoAndPlay(2, _root.slide_names[_root.this_slide]);"
				" } else {"
				" _root.gotoAndStop(2, _root.slide_names[_root.this_slide]);"
				" };"
		);
		restart_action = compileSWFActionCode(message->str);
	} else
	{
		restart_action = compileSWFActionCode(
				" _root.this_slide = 0;"
				" if (true == _root.playing) {"
				" _root.gotoAndPlay(2, _root.slide_names[_root.this_slide]);"
				" } else {"
				" _root.gotoAndStop(2, _root.slide_names[_root.this_slide]);"
				" };");
	}
	SWFButton_addAction(restart_button, restart_action, SWFBUTTON_MOUSEUP);


	// *** Create the Rewind button ***

	if (1 < num_slides) // No need for a Rewind button if there's only one slide in the project
	{
		// Load rewind button's UP state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "2leftarrow_up", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		rewind_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == rewind_shape_up)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			return FALSE;
		}

		// Load rewind button's OVER state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "2leftarrow_over", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		rewind_shape_over = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == rewind_shape_over)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			destroySWFShape(rewind_shape_up);
			return FALSE;
		}

		// Load rewind button's DOWN state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "2leftarrow_down", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		rewind_shape_down = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == rewind_shape_down)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			destroySWFShape(rewind_shape_up);
			destroySWFShape(rewind_shape_over);
			return FALSE;
		}

		// Create an empty button object we can use
		rewind_button = newSWFButton();

		// Add the shapes to the button for its various states
		SWFButton_addShape(rewind_button, (SWFCharacter) rewind_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
		SWFButton_addShape(rewind_button, (SWFCharacter) rewind_shape_over, SWFBUTTON_OVER);
		SWFButton_addShape(rewind_button, (SWFCharacter) rewind_shape_down, SWFBUTTON_DOWN);

		// Add the rewind action to the rewind button
		if (get_debug_level())
		{
			// If we're debugging, then generate debugging swf's too
			g_string_printf(message,

					// Format string, grouped as per the strings directly below
					"%s %s%s%s %s%s%s %s%s%s %s %s%s%s %s %s%s%s %s",

					// The grouped strings
					"if (0 == _root.this_slide)"															// %s
					" {"
						// We're in the first slide, so jump back to the start of the movie
						" _root.this_slide = 0;",

						" trace(\"",																		// %s
						_("Rewind button pressed while in first slide, slide counter has been set to:"),	// %s
						" \" + _root.this_slide + \".\");",													// %s

						" trace(\"",																		// %s
						_("'playing' variable is unchanged, at:"),											// %s
						" \" + _root.playing + \".\");",													// %s

						" trace(\"",																		// %s
						_("We should now jump to the first frame of the movie."),							// %s
						" \");",																			// %s

						" if (true == _root.playing)"														// %s
						" {",

							" trace(\"",																	// %s
							_("Using"),																		// %s
							" gotoAndPlay. \");",															// %s

							" _root.gotoAndPlay(2);"														// %s
						" }"
						" else"
						" {",

							" trace(\"",																	// %s
							_("Using"),																		// %s
							" gotoAndStop. \");",															// %s

							" _root.gotoAndStop(2);"														// %s
						" }"
					" }"
			);

			g_string_append_printf(message,

					// Format string, grouped as per the strings directly below
					"%s %s%s%s %s%s%s %s%s%s %s%s%s %s %s%s%s %s %s%s%s %s",

					// The grouped strings
					" else"																					// %s
					" {"
						// We're past the first slide, so jump back to the start of the previous slide
						" _root.this_slide -= 1;"
						" _root.reversing = true;",

						" trace(\"",																		// %s
						_("Rewind button pressed, slide counter has been set to:"),							// %s
						" \" + _root.this_slide + \".\");",													// %s

						" trace(\"",																		// %s
						_("'_root.reversing' variable has been set to true."),								// %s
						" \");",																			// %s

						" trace(\"",																		// %s
						_("'playing' variable is unchanged, at:"),											// %s
						" \" + _root.playing + \".\");",													// %s

						" trace(\"",																		// %s
						_("We should now jump to the slide named"),											// %s
						" '\" + _root.slide_names[_root.this_slide] + \"'.\");",							// %s

						" if (true == _root.playing)"														// %s
						" {",

							" trace(\"",																	// %s
							_("Using"),																		// %s
							" gotoAndPlay.\");",															// %s

							" _root.gotoAndPlay(_root.slide_names[_root.this_slide]);"						// %s
						" }"
						" else"
						" {",

							" trace(\"",																	// %s
							_("Using"),																		// %s
							" gotoAndStop.\");",															// %s

							" _root.gotoAndStop(_root.slide_names[_root.this_slide]);"						// %s
						" }"
					" };"

			);

			rewind_action = compileSWFActionCode(message->str);
		} else
		{
			rewind_action = compileSWFActionCode(
					"if (0 == _root.this_slide)"
					" {"  // We're in the first slide, so jump back to the start of the movie
						" _root.this_slide = 0;"
						" if (true == _root.playing)"
						" {"
							" _root.gotoAndPlay(2);"
						" }"
						" else"
						" {"
							" _root.gotoAndStop(2);"
						" }"
					" }"
					" else"
					" {"  // We're past the first slide, so jump back to the start of the previous slide
						" _root.this_slide -= 1;"
						" _root.reversing = true;"
						" if (true == _root.playing)"
						" {"
							" _root.gotoAndPlay(_root.slide_names[_root.this_slide]);"
						" }"
						" else"
						" {"
							" _root.gotoAndStop(_root.slide_names[_root.this_slide]);"
						" }"
					" };");
		}
		SWFButton_addAction(rewind_button, rewind_action, SWFBUTTON_MOUSEUP);
	}

	// *** Create the Pause button ***

	// Load pause button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "player_pause_up", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	pause_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == pause_shape_up)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		return FALSE;
	}

	// Load pause button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "player_pause_over", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	pause_shape_over = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == pause_shape_over)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		destroySWFShape(pause_shape_up);
		return FALSE;
	}

	// Load pause button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "player_pause_down", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	pause_shape_down = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == pause_shape_down)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		destroySWFShape(pause_shape_up);
		destroySWFShape(pause_shape_over);
		return FALSE;
	}

	// Create an empty button object we can use
	pause_button = newSWFButton();

	// Add the shapes to the button for its various states
	SWFButton_addShape(pause_button, (SWFCharacter) pause_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	SWFButton_addShape(pause_button, (SWFCharacter) pause_shape_over, SWFBUTTON_OVER);
	SWFButton_addShape(pause_button, (SWFCharacter) pause_shape_down, SWFBUTTON_DOWN);

	// Add the pause action to the pause button
	if (get_debug_level())
	{
		// If we're debugging, then generate debugging swf's too
		g_string_printf(message,

				// Format string, grouped as per the strings directly below
				"%s %s%s%s %s%s%s %s",

				// The grouped strings
				"cb_play._visible = true;"							// %s
				" _root.playing = false;",

				" trace(\"",										// %s
				_("Pause button pressed. Slide counter equals:"),	// %s
				" \" + _root.this_slide + \".\");",					// %s

				" trace(\"",										// %s
				_("'_root.playing' variable set to false."),		// %s
				" \");",											// %s

				" _root.stop();"									// %s
		);
		pause_action = compileSWFActionCode(message->str);
	} else
	{
		pause_action = compileSWFActionCode(
				"cb_play._visible = true;"
				" _root.playing = false;"
				" _root.stop();");
	}
	SWFButton_addAction(pause_button, pause_action, SWFBUTTON_MOUSEUP);


	// *** Create the Play button ***

	// Load play button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "1rightarrow_up", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	play_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == play_shape_up)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		return FALSE;
	}

	// Load play button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "1rightarrow_over", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	play_shape_over = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == play_shape_over)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		destroySWFShape(play_shape_up);
		return FALSE;
	}

	// Load play button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "1rightarrow_down", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	play_shape_down = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == play_shape_down)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		destroySWFShape(play_shape_up);
		destroySWFShape(play_shape_over);
		return FALSE;
	}

	// Create an empty button object we can use
	play_button = newSWFButton();

	// Add the shapes to the button for its various states
	SWFButton_addShape(play_button, (SWFCharacter) play_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	SWFButton_addShape(play_button, (SWFCharacter) play_shape_over, SWFBUTTON_OVER);
	SWFButton_addShape(play_button, (SWFCharacter) play_shape_down, SWFBUTTON_DOWN);

	// Add the play action to the play button
	if (get_debug_level())
	{
		// If we're debugging, then generate debugging swf's too
		g_string_printf(message,

				// Format string, grouped as per the strings directly below
				"%s %s%s%s %s%s%s %s%s%s %s%s%s %s",

				// The grouped strings
				" cb_play._visible = false;"							// %s
				" _root.reversing = false;"
				" _root.playing = true;",

				" trace(\"",											// %s
				_("Play button pressed. Slide counter equals:"),		// %s
				" \" + _root.this_slide + \".\");",						// %s

				" trace(\"",											// %s
				_("'_root.reversing' variable has been set to false."),	// %s
				" \");",												// %s

				" trace(\"",											// %s
				_("'_root.playing' variable set to true."),				// %s
				" \");",												// %s

				" trace(\"",											// %s
				_("Slide name array is:"),								// %s
				" \" + _root.slide_names + \".\");",					// %s

				" _root.play();"										// %s
		);
		play_action = compileSWFActionCode(message->str);
	} else
	{
		play_action = compileSWFActionCode(
				" cb_play._visible = false;"
				" _root.reversing = false;"
				" _root.playing = true;"
				" _root.play();");
	}
	SWFButton_addAction(play_button, play_action, SWFBUTTON_MOUSEUP);


	// *** Create the Fast Forward button ***

	if (1 < num_slides) // No need for a Forward button if there's only one slide in the project
	{
		// Load forward button's UP state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "2rightarrow_up", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		forward_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == forward_shape_up)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			return FALSE;
		}

		// Load forward button's OVER state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "2rightarrow_over", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		forward_shape_over = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == forward_shape_over)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			destroySWFShape(forward_shape_up);
			return FALSE;
		}

		// Load forward button's DOWN state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "2rightarrow_down", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		forward_shape_down = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == forward_shape_down)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			destroySWFShape(forward_shape_up);
			destroySWFShape(forward_shape_over);
			return FALSE;
		}

		// Create an empty button object we can use
		forward_button = newSWFButton();

		// Add the shapes to the button for its various states
		SWFButton_addShape(forward_button, (SWFCharacter) forward_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
		SWFButton_addShape(forward_button, (SWFCharacter) forward_shape_over, SWFBUTTON_OVER);
		SWFButton_addShape(forward_button, (SWFCharacter) forward_shape_down, SWFBUTTON_DOWN);

		// Add the forward action to the forward button
		if (get_debug_level())
		{
			// If we're debugging, then generate debugging swf's too
			g_string_printf(message,

					// Format string, grouped as per the strings directly below
					"%s %s%s%s %s%s%s %s%s%s %s",

					// The grouped strings
					"if (_root.this_slide >= (_root.num_slides - 1))"								// %s
					" {"
						// We're in the last slide, so we jump to the end of the movie
						" _root.reversing = false;",

						" trace(\"",																// %s
						_("Fast forward button pressed while in last slide, slide counter is:"),	// %s
						" \" + _root.this_slide + \".\");",											// %s

						" trace(\"",																// %s
						_("'_root.reversing' variable has been set to false."),						// %s
						" \");",																	// %s

						" trace(\"",																// %s
						_("Now jumping to the last frame of the movie using gotoAndStop."),			// %s
						" \");",																	// %s

						// Jump to the last frame of the movie
						" _root.gotoAndStop(_root._totalframes);"									// %s
					" }"
			);

			g_string_append_printf(message,

					// Format string, grouped as per the strings directly below
					"%s %s%s%s %s%s%s %s%s%s %s %s%s%s %s %s%s%s %s",

					// The grouped strings
					" else"
					" {"
						// We're not in the last slide yet, so jump to the start of the next slide
						" _root.reversing = false;",										// %s

						" trace(\"",														// %s
						_("Fast forward button pressed, slide counter is:"),				// %s
						" \" + _root.this_slide);",											// %s

						" trace(\"",														// %s
						_("'_root.reversing' variable has been set to false."),				// %s
						" \");",															// %s

						" trace(\"",														// %s
						_("We should now jump to the slide named"),							// %s
						" '\" + _root.slide_names[_root.this_slide + 1] + \"'.\");",		// %s

						" if (true == _root.playing)"										// %s
						" {",

							" trace(\"",													// %s
							_("Using"),														// %s
							" gotoAndPlay.\");",											// %s

							" _root.gotoAndPlay(_root.slide_names[_root.this_slide + 1]);"	// %s
						" }"
						" else"
						" {",

							" trace(\"",													// %s
							_("Using"),														// %s
							" gotoAndStop.\");",											// %s

							" _root.gotoAndStop(_root.slide_names[_root.this_slide + 1]);"	// %s
						" }"
					" };"
			);
			forward_action = compileSWFActionCode(message->str);
		} else
		{
			forward_action = compileSWFActionCode(
					"if (_root.this_slide >= (_root.num_slides - 1))"
					" {"
						// We're in the last slide, so we jump to the end of the movie
						" cb_play._visible = true;"
						" _root.reversing = false;"

						// Jump to the last frame of the movie
						" _root.gotoAndStop(_root._totalframes);"
					" }"
					" else"
					" {"
						// We're not in the last slide yet, so jump to the start of the next slide
						" _root.reversing = false;"
						" if (true == _root.playing)"
						" {"
							" _root.gotoAndPlay(_root.slide_names[_root.this_slide + 1]);"
						" }"
						" else"
						" {"
							" _root.gotoAndStop(_root.slide_names[_root.this_slide + 1]);"
						" }"
					" };");
		}
		SWFButton_addAction(forward_button, forward_action, SWFBUTTON_MOUSEUP);
	}

	// *** Create the Finish button ***

	// Load finish button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "3rightarrow_up", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	finish_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == finish_shape_up)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		return FALSE;
	}

	// Load finish button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "3rightarrow_over", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	finish_shape_over = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == finish_shape_over)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		destroySWFShape(finish_shape_up);
		return FALSE;
	}

	// Load finish button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "3rightarrow_down", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
	g_free(image_path);
	finish_shape_down = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == finish_shape_down)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		destroySWFShape(finish_shape_up);
		destroySWFShape(finish_shape_over);
		return FALSE;
	}

	// Create an empty button object we can use
	finish_button = newSWFButton();

	// Add the shapes to the button for its various states
	SWFButton_addShape(finish_button, (SWFCharacter) finish_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	SWFButton_addShape(finish_button, (SWFCharacter) finish_shape_over, SWFBUTTON_OVER);
	SWFButton_addShape(finish_button, (SWFCharacter) finish_shape_down, SWFBUTTON_DOWN);

	// Add the finish action to the finish button
	if (get_debug_level())
	{
		// If we're debugging, then generate debugging swf's too
		g_string_printf(message,

				// Format string, grouped as per the strings directly below
				"%s %s%s%s %s%s%s %s%s%s %s",

				// The grouped strings
				"cb_play._visible = true;"								// %s
				" _root.this_slide = _root.num_slides - 1;"
				" _root.reversing = false;",

				" trace(\"",											// %s
				_("Finish button pressed."),							// %s
				"\");",													// %s

				" trace(\"",											// %s
				_("'_root.reversing' variable has been set to false."),	// %s
				"\");",													// %s

				" trace(\"",											// %s
				_("Now jumping to last frame of movie."),				// %s
				"\");",													// %s

				// Jump to the last frame of the movie
				" _root.gotoAndStop(_root._totalframes);"				// %s
		);
		finish_action = compileSWFActionCode(message->str);
	} else
	{
		finish_action = compileSWFActionCode(
				"cb_play._visible = true;"
				" _root.this_slide = _root.num_slides - 1;"
				" _root.reversing = false;"
				" _root.gotoAndStop(_root._totalframes);");  // Jump to the last frame of the movie
	}
	SWFButton_addAction(finish_button, finish_action, SWFBUTTON_MOUSEUP);

	// * Create the information button *

	if (TRUE == get_info_display())
	{
		// Create an empty button object we can use
		info_button = newSWFButton();

		// Load info button's UP state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "info_up", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		info_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == info_shape_up)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			return FALSE;
		}

		// Load info button's OVER state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "info_over", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		info_shape_over = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == info_shape_over)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			destroySWFShape(info_shape_up);
			return FALSE;
		}

		// Load finish button's DOWN state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, get_icon_path(), "control_bar", "info_down", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, get_icon_extension());
		g_free(image_path);
		info_shape_down = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == info_shape_down)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			destroySWFShape(info_shape_up);
			destroySWFShape(info_shape_over);
			return FALSE;
		}

		// Add the shapes to the button for its various states
		SWFButton_addShape(info_button, (SWFCharacter) info_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
		SWFButton_addShape(info_button, (SWFCharacter) info_shape_over, SWFBUTTON_OVER);
		SWFButton_addShape(info_button, (SWFCharacter) info_shape_down, SWFBUTTON_DOWN);

		// Add the info action to the info button
		if (get_debug_level())
		{
			// If we're debugging, then generate debugging swf's too
			g_string_printf(message,

					// Format string, grouped as per the strings directly below
					"%s %s%s%s",

					// The grouped strings
					" _root.info_text._visible = !(_root.info_text._visible);",	// %s

					// Toggle the visible state of the info button text
					" trace(\"",												// %s
					_("info_text visibility set to:"),							// %s
					" \" + _root.info_text._visible);"							// %s
			);
			info_action = compileSWFActionCode(message->str);
		} else
		{
			info_action = compileSWFActionCode(
					" _root.info_text._visible = !(_root.info_text._visible); "); // Toggle the visible state of the info button text
		}
		SWFButton_addAction(info_button, info_action, SWFBUTTON_MOUSEUP);

		// * Create the information text object *

		// Calculate the height and width scaling values needed for this swf shape
		scaled_height_ratio = (gfloat) get_output_height() / (gfloat) get_project_height();
		scaled_width_ratio = (gfloat) get_output_width() / (gfloat) get_project_width();

		// Create the text object we'll be using
		info_object = newSWFText();

		// Assign a font to the text object
		SWFText_setFont(info_object, get_fdb_font_object(FONT_DEJAVU_SANS));  // Hard code DejaVu Sans for now.  Should be adjustable later on.

		// Set the height we want for the text
		scaled_font_size = scaled_height_ratio * 26;
		SWFText_setHeight(info_object, scaled_font_size);
		info_real_font_size = SWFText_getAscent(info_object) + SWFText_getDescent(info_object);

		// Set the foreground color for the text
		SWFText_setColor(info_object, 0, 0, 0, 0xff);

		// Work out how many lines of text we're dealing with
		num_text_lines = gtk_text_buffer_get_line_count(get_info_text());

		// Add each line of text to the output
		widest_text_string_width = 0;
		for (text_lines_counter = 0; text_lines_counter < num_text_lines; text_lines_counter++)
		{
			// Select the start and end positions for the given line, in the text buffer
			gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(get_info_text()), &text_start, text_lines_counter);
			text_end = text_start;
			gtk_text_iter_forward_to_line_end(&text_end);

			// Retrieve the text for the given line, and add it to the text object
			visible_string = gtk_text_iter_get_visible_text(&text_start, &text_end);
			SWFText_addUTF8String(info_object, visible_string, NULL);

			// * We need to know which of the strings is widest, so we can calculate the width of the text background box *

			// If this is the widest string, we keep the value of this one
			this_text_string_width = SWFText_getUTF8StringWidth(info_object, (guchar *) visible_string);
			if (this_text_string_width > widest_text_string_width)
				widest_text_string_width = this_text_string_width;

			// * Move the pen down to the start of the next line *

			// Move to the appropriate Y position
			SWFText_moveTo(info_object, 0, (text_lines_counter + 1) * info_real_font_size);

			// Try and move X as close as possible to 0.  We can't use 0 in SWFText_moveTo() due to a bug in Ming
			current_ming_scale = Ming_getScale();
			Ming_setScale(1);
			SWFText_moveTo(info_object, 1, 0);
			Ming_setScale(current_ming_scale);
		}

		// Create the information text background object
		info_text_bg = newSWFShape();
		if (NULL == info_text_bg)
		{
			// Something went wrong when creating the empty shape, so we skip this layer
			g_string_printf(message, "%s ED406: %s", _("Error"), _("Something went wrong when creating the information button background shape."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}

		// Set the fill and border style for the information text background object
		info_text_fill_style = SWFShape_addSolidFillStyle(info_text_bg, 0xff, 0xff, 0xe6, 0xff);
		SWFShape_setRightFillStyle(info_text_bg, info_text_fill_style);
		SWFShape_setLine(info_text_bg, 1, 0x00, 0x00, 0x00, 0xff);

		// Work out the scaled dimensions of the information text background object
		info_leading = SWFText_getLeading(info_object);
		info_bg_box_height = (info_real_font_size * num_text_lines) * 1.02;
		info_bg_box_width = widest_text_string_width + (info_leading * 2);

		// Create the information text background
		SWFShape_drawLine(info_text_bg, info_bg_box_width, 0.0);
		SWFShape_drawLine(info_text_bg, 0.0, info_bg_box_height);
		SWFShape_drawLine(info_text_bg, -(info_bg_box_width), 0.0);
		SWFShape_drawLine(info_text_bg, 0.0, -(info_bg_box_height));

		// Create the movie clip the information button text object will go into
		info_movie_clip = newSWFMovieClip();

		// Add the information text background to the movie clip
		info_bg_display_item = SWFMovieClip_add(info_movie_clip, (SWFBlock) info_text_bg);

		// Position the background
		SWFDisplayItem_moveTo(info_bg_display_item, 0.0, 0.0);

		// Add the text object to the movie clip
		info_bg_display_item = SWFMovieClip_add(info_movie_clip, (SWFBlock) info_object);

		// Position the text elements
		SWFDisplayItem_moveTo(info_bg_display_item, info_leading, SWFText_getAscent(info_object));

		// Advance the movie clip one frame, else it won't be displayed
		SWFMovieClip_nextFrame(info_movie_clip);

		// Create an empty button object we can use
		info_text_button = newSWFButton();

		// Add the shape to the button for all of its states, excluding the hit state
		SWFButton_addShape(info_text_button, (SWFCharacter) info_movie_clip, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN);

		// Use the text background area as the hit state
		SWFButton_addShape(info_text_button, (SWFCharacter) info_text_bg, SWFBUTTON_HIT);

		// Add action script to the button, jumping to the external link
		g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", get_info_link(), get_info_link_target());
		info_action = compileSWFActionCode(as_gstring->str);
		SWFButton_addAction(info_text_button, info_action, SWFBUTTON_MOUSEUP);

		// Add the information text button to the movie clip
		info_button_movie_clip = newSWFMovieClip();
		SWFMovieClip_add(info_button_movie_clip, (SWFBlock) info_text_button);

		// Advance the movie clip one frame, else it won't be displayed
		SWFMovieClip_nextFrame(info_button_movie_clip);
	}


	// *** Add the buttons to a movie clip and attach it to the main movie ***

	// Embed the buttons in a movie clip
	movie_clip = newSWFMovieClip();

	// Add a background to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) cb_background);
	SWFDisplayItem_setDepth(mc_display_item, 1);
	SWFDisplayItem_setName(mc_display_item, "cb_background");

	// Add the restart button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) restart_button);
	SWFDisplayItem_setDepth(mc_display_item, 2);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_restart");
	button_x = button_x + button_width + button_spacing;

	// No need for a Rewind button if there's only one slide in the project
	if (1 < num_slides)
	{
		// Add the rewind button to the control bar
		mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) rewind_button);
		SWFDisplayItem_setDepth(mc_display_item, 3);
		SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
		SWFDisplayItem_setName(mc_display_item, "cb_rewind");
		button_x = button_x + button_width + button_spacing;
	}

	// Add the pause button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) pause_button);
	SWFDisplayItem_setDepth(mc_display_item, 4);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_pause");

	// Add the play button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) play_button);
	SWFDisplayItem_setDepth(mc_display_item, 5);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_play");
	button_x = button_x + button_width + button_spacing;

	// No need for a Forward button if there's only one slide in the project
	if (1 < num_slides)
	{
		// Add the rewind button to the control bar
		mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) forward_button);
		SWFDisplayItem_setDepth(mc_display_item, 6);
		SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
		SWFDisplayItem_setName(mc_display_item, "cb_rewind");
		button_x = button_x + button_width + button_spacing;
	}

	// Add the finish button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) finish_button);
	SWFDisplayItem_setDepth(mc_display_item, 7);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_finish");
	button_x = button_x + button_width + button_spacing;

	// Add the information button components
	if (TRUE == get_info_display())
	{
		// Add the info button to the control bar
		mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) info_button);
		SWFDisplayItem_setDepth(mc_display_item, 8);
		SWFDisplayItem_setName(mc_display_item, "cb_info");
		SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
		button_x = button_x + button_width + button_spacing;

		// Add the information button text to the main movie
		info_bg_display_item = SWFMovie_add(main_movie, (SWFBlock) info_button_movie_clip);
		SWFDisplayItem_setName(info_bg_display_item, "info_text");
		SWFDisplayItem_setDepth(info_bg_display_item, depth_number + 1);
		SWFDisplayItem_moveTo(info_bg_display_item,
				control_bar_x + (control_bar_width / 2) - (widest_text_string_width / 2),
				control_bar_y - (info_real_font_size * num_text_lines) - 4);
	}

	// Advance the movie clip one frame, else it won't be displayed
	SWFMovieClip_nextFrame(movie_clip);

	// Add the movie clip to the main movie
	buttons_display_item = SWFMovie_add(main_movie, (SWFBlock) movie_clip);

	// Name the movie clip
	SWFDisplayItem_setName(buttons_display_item, "cb_main");

	// Set the movie clip to be shown higher in the display stack than the main movie
	SWFDisplayItem_setDepth(buttons_display_item, depth_number);

	// Position the entire control bar object in the correct position onscreen
	SWFDisplayItem_moveTo(buttons_display_item, control_bar_x, control_bar_y);

	// Free the memory allocated in the function thus far
	g_string_free(file_name_full, TRUE);
	g_string_free(message, TRUE);

	// Control bar was created successfully, so indicate this
	return TRUE;
}

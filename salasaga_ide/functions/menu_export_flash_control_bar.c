/*
 * $Id$
 *
 * Salasaga: Creates a swf control bar for the given swf movie 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */


// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "swf_shape_from_image_file.h"


gboolean menu_export_flash_control_bar(SWFMovie main_movie, guint cb_index)
{
	// Create local variables
	gfloat				button_height;
	gfloat				button_spacing;
	gfloat				button_width;
	gfloat				button_x;
	gfloat				button_y;
	SWFDisplayItem		buttons_display_item;
	GString				*file_name_full;
	gchar				*image_path;
	gint				i;
	SWFAction			main_movie_action;
	SWFDisplayItem		mc_display_item;
	SWFMovieClip		movie_clip;
	gint				num_slides;
	slide				*slide_data;
	GString				*slide_name_tmp;
	GString				*slide_names_gstring;

	// Variables used creating the control bar background
	SWFShape			cb_background;

	// Variables used in working out control bar dimensions
	gfloat				control_bar_x;
	gfloat				control_bar_y;
	gfloat				control_bar_height;
	gfloat				control_bar_width;

	// Variables used for the finish button
	SWFAction			finish_action;
	SWFButton			finish_button;
	SWFButtonRecord		finish_record_down;
	SWFButtonRecord		finish_record_over;
	SWFButtonRecord		finish_record_up;
	SWFShape			finish_shape_down;
	SWFShape			finish_shape_over;
	SWFShape			finish_shape_up;

	// Variables used for the forward button
	SWFAction			forward_action;
	SWFButton			forward_button;
	SWFButtonRecord		forward_record_down;
	SWFButtonRecord		forward_record_over;
	SWFButtonRecord		forward_record_up;
	SWFShape			forward_shape_down;
	SWFShape			forward_shape_over;
	SWFShape			forward_shape_up;

	// Variables used for the pause button
	SWFAction			pause_action;
	SWFButton			pause_button;
	SWFButtonRecord		pause_record_down;
	SWFButtonRecord		pause_record_over;
	SWFButtonRecord		pause_record_up;
	SWFShape			pause_shape_down;
	SWFShape			pause_shape_over;
	SWFShape			pause_shape_up;

	// Variables used for the play button
	SWFAction			play_action;
	SWFButton			play_button;
	SWFButtonRecord		play_record_down;
	SWFButtonRecord		play_record_over;
	SWFButtonRecord		play_record_up;
	SWFShape			play_shape_down;
	SWFShape			play_shape_over;
	SWFShape			play_shape_up;

	// Variables used for the restart button
	SWFAction			restart_action;
	SWFButton			restart_button;
	SWFButtonRecord		restart_record_down;
	SWFButtonRecord		restart_record_over;
	SWFButtonRecord		restart_record_up;
	SWFShape			restart_shape_down;
	SWFShape			restart_shape_over;
	SWFShape			restart_shape_up;

	// Variables used for the rewind button
	SWFAction			rewind_action;
	SWFButton			rewind_button;
	SWFButtonRecord		rewind_record_down;
	SWFButtonRecord		rewind_record_over;
	SWFButtonRecord		rewind_record_up;
	SWFShape			rewind_shape_down;
	SWFShape			rewind_shape_over;
	SWFShape			rewind_shape_up;

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
			    36,		// control bar height
			   154 },	// control bar width

		{ 1920, 1080,	// 1920 x 1080	=	1
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   883,		// control bar start x
			  1032,		// control bar start y
			    36,		// control bar height
			   154 },	// control bar width

		{ 1600, 1200,	// 1600 x 1200	=	2
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   723,		// control bar start x
			  1152,		// control bar start y
			    36,		// control bar height
			   154 },	// control bar width

		{ 1280, 1024,	// 1280 x 1024	=	3
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   563,		// control bar start x
			   976,		// control bar start y
			    36,		// control bar height
			   154 },	// control bar width

		{ 1280, 720,	// 1280 x 720	=	4
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   563,		// control bar start x
			   672,		// control bar start y
			    36,		// control bar height
			   154 },	// control bar width

		{ 1024, 768,	// 1024 x 768	=	5
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   435,		// control bar start x
			   720,		// control bar start y
			    36,		// control bar height
			   154 },	// control bar width

		{ 800, 600,		// 800 x 600	=	6
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
			   1.6,		// button start y
				30,		// button width
			   323,		// control bar start x
			   556,		// control bar start y
			    36,		// control bar height
			   154 },	// control bar width

		{ 720, 480,		// 720 x 480	=	7
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   283,		// control bar start x
			   430,		// control bar start y
			    36,		// control bar height
			   154 },	// control bar width

		{ 640, 480,		// 640 x 480	=	8
				30,		// button height
				 0,		// button spacing
				 2,		// button start x
				 2,		// button start y
				30,		// button width
			   243,		// control bar start x
			   430,		// control bar start y
			    36,		// control bar height
			   154 },	// control bar width

		{ 352, 288,		// 352 x 288	=	9
				15,		// button height
				 1,		// button spacing
				 1,		// button start x
				 1,		// button start y
				15,		// button width
			   135,		// control bar start x
			   260,		// control bar start y
			    22,		// control bar height
				82 },	// control bar width

		{ 320, 240,		// 320 x 240	=	10
				15,		// button height
				 1,		// button spacing
				 1,		// button start x
				 1,		// button start y
				15,		// button width
			   119,		// control bar start x
			   215,		// control bar start y
			    22,		// control bar height
			    82 },	// control bar width

		{ 176, 144,		// 176 x 144	=	11
				11,		// button height
				 0,		// button spacing
				 1,		// button start x
				 1,		// button start y
				11,		// button width
			  59.5,		// control bar start x
			   129,		// control bar start y
			    14,		// control bar height
			    57 },	// control bar width

		{ 160, 120,		// 160 x 120	=	12
				11,		// button height
				 0,		// button spacing
				 1,		// button start x
				 1,		// button start y
				11,		// button width
			  51.5,		// control bar start x
			   105,		// control bar start y
			    14,		// control bar height
			    57 },	// control bar width

		{ 128,  96,		// 128 x 96		=	13
				11,		// button height
				 0,		// button spacing
				 1,		// button start x
				 1,		// button start y
				11,		// button width
			  35.5,		// control bar start x
			    81,		// control bar start y
			    14,		// control bar height
			    57 }	// control bar width
	};


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

	// Initialise various things
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	file_name_full = g_string_new(NULL); 
	slide_name_tmp = g_string_new(NULL);
	slide_names_gstring = g_string_new(NULL);

	// Ensure the swf output starts out in the correct play state and the play button is correct
	if (START_BEHAVIOUR_PLAY == start_behaviour)
	{
		g_string_printf(slide_names_gstring, " var playing = true; cb_main.cb_play._visible = false;");
	} else
	{
		g_string_printf(slide_names_gstring, " var playing = false; cb_main.cb_play._visible = true;");
	}

	// Create an action script list of slide names in the project
	g_string_append_printf(slide_names_gstring,
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
			g_string_printf(slide_name_tmp, "Slide%u", i);
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
			g_string_printf(slide_name_tmp, "Slide%u", i);
			g_string_append_printf(slide_names_gstring, "\"%s\"]; ", slide_name_tmp->str);  // The lack of comma and closing square bracket are on purpose
		} else
		{
			g_string_append_printf(slide_names_gstring, "\"%s\"]; ", slide_data->name->str);  // The lack of comma and closing square bracket are on purpose
		}
	}

	// Add the action script to keep the animation paused if that's whats requested
	slide_names_gstring = g_string_append(slide_names_gstring, " if (false == _root.playing) { _root.stop(); };");

	// Displaying debugging info if requested
	if (debug_level)
	{
		printf("Slide name array Action Script: %s\n", slide_names_gstring->str);
	}

	// Add the initialisation action to the movie
	main_movie_action = newSWFAction(slide_names_gstring->str);
	SWFMovie_add(main_movie, (SWFBlock) main_movie_action);

	// Create a background for the control bar buttons to go on
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "background", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
	g_free(image_path);
	cb_background = swf_shape_from_image_file(file_name_full->str, control_bar_width, control_bar_height);
	if (NULL == cb_background)
	{
		// Loading images isn't working
		g_string_free(file_name_full, TRUE);
		return FALSE;
	}


	// *** Create the Restart button ***

	// Load restart button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3leftarrow_up", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
	g_free(image_path);
	restart_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == restart_shape_up)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		return FALSE;
	}

	// Load restart button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3leftarrow_over", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3leftarrow_down", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
	restart_record_up = SWFButton_addCharacter(restart_button, (SWFCharacter) restart_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	restart_record_over = SWFButton_addCharacter(restart_button, (SWFCharacter) restart_shape_over, SWFBUTTON_OVER);
	restart_record_down = SWFButton_addCharacter(restart_button, (SWFCharacter) restart_shape_down, SWFBUTTON_DOWN);

	// Add the restart action to the restart button
	if (debug_level)
	{
		// If we're debugging, then generate debugging swf's too
		restart_action = newSWFAction(
				" _root.this_slide = 0;"
				" trace(\"Restart button pressed, slide counter has been set to: \" + _root.this_slide + \".\");"
				" trace(\"We should now jump to the slide named '\" + _root.slide_names[_root.this_slide] + \"'.\");"
				" if (true == _root.playing) {"
				" _root.gotoAndPlay(2, _root.slide_names[_root.this_slide]);"
				" } else {"
				" _root.gotoAndStop(2, _root.slide_names[_root.this_slide]);"
				" };");
	} else
	{
		restart_action = newSWFAction(
				" _root.this_slide = 0;"
				" if (true == _root.playing) {"
				" _root.gotoAndPlay(2, _root.slide_names[_root.this_slide]);"
				" } else {"
				" _root.gotoAndStop(2, _root.slide_names[_root.this_slide]);"
				" };");
	}
	SWFButton_addAction(restart_button, restart_action, SWFBUTTON_MOUSEUP);


	// *** Create the Rewind button ***

// fixme3: Commented out until another, smaller, background image is added specific for this
//	if (1 < num_slides) // No need for a Rewind button if there's only one slide in the project
//	{
		// Load rewind button's UP state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2leftarrow_up", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
		g_free(image_path);
		rewind_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == rewind_shape_up)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			return FALSE;
		}

		// Load rewind button's OVER state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2leftarrow_over", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
		image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2leftarrow_down", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
		rewind_record_up = SWFButton_addCharacter(rewind_button, (SWFCharacter) rewind_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
		rewind_record_over = SWFButton_addCharacter(rewind_button, (SWFCharacter) rewind_shape_over, SWFBUTTON_OVER);
		rewind_record_down = SWFButton_addCharacter(rewind_button, (SWFCharacter) rewind_shape_down, SWFBUTTON_DOWN);

		// Add the rewind action to the rewind button
		if (debug_level)
		{
			// If we're debugging, then generate debugging swf's too
			rewind_action = newSWFAction(
					"if (0 == _root.this_slide)"
					" {"  // We're in the first slide, so jump back to the start of the movie
						" _root.this_slide = 0;"
						" trace(\"Rewind button pressed while in first slide, slide counter has been set to: \" + _root.this_slide + \".\");"
						" trace(\"'playing' variable is unchanged, at: \" + _root.playing + \".\");"
						" trace(\"We should now jump to the first frame of the movie.\");"
						" if (true == _root.playing)"
						" {"
							" trace(\"Using gotoAndPlay.\");"
							" _root.gotoAndPlay(2);"
						" }"
						" else"
						" {"
							" trace(\"Using gotoAndStop.\");"
							" _root.gotoAndStop(2);"
						" }"
					" }"
					" else"
					" {"  // We're past the first slide, so jump back to the start of the previous slide
						" _root.this_slide -= 1;"
						" _root.reversing = true;"
						" trace(\"Rewind button pressed, slide counter has been set to: \" + _root.this_slide + \".\");"
						" trace(\"'_root.reversing' variable has been set to true.\");"
						" trace(\"'playing' variable is unchanged, at: \" + _root.playing + \".\");"
						" trace(\"We should now jump to the slide named '\" + _root.slide_names[_root.this_slide] + \"'.\");"
						" if (true == _root.playing)"
						" {"
							" trace(\"Using gotoAndPlay.\");"
							" _root.gotoAndPlay(_root.slide_names[_root.this_slide]);"
						" }"
						" else"
						" {"
							" trace(\"Using gotoAndStop.\");"
							" _root.gotoAndStop(_root.slide_names[_root.this_slide]);"
						" }"
					" };");
		} else
		{
			rewind_action = newSWFAction(
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
//	}

	// *** Create the Pause button ***

	// Load pause button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "player_pause_up", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
	g_free(image_path);
	pause_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == pause_shape_up)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		return FALSE;
	}

	// Load pause button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "player_pause_over", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "player_pause_down", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
	pause_record_up = SWFButton_addCharacter(pause_button, (SWFCharacter) pause_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	pause_record_over = SWFButton_addCharacter(pause_button, (SWFCharacter) pause_shape_over, SWFBUTTON_OVER);
	pause_record_down = SWFButton_addCharacter(pause_button, (SWFCharacter) pause_shape_down, SWFBUTTON_DOWN);

	// Add the pause action to the pause button
	if (debug_level)
	{
		// If we're debugging, then generate debugging swf's too
		pause_action = newSWFAction(
				"cb_play._visible = true;"
				" _root.playing = false;"
				" trace(\"Pause button pressed. Slide counter equals: \" + _root.this_slide + \".\");"
				" trace(\"'_root.playing' variable set to false.\");"
				" _root.stop();");
	} else
	{
		pause_action = newSWFAction(
				"cb_play._visible = true;"
				" _root.playing = false;"
				" _root.stop();");
	}
	SWFButton_addAction(pause_button, pause_action, SWFBUTTON_MOUSEUP);


	// *** Create the Play button ***

	// Load play button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "1rightarrow_up", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
	g_free(image_path);
	play_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == play_shape_up)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		return FALSE;
	}

	// Load play button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "1rightarrow_over", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "1rightarrow_down", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
	play_record_up = SWFButton_addCharacter(play_button, (SWFCharacter) play_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	play_record_over = SWFButton_addCharacter(play_button, (SWFCharacter) play_shape_over, SWFBUTTON_OVER);
	play_record_down = SWFButton_addCharacter(play_button, (SWFCharacter) play_shape_down, SWFBUTTON_DOWN);

	// Add the Play action to the play button 
	if (debug_level)
	{
		// If we're debugging, then generate debugging swf's too
		play_action = newSWFAction(
				" cb_play._visible = false;"
				" _root.reversing = false;"
				" _root.playing = true;"
				" trace(\"Play button pressed. Slide counter equals: \" + _root.this_slide + \".\");"
				" trace(\"'_root.reversing' variable has been set to false.\");"
				" trace(\"'_root.playing' variable set to true.\");"
				" trace(\"Slide name array is: \" + _root.slide_names + \".\");"
				" _root.play();");
	} else
	{
		play_action = newSWFAction(
				" cb_play._visible = false;"
				" _root.reversing = false;"
				" _root.playing = true;"
				" _root.play();");
	}
	SWFButton_addAction(play_button, play_action, SWFBUTTON_MOUSEUP);


	// *** Create the Fast Forward button ***

// fixme3: Commented out until another, smaller, background image is added specific for this
//	if (1 < num_slides) // No need for a Forward button if there's only one slide in the project
//	{
		// Load forward button's UP state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2rightarrow_up", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
		g_free(image_path);
		forward_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
		if (NULL == forward_shape_up)
		{
			// Loading images isn't working.
			destroySWFShape(cb_background);
			return FALSE;
		}

		// Load forward button's OVER state image
		image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2rightarrow_over", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
		image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2rightarrow_down", NULL);
		g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
		forward_record_up = SWFButton_addCharacter(forward_button, (SWFCharacter) forward_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
		forward_record_over = SWFButton_addCharacter(forward_button, (SWFCharacter) forward_shape_over, SWFBUTTON_OVER);
		forward_record_down = SWFButton_addCharacter(forward_button, (SWFCharacter) forward_shape_down, SWFBUTTON_DOWN);

		// Add the forward action to the forward button
		if (debug_level)
		{
			// If we're debugging, then generate debugging swf's too
			forward_action = newSWFAction(
					"if (_root.this_slide >= (_root.num_slides - 1))"
					" {"
						// We're in the last slide, so we jump to the end of the movie
						" _root.reversing = false;"
						" trace(\"Fast forward button pressed while in last slide, slide counter is: \" + _root.this_slide + \".\");"
						" trace(\"'_root.reversing' variable has been set to false.\");"
						" trace(\"Now jumping to the last frame of the movie using gotoAndStop.\");"
						" _root.gotoAndStop(_root._totalframes);"  // Jump to the last frame of the movie
					" }"
					" else"
					" {"
						// We're not in the last slide yet, so jump to the start of the next slide
						" _root.reversing = false;"
						" trace(\"Fast forward button pressed, slide counter is: \" + _root.this_slide);"
						" trace(\"'_root.reversing' variable has been set to false.\");"
						" trace(\"We should now jump to the slide named '\" + _root.slide_names[_root.this_slide + 1] + \"'.\");"
						" if (true == _root.playing)"
						" {"
							" trace(\"Using gotoAndPlay.\");"
							" _root.gotoAndPlay(_root.slide_names[_root.this_slide + 1]);"
						" }"
						" else"
						" {"
							" trace(\"Using gotoAndStop.\");"
							" _root.gotoAndStop(_root.slide_names[_root.this_slide + 1]);"
						" }"
					" };");
		} else
		{
			forward_action = newSWFAction(
					"if (_root.this_slide >= (_root.num_slides - 1))"
					" {"
						// We're in the last slide, so we jump to the end of the movie
						" cb_play._visible = true;"
						" _root.reversing = false;"
						" _root.gotoAndStop(_root._totalframes);"  // Jump to the last frame of the movie
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
//	}

	// *** Create the Finish button ***

	// Load finish button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3rightarrow_up", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
	g_free(image_path);
	finish_shape_up = swf_shape_from_image_file(file_name_full->str, button_width, button_width);
	if (NULL == finish_shape_up)
	{
		// Loading images isn't working.
		destroySWFShape(cb_background);
		return FALSE;
	}

	// Load finish button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3rightarrow_over", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3rightarrow_down", NULL);
	g_string_printf(file_name_full, "%s.%s", image_path, icon_extension->str);
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
	finish_record_up = SWFButton_addCharacter(finish_button, (SWFCharacter) finish_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	finish_record_over = SWFButton_addCharacter(finish_button, (SWFCharacter) finish_shape_over, SWFBUTTON_OVER);
	finish_record_down = SWFButton_addCharacter(finish_button, (SWFCharacter) finish_shape_down, SWFBUTTON_DOWN);

	// Add the finish action to the finish button 
	if (debug_level)
	{
		// If we're debugging, then generate debugging swf's too
		finish_action = newSWFAction(
				"cb_play._visible = true;"
				" _root.this_slide = _root.num_slides - 1;"
				" _root.reversing = false;"
				" trace(\"Finish button pressed.\");"
				" trace(\"'_root.reversing' variable has been set to false.\");"
				" trace(\"Now jumping to last frame of movie.\");"
				" _root.gotoAndStop(_root._totalframes);");  // Jump to the last frame of the movie
	} else
	{
		finish_action = newSWFAction(
				"cb_play._visible = true;"
				" _root.this_slide = _root.num_slides - 1;"
				" _root.reversing = false;"
				" _root.gotoAndStop(_root._totalframes);");  // Jump to the last frame of the movie
	}
	SWFButton_addAction(finish_button, finish_action, SWFBUTTON_MOUSEUP);


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

// fixme3: Commented out until another, smaller, background image is added specific for this
//	if (1 < num_slides) // No need for a Rewind button if there's only one slide in the project
//	{
		// Add the rewind button to the control bar
		mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) rewind_button);
		SWFDisplayItem_setDepth(mc_display_item, 3);
		SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
		SWFDisplayItem_setName(mc_display_item, "cb_rewind");
		button_x = button_x + button_width + button_spacing;
//	}

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

// fixme3: Commented out until another, smaller, background image is added specific for this
//	if (1 < num_slides) // No need for a Forward button if there's only one slide in the project
//	{
		// Add the rewind button to the control bar
		mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) forward_button);
		SWFDisplayItem_setDepth(mc_display_item, 6);
		SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
		SWFDisplayItem_setName(mc_display_item, "cb_rewind");
		button_x = button_x + button_width + button_spacing;
//	}

	// Add the finish button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) finish_button);
	SWFDisplayItem_setDepth(mc_display_item, 7);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_finish");
	button_x = button_x + button_width + button_spacing;

	// Advance the movie clip one frame, else it won't be displayed
	SWFMovieClip_nextFrame(movie_clip);

	// Add the movie clip to the main movie
	buttons_display_item = SWFMovie_add(main_movie, movie_clip);

	// Name the movie clip
	SWFDisplayItem_setName(buttons_display_item, "cb_main");

	// Set the movie clip to be shown higher in the display stack than the main movie
	SWFDisplayItem_setDepth(buttons_display_item, 200);

	// Position the entire control bar object in the correct position onscreen
	SWFDisplayItem_moveTo(buttons_display_item, control_bar_x, control_bar_y);

	// Free the memory allocated in the function thus far
	g_string_free(file_name_full, TRUE);

	// Control bar was created successfully, so indicate this
	return TRUE;
}

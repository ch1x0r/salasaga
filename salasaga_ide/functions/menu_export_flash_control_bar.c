/*
 * $Id$
 *
 * Flame Project: Creates a swf control bar for the given swf movie 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
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


#define UNSCALED_BUTTON_HEIGHT 50
#define UNSCALED_BUTTON_WIDTH 50
#define UNSCALED_BUTTON_SPACING 5
#define UNSCALED_CONTROL_BAR_HEIGHT 60
#define UNSCALED_CONTROL_BAR_WIDTH 200


// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"
#include "swf_shape_from_image_file.h"


int menu_export_flash_control_bar(SWFMovie main_movie, gfloat height_scale_factor, gfloat width_scale_factor)
{
	// Create local variables
	guint				button_x;
	guint				button_y;
	SWFDisplayItem		buttons_display_item;
	gchar				*image_path;
	SWFFillStyle		light_yellow_fill;
	SWFDisplayItem		mc_display_item;
	SWFMovieClip		movie_clip;
	gfloat				scaled_button_height;
	gfloat				scaled_button_width;
	SWFAction			main_movie_action;

	// Variables used in working out control bar dimensions
	gfloat				scaled_control_bar_height;
	gfloat				scaled_control_bar_width;
	gfloat				control_bar_x;
	gfloat				control_bar_y;

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


	// Initialise various things
	button_x = 0;
	button_y = (height_scale_factor * 10);
	scaled_button_height = UNSCALED_BUTTON_HEIGHT * height_scale_factor;
	scaled_button_width = UNSCALED_BUTTON_WIDTH * width_scale_factor;
	scaled_control_bar_height = UNSCALED_CONTROL_BAR_HEIGHT * height_scale_factor;
	scaled_control_bar_width = UNSCALED_CONTROL_BAR_WIDTH * width_scale_factor;

	// For now, position the control bar in the middle of the screen, 90% of the way to the bottom
	control_bar_x = ((project_width - UNSCALED_CONTROL_BAR_WIDTH) / 2) * width_scale_factor;
	control_bar_y = ((project_height * 0.90) - UNSCALED_CONTROL_BAR_HEIGHT) * height_scale_factor;

	// Ensure the swf output starts out in the "stopped" state when played
	main_movie_action = newSWFAction("_root.stop();");
	SWFMovie_add(main_movie, (SWFBlock) main_movie_action);

	// Create a background for the control bar buttons to go on
	SWFShape cb_background = newSWFShape();
	light_yellow_fill = newSWFSolidFillStyle(0xff, 0xff, 0xf5, 0xff);
	SWFShape_setRightFillStyle(cb_background, light_yellow_fill);
	SWFShape_setLine(cb_background, 1, 0x00, 0x00, 0x00, 0xff);
	SWFShape_drawLine(cb_background, scaled_control_bar_width, 0);
	SWFShape_drawLine(cb_background, 0, scaled_control_bar_height);
	SWFShape_drawLine(cb_background, -(scaled_control_bar_width), 0);
	SWFShape_drawLine(cb_background, 0, -(scaled_control_bar_height));


	// *** Create the Restart button ***

	// Load restart button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3leftarrow_up.svg", NULL);
	restart_shape_up = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == restart_shape_up)
	{
		// Loading images isn't working.
		g_free(image_path);
		return FALSE;
	}

	// Load restart button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3leftarrow_over.svg", NULL);
	restart_shape_over = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == restart_shape_over)
	{
		// Loading images isn't working.
		g_free(image_path);
		destroySWFShape(restart_shape_up);
		return FALSE;
	}

	// Load restart button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3leftarrow_down.svg", NULL);
	restart_shape_down = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == restart_shape_down)
	{
		// Loading images isn't working.
		g_free(image_path);
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
	restart_action = newSWFAction("cb_play._visible = true; _root.gotoAndStop(1);");
	SWFButton_addAction(restart_button, restart_action, SWFBUTTON_MOUSEUP);


	// *** Create the Rewind button ***

	// Load rewind button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2leftarrow_up.svg", NULL);
	rewind_shape_up = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == rewind_shape_up)
	{
		// Loading images isn't working.
		g_free(image_path);
		return FALSE;
	}

	// Load rewind button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2leftarrow_over.svg", NULL);
	rewind_shape_over = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == rewind_shape_over)
	{
		// Loading images isn't working.
		g_free(image_path);
		destroySWFShape(rewind_shape_up);
		return FALSE;
	}

	// Load rewind button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2leftarrow_down.svg", NULL);
	rewind_shape_down = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == rewind_shape_down)
	{
		// Loading images isn't working.
		g_free(image_path);
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
	rewind_action = newSWFAction("cb_play._visible = true; _root.prevScene();");
	SWFButton_addAction(rewind_button, rewind_action, SWFBUTTON_MOUSEUP);


	// *** Create the Pause button ***

	// Load pause button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "player_pause_up.svg", NULL);
	pause_shape_up = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == pause_shape_up)
	{
		// Loading images isn't working.
		g_free(image_path);
		return FALSE;
	}

	// Load pause button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "player_pause_over.svg", NULL);
	pause_shape_over = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == pause_shape_over)
	{
		// Loading images isn't working.
		g_free(image_path);
		destroySWFShape(pause_shape_up);
		return FALSE;
	}

	// Load pause button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "player_pause_down.svg", NULL);
	pause_shape_down = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == pause_shape_down)
	{
		// Loading images isn't working.
		g_free(image_path);
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
	pause_action = newSWFAction("cb_play._visible = true; cb_pause._visible = false; _root.stop();");
	SWFButton_addAction(pause_button, pause_action, SWFBUTTON_MOUSEUP);


	// *** Create the Play button ***

	// Load play button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "1rightarrow_up.svg", NULL);
	play_shape_up = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == play_shape_up)
	{
		// Loading images isn't working.
		g_free(image_path);
		return FALSE;
	}

	// Load play button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "1rightarrow_over.svg", NULL);
	play_shape_over = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == play_shape_over)
	{
		// Loading images isn't working.
		g_free(image_path);
		destroySWFShape(play_shape_up);
		return FALSE;
	}

	// Load play button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "1rightarrow_down.svg", NULL);
	play_shape_down = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == play_shape_down)
	{
		// Loading images isn't working.
		g_free(image_path);
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
	play_action = newSWFAction("cb_pause._visible = true; cb_play._visible = false; _root.play();");
	SWFButton_addAction(play_button, play_action, SWFBUTTON_MOUSEUP);


	// *** Create the Fast Forward button ***

	// Load forward button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2rightarrow_up.svg", NULL);
	forward_shape_up = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == forward_shape_up)
	{
		// Loading images isn't working.
		g_free(image_path);
		return FALSE;
	}

	// Load forward button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2rightarrow_over.svg", NULL);
	forward_shape_over = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == forward_shape_over)
	{
		// Loading images isn't working.
		g_free(image_path);
		destroySWFShape(forward_shape_up);
		return FALSE;
	}

	// Load forward button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "2rightarrow_down.svg", NULL);
	forward_shape_down = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == forward_shape_down)
	{
		// Loading images isn't working.
		g_free(image_path);
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
	forward_action = newSWFAction("cb_play._visible = true; _root.gotoAndStop(_root._totalframes);");
	SWFButton_addAction(forward_button, forward_action, SWFBUTTON_MOUSEUP);


	// *** Create the Finish button ***

	// Load finish button's UP state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3rightarrow_up.svg", NULL);
	finish_shape_up = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == finish_shape_up)
	{
		// Loading images isn't working.
		g_free(image_path);
		return FALSE;
	}

	// Load finish button's OVER state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3rightarrow_over.svg", NULL);
	finish_shape_over = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == finish_shape_over)
	{
		// Loading images isn't working.
		g_free(image_path);
		destroySWFShape(finish_shape_up);
		return FALSE;
	}

	// Load finish button's DOWN state image
	image_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "control_bar", "3rightarrow_down.svg", NULL);
	finish_shape_down = swf_shape_from_image_file(image_path, scaled_button_width, scaled_button_height);
	if (NULL == finish_shape_down)
	{
		// Loading images isn't working.
		g_free(image_path);
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
	finish_action = newSWFAction("cb_play._visible = true; _root.gotoAndStop(_root._totalframes);");
	SWFButton_addAction(finish_button, finish_action, SWFBUTTON_MOUSEUP);


	// *** Add the buttons to a movie clip and attach it to the main movie ***

	// Embed the buttons in a movie clip
	movie_clip = newSWFMovieClip();

	// Add a background to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) cb_background);
	SWFDisplayItem_setDepth(mc_display_item, 1);
	SWFDisplayItem_setName(mc_display_item, "cb_background");
	button_x = button_x + (UNSCALED_BUTTON_SPACING * width_scale_factor);

	// Add the restart button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) restart_button);
	SWFDisplayItem_setDepth(mc_display_item, 2);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_restart");
	button_x = button_x + (UNSCALED_BUTTON_WIDTH * width_scale_factor);

/* Need to figure out how to make this work!
 
	// Add the rewind button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) rewind_button);
	SWFDisplayItem_setDepth(mc_display_item, 3);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_rewind");
	button_x = button_x + (UNSCALED_BUTTON_WIDTH * width_scale_factor);
*/
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
	button_x = button_x + (UNSCALED_BUTTON_WIDTH * width_scale_factor);

/* Need to figure out how to make this work!
	 
	// Add the rewind button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) forward_button);
	SWFDisplayItem_setDepth(mc_display_item, 6);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_rewind");
	button_x = button_x + (UNSCALED_BUTTON_WIDTH * width_scale_factor);
*/
	// Add the finish button to the control bar
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) finish_button);
	SWFDisplayItem_setDepth(mc_display_item, 7);
	SWFDisplayItem_moveTo(mc_display_item, button_x, button_y);
	SWFDisplayItem_setName(mc_display_item, "cb_finish");
	button_x = button_x + (UNSCALED_BUTTON_WIDTH * width_scale_factor);

	// Advance the movie clip one frame, else it won't be displayed
	SWFMovieClip_nextFrame(movie_clip);

	// Add the movie clip to the main movie
	buttons_display_item = SWFMovie_add(main_movie, movie_clip);

	// Set the movie clip to be shown higher in the display stack than the main movie
	SWFDisplayItem_setDepth(buttons_display_item, 200);

	// Position the movie clip in the center, 90% of the way down the screen
	SWFDisplayItem_moveTo(buttons_display_item, control_bar_x, control_bar_y);

	// Free the memory allocated in the function thus far
	g_free(image_path);

	// Control bar was created successfully, so indicate this
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2008/01/24 00:28:26  vapour
 *  + Updated all buttons to be loaded directly from file.  They now position correctly too.
 *  + Action Script is in place for the Rewind, Pause, Play, and Finish buttons.
 *  + Reverse and Forward buttons are commented out from display until I figure out the AS needed for them to work.
 *
 * Revision 1.3  2008/01/23 17:24:49  vapour
 * Mostly re-written.  Now uses the new function for directly loading a swf shape from filename, so the control bar should look graphically a lot better.  Lots of work still remaining however.
 *
 * Revision 1.2  2008/01/23 03:06:13  vapour
 * Added a background layer to the swf output control bar, for better visual contrast.
 *
 * Revision 1.1  2008/01/23 02:09:09  vapour
 * Added new function with initial working code to output a swf control bar.  Only has Stop and Play buttons at the moment (and they work), but it's a start.
 *
 */

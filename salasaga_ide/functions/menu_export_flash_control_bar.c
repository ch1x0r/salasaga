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
#define UNSCALED_CONTROL_BAR_HEIGHT 60
#define UNSCALED_CONTROL_BAR_WIDTH 200


// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


int menu_export_flash_control_bar(SWFMovie main_movie, gfloat height_scale_factor, gfloat width_scale_factor)
{
	// Create local variables
	SWFDisplayItem		buttons_display_item;
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

	// Fill styles we create
	SWFFillStyle		dark_blue_fill;
	SWFFillStyle		light_blue_fill;
	SWFFillStyle		light_yellow_fill;
	SWFFillStyle		green_fill;

	// Variables used for the play button
	SWFAction			play_action;
	SWFButton			play_button;
	SWFButtonRecord		play_record_down;
	SWFButtonRecord		play_record_over;
	SWFButtonRecord		play_record_up;
	SWFShape			play_shape_down;
	SWFShape			play_shape_over;
	SWFShape			play_shape_up;

	// Variables used for the stop button
	SWFAction			stop_action;
	SWFButton			stop_button;
	SWFButtonRecord		stop_record_down;
	SWFButtonRecord		stop_record_over;
	SWFButtonRecord		stop_record_up;
	SWFShape			stop_shape_down;
	SWFShape			stop_shape_over;
	SWFShape			stop_shape_up;


	// Initialise various things
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

	// Create the fill styles we'll be using
	light_blue_fill = newSWFSolidFillStyle(0x00, 0x00, 0xf0, 0xff);
	light_yellow_fill = newSWFSolidFillStyle(0xff, 0xff, 0xf5, 0xff);
	dark_blue_fill = newSWFSolidFillStyle(0x00, 0x00, 0x90, 0xff);
	green_fill = newSWFSolidFillStyle(0x00, 0xcc, 0x00, 0xff);

	// *** Create a background for the control bar buttons to go on ***

	SWFShape cb_background = newSWFShape();
	SWFShape_setRightFillStyle(cb_background, light_yellow_fill);
	SWFShape_setLine(cb_background, 1, 0x00, 0x00, 0x00, 0xff);
	SWFShape_movePenTo(cb_background, control_bar_x, control_bar_y);
	SWFShape_drawLine(cb_background, scaled_control_bar_width, 0);
	SWFShape_drawLine(cb_background, 0, scaled_control_bar_height);
	SWFShape_drawLine(cb_background, -(scaled_control_bar_width), 0);
	SWFShape_drawLine(cb_background, 0, -(scaled_control_bar_height));

	// *** Create the Rewind button ***

	// * We'll probably need a list of slide names first, so we can use
	// * Action Script to go back through the list

	// fixm2: Still needs to be done

	// *** Create the Stop button ***

	// Create a shape to be used in the stop button for its "UP" state
	stop_shape_up = newSWFShape();
	SWFShape_setRightFillStyle(stop_shape_up, dark_blue_fill);  // Use the dark blue fill
	SWFShape_setLine(stop_shape_up, 1, 0x00, 0x00, 0x00, 0xff);
	SWFShape_movePenTo(stop_shape_up, control_bar_x + (scaled_button_width * 0.1), control_bar_y + (scaled_button_height * 0.1));
	SWFShape_drawLine(stop_shape_up, scaled_button_width, 0);
	SWFShape_drawLine(stop_shape_up, 0, scaled_button_height);
	SWFShape_drawLine(stop_shape_up, -(scaled_button_width), 0);
	SWFShape_drawLine(stop_shape_up, 0, -(scaled_button_height));

	// Create a shape to be used in the stop button for its "MOUSE OVER" state
	stop_shape_over = newSWFShape();
	SWFShape_setRightFillStyle(stop_shape_over, light_blue_fill);  // Use the light blue fill
	SWFShape_setLine(stop_shape_over, 1, 0x00, 0x00, 0x00, 0xff);
	SWFShape_movePenTo(stop_shape_over, control_bar_x + (scaled_button_width * 0.1), control_bar_y + (scaled_button_height * 0.1));
	SWFShape_drawLine(stop_shape_over, scaled_button_width, 0);
	SWFShape_drawLine(stop_shape_over, 0, scaled_button_height);
	SWFShape_drawLine(stop_shape_over, -(scaled_button_width), 0);
	SWFShape_drawLine(stop_shape_over, 0, -(scaled_button_height));

	// Create a shape to be used in the stop button for its "DOWN" state
	stop_shape_down = newSWFShape();
	SWFShape_setRightFillStyle(stop_shape_down, green_fill);  // Use the green fill
	SWFShape_setLine(stop_shape_down, 1, 0x00, 0x00, 0x00, 0xff);
	SWFShape_movePenTo(stop_shape_down, control_bar_x + (scaled_button_width * 0.1), control_bar_y + (scaled_button_height * 0.1));
	SWFShape_drawLine(stop_shape_down, scaled_button_width, 0);
	SWFShape_drawLine(stop_shape_down, 0, scaled_button_height);
	SWFShape_drawLine(stop_shape_down, -(scaled_button_width), 0);
	SWFShape_drawLine(stop_shape_down, 0, -(scaled_button_height));

	// Create an empty button object we can use
	stop_button = newSWFButton();

	// Add the shapes to the button for its various states
	stop_record_up = SWFButton_addCharacter(stop_button, (SWFCharacter) stop_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	stop_record_over = SWFButton_addCharacter(stop_button, (SWFCharacter) stop_shape_over, SWFBUTTON_OVER);
	stop_record_down = SWFButton_addCharacter(stop_button, (SWFCharacter) stop_shape_down, SWFBUTTON_DOWN);

	// Add the Stop action to the stop button
	stop_action = newSWFAction("_root.stop();");
	SWFButton_addAction(stop_button, stop_action, SWFBUTTON_MOUSEUP);

	// *** Create the Play button ***

	// Create a shape to be used in the play button for its "UP" state
	play_shape_up = newSWFShape();
	SWFShape_setRightFillStyle(play_shape_up, dark_blue_fill);  // Use the dark blue fill
	SWFShape_setLine(play_shape_up, 1, 0x00, 0x00, 0x00, 0xff);
	SWFShape_movePenTo(play_shape_up, control_bar_x + (scaled_button_width * 1.2), control_bar_y + (scaled_button_height * 0.1));
	SWFShape_drawLine(play_shape_up, (scaled_button_width * 0.5), (scaled_button_height * 0.5));
	SWFShape_drawLine(play_shape_up, -(scaled_button_width * 0.5), (scaled_button_height * 0.5));
	SWFShape_drawLine(play_shape_up, 0, -(scaled_button_height));

	// Create a shape to be used in the play button for its "MOUSE OVER" state
	play_shape_over = newSWFShape();
	SWFShape_setRightFillStyle(play_shape_over, light_blue_fill);  // Use the light blue fill
	SWFShape_setLine(play_shape_over, 1, 0x00, 0x00, 0x00, 0xff);
	SWFShape_movePenTo(play_shape_over, control_bar_x + (scaled_button_width * 1.2), control_bar_y + (scaled_button_height * 0.1));
	SWFShape_drawLine(play_shape_over, (scaled_button_width * 0.5), (scaled_button_height * 0.5));
	SWFShape_drawLine(play_shape_over, -(scaled_button_width * 0.5), (scaled_button_height * 0.5));
	SWFShape_drawLine(play_shape_over, 0, -(scaled_button_height));

	// Create a shape to be used in the play button for its "DOWN" state
	play_shape_down = newSWFShape();
	SWFShape_setRightFillStyle(play_shape_down, green_fill);  // Use the green fill
	SWFShape_setLine(play_shape_down, 1, 0x00, 0x00, 0x00, 0xff);
	SWFShape_movePenTo(play_shape_down, control_bar_x + (scaled_button_width * 1.2), control_bar_y + (scaled_button_height * 0.1));
	SWFShape_drawLine(play_shape_down, (scaled_button_width * 0.5), (scaled_button_height * 0.5));
	SWFShape_drawLine(play_shape_down, -(scaled_button_width * 0.5), (scaled_button_height * 0.5));
	SWFShape_drawLine(play_shape_down, 0, -(scaled_button_height));

	// Create an empty button object we can use
	play_button = newSWFButton();

	// Add the shapes to the button for its various states
	play_record_up = SWFButton_addCharacter(play_button, (SWFCharacter) play_shape_up, SWFBUTTON_UP|SWFBUTTON_HIT);
	play_record_over = SWFButton_addCharacter(play_button, (SWFCharacter) play_shape_over, SWFBUTTON_OVER);
	play_record_down = SWFButton_addCharacter(play_button, (SWFCharacter) play_shape_down, SWFBUTTON_DOWN);

	// Add the Play action to the play button 
	play_action = newSWFAction("_root.play();");
	SWFButton_addAction(play_button, play_action, SWFBUTTON_MOUSEUP);

	// *** Create the Fast Forward button ***

	// * We'll probably need a list of slide names first, so we can use
	// * Action Script to go back through the list

	// fixm2: Still needs to be done

	// *** Add the buttons to a movie clip and attach it to the main movie ***

	// Embed the buttons in a movie clip
	movie_clip = newSWFMovieClip();
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) cb_background);
	SWFDisplayItem_setDepth(mc_display_item, 1);
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) stop_button);
	SWFDisplayItem_setDepth(mc_display_item, 2);
	mc_display_item = SWFMovieClip_add(movie_clip, (SWFBlock) play_button);
	SWFDisplayItem_setDepth(mc_display_item, 3);

	// Advance the movie clip one frame, else it won't be displayed
	SWFMovieClip_nextFrame(movie_clip);

	// Add the movie clip to the main movie
	buttons_display_item = SWFMovie_add(main_movie, movie_clip);

	// Set the movie clip to be shown higher in the display stack than the main movie
	SWFDisplayItem_setDepth(buttons_display_item, 200);

	// Control bar was created successfully, so indicate this
	return TRUE;
}

/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2008/01/23 03:06:13  vapour
 * Added a background layer to the swf output control bar, for better visual contrast.
 *
 * Revision 1.1  2008/01/23 02:09:09  vapour
 * Added new function with initial working code to output a swf control bar.  Only has Stop and Play buttons at the moment (and they work), but it's a start.
 *
 */

/*
 * $Id$
 *
 * Salasaga: Turns an individual element of a swf frame element array into swf display list blocks 
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


// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"

gboolean export_swf_process_element(SWFMovie this_movie, swf_frame_element *this_element, gboolean process_removes)
{
	// Local variables
	GString				*as_gstring;				// Used for constructing action script statements
	SWFDisplayItem		display_list_object;		// Temporary display list object
	guint				new_opacity;				// Used when constructing the opacity value for an object
	SWFAction			swf_action;					// Used when constructing action script
	layer				*this_layer_info;			// Used to point to layer data when looping


	// Initialisation
	as_gstring = g_string_new(NULL);

	// Is this a frame in which some action needs to take place?
	if (TRUE == this_element->action_this)
	{
		// * There is something to be done in this frame for this layer *

		this_layer_info = this_element->layer_info;

		// Is this the frame in which the layer is added to the display?
		if (TRUE == this_element->add)
		{
			// * Add the character to the swf display list *
			display_list_object = SWFMovie_add(this_movie, (SWFBlock) this_layer_info->dictionary_shape);

			// Store the display list object for future reference
			this_layer_info->display_list_item = display_list_object;

			// Ensure the object has it's name set, so we can reference it with action script
			SWFDisplayItem_setName(display_list_object, this_element->object_name->str);

			// Ensure the object is at the correct display depth
			SWFDisplayItem_setDepth(display_list_object, this_element->depth);

			// Position the object
			SWFDisplayItem_moveTo(display_list_object, this_element->x_position, this_element->y_position);

			// Ensure the object is visible
			g_string_printf(as_gstring, "_root.%s._visible = true;",
					this_element->object_name->str);
			swf_action = compileSWFActionCode(as_gstring->str);
			SWFMovie_add(this_movie, (SWFBlock) swf_action);

		}

		// Is this the frame in which the layer is removed from the display?
		if ((TRUE == this_element->remove) && (TRUE == process_removes))
		{
			// * Remove the character from the swf display list *

			// Get the appropriate display list object
			display_list_object = this_layer_info->display_list_item;

			// Remove the character from the display list
			g_string_printf(as_gstring, "_root.%s._visible = false; _root.%s._alpha = 0; _root.%s._x = %.2f; _root.%s._y = %.2f;",
					this_element->object_name->str,
					this_element->object_name->str,
					this_element->object_name->str,
					this_element->x_position,
					this_element->object_name->str,
					this_element->y_position);
			swf_action = compileSWFActionCode(as_gstring->str);
			SWFMovie_add(this_movie, (SWFBlock) swf_action);

			// Display debugging info if requested
			if (get_debug_level())
				printf("Remove string: '%s'\n", as_gstring->str);
		}

		// Does the layer have an opacity change that needs to be actioned in this frame?
		if (TRUE == this_element->opacity_change)
		{
			// Get the appropriate display list object
			display_list_object = this_layer_info->display_list_item;

			// Set the opacity level for the object
			new_opacity = this_element->opacity;
			g_string_printf(as_gstring, " _root.%s._x = %.2f; _root.%s._y = %.2f; _root.%s._alpha = %d;",
					this_element->object_name->str,
					this_element->x_position,
					this_element->object_name->str,
					this_element->y_position,
					this_element->object_name->str,
					new_opacity);
			swf_action = compileSWFActionCode(as_gstring->str);
			SWFMovie_add(this_movie, (SWFBlock) swf_action);

			// Display debugging info if requested
			//if (get_debug_level())
			//	printf("Opacity string: '%s'\n", as_gstring->str);
		}

		// Does the layer need to be moved/positioned in this frame?
		if (TRUE == this_element->is_moving)
		{
			// * Adjust the x and y position of the character on the display list *

			// Get the appropriate display list object
			display_list_object = this_layer_info->display_list_item;

			// (Re-)position the object
			g_string_printf(as_gstring, "_root.%s._x = %.2f; _root.%s._y = %.2f;",
					this_element->object_name->str,
					this_element->x_position,
					this_element->object_name->str,
					this_element->y_position);
			swf_action = compileSWFActionCode(as_gstring->str);
			SWFMovie_add(this_movie, (SWFBlock) swf_action);

			// Display debugging info if requested
			//if (get_debug_level())
			//	printf("Move string: '%s'\n", as_gstring->str);
		}

		// Does this layer have a mouse click sound needing to be started in this frame?
		if (MOUSE_NONE != this_element->click_sound_to_play)
		{
			switch (this_element->click_sound_to_play)
			{
				case MOUSE_LEFT_ONE:
				case MOUSE_RIGHT_ONE:
				case MOUSE_MIDDLE_ONE:

					swf_action = compileSWFActionCode("_root.mousesingleclick.gotoAndPlay(1);");
					SWFMovie_add(this_movie, (SWFBlock) swf_action);
					break;

				case MOUSE_LEFT_DOUBLE:
				case MOUSE_RIGHT_DOUBLE:
				case MOUSE_MIDDLE_DOUBLE:

					swf_action = compileSWFActionCode("_root.mousedoubleclick.gotoAndPlay(1);");
					SWFMovie_add(this_movie, (SWFBlock) swf_action);
					break;

				case MOUSE_LEFT_TRIPLE:
				case MOUSE_RIGHT_TRIPLE:
				case MOUSE_MIDDLE_TRIPLE:

					swf_action = compileSWFActionCode("_root.mousetripleclick.gotoAndPlay(1);");
					SWFMovie_add(this_movie, (SWFBlock) swf_action);
					break;
			}
		}
	}

	return TRUE;	
}

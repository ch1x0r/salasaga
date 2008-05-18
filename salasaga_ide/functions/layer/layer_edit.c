/*
 * $Id$
 *
 * Salasaga: Function called when the user clicks the Edit Layer toolbar button 
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_dialog_empty.h"
#include "display_dialog_highlight.h"
#include "display_dialog_image.h"
#include "display_dialog_mouse.h"
#include "display_dialog_text.h"
#include "display_warning.h"
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "film_strip_create_thumbnail.h"
#include "widgets/time_line.h"


void layer_edit(void)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gfloat 				layer_total_time;			// Total length in time that a layer is displayed
	guint				num_layers;					// Number of layers
	gboolean			return_code;				// Did the edit dialog return ok?
	guint				selected_row;				// Holds the row that is selected
	slide				*slide_data;				// Pointer to current slide data
	layer				*tmp_layer;					// Temporary layer


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise some variables
	slide_data = (slide *) current_slide->data;
	layer_pointer = slide_data->layers;

	// Determine the number of layers present in this slide
	layer_pointer = g_list_first(layer_pointer);
	num_layers = slide_data->num_layers;

	// Determine which layer the user has selected in the timeline
	selected_row = time_line_get_selected_layer_num(slide_data->timeline_widget);
	tmp_layer = g_list_nth_data(layer_pointer, selected_row);

	// * Open a dialog box showing the existing values, asking for the new ones *

	switch (tmp_layer->object_type)
	{
		case TYPE_EMPTY:
			// Open a dialog box for the user to edit the background layer values
			return_code = display_dialog_empty(tmp_layer, "Edit background color");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// Invalidate the cached background pixmap for the slide
				slide_data->cached_pixmap_valid = FALSE;

				// Set the changes made variable
				changes_made = TRUE;
			}

			// Nothing to update in the timeline for empty layers
			break;

		case TYPE_GDK_PIXBUF:
			// Open a dialog box for the user to edit the image layer values
			return_code = display_dialog_image(tmp_layer, "Edit image layer");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// If the new layer end time is longer than the slide duration, then extend the duration of the slide and background image
				layer_total_time = tmp_layer->start_time + tmp_layer->duration + tmp_layer->transition_in_duration + tmp_layer->transition_out_duration; 
				if (layer_total_time > slide_data->duration)
				{
					// Change the slide duration
					slide_data->duration = layer_total_time;

					// Change the background layer duration
					tmp_layer = g_list_nth_data(layer_pointer, slide_data->num_layers - 1);
					tmp_layer->duration = layer_total_time;
				}

				// Set the changes made variable
				changes_made = TRUE;
			}
			break;

		case TYPE_MOUSE_CURSOR:
			// Open a dialog box for the user to edit the mouse pointer values
			return_code = display_dialog_mouse(tmp_layer, "Edit mouse pointer", FALSE);
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// If the new layer end time is longer than the slide duration, then extend the slide duration
				layer_total_time = tmp_layer->start_time + tmp_layer->duration + tmp_layer->transition_in_duration + tmp_layer->transition_out_duration; 
				if (layer_total_time > slide_data->duration)
				{
					// Change the slide duration
					slide_data->duration = layer_total_time;

					// Change the background layer duration
					tmp_layer = g_list_nth_data(layer_pointer, slide_data->num_layers - 1);
					tmp_layer->duration = layer_total_time;
				}

				// Set the changes made variable
				changes_made = TRUE;
			}
			break;

		case TYPE_TEXT:

			// Open a dialog box for the user to edit the text layer values
			return_code = display_dialog_text(tmp_layer, "Edit text layer");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// If the new layer end time is longer than the slide duration, then extend the slide duration
				layer_total_time = tmp_layer->start_time + tmp_layer->duration + tmp_layer->transition_in_duration + tmp_layer->transition_out_duration; 
				if (layer_total_time > slide_data->duration)
				{
					// Change the slide duration
					slide_data->duration = layer_total_time;

					// Change the background layer duration
					tmp_layer = g_list_nth_data(layer_pointer, slide_data->num_layers - 1);
					tmp_layer->duration = layer_total_time;
				}

				// Set the changes made variable
				changes_made = TRUE;
			}
			break;

		case TYPE_HIGHLIGHT:

			// Open a dialog box for the user to edit the highlight layer values
			return_code = display_dialog_highlight(tmp_layer, "Edit highlight layer");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// If the new layer end time is longer than the slide duration, then extend the slide duration
				layer_total_time = tmp_layer->start_time + tmp_layer->duration + tmp_layer->transition_in_duration + tmp_layer->transition_out_duration; 
				if (layer_total_time > slide_data->duration)
				{
					// Change the slide duration
					slide_data->duration = layer_total_time;

					// Change the background layer duration
					tmp_layer = g_list_nth_data(layer_pointer, slide_data->num_layers - 1);
					tmp_layer->duration = layer_total_time;
				}

				// Set the changes made variable
				changes_made = TRUE;
			}
			break;

		default:
			display_warning("Error ED34: Unknown layer type");
			return;
	}

	// Regenerate the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail(slide_data);

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Layer edited");
	gdk_flush();
}

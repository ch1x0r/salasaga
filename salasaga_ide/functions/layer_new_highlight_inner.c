/*
 * $Id$
 *
 * Flame Project: Displays a dialog box asking for the values required to make a new highlight layer
 * 
 * Copyright (C) 2008 Justin Clift <justin@postgresql.org>
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_dialog_highlight.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"
#include "regenerate_timeline_duration_images.h"


void layer_new_highlight_inner(guint release_x, guint release_y)
{
	// Local variables
	guint				finish_frame;				// Used when working out a layer's finish frame
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				mouse_drag_height;			// The height the mouse was dragged
	gint				mouse_drag_width;			// The width the mouse was dragged
	gboolean			return_code;				// Catches a TRUE/FALSE return value
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio
	gfloat				scaled_x;					// Scaled starting coordinate
	gfloat				scaled_y;					// Scaled starting coordinate
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	slide				*slide_data;				// Pointer to the data for the current slide

	layer_highlight		*tmp_highlight_ob;			// Temporary highlight layer object
	GtkTreeIter			*tmp_iter;					// Temporary iter
	layer				*tmp_layer;					// Temporary layer
	GtkTreePath			*tmp_path;					// Temporary GtkPath


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Calculate the height and width scaling values for the main drawing area at its present size
	scaled_height_ratio = (gfloat) project_height / (gfloat) main_drawing_area->allocation.height;
	scaled_width_ratio = (gfloat) project_width / (gfloat) main_drawing_area->allocation.width;

	// Work out where the mouse is positioned
	scaled_x = stored_x * scaled_width_ratio;
	scaled_y = stored_y * scaled_height_ratio;
	mouse_drag_width = roundf((gfloat) (release_x - stored_x) * scaled_width_ratio);
	mouse_drag_height = roundf((gfloat) (release_y - stored_y) * scaled_height_ratio);

	// * Create a new highlight layer in memory using reasonable defaults *

	// Simplify pointing to the current slide structure in memory
	slide_data = current_slide->data;

	// Create the highlight layer data
	tmp_highlight_ob = g_new(layer_highlight, 1);
	tmp_highlight_ob->width = mouse_drag_width;
	tmp_highlight_ob->height = mouse_drag_height;

	// Constuct the new highlight layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_HIGHLIGHT;
	tmp_layer->object_data = (GObject *) tmp_highlight_ob;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_data->duration;
	tmp_layer->x_offset_start = roundf(scaled_x);
	tmp_layer->y_offset_start = roundf(scaled_y);
	tmp_layer->x_offset_finish = roundf(scaled_x);
	tmp_layer->y_offset_finish = roundf(scaled_y);
	tmp_layer->visible = TRUE;
	tmp_layer->background = FALSE;
	tmp_layer->name = g_string_new("Highlight");
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->external_link_window = g_string_new("_self");

	// Display a dialog box to edit these values, using our new highlight layer object
	return_code = display_dialog_highlight(tmp_layer, "Add new highlight layer");
	if (TRUE != return_code)
	{
		// The user cancelled out of the dialog box, so destroy our new layer and return
		g_string_free(tmp_layer->name, TRUE);
		g_string_free(tmp_layer->external_link, TRUE);
		g_string_free(tmp_layer->external_link_window, TRUE);
		g_free(tmp_layer);
		g_free(tmp_highlight_ob);

		// Change the cursor back to normal
		gdk_window_set_cursor(main_drawing_area->window, NULL);

		// Redraw the workspace
		draw_workspace();

		// Update the status bar
		gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " New layer cancelled");
		gdk_flush();

		return;
	}

	// * To get here, the user must have clicked OK in the create highlight layer dialog box, so we process the results *

	// Add the new layer to the slide
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);

	// Simplify the ending frame for this layer
	finish_frame = tmp_layer->finish_frame;

	// If the user gave a finish frame that's longer than the present slide duration, we need to increase the slide duration to match
	if (finish_frame > slide_data->duration)
	{
		// Update slide duration data
		slide_data->duration = finish_frame;
	}

	// Add the new layer to slide list store
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	gtk_list_store_prepend(slide_data->layer_store, tmp_iter);
	gtk_list_store_set(slide_data->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_DURATION, NULL,
						TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
						TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
						TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
						-1);

	// Regenerate the timeline duration images
	regenerate_timeline_duration_images(slide_data);

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();

	// Select the new layer in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(slide_data->timeline_widget), tmp_path, NULL, FALSE);

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Highlight layer added");
	gdk_flush();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/03/05 12:48:59  vapour
 * Updated to support double buffering.
 *
 * Revision 1.2  2008/03/05 11:01:13  vapour
 * Added code to change cursor back to normal upon cancellation, and update the status bar appropriately.
 *
 * Revision 1.1  2008/03/05 10:09:35  vapour
 * Moved the code for the highlight, mouse, and text layers into an inner version of each function.
 *
 */

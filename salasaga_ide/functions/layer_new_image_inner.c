/*
 * $Id$
 *
 * Flame Project: Displays a dialog box asking for the values required to make a new image layer
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


// GTK includes
#include <gtk/gtk.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_dialog_image.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"
#include "regenerate_timeline_duration_images.h"


void layer_new_image_inner(guint release_x, guint release_y)
{
	// Local variables
	guint				finish_frame;				// Used when working out a layer's finish frame
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gboolean			return_code;				// Catches a TRUE/FALSE return value
	slide				*slide_data;				// Pointer to the data for the current slide

	layer_image			*tmp_image_ob;				// Temporary image layer object
	GtkTreeIter			*tmp_iter;					// Temporary iter
	layer				*tmp_layer;					// Temporary layer
	GtkTreePath			*tmp_path;					// Temporary GtkPath

	
	// * Create a new image layer in memory using reasonable defaults *
	
	// Simplify pointing to the current slide structure in memory
	slide_data = current_slide->data;

	// Create the image layer data
	tmp_image_ob = g_new(layer_image, 1);
	tmp_image_ob->width = 400;
	tmp_image_ob->height = 300;
	tmp_image_ob->modified = FALSE;

	// Constuct the new image layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_GDK_PIXBUF;
	tmp_layer->object_data = (GObject *) tmp_image_ob;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_data->duration;
	tmp_layer->x_offset_start = release_x;
	tmp_layer->y_offset_start = release_y;
	tmp_layer->x_offset_finish = release_x;
	tmp_layer->y_offset_finish = release_y;
	tmp_layer->visible = TRUE;
	tmp_layer->background = FALSE;
	tmp_layer->name = g_string_new("Image");
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->external_link_window = g_string_new("_self");

	// Display a dialog box to edit these values, using our new image layer object
	return_code = display_dialog_image(tmp_layer, "Add new image layer", TRUE);
	if (TRUE != return_code)
	{
		// The user cancelled out of the dialog box, so destroy our new layer and return
		g_string_free(tmp_layer->name, TRUE);
		g_string_free(tmp_layer->external_link, TRUE);
		g_string_free(tmp_layer->external_link_window, TRUE);
		g_free(tmp_layer);
		g_free(tmp_image_ob);

		// Change the cursor back to normal
		gdk_window_set_cursor(main_drawing_area->window, NULL);

		return;
	}

	// Change the cursor back to normal
	gdk_window_set_cursor(main_drawing_area->window, NULL);

	// * To get here, the user must have clicked OK in the dialog box, so we process the results *

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
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Image layer added");
	gdk_flush();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2008/03/05 10:15:15  vapour
 * Added a function description.
 *
 * Revision 1.1  2008/03/05 09:15:06  vapour
 * Moved most of the code from the layer new image function into here.
 *
 */

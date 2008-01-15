/*
 * $Id$
 *
 * Flame Project: Function called when the user clicks the Add Mouse Pointer toolbar button 
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "create_timeline_slider.h"
#include "display_dialog_mouse.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"
#include "sound_beep.h"


void layer_new_mouse(void)
{
	// Local variables
	guint				finish_frame;				// Used when working out a layer's finish frame
	gfloat				finish_pixel;				// Ending slider pixel to fill in
	GdkPixbuf			*layer_pixbuf;				// Pointer used when creating duration images for layers
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gfloat				pixel_width;				// Width of pixels to fill
	gboolean			return_code;				// Catches a TRUE/FALSE return value
	slide				*slide_data;				// Pointer to the data for the current slide
	guint				start_frame;				// Used when working out a layer's start frame
	gfloat				start_pixel;				// Starting slider pixel to fill in

	layer_mouse			*tmp_mouse_ob;				// Temporary mouse layer object
	GtkTreeIter			*tmp_iter;				// Temporary iter
	layer				*tmp_layer;				// Temporary layer
	GtkTreePath			*tmp_path;				// Temporary GtkPath


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// * Create a new mouse layer in memory using reasonable defaults *

	// Create the mouse pointer layer data
	tmp_mouse_ob = g_new(layer_mouse, 1);
	tmp_mouse_ob->x_offset_start = 100;
	tmp_mouse_ob->y_offset_start = 100;
	tmp_mouse_ob->x_offset_finish = 100;
	tmp_mouse_ob->y_offset_finish = 100;
	tmp_mouse_ob->width = 22;
	tmp_mouse_ob->height = 32;
	tmp_mouse_ob->click = MOUSE_NONE;
	tmp_mouse_ob->image_path = g_string_new("");

	// Constuct the new mouse pointer layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_MOUSE_CURSOR;
	tmp_layer->object_data = (GObject *) tmp_mouse_ob;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = ((slide *) current_slide->data)->duration;
	tmp_layer->name = g_string_new("Mouse Pointer");
	tmp_layer->external_link = g_string_new(NULL);

	// Display a dialog box to edit these values, using our new mouse pointer layer object
	return_code = display_dialog_mouse(tmp_layer, "Add mouse pointer", TRUE);
	if (TRUE != return_code)
	{
		// The user cancelled out of the dialog box, so destroy our new layer and return
		g_string_free(tmp_layer->name, TRUE);
		g_free(tmp_layer);
		g_free(tmp_mouse_ob);
		return;
	}

	// * To get here, the user must have clicked OK in the dialog box, so we process the results *

	// Add the new layer to the slide
	slide_data = current_slide->data;
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);

	// Work out the start and ending frames for this layer
	start_frame = tmp_layer->start_frame;
	finish_frame = tmp_layer->finish_frame;

	// Calculate the duration of the layer for drawing inside the slider
	start_pixel = 180 * ((gfloat) start_frame / (gfloat) slide_data->duration);
	finish_pixel = 180 * ((gfloat) finish_frame / (gfloat) slide_data->duration);
	pixel_width = finish_pixel - start_pixel;

	// Create duration image
	layer_pixbuf = NULL;
	layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, start_pixel, pixel_width);

	// Add the new layer to slide list store
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	gtk_list_store_prepend(((slide *) current_slide->data)->layer_store, tmp_iter);
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_DURATION, layer_pixbuf,
						TIMELINE_X_OFF_START, tmp_mouse_ob->x_offset_start,
						TIMELINE_Y_OFF_START, tmp_mouse_ob->y_offset_start,
						TIMELINE_X_OFF_FINISH, tmp_mouse_ob->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, tmp_mouse_ob->y_offset_finish,
						-1);

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();

	// Select the new layer in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

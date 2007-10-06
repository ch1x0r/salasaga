/*
 * $Id$
 *
 * Flame Project: Function called when the user clicks the Crop Image toolbar button 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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
#include "display_warning.h"
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"
#include "sound_beep.h"


void image_crop(void)
{
	// Local variables
	GtkDialog			*crop_dialog;				// Widget for the dialog
	GtkWidget			*crop_table;				// Table used for neat layout of the dialog box
	guint				row_counter = 0;			// Used to count which row things are up to
	gint				dialog_result;				// Catches the return code from the dialog box

	GList				*layer_pointer;				// Points to the layers in the selected slide
	GtkWidget			*list_widget;				// Points to the timeline widget
	gint				new_height;				// Hold the height of the cropped area
	GdkPixbuf			*new_pixbuf;				// Holds the cropped image data
	gint				new_width;				// Hold the width of the cropped area
	guint				selected_row;				// Holds the number of the row that is selected
	layer				*this_layer;				// Temporary layer

	GtkWidget			*left_label;				// Label widget
	GtkWidget			*left_button;				//

	GtkWidget			*right_label;				// Label widget
	GtkWidget			*right_button;				//

	GtkWidget			*top_label;				// Label widget
	GtkWidget			*top_button;				//

	GtkWidget			*bottom_label;				// Label widget
	GtkWidget			*bottom_button;				//

	GtkTreeViewColumn		*tmp_column;				// Temporary column
	gint				tmp_int;				// Temporary int
	GtkTreePath			*tmp_path;				// Temporary path
	GdkPixbuf			*tmp_pixbuf;				// Temporary pixbuf


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// Initialise some variables
	layer_pointer = ((slide *) current_slide->data)->layers;
	list_widget = ((slide *) current_slide->data)->timeline_widget;

	// * Check if the selected layer is an image *

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	layer_pointer = g_list_first(layer_pointer);
	this_layer = g_list_nth_data(layer_pointer, selected_row);

	// Is this layer an image?
	if (TYPE_GDK_PIXBUF != this_layer->object_type)
	{
		// Give the user feedback
		sound_beep();
		display_warning("ED36: Only Image layers can be cropped");
		return;
	}

	// Is this layer the background?
	tmp_int = g_ascii_strncasecmp(this_layer->name->str, "Background", 10);
	if (0 == tmp_int)
	{
		// Give the user feedback
		sound_beep();
		display_warning("ED37: Background layers can not be cropped");
		return;
	}

	// * Pop open a dialog box asking the user how much to crop off each side of the image *

	// Create the dialog window, and table to hold its children
	crop_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Crop image layer", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	crop_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(crop_dialog->vbox), GTK_WIDGET(crop_table), FALSE, FALSE, 10);

	// Create the label asking for the left side crop amount
	left_label = gtk_label_new("Left crop: ");
	gtk_misc_set_alignment(GTK_MISC(left_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(left_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the left side crop amount
	left_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(left_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(left_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	right_label = gtk_label_new("Right crop: ");
	gtk_misc_set_alignment(GTK_MISC(right_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(right_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the right side crop amount
	right_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(right_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(right_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the top crop amount
	top_label = gtk_label_new("Top crop: ");
	gtk_misc_set_alignment(GTK_MISC(top_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(top_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the left side crop amount
	top_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(top_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(top_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	bottom_label = gtk_label_new("Bottom crop: ");
	gtk_misc_set_alignment(GTK_MISC(bottom_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(bottom_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the right side crop amount
	bottom_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(bottom_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(bottom_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(crop_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(crop_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// The dialog was cancelled, so destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(crop_dialog));
		return;
	}

	// Create a new pixbuf, for storing the cropped image in
	new_height = gdk_pixbuf_get_height(((layer_image *) this_layer->object_data)->image_data)
		- (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(top_button))
		- (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(bottom_button));
	new_width = gdk_pixbuf_get_width(((layer_image *) this_layer->object_data)->image_data)
		- (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(left_button))
		- (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(right_button));
	new_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, new_width, new_height);

	// Create a new pixbuf, having just the cropped image data in it
	gdk_pixbuf_copy_area(((layer_image *) this_layer->object_data)->image_data, // Source pixbuf
		(gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(left_button)),  // Left crop
		(gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(top_button)),  // Top crop
		new_width,  // Width
		new_height,  // Height
		new_pixbuf,  // Destination
		0, 0);

	// Update the layer with the new cropped data
	tmp_pixbuf = ((layer_image *) this_layer->object_data)->image_data;
	((layer_image *) this_layer->object_data)->image_data = new_pixbuf;
	((layer_image *) this_layer->object_data)->width = new_width;
	((layer_image *) this_layer->object_data)->height = new_height;
	g_string_assign(((layer_image *) this_layer->object_data)->image_path, "");
	((layer_image *) this_layer->object_data)->modified = TRUE;

	// Free the memory used by the old pixbuf
	g_object_unref(tmp_pixbuf);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(crop_dialog));

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

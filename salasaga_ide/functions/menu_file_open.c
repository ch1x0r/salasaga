/*
 * $Id$
 *
 * Flame Project: Function called when the user selects File -> Open from the top menu 
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


void menu_file_open(void)
{
	// Local variables
	GtkFileFilter			*all_filter;
	gchar				*filename;				// Pointer to the chosen file name
	GtkFileFilter			*flame_filter;
	GtkTreePath			*new_path;				// Temporary path
	GtkWidget 			*open_dialog;
	gboolean			return_code;

	GString				*tmp_gstring;				// Temporary gstring


	// Create the dialog asking the user to select a Flame Project file
	open_dialog = gtk_file_chooser_dialog_new("Open a Flame Project",
						  GTK_WINDOW(main_window),
						  GTK_FILE_CHOOSER_ACTION_OPEN,
						  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						  NULL);

	// Create the filter so only *.flame files are displayed
	flame_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(flame_filter, "*.flame");
	gtk_file_filter_set_name(flame_filter, "Flame Project file (*.flame)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(open_dialog), flame_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, "All files (*.*)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(open_dialog), all_filter);

	// Set the path and name of the file to open
	if (NULL != file_name)
	{
		// Select the last opened file if possible
		gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(open_dialog), file_name->str);
	} else
	{
		// Change to the default project directory
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(open_dialog), default_project_folder->str);
	}

	// Run the dialog and wait for user input
	if (gtk_dialog_run(GTK_DIALOG(open_dialog)) != GTK_RESPONSE_ACCEPT)
	{
		// * The user didn't choose a file, so destroy the dialog box and return *
		gtk_widget_destroy(open_dialog);
		return;		
	}

	// * We only get to here if a file was chosen *

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Get the filename from the dialog box
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(open_dialog));

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(open_dialog);

	// Open and parse the selected file
	return_code = flame_read(filename);

	// Keep the full file name around for future reference
	file_name = g_string_new(NULL);
	file_name = g_string_assign(file_name, filename);

	// Destroy the existing output resolution selector
	g_signal_handler_disconnect(G_OBJECT(resolution_selector), resolution_callback);
	gtk_container_remove(GTK_CONTAINER(message_bar), GTK_WIDGET(resolution_selector));

	// Create a new output resolution selector, including the resolution of the loaded project
	resolution_selector = GTK_COMBO_BOX(create_resolution_selector(res_array, num_res_items, output_width, output_height));
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_selector), 5, 6, 0, 1);
	resolution_callback = g_signal_connect(G_OBJECT(resolution_selector), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL);
	gtk_widget_show_all(GTK_WIDGET(message_bar));

	// Use the status bar to communicate the successful loading of the project
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "Project '%s' successfully loaded.", filename);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// Make the current slide point to the first slide
	current_slide = slides;

	// Create the tooltips for the slides
	create_tooltips();

	// Draw the timeline area
	draw_timeline();

	// Draw the workspace area
	draw_workspace();

	// Scroll the film strip to show the new thumbnail position
	new_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.0, 0.0);
//	gtk_tree_path_free(new_path);

	// Enable the project based menu items
	menu_enable("/Slide", TRUE);
	menu_enable("/Layer", TRUE);
	menu_enable("/Export", TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();

	// Frees the memory allocated in this function
	g_free(filename);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:14  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

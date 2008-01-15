/*
 * $Id$
 *
 * Flame Project: Function called when the user selects File -> New from the top menu 
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
#include "destroy_slide.h"
#include "disable_layer_toolbar_buttons.h"
#include "disable_main_toolbar_buttons.h"
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "enable_layer_toolbar_buttons.h"
#include "enable_main_toolbar_buttons.h"
#include "menu_enable.h"
#include "slide_insert.h"


void menu_file_new(void)
{
	// Local variables
	GtkDialog			*project_dialog;		// Widget for the dialog
	GtkWidget			*project_table;			// Table used for neat layout of the dialog box
	guint				row_counter = 0;		// Used to count which row things are up to
	gint				dialog_result;			// Catches the return code from the dialog box

	GtkWidget			*name_label;			// Label widget
	GtkWidget			*name_entry;			// Widget for accepting the name of the new project

	GtkWidget			*width_label;			// Label widget
	GtkWidget			*width_button;			//

	GtkWidget			*height_label;			// Label widget
	GtkWidget			*height_button;			//

	GtkWidget			*fps_label;				// Label widget
	GtkWidget			*fps_button;			//

	GtkWidget			*bg_color_label;		// Label widget
	GtkWidget			*bg_color_button;		// Background color selection button


	// * Pop open a dialog box asking the user for the details of the new project *

	// Create the dialog window, and table to hold its children
	project_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Create new Flame Project", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	project_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(project_dialog->vbox), GTK_WIDGET(project_table), FALSE, FALSE, 10);

	// Create the label asking for the new project name
	name_label = gtk_label_new("Project Name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the new project name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), 20);
	gtk_entry_set_text(GTK_ENTRY(name_entry), project_name->str);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the project width
	width_label = gtk_label_new("Width: ");
	gtk_misc_set_alignment(GTK_MISC(width_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(width_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the project width
	width_button = gtk_spin_button_new_with_range(0, 6000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(width_button), 1280);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(width_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the project height
	height_label = gtk_label_new("Height: ");
	gtk_misc_set_alignment(GTK_MISC(height_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(height_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the project height
	height_button = gtk_spin_button_new_with_range(0, 6000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(height_button), 1024);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(height_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the number of frames per second
	fps_label = gtk_label_new("Frames per second: ");
	gtk_misc_set_alignment(GTK_MISC(fps_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(fps_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the number of frames per second
	fps_button = gtk_spin_button_new_with_range(0, 48, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(fps_button), frames_per_second);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(fps_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label next to the color swatch
	bg_color_label = gtk_label_new("Background color: ");
	gtk_misc_set_alignment(GTK_MISC(bg_color_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(bg_color_label), 0, 1, row_counter, row_counter + 1);

	// Create the background color selection button
	bg_color_button = gtk_color_button_new_with_color(&default_bg_colour);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(bg_color_button), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(bg_color_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(project_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(project_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// The dialog was cancelled, so destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(project_dialog));
		return;
	}

	// If there's a project presently loaded in memory, we unload it
	if (NULL != slides)
	{
		// Free the resources presently allocated to slides
		g_list_foreach(slides, destroy_slide, NULL);

		// Re-initialise pointers
		slides = NULL;
		current_slide = NULL;
	}

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// If there's an existing film strip, we unload it
	gtk_list_store_clear(GTK_LIST_STORE(film_strip_store));	

	// Set the project name
	g_string_printf(project_name, "%s", gtk_entry_get_text(GTK_ENTRY(name_entry)));

	// Set the project width
	project_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(width_button));

	// Set the project height
	project_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(height_button));

	// Set the number of frames per second
	frames_per_second = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(fps_button));

	// Set the default background color
	gtk_color_button_get_color(GTK_COLOR_BUTTON(bg_color_button), &default_bg_colour);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(project_dialog));

	// Create a blank slide to start things from
	slide_insert();
	current_slide = slides;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, "New project created.");
	gdk_flush();

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Enable the project based menu items
	menu_enable("/Slide", TRUE);
	menu_enable("/Layer", TRUE);
	menu_enable("/Export", TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

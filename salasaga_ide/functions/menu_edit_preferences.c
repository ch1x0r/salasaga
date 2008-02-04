/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Edit -> Preferences from the top menu 
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


void menu_edit_preferences(void)
{
	// Local variables
	GtkDialog			*main_dialog;						// Widget for the main dialog
	GtkWidget			*app_dialog_table;					// Table used for neat layout of the labels and fields in application preferences
	gint				app_row_counter;					// Used when building the application preferences dialog box
	gint				dialog_result;						// Catches the return code from the dialog box

	GtkWidget			*label_default_project_folder;		// Default Project Folder
	GtkWidget			*button_default_project_folder;		//

	GtkWidget			*label_screenshot_folder;			// Screenshot Folder
	GtkWidget			*button_screenshot_folder;			//

	GtkWidget			*label_default_output_folder;		// Default Output path
	GtkWidget			*button_default_output_folder;		//

	GtkWidget			*label_default_output_width;		// Default Output Width
	GtkWidget			*button_default_output_width;		//

	GtkWidget			*label_default_output_height;		// Default Output Height
	GtkWidget			*button_default_output_height;		//

	GtkWidget			*label_default_output_quality;		// Default Output Quality
	GtkWidget			*button_default_output_quality;		//

	GtkWidget			*label_default_slide_length;		// Default Slide Length
	GtkWidget			*button_default_slide_length;		//

	GtkWidget			*label_preview_width;				// Preview Width
	GtkWidget			*button_preview_width;				//

	GtkWidget			*label_icon_height;					// Icon Height
	GtkWidget			*button_icon_height;				//

	GtkWidget			*label_default_zoom_level;			// Default Zoom Level
	GtkWidget			*entry_default_zoom_level;			//

	GtkWidget			*label_default_bg_colour;			// Default background colour
	GtkWidget			*button_default_bg_colour;			// Color button

	GtkWidget			*label_scaling_quality;				// Scaling quality
	GtkWidget			*button_scaling_quality;			//


	// Initialise various things
	app_row_counter = 0;

	// Create the main dialog window
	main_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Application Preferences", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	app_dialog_table = gtk_table_new(10, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(main_dialog->vbox), GTK_WIDGET(app_dialog_table), FALSE, FALSE, 5);

	// Default Project Folder
	label_default_project_folder = gtk_label_new("Default Project Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_project_folder), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_project_folder), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_project_folder = gtk_file_chooser_button_new("Select the Default Project Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_default_project_folder), default_project_folder->str);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_project_folder), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Screenshot Folder
	label_screenshot_folder = gtk_label_new("Screenshots Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_screenshot_folder), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_screenshot_folder), 0, 1, app_row_counter, app_row_counter + 1);
	button_screenshot_folder = gtk_file_chooser_button_new("Select the Screenshot Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_screenshot_folder), screenshots_folder->str);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_screenshot_folder), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Output Folder
	label_default_output_folder = gtk_label_new("Default Output Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_folder), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_folder), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_output_folder = gtk_file_chooser_button_new("Select the Default Output Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_default_output_folder), default_output_folder->str);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_folder), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Output Width
	label_default_output_width = gtk_label_new("Default Output Width: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_width), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_width), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_output_width = gtk_spin_button_new_with_range(0, 6000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_output_width), default_output_width);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_width), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Output Height
	label_default_output_height = gtk_label_new("Default Output Height: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_height), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_height), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_output_height = gtk_spin_button_new_with_range(0, 6000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_output_height), default_output_height);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_height), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Output Quality
	label_default_output_quality = gtk_label_new("Default Output Quality: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_quality), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_quality), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_output_quality = gtk_spin_button_new_with_range(0, 9, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_output_quality), default_output_quality);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_quality), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Slide Length
	label_default_slide_length = gtk_label_new("Default Slide Length: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_slide_length), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_slide_length), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_slide_length = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_slide_length), default_slide_length);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_slide_length), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Preview width
	label_preview_width = gtk_label_new("Preview Width: ");
	gtk_misc_set_alignment(GTK_MISC(label_preview_width), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_preview_width), 0, 1, app_row_counter, app_row_counter + 1);
	button_preview_width = gtk_spin_button_new_with_range(0, 1200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_preview_width), preview_width);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_preview_width), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Icon Height
	label_icon_height = gtk_label_new("Icon Height: ");
	gtk_misc_set_alignment(GTK_MISC(label_icon_height), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_icon_height), 0, 1, app_row_counter, app_row_counter + 1);
	button_icon_height = gtk_spin_button_new_with_range(0, 200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_icon_height), icon_height);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_icon_height), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Zoom level
	label_default_zoom_level = gtk_label_new("Default Zoom Level: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_zoom_level), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_zoom_level), 0, 1, app_row_counter, app_row_counter + 1);
	// fixme3: We don't have a selector for Default Zoom Level yet, so this is just stub code for now
	entry_default_zoom_level = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_default_zoom_level), 14);
	gtk_entry_set_text(GTK_ENTRY(entry_default_zoom_level), gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector)));
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(entry_default_zoom_level), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Background Colour
	label_default_bg_colour = gtk_label_new("Default Background Colour: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_bg_colour), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_bg_colour), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_bg_colour = gtk_color_button_new_with_color(&default_bg_colour);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button_default_bg_colour), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_bg_colour), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Scaling Quality
	label_scaling_quality = gtk_label_new("Scaling Quality: ");
	gtk_misc_set_alignment(GTK_MISC(label_scaling_quality), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_scaling_quality), 0, 1, app_row_counter, app_row_counter + 1);
	button_scaling_quality = gtk_spin_button_new_with_range(0, 3, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_scaling_quality), scaling_quality);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_scaling_quality), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Set the dialog going
	gtk_widget_show_all(GTK_WIDGET(main_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(main_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT == dialog_result)
	{
		// * Yes, so update the application variables with their new values *

		// fixme3: No validation is done on this input yet.  To make a secure application, it really needs to be

		// Default Project Folder
		default_project_folder = g_string_assign(default_project_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_default_project_folder)));

		// Default Project Folder
		screenshots_folder = g_string_assign(screenshots_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_screenshot_folder)));

		// Default Output Folder
		default_output_folder = g_string_assign(default_output_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_default_output_folder)));

		// Default Output Width
		// fixme2: This should be a selector, like the output selector
		default_output_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_output_width));

		// Default Output Height
		default_output_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_output_height));

		// Default Output Quality
		default_output_quality = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_output_quality));

		// Default Slide Length
		default_slide_length = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_slide_length));

		// Preview width
		preview_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_preview_width));

		// Icon Height
		icon_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_icon_height));

		// Default Zoom Level
		// fixme3: Couldn't be bothered coding this yet
//		default_zoom_level = g_string_assign(default_zoom_level, gtk_entry_get_text(GTK_ENTRY(entry_default_zoom_level)));

		// Default Background Colour
		gtk_color_button_get_color(GTK_COLOR_BUTTON(button_default_bg_colour), &default_bg_colour);

		// Scaling Quality
		scaling_quality = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_scaling_quality));
	}

	// Free up the memory allocated in this function
	// fixme2: Once this function works properly, I should revisit this code and ensure nothing got missed in the cleanup
	gtk_widget_destroy(GTK_WIDGET(label_default_project_folder));
	gtk_widget_destroy(GTK_WIDGET(button_default_project_folder));
	gtk_widget_destroy(GTK_WIDGET(label_default_output_folder));
	gtk_widget_destroy(GTK_WIDGET(button_default_output_folder));
	gtk_widget_destroy(GTK_WIDGET(main_dialog));
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/02/04 10:33:29  vapour
 * Removed the code that went into the new Project Properties function.
 *
 * Revision 1.2  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 04:22:13  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

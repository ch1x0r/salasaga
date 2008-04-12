/*
 * $Id$
 *
 * Salasaga: Opens a dialog box asking the user for the initial information for a new project. 
 *                Called when the user selects File -> New from the top menu.
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "disable_layer_toolbar_buttons.h"
#include "disable_main_toolbar_buttons.h"
#include "display_dialog_save_warning.h"
#include "display_warning.h"
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "enable_layer_toolbar_buttons.h"
#include "enable_main_toolbar_buttons.h"
#include "menu_enable.h"
#include "slide_free.h"
#include "slide_insert.h"
#include "validate_value.h"


void menu_file_new(void)
{
	// Local variables
	guint				guint_val;					// Used in the input validation process
	GdkColor			new_bg_colour;				// Received the new background color for the project
	GtkTreePath			*new_path;					// Path used to select the new film strip thumbnail
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	GtkDialog			*project_dialog;			// Widget for the dialog
	GtkWidget			*project_table;				// Table used for neat layout of the dialog box
	gboolean			return_code_gbool;			// Catches gboolean return codes
	guint				row_counter = 0;			// Used to count which row things are up to
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	guint				valid_fps;					// Receives the new project fps once validated
	guint				valid_height;				// Receives the new project height once validated
	GString				*valid_proj_name;			// Receives the new project name once validated
	guint				valid_width;				// Receives the new project width once validated
	guint				*validated_guint;			// Receives known good guint values from the validation function 
	GString				*validated_string;			// Receives known good strings from the validation function

	GtkWidget			*name_label;				// Label widget
	GtkWidget			*name_entry;				// Widget for accepting the name of the new project

	GtkWidget			*width_label;				// Label widget
	GtkWidget			*width_button;				//

	GtkWidget			*height_label;				// Label widget
	GtkWidget			*height_button;				//

	GtkWidget			*fps_label;					// Label widget
	GtkWidget			*fps_button;				//

	GtkWidget			*bg_color_label;			// Label widget
	GtkWidget			*bg_color_button;			// Background color selection button


	// If we have unsaved changes, warn the user
	if (TRUE == changes_made)
	{
		return_code_gbool = display_dialog_save_warning();
		if (FALSE == return_code_gbool)
		{
			// The user pressed Cancel in the dialog
			return;
		}
	}

	// Initialise some things
	valid_proj_name = g_string_new(NULL);

	// Create the dialog window, and table to hold its children
	project_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Create new Salasaga Project", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	project_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(project_dialog->vbox), GTK_WIDGET(project_table), FALSE, FALSE, 10);

	// Create the label asking for the new project name
	name_label = gtk_label_new("Project Name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the new project name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), valid_fields[PROJECT_NAME].max_value);
	gtk_entry_set_text(GTK_ENTRY(name_entry), project_name->str);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the project width
	width_label = gtk_label_new("Width: ");
	gtk_misc_set_alignment(GTK_MISC(width_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(width_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the project width
	width_button = gtk_spin_button_new_with_range(0, valid_fields[PROJECT_WIDTH].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(width_button), 1280);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(width_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the project height
	height_label = gtk_label_new("Height: ");
	gtk_misc_set_alignment(GTK_MISC(height_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(height_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the project height
	height_button = gtk_spin_button_new_with_range(0, valid_fields[PROJECT_HEIGHT].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(height_button), 1024);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(height_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label asking for the number of frames per second
	fps_label = gtk_label_new("Frames per second: ");
	gtk_misc_set_alignment(GTK_MISC(fps_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(fps_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the entry that accepts the number of frames per second
	fps_button = gtk_spin_button_new_with_range(0, valid_fields[PROJECT_FPS].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(fps_button), default_fps);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(fps_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Create the label next to the color swatch
	bg_color_label = gtk_label_new("Background color: ");
	gtk_misc_set_alignment(GTK_MISC(bg_color_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(bg_color_label), 0, 1, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);

	// Create the background color selection button
	bg_color_button = gtk_color_button_new_with_color(&default_bg_colour);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(bg_color_button), TRUE);
	gtk_table_attach(GTK_TABLE(project_table), GTK_WIDGET(bg_color_button), 1, 2, row_counter, row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	row_counter = row_counter + 1;

	// Ensure everything will show
	gtk_widget_show_all(GTK_WIDGET(project_dialog));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the dialog
		if (GTK_RESPONSE_ACCEPT != gtk_dialog_run(GTK_DIALOG(project_dialog)))
		{
			// The dialog was cancelled, so destroy it and return to the caller
			g_string_free(valid_proj_name, TRUE);
			gtk_widget_destroy(GTK_WIDGET(project_dialog));
			return;
		}

		// Reset the useable input flag
		useable_input = TRUE;

		// Validate the project name input
		validated_string = validate_value(PROJECT_NAME, V_CHAR, (gchar *) gtk_entry_get_text(GTK_ENTRY(name_entry)));
		if (NULL == validated_string)
		{
			display_warning("Error ED118: There was something wrong with the new project name.  Defaulting to 'New Project' instead.");
			g_string_assign(valid_proj_name, "New Project");
		} else
		{
			g_string_assign(valid_proj_name, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Validate the project width
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(width_button));
		validated_guint = validate_value(PROJECT_WIDTH, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED120: There was something wrong with the project width value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_width = *validated_guint;
			g_free(validated_guint);
		}

		// Validate the project height
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(height_button));
		validated_guint = validate_value(PROJECT_HEIGHT, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED121: There was something wrong with the project height value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_height = *validated_guint;
			g_free(validated_guint);
		}

		// Validate the project fps
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(fps_button));
		validated_guint = validate_value(PROJECT_FPS, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED122: There was something wrong with the project frames per second value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_fps = *validated_guint;
			g_free(validated_guint);
		}
	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Get the new background colour
	gtk_color_button_get_color(GTK_COLOR_BUTTON(bg_color_button), &new_bg_colour);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(project_dialog));

	// If there's a project presently loaded in memory, we unload it
	if (NULL != slides)
	{
		// Free the resources presently allocated to slides
		g_list_foreach(slides, slide_free, NULL);

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
	g_string_assign(project_name, valid_proj_name->str);
	g_string_free(valid_proj_name, TRUE);

	// Set the project width
	project_width = valid_width;

	// Set the project height
	project_height = valid_height;

	// Set the number of frames per second
	frames_per_second = valid_fps;

	// Set the default background color
	default_bg_colour = new_bg_colour;

	// Create an initial blank slide for the project
	slide_insert();
	current_slide = slides;

	// Select the thumbnail for the new slide in the film strip
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(film_strip_view), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	new_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.5, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Set the global toggle that a project is now active
	project_active = TRUE;

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Project initialised");
	gdk_flush();

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Enable the project based menu items
	menu_enable("/Project", TRUE);
	menu_enable("/Slide", TRUE);
	menu_enable("/Layer", TRUE);
	menu_enable("/Export", TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();
}

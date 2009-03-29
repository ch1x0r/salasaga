/*
 * $Id$
 *
 * Salasaga: Function called when the user selects File -> Open from the top menu
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../create_resolution_selector.h"
#include "../disable_layer_toolbar_buttons.h"
#include "../disable_main_toolbar_buttons.h"
#include "../draw_timeline.h"
#include "../enable_layer_toolbar_buttons.h"
#include "../enable_main_toolbar_buttons.h"
#include "../project_read.h"
#include "../resolution_selector_changed.h"
#include "../validate_value.h"
#include "../dialog/display_dialog_save_warning.h"
#include "../dialog/display_warning.h"
#include "menu_enable.h"


void menu_file_open(void)
{
	// Local variables
	GtkFileFilter		*all_filter;
	gchar				*filename;					// Pointer to the chosen file name
	GtkFileFilter		*flame_filter;
	GString				*message;					// Used to construct message strings
	GtkTreePath			*new_path;					// Temporary path
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	GtkWidget 			*open_dialog;
	gboolean			return_code = FALSE;
	gint				return_code_gint;			// Used to catch a return code
	GtkFileFilter		*salasaga_filter;
	gboolean			useable_input;				// Used to control loop flow
	GString				*validated_string;			// Receives known good strings from the validation function


	// If we have unsaved changes, warn the user
	if (TRUE == changes_made)
	{
		return_code = display_dialog_save_warning(TRUE);
		if (FALSE == return_code)
		{
			// The user pressed Cancel in the dialog
			return;
		}
	}

	// Initialisation
	message = g_string_new(NULL);

	// Create the dialog asking the user to select a Salasaga Project file
	open_dialog = gtk_file_chooser_dialog_new(_("Open a Salasaga Project"),
						  GTK_WINDOW(main_window),
						  GTK_FILE_CHOOSER_ACTION_OPEN,
						  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						  NULL);

	// Create a filter for *.salasaga files
	salasaga_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(salasaga_filter, "*.salasaga");
	gtk_file_filter_set_name(salasaga_filter, _("Salasaga Project file (*.salasaga)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(open_dialog), salasaga_filter);

	// Create a filter for *.flame files
	flame_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(flame_filter, "*.flame");
	gtk_file_filter_set_name(flame_filter, _("Flame Project file (*.flame)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(open_dialog), flame_filter);

	// Create a filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, _("All files (*.*)"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(open_dialog), all_filter);

	// Set the path and name of the file to open
	if (NULL != file_name)
	{
		// Select the last opened file if possible
		return_code = gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(open_dialog), file_name->str);

		// We couldn't successfully open the given project file
		if (FALSE == return_code)
		{
			// Change to the default project directory instead
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(open_dialog), default_project_folder->str);
		} else
		{
			// If the selected project file has a ".flame" extension, we use the flame filter
			return_code = g_str_has_suffix(file_name->str, ".flame");
			if (TRUE == return_code)
			{
				gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(open_dialog), flame_filter);
			} else
			{
				// Not a flame project file, so we need to decide whether to use the all filter (*.*) or the salasaga one
				return_code = g_str_has_suffix(file_name->str, ".salasaga");
				if (TRUE != return_code)
				{
					// Doesn't have a ".salasaga" file extension either, so we'll use the all filter
					gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(open_dialog), all_filter);
				}
			}
		}
	} else
	{
		// Change to the default project directory
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(open_dialog), default_project_folder->str);
	}

	// Loop around until we have a valid filename or the user cancels out
	useable_input = FALSE;
	validated_string = NULL;
	do
	{
		// Run the dialog and wait for user input
		return_code_gint = gtk_dialog_run(GTK_DIALOG(open_dialog));
		if (return_code_gint != GTK_RESPONSE_ACCEPT)
		{
			// The user didn't choose a file, so destroy the dialog box and return
			gtk_widget_destroy(open_dialog);
			g_string_free(message, TRUE);
			return;
		}

		// Get the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(open_dialog));

		// Validate the filename input
		validated_string = validate_value(FILE_PATH, V_CHAR, filename);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED124: %s", _("Error"), _("There was something wrong with the file name given.  Please try again."));
			display_warning(message->str);
		} else
		{
			// Open and parse the selected file
			return_code = project_read(validated_string->str);
			if (TRUE == return_code)
			{
				// The file was read in fine, so we continue
				useable_input = TRUE;
			}
			else
			{
				// The project file wasn't valid, so we abort the dialog and return
				gtk_widget_destroy(open_dialog);
				g_string_free(validated_string, TRUE);
				g_string_free(message, TRUE);
				g_free(filename);
				return;
			}
		}
	} while (FALSE == useable_input);

	// * We only get to here if a valid file was chosen *

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(open_dialog);

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Keep the full file name around for future reference
	if (NULL == file_name)
	{
		// We don't have a global file_name variable yet, so we create it
		file_name = g_string_new(NULL);
	}
	file_name = g_string_assign(file_name, validated_string->str);

	// Destroy the existing output resolution selector
	g_signal_handler_disconnect(G_OBJECT(resolution_selector), resolution_callback);
	gtk_container_remove(GTK_CONTAINER(message_bar), GTK_WIDGET(resolution_selector));

	// Create a new output resolution selector, including the resolution of the loaded project
	resolution_selector = GTK_COMBO_BOX(create_resolution_selector(output_width, output_height));
	gtk_table_attach(message_bar, GTK_WIDGET(resolution_selector), 5, 6, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
	resolution_callback = g_signal_connect(G_OBJECT(resolution_selector), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL);
	gtk_widget_show_all(GTK_WIDGET(message_bar));

	// Set the global toggle that a project is now active
	project_active = TRUE;

	// Make the current slide point to the first slide
	current_slide = slides;

	// Calculate and set the display size of the working area
	working_width = (project_width * zoom) / 100;
	working_height = (project_height * zoom) / 100;

	// Resize the drawing area so it draws properly
	gtk_widget_set_size_request(GTK_WIDGET(main_drawing_area), working_width, working_height);

	// Free the existing front store for the workspace
	if (NULL != front_store)
	{
		g_object_unref(GDK_PIXMAP(front_store));
		front_store = NULL;
	}

	// Redraw the workspace
	draw_workspace();

	// Draw the timeline area
	draw_timeline();

	// Scroll the film strip to show the new thumbnail position
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(film_strip_view), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	new_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.0, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path
	if (NULL != new_path)
		gtk_tree_path_free(new_path);  // Free the new path

	// Enable the project based menu items
	menu_enable(_("/Project"), TRUE);
	menu_enable(_("/Slide"), TRUE);
	menu_enable(_("/Layer"), TRUE);
	menu_enable(_("/Export"), TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();

	// Clear the changes made variable
	changes_made = FALSE;

	// Use the status bar to communicate the successful loading of the project
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Project loaded"));
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(status_bar), 0.0);

	// Free the memory allocated in this function
	// (note that salasaga_filter, flame_filter and the all_filter seem to be freed when the dialog is destroyed)
	g_string_free(message, TRUE);
	g_string_free(validated_string, TRUE);
	g_free(filename);
}

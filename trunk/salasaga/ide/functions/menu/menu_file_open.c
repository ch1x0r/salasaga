/*
 * $Id$
 *
 * Salasaga: Function called when the user selects File -> Open from the top menu
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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
#include "../../../config.h"
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_dialog_save_warning.h"
#include "../dialog/display_warning.h"
#include "../other/validate_value.h"
#include "../preference/application_preferences.h"
#include "../preference/project_preferences.h"
#include "../read/read_project.h"
#include "../resolution_selector/create_resolution_selector.h"
#include "../resolution_selector/resolution_selector_changed.h"
#include "../time_line/draw_timeline.h"
#include "../time_line/time_line_set_selected_layer_num.h"
#include "../tool_bars/disable_layer_toolbar_buttons.h"
#include "../tool_bars/disable_main_toolbar_buttons.h"
#include "../tool_bars/enable_layer_toolbar_buttons.h"
#include "../tool_bars/enable_main_toolbar_buttons.h"
#include "../undo_redo/undo_functions.h"
#include "../working_area/draw_workspace.h"
#include "menu_enable.h"


void menu_file_open(void)
{
	// Local variables
	GtkFileFilter		*all_filter;
	gchar				*dir_name;					// Temporarily holds the name of the directory being saved into
	gchar				*filename;					// Pointer to the chosen file name
	GtkFileFilter		*flame_filter;
	GString				*message;					// Used to construct message strings
	GtkTreePath			*new_path;					// Temporary path
	guint				num_slides = 0;				// Receives the number of slides loaded from a project
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	GtkWidget 			*open_dialog;
	gboolean			return_code = FALSE;
	gint				return_code_gint;			// Used to catch a return code
	GtkFileFilter		*salasaga_filter;
	gboolean			usable_input;				// Used to control loop flow
	GString				*validated_string;			// Receives known good strings from the validation function


	// If we have unsaved changes, warn the user
	if (TRUE == get_changes_made())
	{
		return_code = display_dialog_save_warning();
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
						  GTK_WINDOW(get_main_window()),
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
	if (NULL != get_file_name())
	{
		// Select the last opened file if possible
		return_code = gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(open_dialog), get_file_name());

		// We couldn't successfully open the given project file
		if (FALSE == return_code)
		{
			// Change to the default project directory instead
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(open_dialog), get_default_project_folder());
		} else
		{
			// If the selected project file has a ".flame" extension, we use the flame filter
			return_code = g_str_has_suffix(get_file_name(), ".flame");
			if (TRUE == return_code)
			{
				gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(open_dialog), flame_filter);
			} else
			{
				// Not a flame project file, so we need to decide whether to use the all filter (*.*) or the salasaga one
				return_code = g_str_has_suffix(get_file_name(), ".salasaga");
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
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(open_dialog), get_default_project_folder());
	}

	// Loop around until we have a valid filename or the user cancels out
	usable_input = FALSE;
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
		validated_string = (GString*)validate_value(FILE_PATH, V_CHAR, filename);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED124: %s", _("Error"), _("There was something wrong with the file name given.  Please try again."));
			display_warning(message->str);
		} else
		{
			// Open and parse the selected file
			return_code = read_project(validated_string->str, &num_slides);
			if (TRUE == return_code)
			{
				// The file was read in fine, so we continue
				usable_input = TRUE;
			}
			else
			{
				// The project file wasn't valid, so we abort the dialog
				gtk_widget_destroy(open_dialog);

				// Use the status bar to communicate the failed loading of the project
				g_string_printf(message, " %s - %s", _("Project load aborted"), g_path_get_basename(validated_string->str));
				gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), message->str);
				gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);

				// Return
				g_string_free(validated_string, TRUE);
				g_string_free(message, TRUE);
				g_free(filename);
				return;
			}
		}
	} while (FALSE == usable_input);

	// * We only get to here if a valid file was chosen *

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(open_dialog);

	// Update the project folder variable with this new path
	dir_name = g_path_get_dirname(validated_string->str);
	set_project_folder(dir_name);
	g_free(dir_name);

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Keep the full file name for future reference
	set_file_name(validated_string->str);

	// Change the title bar to include the project name
	g_string_printf(message, "%s v%s - %s", APP_NAME, APP_VERSION, g_path_get_basename(get_file_name()));
	gtk_window_set_title(GTK_WINDOW(get_main_window()), message->str);

	// Destroy the existing output resolution selector
	g_signal_handler_disconnect(G_OBJECT(get_resolution_selector()), get_resolution_callback());
	gtk_container_remove(GTK_CONTAINER(get_message_bar()), GTK_WIDGET(get_resolution_selector()));

	// Create a new output resolution selector, including the resolution of the loaded project
	set_resolution_selector(GTK_COMBO_BOX(create_resolution_selector(get_output_width(), get_output_height())));
	gtk_table_attach(get_message_bar(), GTK_WIDGET(get_resolution_selector()), 5, 6, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
	set_resolution_callback(g_signal_connect(G_OBJECT(get_resolution_selector()), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL));
	gtk_widget_show_all(GTK_WIDGET(get_message_bar()));

	// Set the global toggle that a project is now active
	set_project_active(TRUE);

	// Make the current slide point to the first slide
	set_current_slide(get_slides());

	// Calculate and set the display size of the working area
	set_working_width((get_project_width() * get_zoom()) / 100);
	set_working_height((get_project_height() * get_zoom()) / 100);

	// Resize the drawing area so it draws properly
	gtk_widget_set_size_request(GTK_WIDGET(get_main_drawing_area()), get_working_width(), get_working_height());

	// Free the existing front store for the workspace
	if (NULL != get_front_store())
	{
		g_object_unref(GDK_PIXMAP(get_front_store()));
		set_front_store(NULL);
	}

	// Redraw the workspace
	draw_workspace();

	// Draw the timeline area
	draw_timeline();
	time_line_set_selected_layer_num(get_current_slide_timeline_widget(), 0);

	// Scroll the film strip to show the new thumbnail position
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(get_film_strip_view()), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	new_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, TRUE, 0.0, 0.0);
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
	set_changes_made(FALSE);

	// Clear the undo/redo history
	undo_history_clear();

	// Use the status bar to communicate the successful loading of the project
	g_string_printf(message, " %s - %u %s", _("Project loaded"), num_slides, _("slides"));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), message->str);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);

	// Free the memory allocated in this function
	// (note that salasaga_filter, flame_filter and the all_filter seem to be freed when the dialog is destroyed)
	g_string_free(message, TRUE);
	g_string_free(validated_string, TRUE);
	g_free(filename);
}

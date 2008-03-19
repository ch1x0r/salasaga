/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Edit -> Preferences from the top menu 
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

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "create_resolution_selector.h"
#include "create_zoom_selector.h"
#include "display_warning.h"
#include "regenerate_film_strip_thumbnails.h"
#include "validate_value.h"


void menu_edit_preferences(void)
{
	// Local variables
	GtkWidget			*app_dialog_table;					// Table used for neat layout of the labels and fields in application preferences
	gint				app_row_counter;					// Used when building the application preferences dialog box
	guint				guint_val;							// Temporary guint value used for validation
	GValue				*handle_size;						// The size of the dividing handle for the film strip
	GtkDialog			*main_dialog;						// Widget for the main dialog
	gchar				**strings;							// Text string are split apart with this
	GString				*tmp_gstring;						// Text strings are constructed in this
	gboolean			useable_input;						// Used to control loop flow
	guint				valid_default_fps;					// Receives the new default fps once validated
	guint				valid_icon_height;					// Receives the new icon height once validated
	GString				*valid_output_folder;				// Receives the new output folder once validated
	GString				*valid_output_resolution;			// Receives the new default output resolution once validated
	guint				valid_preview_width;				// Receives the new film strip thumbnail width once validated
	GString				*valid_project_folder;				// Receives the new default project folder once validated
	GString				*valid_screenshot_folder;			// Receives the new screenshot folder once validated
	guint				valid_slide_length;					// Receives the new default slide length once validated
	GString				*valid_zoom_level;					// Receives the new default zoom level once validated
	guint				*validated_guint;					// Receives known good guint values from the validation function 
	GString				*validated_string;					// Receives known good strings from the validation function

	
	GtkWidget			*label_default_project_folder;		// Default Project Folder
	GtkWidget			*button_default_project_folder;		//

	GtkWidget			*label_screenshot_folder;			// Screenshot Folder
	GtkWidget			*button_screenshot_folder;			//

	GtkWidget			*label_default_output_folder;		// Default Output Path
	GtkWidget			*button_default_output_folder;		//

	GtkWidget			*label_default_output_res;			// Default Output Resolution
	GtkWidget			*selector_default_output_res;		//

	GtkWidget			*label_default_slide_length;		// Default Slide Length
	GtkWidget			*button_default_slide_length;		//

	GtkWidget			*label_default_fps;					// Default Frames Per Second
	GtkWidget			*button_default_fps;				//

	GtkWidget			*label_preview_width;				// Preview Width
	GtkWidget			*button_preview_width;				//

	GtkWidget			*label_icon_height;					// Icon Height
	GtkWidget			*button_icon_height;				//

	GtkWidget			*label_default_zoom_level;			// Default Zoom Level
	GtkWidget			*selector_default_zoom_level;		//

	GtkWidget			*label_default_bg_colour;			// Default background colour
	GtkWidget			*button_default_bg_colour;			// Color button


	// Initialise various things
	app_row_counter = 0;
	tmp_gstring = g_string_new(NULL);
	valid_project_folder = g_string_new(NULL);
	valid_screenshot_folder = g_string_new(NULL);
	valid_output_folder = g_string_new(NULL);
	valid_output_resolution = g_string_new(NULL);
	valid_zoom_level = g_string_new(NULL);

	// Create the main dialog window
	main_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Application Preferences", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	app_dialog_table = gtk_table_new(10, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(main_dialog->vbox), GTK_WIDGET(app_dialog_table), FALSE, FALSE, 5);

	// Default Project Folder
	label_default_project_folder = gtk_label_new("Default Project Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_project_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_project_folder), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_default_project_folder = gtk_file_chooser_button_new("Select the Default Project Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_default_project_folder), default_project_folder->str);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_project_folder), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Screenshot Folder
	label_screenshot_folder = gtk_label_new("Screenshots Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_screenshot_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_screenshot_folder), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_screenshot_folder = gtk_file_chooser_button_new("Select the Screenshot Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_screenshot_folder), screenshots_folder->str);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_screenshot_folder), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Default Output Folder
	label_default_output_folder = gtk_label_new("Default Output Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_folder), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_default_output_folder = gtk_file_chooser_button_new("Select the Default Output Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_default_output_folder), default_output_folder->str);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_folder), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Default Output Resolution
	label_default_output_res = gtk_label_new("Default Output Resolution: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_res), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_res), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_default_output_res = GTK_WIDGET(create_resolution_selector(default_output_width, default_output_height));
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(selector_default_output_res), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Default Slide Length
	label_default_slide_length = gtk_label_new("Default Slide Length: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_slide_length), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_slide_length), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_default_slide_length = gtk_spin_button_new_with_range(0, valid_fields[SLIDE_LENGTH].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_slide_length), default_slide_length);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_slide_length), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Default Frames Per Second
	label_default_fps = gtk_label_new("Default Frames Per Second: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_fps), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_fps), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_default_fps = gtk_spin_button_new_with_range(0, valid_fields[PROJECT_FPS].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_fps), default_fps);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_fps), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Preview width
	label_preview_width = gtk_label_new("Film Strip Width: ");
	gtk_misc_set_alignment(GTK_MISC(label_preview_width), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_preview_width), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_preview_width = gtk_spin_button_new_with_range(0, valid_fields[PREVIEW_WIDTH].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_preview_width), preview_width);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_preview_width), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Icon Height
	label_icon_height = gtk_label_new("Icon Height: ");
	gtk_misc_set_alignment(GTK_MISC(label_icon_height), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_icon_height), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_icon_height = gtk_spin_button_new_with_range(0, valid_fields[ICON_HEIGHT].max_value, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_icon_height), icon_height);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_icon_height), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Default Zoom level
	label_default_zoom_level = gtk_label_new("Default Zoom Level: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_zoom_level), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_zoom_level), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	selector_default_zoom_level = create_zoom_selector(default_zoom_level->str);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(selector_default_zoom_level), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Default Background Colour
	label_default_bg_colour = gtk_label_new("Default Background Colour: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_bg_colour), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_bg_colour), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	button_default_bg_colour = gtk_color_button_new_with_color(&default_bg_colour);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button_default_bg_colour), TRUE);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_bg_colour), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_x_padding, table_y_padding);
	app_row_counter = app_row_counter + 1;

	// Ensure everything will show
	gtk_widget_show_all(GTK_WIDGET(main_dialog));

	// Loop around until we have all valid values, or the user cancels out
	validated_string = NULL;
	do
	{
		// Display the application preferences dialog
		if (gtk_dialog_run(GTK_DIALOG(main_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so destroy it, free memory, and return to the caller
			g_string_free(tmp_gstring, TRUE);
			g_string_free(valid_project_folder, TRUE);
			g_string_free(valid_screenshot_folder, TRUE);
			g_string_free(valid_output_folder, TRUE);
			g_string_free(valid_output_resolution, TRUE);
			g_string_free(valid_zoom_level, TRUE);
			gtk_widget_destroy(GTK_WIDGET(main_dialog));
			return;
		}

		// Reset the useable input flag
		useable_input = TRUE;

		// Retrieve the new default project folder input
		validated_string = validate_value(FOLDER_PATH, V_CHAR, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_default_project_folder)));
		if (NULL == validated_string)
		{
			display_warning("Error ED126: There was something wrong with the project folder given.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_project_folder = g_string_assign(valid_project_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new default screenshot folder input
		validated_string = validate_value(FOLDER_PATH, V_CHAR, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_screenshot_folder)));
		if (NULL == validated_string)
		{
			display_warning("Error ED127: There was something wrong with the screenshot folder given.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_screenshot_folder = g_string_assign(valid_screenshot_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new default output folder input
		validated_string = validate_value(FOLDER_PATH, V_CHAR, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_default_output_folder)));
		if (NULL == validated_string)
		{
			display_warning("Error ED128: There was something wrong with the output folder given.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_output_folder = g_string_assign(valid_output_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new default output resolution input
		validated_string = validate_value(RESOLUTION, V_INT_UNSIGNED, gtk_combo_box_get_active_text(GTK_COMBO_BOX(selector_default_output_res)));
		if (NULL == validated_string)
		{
			display_warning("Error ED129: There was something wrong with the default output resolution given.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_output_resolution = g_string_assign(valid_output_resolution, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new default slide length input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_slide_length));
		validated_guint = validate_value(SLIDE_LENGTH, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED130: There was something wrong with the default slide length value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_slide_length = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new default frames per second input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_fps));
		validated_guint = validate_value(PROJECT_FPS, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED134: There was something wrong with the default frames per second value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_default_fps = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new preview width input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_preview_width));
		validated_guint = validate_value(PREVIEW_WIDTH, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED131: There was something wrong with the preview width value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_preview_width = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new icon height input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_icon_height));
		validated_guint = validate_value(ICON_HEIGHT, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED132: There was something wrong with the icon height value.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_icon_height = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new default zoom level input
		validated_string = validate_value(ZOOM_LEVEL, V_ZOOM, gtk_combo_box_get_active_text(GTK_COMBO_BOX(selector_default_zoom_level)));
		if (NULL == validated_string)
		{
			display_warning("Error ED133: There was something wrong with the default zoom level given.  Please try again.");
			useable_input = FALSE;
		} else
		{
			valid_zoom_level = g_string_assign(valid_zoom_level, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}
	} while (FALSE == useable_input);

	// * We only get here after all input is considered valid *

	// Default Project Folder
	default_project_folder = g_string_assign(default_project_folder, valid_project_folder->str);
	g_string_free(valid_project_folder, TRUE);

	// Default Screenshot Folder
	screenshots_folder = g_string_assign(screenshots_folder, valid_screenshot_folder->str);
	g_string_free(valid_screenshot_folder, TRUE);

	// Default Output Folder
	default_output_folder = g_string_assign(default_output_folder, valid_output_folder->str);
	g_string_free(valid_output_folder, TRUE);

	// Default Output Resolution
	valid_output_resolution = g_string_truncate(valid_output_resolution, valid_output_resolution->len - 7);
	strings = g_strsplit(valid_output_resolution->str, "x", 2);
	default_output_width = atoi(strings[0]);
	default_output_height = atoi(strings[1]);
	g_string_free(valid_output_resolution, TRUE);
	g_strfreev(strings);

	// Default Slide Length
	default_slide_length = valid_slide_length;

	// Default Frames Per Second
	default_fps = valid_default_fps;

	// Preview width
	if (preview_width != valid_preview_width)
	{
		// The desired film strip width has changed
		preview_width = valid_preview_width;

		// Regenerate the film strip thumbnails
		regenerate_film_strip_thumbnails();

		// Set the new width of the film strip widget
		handle_size = g_new0(GValue, 1);
		g_value_init(handle_size, G_TYPE_INT);
		gtk_widget_style_get_property(GTK_WIDGET(main_area), "handle-size", handle_size);
		gtk_paned_set_position(GTK_PANED(main_area), g_value_get_int(handle_size) + preview_width + 15);
		g_free(handle_size);
	}

	// Icon Height
	icon_height = valid_icon_height;

	// Default Zoom Level
	default_zoom_level = g_string_assign(default_zoom_level, valid_zoom_level->str);
	g_string_free(valid_zoom_level, TRUE);

	// Default Background Colour
	gtk_color_button_get_color(GTK_COLOR_BUTTON(button_default_bg_colour), &default_bg_colour);

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Application preferences updated");
	gdk_flush();

	// Free up the memory allocated in this function
	g_string_free(tmp_gstring, TRUE);
	gtk_widget_destroy(GTK_WIDGET(label_default_project_folder));
	gtk_widget_destroy(GTK_WIDGET(button_default_project_folder));
	gtk_widget_destroy(GTK_WIDGET(label_default_output_folder));
	gtk_widget_destroy(GTK_WIDGET(button_default_output_folder));
	gtk_widget_destroy(GTK_WIDGET(main_dialog));
}

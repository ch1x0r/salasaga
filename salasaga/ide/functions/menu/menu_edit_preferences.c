/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Edit -> Preferences from the top menu
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../film_strip/regenerate_film_strip_thumbnails.h"
#include "../other/validate_value.h"
#include "../preference/application_preferences.h"
#include "../resolution_selector/create_resolution_selector.h"
#include "../zoom_selector/create_zoom_selector.h"


void menu_edit_preferences(void)
{
	// Local variables
	GtkWidget			*app_dialog_table;					// Table used for neat layout of the labels and fields in application preferences
	gint				app_row_counter;					// Used when building the application preferences dialog box
	GVfs				*default_gvfs;
	gchar				*directory;
	GError				*error = NULL;						// Pointer to error return structure
	gchar				*full_file_name;					// Holds the fully worked out lock file name
	gfloat				gfloat_val;							// Temporary gfloat value used for validation
	guint				guint_val;							// Temporary guint value used for validation
	GValue				*handle_size;						// The size of the dividing handle for the film strip
	GtkDialog			*main_dialog;						// Widget for the main dialog
	GKeyFile			*lock_file;							// Pointer to the lock file structure
	GString				*message;							// Used to construct message strings
	GIOChannel			*output_file;						// The output output xlock file handle
	gchar				*retrieved_uri;
	GIOStatus			return_value;						// Return value used in most GIOChannel functions
	gboolean			return_gbool;						// Receives the true/false return code when opening a lock file
	gdouble				scale_mark_counter;					// Simple counter used when constructing scale marks for sliders
	gchar				**strings;							// Text strings are split apart with this
	gint				table_padding_x;					// Amount of padding to use in the table
	gint				table_padding_y;					// Amount of padding to use in the table
	GdkColor			*temp_bg_colour;					// Temporarily holds the default background colour value
	GdkColor			temp_colour;						// Temporarily holds a colour value
	GFile				*temp_gfile;
	gchar				*tmp_gchar;							// Temporary gchar
	gsize				tmp_gsize;							// Temporary gsize
	GString				*tmp_gstring;						// Text strings are constructed in this
	gpointer			tmp_ptr;							// Temporary pointer
	gboolean			usable_input;						// Used to control loop flow
	guint				valid_default_fps = 0;				// Receives the new default fps once validated
	guint				valid_icon_height = 0;				// Receives the new icon height once validated
	gfloat				valid_layer_duration = 0;			// Receives the new default layer duration once validated
	GString				*valid_output_folder;				// Receives the new output folder once validated
	GString				*valid_output_resolution;			// Receives the new default output resolution once validated
	guint				valid_preview_width = 0;			// Receives the new film strip thumbnail width once validated
	GString				*valid_project_folder;				// Receives the new default project folder once validated
	guint				valid_screenshot_delay = 0;			// Receives the new screenshot delay once validated
	GString				*valid_screenshot_folder;			// Receives the new screenshot folder once validated
	gfloat				valid_slide_duration = 0;			// Receives the new default slide duration once validated
	GString				*valid_zoom_level;					// Receives the new default zoom level once validated
	gfloat				*validated_gfloat;					// Receives known good gfloat values from the validation function
	guint				*validated_guint;					// Receives known good guint values from the validation function
	GString				*validated_string;					// Receives known good strings from the validation function
	gchar 				tab_name[50];						// For storing the temperary tab name

	GtkWidget			*label_default_project_folder;		// Default Project Folder
	GtkWidget			*button_default_project_folder;		//

	GtkWidget			*label_screenshot_folder;			// Screenshot Folder
	GtkWidget			*button_screenshot_folder;			//

	GtkWidget			*label_default_output_folder;		// Default Output Path
	GtkWidget			*button_default_output_folder;		//

	GtkWidget			*label_default_output_res;			// Default Output Resolution
	GtkWidget			*selector_default_output_res;		//

	GtkWidget			*default_font_face_label;			// Label widget
	GtkWidget			*default_font_face_selector;		//

	GtkWidget			*default_font_size_label;			// Label widget
	GtkWidget			*default_font_size_slider;			//
	GtkWidget			*default_font_size_points_label;	//

	GtkWidget			*default_font_fg_colour_label;		// Label widget
	GtkWidget			*default_font_fg_colour_button;		// Default text layer foreground colour selector

	GtkWidget			*label_default_slide_duration;		// Default Slide Duration
	GtkWidget			*button_default_slide_duration;		//
	GtkWidget			*label_default_slide_duration_seconds;

	GtkWidget			*label_default_layer_duration;		// Default Layer Duration
	GtkWidget			*button_default_layer_duration;		//
	GtkWidget			*label_default_layer_duration_seconds;

	GtkWidget			*label_default_fps;					// Default Frames Per Second
	GtkWidget			*button_default_fps;				//

	GtkWidget			*label_preview_width;				// Preview Width
	GtkWidget			*button_preview_width;				//
	GtkWidget			*label_preview_width_pixels;		//

	GtkWidget			*label_icon_height;					// Icon Height
	GtkWidget			*button_icon_height;				//
	GtkWidget			*label_icon_height_pixels;			//

	GtkWidget			*label_default_zoom_level;			// Default Zoom Level
	GtkWidget			*selector_default_zoom_level;		//

	GtkWidget			*label_default_bg_colour;			// Default background colour
	GtkWidget			*button_default_bg_colour;			// Color button

	GtkWidget			*label_screenshot_delay;			// Screenshot delay time
	GtkWidget			*button_screenshot_delay;			//
	GtkWidget			*label_screenshot_delay_seconds;	//

	GtkWidget			*check_help_text;					// Whether to display help text or not

	GtkWidget			*check_screenshot_key_bind;			// Label widget

	GtkWidget			* notebook;							// Notebook Widget for tabbed window

	GtkWidget			*tab_label;							// Widget For storing the label of tabbed window.


	// Initialise various things
	app_row_counter = 0;
	default_gvfs = g_vfs_get_default();
	message = g_string_new(NULL);
	table_padding_x = get_table_x_padding();
	table_padding_y = get_table_y_padding();
	tmp_gstring = g_string_new(NULL);
	valid_project_folder = g_string_new(NULL);
	valid_screenshot_folder = g_string_new(NULL);
	valid_output_folder = g_string_new(NULL);
	valid_output_resolution = g_string_new(NULL);
	valid_zoom_level = g_string_new(NULL);

	// Create the main dialog window
	main_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Application Preferences"), GTK_WINDOW(get_main_window()), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL));
	app_dialog_table = gtk_table_new(3, 3, FALSE);
	// Create Note Book Widget
	notebook = gtk_notebook_new();
	// Box pack with the notebook
	gtk_box_pack_start(GTK_BOX(main_dialog->vbox), GTK_WIDGET(notebook), FALSE, FALSE, 5);

	//Label for first tab
	sprintf(tab_name,_("General Preferences"));
	tab_label = gtk_label_new(tab_name);

	// Default Project Folder
	label_default_project_folder = gtk_label_new(_("Default Project Folder: "));
	gtk_misc_set_alignment(GTK_MISC(label_default_project_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_project_folder), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_default_project_folder = gtk_file_chooser_button_new(_("Select the Default Project Folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_default_project_folder), get_default_project_folder());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_project_folder), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Screenshot Folder
	label_screenshot_folder = gtk_label_new(_("Screenshots Folder: "));
	gtk_misc_set_alignment(GTK_MISC(label_screenshot_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_screenshot_folder), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_screenshot_folder = gtk_file_chooser_button_new(_("Select the Screenshot Folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_screenshot_folder), get_screenshots_folder());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_screenshot_folder), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Default Output Folder
	label_default_output_folder = gtk_label_new(_("Default Output Folder: "));
	gtk_misc_set_alignment(GTK_MISC(label_default_output_folder), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_folder), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_default_output_folder = gtk_file_chooser_button_new(_("Select the Default Output Folder"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_default_output_folder), get_default_output_folder());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_folder), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Default Output Resolution
	label_default_output_res = gtk_label_new(_("Default Output Resolution: "));
	gtk_misc_set_alignment(GTK_MISC(label_default_output_res), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_res), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	selector_default_output_res = GTK_WIDGET(create_resolution_selector(get_default_output_width(), get_default_output_height()));
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(selector_default_output_res), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Create the label asking for the font face
	default_font_face_label = gtk_label_new(_("Default text layer font face:"));
	gtk_misc_set_alignment(GTK_MISC(default_font_face_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(default_font_face_label), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, table_padding_x, table_padding_y);

	// Create the drop down list of font faces
	default_font_face_selector = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Bold"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Bold Oblique"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Condensed"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Condensed Bold"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Condensed Bold Oblique"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Condensed Oblique"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Extra Light"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Mono"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Mono Bold"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Mono Bold Oblique"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Mono Oblique"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Sans Oblique"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Serif"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Serif Bold"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Serif Bold Italic"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Serif Condensed"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Serif Condensed Bold"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Serif Condensed Bold Italic"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Serif Condensed Italic"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(default_font_face_selector), _("DejaVu Serif Italic"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(default_font_face_selector), get_default_text_font_face());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(default_font_face_selector), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Create the label asking for the default text layer font size
	default_font_size_label = gtk_label_new(_("Default text layer font size: "));
	gtk_misc_set_alignment(GTK_MISC(default_font_size_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(default_font_size_label), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, table_padding_x, table_padding_y);

	// Create the entry that accepts the default text layer font size input
	default_font_size_slider = gtk_hscale_new_with_range(get_valid_fields_min_value(FONT_SIZE), get_valid_fields_max_value(FONT_SIZE), 0.1);
	gtk_scale_set_digits(GTK_SCALE(default_font_size_slider), 1);
	gtk_scale_set_draw_value(GTK_SCALE(default_font_size_slider), TRUE);
	gtk_range_set_value(GTK_RANGE(default_font_size_slider), get_default_text_font_size());
	for (scale_mark_counter = 10.0; scale_mark_counter <= get_valid_fields_max_value(FONT_SIZE); scale_mark_counter += 20.0)
	{
		// Add scale marks
		gtk_scale_add_mark(GTK_SCALE(default_font_size_slider), scale_mark_counter, GTK_POS_BOTTOM, NULL);
	}
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(default_font_size_slider), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, table_padding_x, table_padding_y);

	// Create the "points" label
	default_font_size_points_label = gtk_label_new(_("points"));
	gtk_misc_set_alignment(GTK_MISC(default_font_size_points_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(default_font_size_points_label), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Create the foreground colour selection label
	default_font_fg_colour_label = gtk_label_new(_("Default text layer text foreground color: "));
	gtk_misc_set_alignment(GTK_MISC(default_font_fg_colour_label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(default_font_fg_colour_label), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, table_padding_x, table_padding_y);

	// Create the foreground colour selection button
	temp_colour = get_default_text_fg_colour();
	default_font_fg_colour_button = gtk_color_button_new_with_color(&temp_colour);
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(default_font_fg_colour_button), TRUE);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(default_font_fg_colour_button), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;


	// Default Background Colour
	temp_bg_colour = get_default_bg_colour();
	label_default_bg_colour = gtk_label_new(_("Default Background Color: "));
	gtk_misc_set_alignment(GTK_MISC(label_default_bg_colour), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_bg_colour), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_default_bg_colour = gtk_color_button_new_with_color(temp_bg_colour);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button_default_bg_colour), TRUE);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_bg_colour), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;




	// Whether to display help text or not
		check_help_text = gtk_check_button_new_with_label(_("Display help text?"));
		if (FALSE == get_display_help_text())
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_help_text), FALSE);
		} else
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_help_text), TRUE);
		}
		gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(check_help_text), 0, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
		app_row_counter = app_row_counter + 1;


	// Append the table widget filled so far to the notebook and create the first page
	gtk_notebook_prepend_page (GTK_NOTEBOOK (notebook), app_dialog_table, tab_label);
	// allocate a new table for the second page of the notebook
	app_dialog_table = gtk_table_new(10, 3, FALSE);
	// Label creation as Timing and Window Preferences
	sprintf(tab_name,_("Timing & Window Preferences"));
	// Label creation with the new name
	tab_label = gtk_label_new(tab_name);
	// resetting the row counter
	app_row_counter = 0;

	// Default Slide Duration
	g_string_printf(message, "%s:", _("Default Slide Duration"));
	label_default_slide_duration = gtk_label_new(message->str);
	gtk_misc_set_alignment(GTK_MISC(label_default_slide_duration), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_slide_duration), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_default_slide_duration = gtk_spin_button_new_with_range(get_valid_fields_min_value(SLIDE_DURATION), get_valid_fields_max_value(SLIDE_DURATION), 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(button_default_slide_duration), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_slide_duration), get_default_slide_duration());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_slide_duration), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	label_default_slide_duration_seconds = gtk_label_new(_("seconds"));
	gtk_misc_set_alignment(GTK_MISC(label_default_slide_duration_seconds), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_slide_duration_seconds), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Default Layer Duration
	g_string_printf(message, "%s:", _("Default Layer Duration"));
	label_default_layer_duration = gtk_label_new(message->str);
	gtk_misc_set_alignment(GTK_MISC(label_default_layer_duration), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_layer_duration), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_default_layer_duration = gtk_spin_button_new_with_range(get_valid_fields_min_value(LAYER_DURATION), get_valid_fields_max_value(LAYER_DURATION), 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(button_default_layer_duration), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_layer_duration), get_default_layer_duration());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_layer_duration), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	label_default_layer_duration_seconds = gtk_label_new(_("seconds"));
	gtk_misc_set_alignment(GTK_MISC(label_default_layer_duration_seconds), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_layer_duration_seconds), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Default Frames Per Second
	label_default_fps = gtk_label_new(_("Default Frames Per Second: "));
	gtk_misc_set_alignment(GTK_MISC(label_default_fps), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_fps), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_default_fps = gtk_spin_button_new_with_range(get_valid_fields_min_value(PROJECT_FPS), get_valid_fields_max_value(PROJECT_FPS), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_fps), get_default_fps());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_fps), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;


	// Screenshot delay time
	label_screenshot_delay = gtk_label_new(_("Number of seconds for screenshot delay: "));
	gtk_misc_set_alignment(GTK_MISC(label_screenshot_delay), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_screenshot_delay), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_screenshot_delay = gtk_spin_button_new_with_range(get_valid_fields_min_value(SCREENSHOT_DELAY), get_valid_fields_max_value(SCREENSHOT_DELAY), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_screenshot_delay), get_screenshot_delay_time());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_screenshot_delay), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	label_screenshot_delay_seconds = gtk_label_new(_("seconds"));
	gtk_misc_set_alignment(GTK_MISC(label_screenshot_delay_seconds), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_screenshot_delay_seconds), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Preview width
	label_preview_width = gtk_label_new(_("Film Strip Width: "));
	gtk_misc_set_alignment(GTK_MISC(label_preview_width), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_preview_width), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_preview_width = gtk_spin_button_new_with_range(get_valid_fields_min_value(PREVIEW_WIDTH), get_valid_fields_max_value(PREVIEW_WIDTH), 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_preview_width), get_preview_width());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_preview_width), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	label_preview_width_pixels = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(label_preview_width_pixels), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_preview_width_pixels), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Icon Height
	label_icon_height = gtk_label_new(_("Icon Height: "));
	gtk_misc_set_alignment(GTK_MISC(label_icon_height), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_icon_height), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	button_icon_height = gtk_spin_button_new_with_range(get_valid_fields_min_value(ICON_HEIGHT), get_valid_fields_max_value(ICON_HEIGHT), 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_icon_height), get_icon_height());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_icon_height), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	label_icon_height_pixels = gtk_label_new(_("pixels"));
	gtk_misc_set_alignment(GTK_MISC(label_icon_height_pixels), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_icon_height_pixels), 2, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Default Zoom level
	label_default_zoom_level = gtk_label_new(_("Default Zoom Level: "));
	gtk_misc_set_alignment(GTK_MISC(label_default_zoom_level), 0, 0.5);
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_zoom_level), 0, 1, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	selector_default_zoom_level = create_zoom_selector(get_default_zoom_level());
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(selector_default_zoom_level), 1, 2, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;



	// Whether to display warning when screenshot key bind fails
	check_screenshot_key_bind = gtk_check_button_new_with_label(_("Display warning when screenshot key isn't enabled?"));
	if (FALSE == get_screenshot_key_warning())
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_screenshot_key_bind), FALSE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_screenshot_key_bind), TRUE);
	}
	gtk_table_attach(GTK_TABLE(app_dialog_table), GTK_WIDGET(check_screenshot_key_bind), 0, 3, app_row_counter, app_row_counter + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, table_padding_x, table_padding_y);
	app_row_counter = app_row_counter + 1;

	// Appending the table to the second page of the note book widget
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), app_dialog_table, tab_label);




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

		// Reset the usable input flag
		usable_input = TRUE;

		// Retrieve the new default project folder input
		retrieved_uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(button_default_project_folder));
		temp_gfile = g_vfs_get_file_for_uri(default_gvfs, retrieved_uri);
		directory = g_file_get_path(temp_gfile);
		validated_string = validate_value(FOLDER_PATH, V_CHAR, directory);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED126: %s", _("Error"), _("There was something wrong with the project folder given.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_project_folder = g_string_assign(valid_project_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}
		g_object_unref(temp_gfile);
		g_free(directory);

		// Retrieve the new default screenshot folder input
		retrieved_uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(button_screenshot_folder));
		temp_gfile = g_vfs_get_file_for_uri(default_gvfs, retrieved_uri);
		directory = g_file_get_path(temp_gfile);
		validated_string = validate_value(FOLDER_PATH, V_CHAR, directory);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED127: %s", _("Error"), _("There was something wrong with the screenshot folder given.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_screenshot_folder = g_string_assign(valid_screenshot_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}
		g_object_unref(temp_gfile);
		g_free(directory);

		// Retrieve the new default output folder input
		retrieved_uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(button_default_output_folder));
		temp_gfile = g_vfs_get_file_for_uri(default_gvfs, retrieved_uri);
		directory = g_file_get_path(temp_gfile);
		validated_string = validate_value(FOLDER_PATH, V_CHAR, directory);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED128: %s", _("Error"), _("There was something wrong with the output folder given.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_output_folder = g_string_assign(valid_output_folder, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}
		g_object_unref(temp_gfile);
		g_free(directory);

		// Retrieve the new default output resolution input
		validated_string = validate_value(RESOLUTION, V_INT_UNSIGNED, gtk_combo_box_get_active_text(GTK_COMBO_BOX(selector_default_output_res)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED129: %s", _("Error"), _("There was something wrong with the default output resolution given.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_output_resolution = g_string_assign(valid_output_resolution, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new default slide duration input
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_slide_duration));
		validated_gfloat = validate_value(SLIDE_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED130: %s", _("Error"), _("There was something wrong with the default slide duration value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_slide_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the new default layer duration input
		gfloat_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_layer_duration));
		validated_gfloat = validate_value(LAYER_DURATION, V_FLOAT_UNSIGNED, &gfloat_val);
		if (NULL == validated_gfloat)
		{
			g_string_printf(message, "%s ED334: %s", _("Error"), _("There was something wrong with the default layer duration value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_layer_duration = *validated_gfloat;
			g_free(validated_gfloat);
		}

		// Retrieve the new default frames per second input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_fps));
		validated_guint = validate_value(PROJECT_FPS, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED134: %s", _("Error"), _("There was something wrong with the default frames per second value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
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
			g_string_printf(message, "%s ED131: %s", _("Error"), _("There was something wrong with the preview width value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
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
			g_string_printf(message, "%s ED132: %s", _("Error"), _("There was something wrong with the icon height value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_icon_height = *validated_guint;
			g_free(validated_guint);
		}

		// Retrieve the new default zoom level input
		validated_string = validate_value(ZOOM_LEVEL, V_ZOOM, gtk_combo_box_get_active_text(GTK_COMBO_BOX(selector_default_zoom_level)));
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED133: %s", _("Error"), _("There was something wrong with the default zoom level given.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_zoom_level = g_string_assign(valid_zoom_level, validated_string->str);
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Retrieve the new screenshot delay input
		guint_val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_screenshot_delay));
		validated_guint = validate_value(SCREENSHOT_DELAY, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED427: %s", _("Error"), _("There was something wrong with the screenshot delay value.  Please try again."));
			display_warning(message->str);
			usable_input = FALSE;
		} else
		{
			valid_screenshot_delay = *validated_guint;
			g_free(validated_guint);
		}
	} while (FALSE == usable_input);

	// * We only get here after all input is considered valid *

	// Default Project Folder
	set_default_project_folder(valid_project_folder->str);
	g_string_free(valid_project_folder, TRUE);

	// Default Screenshot Folder
	set_screenshots_folder(valid_screenshot_folder->str);
	g_string_free(valid_screenshot_folder, TRUE);

	// Default Output Folder
	set_default_output_folder(valid_output_folder->str);
	g_string_free(valid_output_folder, TRUE);

	// Default Output Resolution
	strings = g_strsplit(valid_output_resolution->str, "x", 2);
	set_default_output_width(atoi(strings[0]));
	set_default_output_height(atoi(strings[1]));
	g_string_free(valid_output_resolution, TRUE);
	g_strfreev(strings);

	// Default text layer font face
	set_default_text_font_face(gtk_combo_box_get_active(GTK_COMBO_BOX(default_font_face_selector)));

	// Default text layer font size
	set_default_text_font_size(gtk_range_get_value(GTK_RANGE(default_font_size_slider)));

	// Default text layer foreground font colour
	gtk_color_button_get_color(GTK_COLOR_BUTTON(default_font_fg_colour_button), &temp_colour);
	set_default_text_fg_colour(temp_colour);

	// Default Slide Duration
	set_default_slide_duration(valid_slide_duration);

	// Default Layer Duration
	set_default_layer_duration(valid_layer_duration);

	// Default Frames Per Second
	set_default_fps(valid_default_fps);

	// Preview width
	if (get_preview_width() != valid_preview_width)
	{
		// The desired film strip width has changed
		set_preview_width(valid_preview_width);

		// Regenerate the film strip thumbnails
		regenerate_film_strip_thumbnails();

		// Set the new width of the film strip widget
		handle_size = g_new0(GValue, 1);
		g_value_init(handle_size, G_TYPE_INT);
		gtk_widget_style_get_property(GTK_WIDGET(get_main_area()), "handle-size", handle_size);
		gtk_paned_set_position(GTK_PANED(get_main_area()), g_value_get_int(handle_size) + get_preview_width() + 15);
		g_free(handle_size);
	}

	// Icon Height
	set_icon_height(valid_icon_height);

	// Default Zoom Level
	set_default_zoom_level(valid_zoom_level->str);
	g_string_free(valid_zoom_level, TRUE);

	// Default Background Colour
	gtk_color_button_get_color(GTK_COLOR_BUTTON(button_default_bg_colour), temp_bg_colour);
	set_default_bg_colour(temp_bg_colour);
	g_free(temp_bg_colour);

	// Screenshot delay
	set_screenshot_delay_time(valid_screenshot_delay);

	// * Update the screenshot delay time and folder in the .lock file, if one exists *

	// Construct the fully qualified path name to the lock file, to hold capture settings in
	tmp_ptr = (gchar *) g_get_home_dir();
	full_file_name = g_build_filename(tmp_ptr, ".salasaga-lock", NULL);

	// Check if the ~/.salasaga-lock file exists
	lock_file = g_key_file_new();
	return_gbool = g_key_file_load_from_file(lock_file, full_file_name, G_KEY_FILE_NONE, &error);
	if (TRUE == return_gbool)
	{
		// * The lock file exists, so we modify the existing contents *

		// Set the new screenshot delay value
		g_key_file_set_integer(lock_file, "Project", "Screenshot_Delay", get_screenshot_delay_time());  // Number of seconds to delay the screenshot capture

		// Set the new screenshot folder value
		g_key_file_set_string(lock_file, "Project", "Directory", get_screenshots_folder());  // Directory to save screenshots in

		// Create IO channel for writing to
		output_file = g_io_channel_new_file(full_file_name, "w", &error);
		if (NULL == output_file)
		{
			// An error occurred when opening the file for writing, so alert the user, and return to the calling routine indicating failure

			// Display a warning message
			g_string_printf(message, "%s ED429: ", _("Error"));
			g_string_append_printf(message, _("An error '%s' occurred when opening the lock file '%s' for writing."), error->message, full_file_name);
			display_warning(message->str);

			// Free the memory allocated in this function
			g_string_free(message, TRUE);
			g_string_free(tmp_gstring, TRUE);
			g_error_free(error);

			return;
		}

		// Write the ~/.salasaga-lock file to disk
		tmp_gchar = g_key_file_to_data(lock_file, NULL, NULL);
		return_value = g_io_channel_write_chars(output_file, tmp_gchar, strlen(tmp_gchar), &tmp_gsize, &error);
		if (G_IO_STATUS_ERROR == return_value)
		{
			// * An error occurred when writing to the output file, so alert the user, and return to the calling routine indicating failure *

			// Display a warning message
			g_string_printf(message, "%s ED430: ", _("Error"));
			g_string_append_printf(message, _("An error '%s' occurred when writing data to the lock file '%s'."), error->message, full_file_name);
			display_warning(message->str);

			// Free the memory allocated in this function
			g_string_free(message, TRUE);
			g_string_free(tmp_gstring, TRUE);
			g_error_free(error);

			return;
		}

		// Close the IO channel
		return_value = g_io_channel_shutdown(output_file, TRUE, &error);
		if (G_IO_STATUS_ERROR == return_value)
		{
			// * An error occurred when closing the output file, so alert the user, and return to the calling routine indicating failure *

			// Display a warning message
			g_string_printf(message, "%s ED431: ", _("Error"));
			g_string_append_printf(message, _("An error '%s' occurred when closing the lock file '%s'."), error->message, full_file_name);
			display_warning(message->str);

			// Free the memory allocated in this function
			g_string_free(message, TRUE);
			display_warning(tmp_gstring->str);

			// Free the memory allocated in this function
			g_string_free(tmp_gstring, TRUE);
			g_error_free(error);

			return;
		}

		// Close the lock file
		g_key_file_free(lock_file);
	}

	// Find out if we should display help text or not
	if (TRUE == gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_help_text)))
	{
		set_display_help_text(TRUE);
	} else
	{
		set_display_help_text(FALSE);
	}

	// Find out if we should display a warning when the screenshot key isn't bound
	if (TRUE == gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_screenshot_key_bind)))
	{
		set_screenshot_key_warning(TRUE);
	} else
	{
		set_screenshot_key_warning(FALSE);
	}

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Application preferences updated"));
	gdk_flush();

	// Free up the memory allocated in this function
	g_string_free(message, TRUE);
	g_string_free(tmp_gstring, TRUE);
	gtk_widget_destroy(GTK_WIDGET(label_default_project_folder));
	gtk_widget_destroy(GTK_WIDGET(button_default_project_folder));
	gtk_widget_destroy(GTK_WIDGET(label_default_output_folder));
	gtk_widget_destroy(GTK_WIDGET(button_default_output_folder));
	gtk_widget_destroy(GTK_WIDGET(main_dialog));
}

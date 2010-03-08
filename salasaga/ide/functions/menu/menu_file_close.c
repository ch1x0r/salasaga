/*
 * $Id$
 *
 * Salasaga: Called when the user chooses File -> Close from the top menu
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../../../config.h"
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_dialog_save_warning.h"
#include "../slide/slide_free.h"
#include "../text_tags/reset_global_text_tags_table.h"
#include "../tool_bars/disable_layer_toolbar_buttons.h"
#include "../tool_bars/disable_main_toolbar_buttons.h"
#include "menu_enable.h"


void menu_file_close()
{
	// Local variables
	gint				pixmap_height;				// Height of the front stoe
	gint				pixmap_width;				// Width of the front store
	gboolean			return_code_gbool;			// Catches gboolean return codes
	GString				*title_bar;					// Used to update the title bar
	GdkRectangle		tmp_rectangle;


	// If there isn't an active project, we just return
	if (FALSE == get_project_active())
		return;

	// If we have unsaved changes, warn the user
	if (TRUE == get_changes_made())
	{
		return_code_gbool = display_dialog_save_warning();
		if (FALSE == return_code_gbool)
		{
			// The user pressed Cancel in the dialog
			return;
		}
	}

	// If there's a project presently loaded in memory, we unload it
	if (NULL != slides)
	{
		// Free the resources presently allocated to slides
		g_list_foreach(slides, slide_free, NULL);

		// Re-initialise pointers
		slides = NULL;
		set_current_slide(NULL);
	}

	// Update the global project active flag
	set_project_active(FALSE);

	// Clear the changes made variable
	set_changes_made(FALSE);

	// Clear the saved file name string
	if (NULL != get_file_name())
	{
		set_file_name(NULL);
	}

	// Change the title bar
	title_bar = g_string_new(NULL);
	g_string_printf(title_bar, "%s v%s", APP_NAME, APP_VERSION);
	gtk_window_set_title(GTK_WINDOW(get_main_window()), title_bar->str);
	g_string_free(title_bar, TRUE);

	// Reset the global text tag table
	reset_global_text_tags_table();

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Project closed"));
	gdk_flush();

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Disable the project based menu items
	menu_enable(_("/Project"), FALSE);
	menu_enable(_("/Slide"), FALSE);
	menu_enable(_("/Layer"), FALSE);
	menu_enable(_("/Export"), FALSE);

	// If there's an existing film strip, we unload it
	gtk_list_store_clear(GTK_LIST_STORE(get_film_strip_store()));

	// Clear the working area
	gdk_drawable_get_size(GDK_PIXMAP(get_front_store()), &pixmap_width, &pixmap_height);
	tmp_rectangle.x = 0;
	tmp_rectangle.y = 0;
	tmp_rectangle.width = pixmap_width;
	tmp_rectangle.height = pixmap_height;
	gdk_window_invalidate_rect(get_main_drawing_area()->window, &tmp_rectangle, TRUE);
}

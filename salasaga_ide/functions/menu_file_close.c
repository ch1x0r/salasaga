/*
 * $Id$
 *
 * Salasaga: Called when the user chooses File -> Close from the top menu
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "disable_layer_toolbar_buttons.h"
#include "disable_main_toolbar_buttons.h"
#include "display_dialog_save_warning.h"
#include "menu_enable.h"
#include "slide_free.h"


void menu_file_close()
{
	// Local variables
	gboolean			return_code_gbool;			// Catches gboolean return codes
	GdkRectangle		tmp_rectangle;


	// If there isn't an active project, we just return
	if (FALSE == project_active)
		return;

	// If we have unsaved changes, warn the user
	if (TRUE == changes_made)
	{
		return_code_gbool = display_dialog_save_warning(TRUE);
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
		current_slide = NULL;
	}

	// Update the global project active flag
	project_active = FALSE;

	// Clear the changes made variable
	changes_made = FALSE;

	// Clear the saved file name string
	if (NULL != file_name)
	{
		g_string_free(file_name, TRUE);
		file_name = NULL;
	}

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Project closed");
	gdk_flush();

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Disable the project based menu items
	menu_enable("/Project", FALSE);
	menu_enable("/Slide", FALSE);
	menu_enable("/Layer", FALSE);
	menu_enable("/Export", FALSE);

	// If there's an existing film strip, we unload it
	gtk_list_store_clear(GTK_LIST_STORE(film_strip_store));

	// Clear the working area
	tmp_rectangle.x = 0;
	tmp_rectangle.y = 0;
	tmp_rectangle.width = main_drawing_area->allocation.width;
	tmp_rectangle.height = main_drawing_area->allocation.height;
	gdk_window_invalidate_rect(main_drawing_area->window, &tmp_rectangle, TRUE);
}

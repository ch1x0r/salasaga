/*
 * $Id$
 *
 * Salasaga: Function called when the x offset start field in the timeline widget is edited 
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
#include "display_warning.h"
#include "draw_workspace.h"
#include "film_strip_create_thumbnail.h"
#include "validate_value.h"


void timeline_edited_x_offset_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;
	guint				*validated_guint;			// Receives known good guint values from the validation function


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	switch (layer_data->object_type)
	{
		case TYPE_EMPTY:
			// Nothing to do here
			break;

		case TYPE_GDK_PIXBUF:

			// If this is a background layer, we ignore the edit
			if (TRUE == layer_data->background)
			{
				if (layer_data->x_offset_start != atoi(new_value))
				{
					// If the user attempted to change the background layer offset, we give them feedback not to
					display_warning("Error ED350: Background layers must remain at 0.");
				}
				return;
			}

			// Not a background layer, so we drop through and process it

		case TYPE_HIGHLIGHT:
		case TYPE_MOUSE_CURSOR:
		case TYPE_TEXT:

			// If no changes have been made, then we ignore the edit
			if (layer_data->x_offset_start == atoi(new_value))
			{
				return;
			}

			// Validate the new value
			validated_guint = validate_value(X_OFFSET, V_CHAR, new_value);
			if (NULL == validated_guint)
			{
				display_warning("Error ED352: There was something wrong with the new x offset start value.  Ignoring it.");
				return;
			} else
			{
				// Apply the change
				layer_data->x_offset_start = *validated_guint;
				g_free(validated_guint);
			}
			break;

		default:
			display_warning("Error ED59: Unknown layer type\n");
	}

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_X_OFF_START, atoi(new_value),
						-1);

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail((slide *) current_slide->data);

	// Set the changes made variable
	changes_made = TRUE;

	// Use the status bar to communicate the change
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Offset changed");
	gdk_flush();
}

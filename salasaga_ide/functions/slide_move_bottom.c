/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Slide -> Move to bottom from the top menu 
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
#include "create_tooltips.h"


void slide_move_bottom(void)
{
	// Local variables
	GtkTreeIter			new_iter;
	GtkTreePath			*new_path;				// Temporary path
	gint				num_slides;				// The total number of slides
	gint				slide_position;				// Which slide in the slide list we are moving
	slide				*this_slide_data;			// Pointer to the data for this slide
	GString				*tmp_gstring;


	// Initialise some things
	tmp_gstring = g_string_new(NULL);	

	// Safety check
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, current_slide);
	if (debug_level) printf("slide_move_bottom: slide position: %u\n", slide_position);
	num_slides = g_list_length(slides);
	if (num_slides == (slide_position + 1))
	{
		// We can't move the bottom most slide any further down, so just return
		// fixme5: Should probably beep or somehow indicate an error here in a non-blocking way
		return;
	}

	// Remove this slide from the slides list, then re-attach it on the end
	this_slide_data = current_slide->data;
	slides = g_list_remove(slides, this_slide_data);
	slides = g_list_append(slides, this_slide_data);
	current_slide = g_list_last(slides);

	// Move the thumbnail to the end of the slides list
	g_string_printf(tmp_gstring, "%u", slide_position);
	if (TRUE == gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(film_strip_store), &new_iter, tmp_gstring->str))
	{
		if (debug_level) printf("slide_move_bottom: Iter is valid\n");
		gtk_list_store_move_before(GTK_LIST_STORE(film_strip_store), &new_iter, NULL);
	} else
	{
		if (debug_level) printf("slide_move_bottom: Iter is not valid\n");
	}
	g_string_free(tmp_gstring, TRUE);

	// Recreate the slide tooltips
	create_tooltips();

	// Scroll the film strip to show the new thumbnail position
	new_path = gtk_tree_path_new_from_indices(num_slides - 1, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 1.0, 0.0);
//	gtk_tree_path_free(new_path);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/01/15 16:18:58  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:37:24  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:11  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

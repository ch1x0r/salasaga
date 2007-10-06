/*
 * $Id$
 *
 * Flame Project: Function to regenerate the film strip thumbnails
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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
#include <gtk/gtk.h>

#ifndef _WIN32
	// Non-windows code
	#include <gconf/gconf.h>
	#include <libgnome/libgnome.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "compress_layers.h"


void regenerate_film_strip_thumbnails()
{
	// Local variables
	gint				num_slides;
	gint				slide_counter, slide_position;
	GtkTreeIter			film_strip_iter;
	GdkPixbuf			*new_thumbnail;
	GList				*this_slide;


	// Safety check
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, current_slide);
	num_slides = g_list_length(slides);
	if (0 == num_slides)
	{
		// There aren't any slides in this project yet, so just return
		return;
	}

	// Remove the existing film strip thumbnails
	gtk_list_store_clear(GTK_LIST_STORE(film_strip_store));

	// Generate new thumbnails
	for (slide_counter = 0; slide_counter < num_slides; slide_counter++)
	{
		// Create the thumbnail for the slide
		this_slide = g_list_nth(slides, slide_counter);
		new_thumbnail = compress_layers(this_slide, preview_width, (guint) preview_width * 0.75);
		((slide *) this_slide->data)->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(GDK_PIXBUF(new_thumbnail)));

		// Add the thumbnail to the film strip
		gtk_list_store_append(film_strip_store, &film_strip_iter);
		gtk_list_store_set(film_strip_store, &film_strip_iter, 0, gtk_image_get_pixbuf(((slide *) this_slide->data)->thumbnail), -1);
	}
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.3  2007/09/29 04:22:17  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:08  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:45  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

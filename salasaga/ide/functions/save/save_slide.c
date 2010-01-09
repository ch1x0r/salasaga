/*
 * $Id$
 *
 * Salasaga: Function that takes a slide structure pointer, and adds it's data to an in-memory XML document
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/parser.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"
#include "../save/save_layer.h"


void save_slide(gpointer element, gpointer user_data)
{
	// Local variables
	slide				*slide_pointer;			// Points to the present slide
	GList				*layer_pointer;			// Points to the presently processing layer

	xmlNodePtr			slide_root;				// Points to the root of the slide data
	xmlNodePtr			slide_node;				// Pointer to the new slide node

	GString				*tmp_gstring;			// Temporary GString
	GdkRectangle		tmp_rect = {0, 0, main_window->allocation.width, main_window->allocation.height};  // Temporary rectangle covering the area of the whole Salasaga window


	// Repaint the entire window (where the dialog box was, plus the status bar)
	gtk_widget_draw(main_window, &tmp_rect);
	gdk_flush();

	// Initialise various things
	slide_pointer = element;
	slide_root = user_data;
	layer_pointer = slide_pointer->layers;
	tmp_gstring = g_string_new(NULL);

    // Create the slide container
	slide_node = xmlNewChild(slide_root, NULL, (const xmlChar *) "slide", NULL);
	if (NULL == slide_node)
	{
		g_string_printf(tmp_gstring, "%s ED23: %s", _("Error"), _("Error creating the slide node."));
		display_warning(tmp_gstring->str);
		g_string_free(tmp_gstring, TRUE);
		return;
	}

	// Add the slide name to the slide container attributes
	if (NULL != slide_pointer->name)
	{
		xmlNewProp(slide_node, (const xmlChar *) "name", (const xmlChar *) slide_pointer->name->str);
	}

	// Add the slide duration to the slide container attributes
	g_string_printf(tmp_gstring, "%0.4f", slide_pointer->duration);
	xmlNewProp(slide_node, (const xmlChar *) "duration", (const xmlChar *) tmp_gstring->str);

	// Add the layer data to the slide container
	layer_pointer = g_list_first(layer_pointer);
	g_list_foreach(layer_pointer, save_layer, slide_node);

	// Free the memory allocated in this function
	g_string_free(tmp_gstring, TRUE);

	// Update the status bar to show progress
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(status_bar));

	return;
}

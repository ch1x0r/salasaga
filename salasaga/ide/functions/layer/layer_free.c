/*
 * $Id$
 *
 * Salasaga: Frees the memory allocated for a layer
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"


gboolean layer_free(layer *this_layer)
{
	// Local variables
	GString				*message;					// Used to construct message strings


	// Initialisation
	message = g_string_new(NULL);

	// Free the easy variables first
	if (NULL != this_layer->name)
		g_string_free(this_layer->name, TRUE);
	if (NULL != this_layer->external_link)
		g_string_free(this_layer->external_link, TRUE);
	if (NULL != this_layer->external_link_window)
		g_string_free(this_layer->external_link_window, TRUE);

	// Free the layer type specific data
	switch (this_layer->object_type)
	{
		case TYPE_EMPTY:
			// Nothing here needs freeing
			break;

		case TYPE_GDK_PIXBUF:
			if (NULL != ((layer_image *) this_layer->object_data)->image_data)
				g_object_unref(GDK_PIXBUF(((layer_image *) this_layer->object_data)->image_data));
			if (NULL != ((layer_image *) this_layer->object_data)->cairo_pattern)
			{
				cairo_pattern_destroy(((layer_image *) this_layer->object_data)->cairo_pattern);
				((layer_image *) this_layer->object_data)->cairo_pattern = NULL;
			}
			break;

		case TYPE_HIGHLIGHT:
			// Nothing here needs freeing
			break;

		case TYPE_MOUSE_CURSOR:
			// Nothing here needs freeing
			break;

		case TYPE_TEXT:
			if (NULL != ((layer_text *) this_layer->object_data)->text_buffer)
				g_object_unref(GTK_TEXT_BUFFER(((layer_text *) this_layer->object_data)->text_buffer));
			break;

		default:
			g_string_printf(message, "%s ED286: %s", _("Error"), _("Unknown layer type when destroying a layer."));
			display_warning(message->str);
	}
	g_free(this_layer->object_data);

	// Free the layer structure itself
	g_free(this_layer);

	// Free the memory used in this function
	g_string_free(message, TRUE);

	return TRUE;
}

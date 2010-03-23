/*
 * $Id$
 *
 * Salasaga: Saves font size and text foreground colour tags in a project file
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

// XML includes
#include <libxml/parser.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"


void save_text_tags(xmlNodePtr tag_root)
{
	// Local variables
	xmlNodePtr			colour_root;			// Pointer to the root of the foreground colour tags
	GdkColor			*fg_colour;				// The colour of a foreground colour tag gets retrieved into this
	gdouble				font_size;				// The font size of a text tag gets retrieved into this
	gint				loop_counter;
	GString				*message;				// Messages are constructed in this
	guint				num_tags;
	xmlNodePtr			size_root;				// Pointer to the root of the font size tags
	gchar				*tag_name;				// Pointer to a tag name
	xmlNodePtr			tag_node;				// Pointer to a new XML text tag node
	GtkTextTag			*text_tag;				// Pointer to the text tag we are working with


	// Initialise various things
	message = g_string_new(NULL);

    // Create a container for the text size tags
	size_root = xmlNewChild(tag_root, NULL, (const xmlChar *) "size-tags", NULL);
	if (NULL == size_root)
	{
		g_string_printf(message, "%s ED436: %s", _("Error"), _("Error creating the size tags container."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Add all of the font size tags
	num_tags = g_slist_length(get_text_tags_size_slist());
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		// Create the tag container
		tag_node = xmlNewChild(size_root, NULL, (const xmlChar *) "tag", NULL);
		if (NULL == tag_node)
		{
			g_string_printf(message, "%s ED435: %s", _("Error"), _("Error creating a text tag node."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return;
		}

		// Retrieve the properties of the text tag
		text_tag = g_slist_nth_data(get_text_tags_size_slist(), loop_counter);
		g_object_get(G_OBJECT(text_tag), "name", &tag_name, "size-points", &font_size, NULL);

		// Add the tag name to the tag container attributes
		xmlNewProp(tag_node, (const xmlChar *) "label", (const xmlChar *) tag_name);

		// Add the tag size to the tag container attributes
		g_string_printf(message, "%0.4f", font_size);
		xmlNewProp(tag_node, (const xmlChar *) "size", (const xmlChar *) message->str);

		// Free the data allocated in this loop
		g_free(tag_name);
	}

    // Create a container for the text foreground colour tags
	colour_root = xmlNewChild(tag_root, NULL, (const xmlChar *) "fg-colour-tags", NULL);
	if (NULL == colour_root)
	{
		g_string_printf(message, "%s ED437: %s", _("Error"), _("Error creating the foreground color tags container."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Add all of the foreground colour tags
	num_tags = g_slist_length(get_text_tags_fg_colour_slist());
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		// Create the tag container
		tag_node = xmlNewChild(colour_root, NULL, (const xmlChar *) "tag", NULL);
		if (NULL == tag_node)
		{
			g_string_printf(message, "%s ED438: %s", _("Error"), _("Error creating a foreground color tag node."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return;
		}

		// Retrieve the properties of the text tag
		text_tag = g_slist_nth_data(get_text_tags_fg_colour_slist(), loop_counter);
		g_object_get(G_OBJECT(text_tag), "name", &tag_name, "foreground-gdk", &fg_colour, NULL);

		// Add the tag name to the tag container attributes
		xmlNewProp(tag_node, (const xmlChar *) "label", (const xmlChar *) tag_name);

		// Add the foreground colour to the tag container attributes
		g_string_printf(message, "%u", fg_colour->red);
		xmlNewProp(tag_node, (const xmlChar *) "red", (const xmlChar *) message->str);
		g_string_printf(message, "%u", fg_colour->green);
		xmlNewProp(tag_node, (const xmlChar *) "green", (const xmlChar *) message->str);
		g_string_printf(message, "%u", fg_colour->blue);
		xmlNewProp(tag_node, (const xmlChar *) "blue", (const xmlChar *) message->str);

		// Free the data allocated in this loop
		g_free(tag_name);
	}

	// Free the memory allocated in this function
	g_string_free(message, TRUE);

	return;
}

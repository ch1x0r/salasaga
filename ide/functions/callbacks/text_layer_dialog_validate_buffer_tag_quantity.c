/*
 * $Id$
 *
 * Salasaga: Function called after the tags in a text layer text buffer is changed, to double check the
 *           correct number of tags are present (should be 3 present for each character)
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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
#include "../../salasaga_types.h"
#include "../../externs.h"

void text_layer_dialog_validate_buffer_tag_quantity(GtkTextBuffer *text_buffer)
{
	// Local variables
	GtkTextIter			buffer_end;
	GtkTextIter			buffer_start;
	gint				end_offset;
	gint				i;
	guint				loop_counter;
	GtkTextIter 		loop_iter;
	guint				num_tags;
	gint				start_offset;
	GSList				*tag_list;
	gchar				*tag_name;
	GtkTextTag			*tag_ptr;


	// If no text is selected then we skip the rest of the function
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(text_buffer), &buffer_start, &buffer_end);
	gtk_text_iter_order(&buffer_start, &buffer_end);
	start_offset = gtk_text_iter_get_offset(&buffer_start);
	end_offset = gtk_text_iter_get_offset(&buffer_end);

	// Scan through the text buffer, looking for any characters that have a quantity of tags other than 3
	for (i = 0; i < end_offset; i++)
	{
		gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(text_buffer), &loop_iter, i);
		tag_list = gtk_text_iter_get_tags(&loop_iter);
		num_tags = g_slist_length(tag_list);
		if (3 != num_tags)
		{
			// If characters are found without the correct number of tags, write a message to stdout
			printf("*****************************\n");
			printf("**Incorrect # of tags found**\n");
			printf("*****************************\n");
			printf("Character at this offset: %c\n", gtk_text_iter_get_char(&loop_iter));
			printf("Found %d tags\n", num_tags);

			// Display the tags found
			for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
			{
				tag_ptr = g_slist_nth_data(tag_list, loop_counter);
				if (tag_ptr->name == NULL)
				{
					printf("Anonymous tag with no name\n");
				}
				else
				{
					tag_name = tag_ptr->name;
					printf("Tag name: %s\n", tag_name);
				}
			}
		}
		g_slist_free(tag_list);
	}

	return;
}

/*
 * $Id$
 *
 * Salasaga: Debugging function to display the contents of a text layer buffer
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"


gboolean dump_selection_info(GtkWidget *calling_widget, text_dialog_widgets *text_widgets)
{
	// Local variables
	GtkTextIter			buffer_end_iter;
	gint				buffer_end_offset;
	gint				end_offset;
	gint				i;
	guint				loop_counter;
	GtkTextIter 		loop_iter;
	guint				num_tags;
	GtkTextIter			selection_end;
	GtkTextIter			selection_start;
	gint				start_offset;
	GSList				*tag_list;
	gchar				*tag_name;
	GtkTextTag			*tag_ptr;
	GtkTextBuffer		*text_buffer;
	gboolean			text_selected;
	GtkWidget			*text_view;


	// Initialisation
	text_view = text_widgets->text_view;
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	// If no text is selected then we skip the rest of the function
	text_selected = gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(text_buffer), &selection_start, &selection_end);
	gtk_text_iter_order(&selection_start, &selection_end);

	// Dump a count of tags that are present from the start to the end of the selection
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(text_buffer), &buffer_end_iter);
	start_offset = gtk_text_iter_get_offset(&selection_start);
	end_offset = gtk_text_iter_get_offset(&selection_end);
	buffer_end_offset = gtk_text_iter_get_offset(&buffer_end_iter);

	// Display separator
	printf("*******************\n");
	printf("*******************\n");

	// Display the characters up to the start of the selection (if any)
	for (i = 0; i < start_offset; i++)
	{
		gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(text_buffer), &loop_iter, i);
		tag_list = gtk_text_iter_get_tags(&loop_iter);

		num_tags = g_slist_length(tag_list);
		printf("Offset: %d\t # tags at this offset: %d\n", i, num_tags);
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
		g_slist_free(tag_list);
		printf("Character at this offset: %c\n", gtk_text_iter_get_char(&loop_iter));
	}

	// Display separator
	printf("*** Selection start ***\n");

	// Display the characters in the selection (if any)
	for (i = start_offset; i < end_offset; i++)
	{
		gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(text_buffer), &loop_iter, i);
		tag_list = gtk_text_iter_get_tags(&loop_iter);

		num_tags = g_slist_length(tag_list);
		printf("Offset: %d\t # tags at this offset: %d\n", i, num_tags);
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
		g_slist_free(tag_list);
		printf("Character at this offset: %c\n", gtk_text_iter_get_char(&loop_iter));
	}

	// Display separator
	printf("*** Selection end ***\n");

	// Display the characters after the end of the selection (if any)
	for (i = end_offset; i < buffer_end_offset; i++)
	{
		gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(text_buffer), &loop_iter, i);
		tag_list = gtk_text_iter_get_tags(&loop_iter);

		num_tags = g_slist_length(tag_list);
		printf("Offset: %d\t # tags at this offset: %d\n", i, num_tags);
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
		g_slist_free(tag_list);
		printf("Character at this offset: %c\n", gtk_text_iter_get_char(&loop_iter));
	}

	// Add a newline separator to help space things visually in the dump
	printf("\n");

	return FALSE;
}

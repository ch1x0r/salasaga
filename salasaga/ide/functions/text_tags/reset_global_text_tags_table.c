/*
 * $Id$
 *
 * Salasaga: Resets the global text tab table, freeing all of the font size and foreground colour text tags
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
#include "../global_functions.h"


gboolean reset_global_text_tags_table(void)
{
	// Local variables
	gint				loop_counter;
	guint				num_tags;


	// Free all of the font size text tags
	num_tags = g_slist_length(get_text_tags_size_slist());
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		gtk_text_tag_table_remove(get_text_tags_table(), g_slist_nth_data(get_text_tags_size_slist(), loop_counter));
	}
	g_slist_free(get_text_tags_size_slist());
	set_text_tags_size_slist(NULL);

	// Free all of the foreground font colour text tags
	num_tags = g_slist_length(get_text_tags_fg_colour_slist());
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		gtk_text_tag_table_remove(get_text_tags_table(), g_slist_nth_data(get_text_tags_fg_colour_slist(), loop_counter));
	}
	g_slist_free(get_text_tags_fg_colour_slist());
	set_text_tags_fg_colour_slist(NULL);

	return TRUE;
}

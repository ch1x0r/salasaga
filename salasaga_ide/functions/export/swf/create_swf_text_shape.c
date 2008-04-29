/*
 * $Id$
 *
 * Salasaga: Creates a swf text object from a given layer_text structure
 *
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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


// Turn on C99 compatibility
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "../../display_warning.h"


SWFText create_swf_text_shape(layer_text *text_data, gfloat scaled_width_ratio, gfloat scaled_height_ratio, gint *num_text_lines, gfloat *widest_text_string_width, gfloat *text_real_font_size)
{
	// Local variables
	guint16				blue_component;				// Used when retrieving the foreground color of text
	gfloat				current_ming_scale;			// Used when creating text swf output
	FILE				*font_file;					// The file we load the font from
	SWFFont				font_object;				// The font we use gets loaded into this
	gchar				*font_pathname;				// Full pathname to a font file to load is constructed in this
	guint16				green_component;			// Used when retrieving the foreground color of text
	guint16				red_component;				// Used when retrieving the foreground color of text
	gfloat				scaled_font_size;			// Display height of a font in swf, when scaled to the desired output size
	GtkTextIter			text_end;					// End position of text buffer
	gint				text_lines_counter;			// Counter used when processing text
	SWFText				text_object;
	GtkTextIter			text_start;					// Start position of text buffer
	gfloat				this_text_string_width;		// Used when calculating how wide to draw the text background box
	gchar				*visible_string;			// Text string is retrieved into this


	// Create the font object we'll be using
	font_pathname = g_build_path(G_DIR_SEPARATOR_S, font_path, "fdb", "Bitstream Vera Sans.fdb", NULL);
	if (debug_level) printf("Full path name to font file is: %s\n", font_pathname);

	// Load the font file if needed
	font_file = fopen(font_pathname, "r");
	if (NULL == font_file)
	{
		// Something went wrong when loading the font file, so return
		display_warning("Error ED380: Something went wrong when opening the font file");

		return FALSE;
	}
	font_object = loadSWFFontFromFile(font_file);
	if (NULL == font_object)
	{
		// Something went wrong when loading the font file, so return
		display_warning("Error ED96: Something went wrong when loading the font file");

		// Free the memory allocated in this function
		// fixme3: Ming 0.3.0 doesn't have destroySWFFillStyle()
		//destroySWFFillStyle(highlight_fill_style);
		//destroySWFFillStyle(text_bg_fill_style);
		g_free(font_pathname);

		return FALSE;
	}

	// Create the text object we'll be using
	text_object = newSWFText();

	// Assign a font to the text object
	SWFText_setFont(text_object, font_object);

	// Set the height we want for the text
	scaled_font_size = scaled_height_ratio * text_data->font_size;
	SWFText_setHeight(text_object, scaled_font_size);
	*text_real_font_size = SWFText_getAscent(text_object) + SWFText_getDescent(text_object);

	// Displaying debugging info if requested
	if (debug_level)
	{
		printf("Scaled font size: '%.2f'\n", scaled_font_size);
		printf("Real font size: '%.2f'\n", *text_real_font_size);
		printf("'Leading' for this text is: %.2f\n", SWFText_getLeading(text_object));
		printf("'Ascent' for this text is: %.2f\n", SWFText_getAscent(text_object));
		printf("'Descent' for this text is: %.2f\n", SWFText_getDescent(text_object));
	}

	// Set the foreground color for the text
	red_component = text_data->text_color.red;
	green_component = text_data->text_color.green;
	blue_component = text_data->text_color.blue;
	SWFText_setColor(text_object, roundf(red_component / 255), roundf(green_component / 255), roundf(blue_component / 255), 0xff);

	// Work out how many lines of text we're dealing with
	*num_text_lines = gtk_text_buffer_get_line_count(text_data->text_buffer);

	// Displaying debugging info if requested
	if (debug_level)
	{
		printf("Number of lines of text: %d\n", *num_text_lines);
	}

	// Add each line of text to the output
	*widest_text_string_width = 0;
	for (text_lines_counter = 0; text_lines_counter < *num_text_lines; text_lines_counter++)
	{
		// Select the start and end positions for the given line, in the text buffer
		gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(text_data->text_buffer), &text_start, text_lines_counter);
		text_end = text_start;
		gtk_text_iter_forward_to_line_end(&text_end);

		// Retrieve the text for the given line, and add it to the text object
		visible_string = gtk_text_iter_get_visible_text(&text_start, &text_end);
		SWFText_addString(text_object, visible_string, NULL);

		// * We need to know which of the strings is widest, so we can calculate the width of the text background box *

		// If this is the widest string, we keep the value of this one
		this_text_string_width = SWFText_getStringWidth(text_object, (guchar *) visible_string);
		if (this_text_string_width > *widest_text_string_width)
			*widest_text_string_width = this_text_string_width;

		// * Move the pen down to the start of the next line *

		// Move to the appropriate Y position
		SWFText_moveTo(text_object, 0, (text_lines_counter + 1) * (*text_real_font_size));

		// Try and move X as close as possible to 0.  We can't use 0 in SWFText_moveTo() due to a bug in Ming
		current_ming_scale = Ming_getScale();
		Ming_setScale(1);
		SWFText_moveTo(text_object, 1, 0);
		Ming_setScale(current_ming_scale);

		// Displaying debugging info if requested
		if (debug_level)
		{
			printf("Line %d of %d: %s\n", text_lines_counter, *num_text_lines, gtk_text_iter_get_visible_text(&text_start, &text_end));
			printf("Width of this string: %.2f\n", this_text_string_width);
			printf("Width of widest string thus far: %.2f\n", *widest_text_string_width);
		}
	}

	return text_object;
}

#include "flex_mxml_create_shape.h"

gboolean flex_create_shape( flex_mxml_dom_t* dom,layer* this_layer_data) {
	// Local variables
	GSList				*applied_tags;				// Receives a list of text tags applied at a position in a text buffer
	GString				*as_gstring = NULL;			// Used for constructing action script statements
	guint16				blue_component;				// Used when retrieving the foreground color of text
	gfloat				char_descent;				// Used for calculating the descent of a line
	gint				char_font_face = -1;		// Used for calculating the font face of a character
	gint				*char_font_ptr;				// Used for calculating the font face of a character
	SWFDisplayItem		container_display_item;
	SWFMovieClip		container_movie_clip;		// The movie clip that contains the layer object
	gchar				*conversion_buffer;			// Used when converting between unicode character types
	gfloat				current_ming_scale;			// Used when creating text swf output
	GtkTextIter			cursor_iter;				// Used for positioning in a text layer's text buffer
	layer_empty			*empty_data;				// Points to the empty object data inside the layer
	SWFFillStyle		empty_layer_fill;			// Fill style used when constructing empty layer shapes
	SWFShape			empty_layer_shape;			// Temporary swf shape used when constructing empty layers
	GError				*error = NULL;				// Pointer to error return structure
	GdkColor			*fg_colour;					// Foreground colour to set a text layer character to
	guint				final_opacity;				// Used when calculating the final opacity figure for a highlight layer
	gboolean			font_face_found;			// Used to track if we've found the font face
	gdouble				font_size;					// Used for retrieving the desired size of a character in a text layer
	gint				font_size_int;				// Used for calculating the scaled font size to display a text layer character at
	guint16				green_component;			// Used when retrieving the foreground color of text
	SWFShape			highlight_box = NULL;		// Temporary swf shape used when constructing highlight boxes
	gint				highlight_box_height;		// Used while generating swf output for highlight boxes
	gint				highlight_box_width;		// Used while generating swf output for highlight boxes
	layer_highlight		*highlight_data;			// Points to the highlight object data inside the layer
	SWFFillStyle		highlight_fill_style;		// Fill style used when constructing highlight boxes
	SWFBitmap			image_bitmap;				// Used to hold a scaled bitmap object
	gint				image_height;				// Temporarily used to store the height of an image
	SWFInput			image_input;				// Used to hold a swf input object
	SWFShape			image_shape;				// Used to hold a swf shape object
	gint				image_width;				// Temporarily used to store the width of an image
	gint				line_counter;				// Simple loop counter variable
	gfloat				*line_descents = NULL;		// Array used to store the descents of text lines
	gfloat				line_height;
	gfloat				*line_heights = NULL;		// Array used to store the heights of text lines
	gfloat				line_leading;
	gboolean			line_leading_known;			// Simple boolean to track if we know the "leading" for a line yet
	gfloat				line_width;
	gint				loop_counter;				// Simple counter used in loops
	gfloat				max_char_descent;			// Used for calculating the descent of a line
	gfloat				max_line_width = 0;
	GString				*message;					// Used to construct message strings
	gboolean			more_chars;					// Simple boolean used when rendering a text layer
	layer_mouse			*mouse_data;				// Points to the mouse object data inside the layer
	gint				mouse_ptr_width;			// Holds the calculated width for a mouse pointer graphic
	gint				mouse_ptr_height;			// Holds the calculated height for a mouse pointer graphic
	SWFShape			mouse_shape = NULL;			//
	gint				num_lines;
	gint				num_tags;					// Receives the total number of tags applied to a text character
	SWFBlock			our_shape;					// The swf shape before it gets added to a swf movie clip
	PangoFontDescription	*pango_font_face;		// Text layer font face to use when drawing a character
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed png image
	gsize				pixbuf_size;				// Is given the size of a compressed png image
	gfloat				radius = 8;					// Radius to use for rounded rectangles
	guint16				red_component;				// Used when retrieving the foreground colour of text
	GString				*render_string;				// Used for rendering a text layer, one character at a time
	GdkPixbuf			*resized_pixbuf;			// Temporary pixbuf used while scaling images
	gboolean			return_code_bool;			// Receives boolean return codes
	gfloat				scaled_font_size;			// Display height of a font in swf, when scaled to the desired output size
	gint				scaled_height;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to
	gint				scaled_width;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	SWFAction			swf_action;					// Used when constructing action script
	SWFButton			swf_button;					// Holds a swf button
	GtkTextAppearance	*text_appearance;			// Used in text layer rendering, to determine some of the attributes needed
	GtkTextAttributes	*text_attributes;			// Pointer to the attributes for a text layer character
	SWFShape			text_bg = NULL;				// The text background shape goes in this
	gfloat				text_bg_box_height;			// Used while generating swf output for text boxes
	gfloat				text_bg_box_width;			// Used while generating swf output for text boxes
	gunichar			temp_char;					// Used when converting between unicode character types
	SWFText				temp_text_object;			// A temporary text layer text is assembled in this
	SWFDisplayItem		text_bg_display_item;
	SWFFillStyle		text_bg_fill_style;			// Fill style used when constructing text background shape
	GtkTextBuffer		*text_buffer;				// Pointer to the text buffer we're using
	layer_text			*text_data;					// Points to the text object data inside the layer
	SWFDisplayItem		text_display_item;
	gfloat				text_height = 0;			// Height of the text only component of a text layer
	SWFMovieClip		text_movie_clip;			// The movie clip that contains the text background and text
	SWFText				text_object;				// The text layer text is assembled in this
	gdouble				text_pos_x = 0;				// Used for positioning where swf text will be drawn
	gdouble				text_pos_y = 0;				// Used for positioning where swf text will be drawn
	GtkWidget			*text_view;					// Pointer to a temporary text view
	GtkTextTag			*this_tag = NULL;			// Used in a loop for pointing to individual text tags

	// Initialisation
	as_gstring = g_string_new(NULL);
	message = g_string_new(NULL);
	render_string = g_string_new(NULL);

	// Calculate the height and width scaling values needed for this swf output
	scaled_height_ratio = (gfloat) get_output_height() / (gfloat) get_project_height();
	scaled_width_ratio = (gfloat) get_output_width() / (gfloat) get_project_width();

	// Create the dictionary shape for this layer
	switch (this_layer_data->object_type) {

	case TYPE_GDK_PIXBUF:



		return FALSE;

	case TYPE_TEXT:
		// We're processing a text layer

		// Simplify pointers and work out element positioning info
		text_data = (layer_text *) this_layer_data->object_data;
		text_buffer = text_data->text_buffer;

		// Create the text objects we'll be using

		// Create a text view, not for display, but instead to retrieve attribute information from the layer's text buffer
		text_view = gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(text_buffer));

		// Retrieve the default attributes for the text view
		text_attributes = gtk_text_view_get_default_attributes(GTK_TEXT_VIEW(text_view));

		// Determine how many lines are in the text buffer, then loop through them
		num_lines = gtk_text_buffer_get_line_count(text_buffer);

		line_heights = g_new0(gfloat, num_lines);
		line_descents = g_new0(gfloat, num_lines);

		for (line_counter = 0; line_counter < num_lines; line_counter++) {
			// Position the text iter at the start of the line
			gtk_text_buffer_get_iter_at_line(text_buffer, &cursor_iter,	line_counter);

			// Loop around, processing all the characters in the text buffer
			line_height = 0;
			max_char_descent = 0.0;
			more_chars = TRUE;
			while (more_chars) {
				// Retrieve the attributes at this cursor position
				gtk_text_iter_get_attributes(&cursor_iter, text_attributes);

				// Simplify pointers
				text_appearance = &(text_attributes->appearance);
				pango_font_face = text_attributes->font;

				// Calculate the size the character should be displayed at
				font_size_int = pango_font_description_get_size(pango_font_face);

				font_size = rint(scaled_height_ratio * font_size_int / PANGO_SCALE);

				// Get the text tags that apply to this iter
				applied_tags = gtk_text_iter_get_tags(&cursor_iter);

				// Run through the list of tags and extract the info that tells us which font face to use
				num_tags = g_slist_length(applied_tags);
				font_face_found = FALSE;
				for (loop_counter = 0; loop_counter < num_tags; loop_counter++) {
					this_tag = g_slist_nth_data(applied_tags, loop_counter);
					char_font_ptr = g_object_get_data(G_OBJECT(this_tag),"font-num");
					if (NULL != char_font_ptr) {
						// Found the required font face info, so set the font face with it
						char_font_face = *char_font_ptr;
						font_face_found = TRUE;

						//TODO: set font
						//SWFText_setFont(temp_text_object, get_fdb_font_object(char_font_face));
					}
				}
				if (FALSE == font_face_found) {
					// Couldn't determine the font face to be applied to this text, so we use the default text layer font face
					//SWFText_setFont(temp_text_object, get_fdb_font_object(get_default_text_font_face()));
				}

				// Free the list of tags, as they're no longer needed
				g_slist_free(applied_tags);

				// Scale the font size
				scaled_font_size = scaled_height_ratio * font_size;

				// text height
				//SWFText_setHeight(temp_text_object, scaled_font_size);

				// Keep track of the largest descent size for this line
				//char_descent = SWFText_getDescent(temp_text_object);
				//if (char_descent > max_char_descent) {
					max_char_descent = char_descent;
				//}

				// Keep track of the largest font size for this line
				if (scaled_font_size > line_height) {
					line_height = scaled_font_size;
				}

				// Are we at the end of a line?
				if (TRUE == gtk_text_iter_ends_line(&cursor_iter)) {
					// We're at the end of a line.  Next, check if we're at the start of a line (ie it's blank)
					// and then skip to the next line if it is
					if (TRUE == gtk_text_iter_starts_line(&cursor_iter)) {
						break;
					}
				}

				// Move to the next character in the text buffer
				gtk_text_iter_forward_cursor_position(&cursor_iter);

				// If we're at the end of the line, then break out of this loop
				if (TRUE == gtk_text_iter_ends_line(&cursor_iter)) {
					more_chars = FALSE;
				}
			}

			// * At this point we've worked out the height of this line *

			// Save the height and descent of this line
			line_heights[line_counter] = line_height;
			line_descents[line_counter] = max_char_descent;

			// Add this line height to the total height of the text
			text_height += line_height;
		}

		// * At this point we have processed all the lines of text, and should therefore know their heights *

		// Loop through the lines again, but this time create the text object
		text_pos_x = 0;
		text_pos_y = line_heights[0] - line_descents[0];
		for (line_counter = 0; line_counter < num_lines; line_counter++) {
			// Position the SWF cursor to the start of the line
			if (line_counter > 0) {
				//SWFText_moveTo(text_object, text_pos_x, text_pos_y);
			} else {
				// if it is first line, fix position of text in text
				//SWFText_moveTo(text_object, text_pos_x, text_pos_y);
			}

			// Ming has a bug that stops us from moving to x position 0, so we have to detect that and work around it
			/*if (0 == text_pos_x) {
				// Try and move X as close as possible to 0.  We can't use 0 in SWFText_moveTo() due to a bug in Ming
				current_ming_scale = Ming_getScale();
				Ming_setScale(1);
				SWFText_moveTo(text_object, 1, 0);
				Ming_setScale(current_ming_scale);
			}*/

			// Position the text iter at the start of the line
			gtk_text_buffer_get_iter_at_line(text_buffer, &cursor_iter, line_counter);

			// Loop around, processing all the characters in this line of the text buffer
			line_height = 0;
			line_width = 0;
			line_leading = 0;
			line_leading_known = FALSE;
			more_chars = TRUE;

			while (more_chars) {
				// Retrieve the attributes at this cursor position
				gtk_text_iter_get_attributes(&cursor_iter, text_attributes);

				// Simplify pointers
				text_appearance = &(text_attributes->appearance);
				pango_font_face = text_attributes->font;
				fg_colour = &(text_appearance->fg_color);

				// Calculate the size the character should be displayed at
				font_size_int = pango_font_description_get_size(pango_font_face);
				font_size = rint(scaled_height_ratio * font_size_int / PANGO_SCALE);

				// Get the text tags that apply to this iter
				applied_tags = gtk_text_iter_get_tags(&cursor_iter);

				// Run through the list of tags and extract the info that tells us which font face to use
				num_tags = g_slist_length(applied_tags);
				font_face_found = FALSE;
				for (loop_counter = 0; loop_counter < num_tags; loop_counter++) {
					this_tag = g_slist_nth_data(applied_tags, loop_counter);
					char_font_ptr = g_object_get_data(G_OBJECT(this_tag), "font-num");
					if (NULL != char_font_ptr) {
						// Found the required font face info, so set the font face with it
						char_font_face = *char_font_ptr;
						font_face_found = TRUE;
						//SWFText_setFont(text_object, get_fdb_font_object(char_font_face));
					}
				}
				if (FALSE == font_face_found) {
					// Couldn't determine the font face to be applied to this text, so we use the default text layer font face
					//SWFText_setFont(text_object, get_fdb_font_object(get_default_text_font_face()));
				}

				// Free the list of tags, as they're no longer needed
				g_slist_free(applied_tags);

				// Set the font size
				//scaled_font_size = scaled_height_ratio * font_size;
				//SWFText_setHeight(text_object, scaled_font_size);

				// Set the foreground colour for the text
				red_component = roundf(fg_colour->red / 256);
				green_component = roundf(fg_colour->green / 256);
				blue_component = roundf(fg_colour->blue / 256);
				//SWFText_setColor(text_object, red_component, green_component,blue_component, 0xff);

				// Retrieve the character we're working with
				temp_char = gtk_text_iter_get_char(&cursor_iter);
				conversion_buffer = g_ucs4_to_utf8(&temp_char, 1, NULL, NULL, NULL);
				g_string_printf(render_string, "%s", conversion_buffer);

				// If there is a character to be displayed, add it to the text object
				if (FALSE == gtk_text_iter_ends_line(&cursor_iter)) {
					// Set the character to be written
					//flex_mxml_rgb_t rgb = {red_component,green_component,blue_component};
					flex_mxml_rgb_t rgb = {255,255,255};
					flex_mxml_shape_add_text(dom, text_pos_x, text_pos_y, render_string->str, "tahoma", 36, rgb);

					printf("render string %s\n",render_string->str);

					//SWFText_addUTF8String(text_object, render_string->str, NULL);
				} else {
					// We're at the end of a line, lets also check if we're at the start of a line (ie it's blank)
					// and then skip to the next one if so
					if (TRUE == gtk_text_iter_starts_line(&cursor_iter)) {
						break;
					}
				}
				if (FALSE == line_leading_known) {
					// We add the "leading" for the very first character of a line, so
					// the display of text starts that little bit further to the right
					//line_leading = SWFText_getLeading(text_object);
					line_width += line_leading;
					line_leading_known = TRUE;
				}

				line_width += 150;

				//line_width += SWFText_getUTF8StringWidth(text_object,(guchar *) render_string->str);

				// Move to the next character in the text buffer
				gtk_text_iter_forward_cursor_position(&cursor_iter);

				// If we're at the end of the line, then break out of this loop
				if (TRUE == gtk_text_iter_ends_line(&cursor_iter)) {
					more_chars = FALSE;
				}
				text_pos_x += 150;
			}

			text_pos_y += 150;


			// fixme4: Unsure if this "line leading" stuff is useful for us in this function
			/*if (TRUE == line_leading_known) {
				text_pos_x = line_leading;
			}*/

			// Move the swf pen to the start of the next line
			if (line_counter < num_lines - 1) {
				//text_pos_y += line_heights[line_counter + 1] + line_descents[line_counter] - line_descents[line_counter + 1] + (scaled_height_ratio * TEXT_BORDER_PADDING_HEIGHT);
			} else {
				// text_pos_y already have height, just add board height
				//text_pos_y += (scaled_height_ratio * TEXT_BORDER_PADDING_HEIGHT);
			}

			// Keep the largest line width known
			if (line_width > max_line_width) {
				max_line_width = line_width;
			}
		}

		// * Create the swf movie clip object that holds the text background and text *
		//text_movie_clip = newSWFMovieClip();

		// * Create the background for the text object *
		/*if (TRUE == text_data->show_bg) {
			text_bg = newSWFShape();
			if (NULL == text_bg) {
				// Something went wrong when creating the empty shape, so we skip this layer
				g_string_printf(
						message,
						"%s ED101: %s",
						_("Error"),
						_("Something went wrong when creating a text layer background for swf output."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
			}

			// Set the solid fill for the text background box
			red_component = roundf(text_data->bg_fill_colour.red / 256);
			green_component = roundf(text_data->bg_fill_colour.green / 256);
			blue_component = roundf(text_data->bg_fill_colour.blue / 256);
			text_bg_fill_style = SWFShape_addSolidFillStyle(text_bg,
					red_component, green_component, blue_component, 0xff); // Alpha of 0xff is full opacity
			SWFShape_setRightFillStyle(text_bg, text_bg_fill_style);

			// Set the line style
			red_component = roundf(text_data->bg_border_colour.red / 256);
			green_component = roundf(text_data->bg_border_colour.green / 256);
			blue_component = roundf(text_data->bg_border_colour.blue / 256);

			// gtk line border width and flash border line width is not identical. gtk line width is more than 2 times greate then flash line widht
			SWFShape_setLine(text_bg, text_data->bg_border_width / 2,
					red_component, green_component, blue_component, 0xff); // Alpha of 0xff is full opacity

			// Calculate the dimensions of the text background box
			text_bg_box_height = text_pos_y + (scaled_height_ratio * TEXT_BORDER_PADDING_HEIGHT * 2);
			text_bg_box_width = max_line_width + (scaled_width_ratio * TEXT_BORDER_PADDING_WIDTH * 2);

			// Draw the background curved rectangle
			SWFShape_movePenTo(text_bg, radius, 0);
			SWFShape_drawLineTo(text_bg, text_bg_box_width - radius, 0);
			SWFShape_drawCurveTo(text_bg, text_bg_box_width, 0,
					text_bg_box_width + 0.5, radius);
			SWFShape_drawLineTo(text_bg, text_bg_box_width, text_bg_box_height
					- radius);
			SWFShape_drawCurveTo(text_bg, text_bg_box_width,
					text_bg_box_height, text_bg_box_width - radius,
					text_bg_box_height + 0.5);
			SWFShape_drawLineTo(text_bg, radius, text_bg_box_height);
			SWFShape_drawCurveTo(text_bg, 0, text_bg_box_height, 0.5,
					(text_bg_box_height - radius));
			SWFShape_drawLineTo(text_bg, 0, radius);
			SWFShape_drawCurveTo(text_bg, 0, 0, radius, 0.5);

			// Add the text background to the movie clip
			text_bg_display_item = SWFMovieClip_add(text_movie_clip,
					(SWFBlock) text_bg);
		}*/

		// Add the text object to the movie clip
		//text_display_item = SWFMovieClip_add(text_movie_clip,(SWFBlock) text_object);

		// Position the text element
		/*SWFDisplayItem_moveTo(text_display_item, scaled_width_ratio
				* TEXT_BORDER_PADDING_WIDTH, scaled_height_ratio
				* TEXT_BORDER_PADDING_HEIGHT);*/

		// Advance the movie clip one frame, else it won't be displayed
		//SWFMovieClip_nextFrame(text_movie_clip);

		// If this layer has an external link associated with it, turn it into a button
		/*if (0 < this_layer_data->external_link->len) {
			// Displaying debugging info if requested
			if (get_debug_level()) {
				printf(_("This text has an external link: '%s'\n"),
						this_layer_data->external_link->str);
			}

			// Create an empty button object we can use
			swf_button = newSWFButton();

			// Add the shape to the button for all of its states, excluding the hit state
			SWFButton_addShape(swf_button, (SWFCharacter) text_movie_clip, SWFBUTTON_UP | SWFBUTTON_OVER | SWFBUTTON_DOWN);

			// Use the text background area as the hit state
			if (TRUE == text_data->show_bg) {
				SWFButton_addShape(swf_button, (SWFCharacter) text_bg,
						SWFBUTTON_HIT);
			} else {
				SWFButton_addShape(swf_button, (SWFCharacter) text_movie_clip,
						SWFBUTTON_HIT);
			}

			// Add action script to the button, jumping to the external link
			g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");",
					this_layer_data->external_link->str,
					this_layer_data->external_link_window->str);
			swf_action = compileSWFActionCode(as_gstring->str);
			SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

			// Add the dictionary shape to a movie clip, then store for future reference
			our_shape = (SWFBlock) swf_button;
			this_layer_data->dictionary_shape = newSWFMovieClip();
			SWFMovieClip_add(this_layer_data->dictionary_shape,
					(SWFBlock) our_shape);

			// Advance the movie clip one frame, else it doesn't get displayed
			SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
		} else {
			// Add the dictionary shape to a movie clip, then store for future reference
			our_shape = (SWFBlock) text_movie_clip;
			this_layer_data->dictionary_shape = newSWFMovieClip();
			SWFMovieClip_add(this_layer_data->dictionary_shape,
					(SWFBlock) our_shape);

			// Advance the movie clip one frame, else it doesn't get displayed
			SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
		}*/

		// Free the memory used in this function
		g_string_free(render_string, TRUE);
		g_free(line_heights);
		g_free(line_descents);

		// Indicate that the dictionary shape for this layer was created ok
		return TRUE;

		break;

	default:
		// Unknown type
		/*g_string_printf(message, "%s ED83: %s", _("Error"),
				_("Unknown layer type in swf output."));
		display_warning(message->str);
		g_string_free(message, TRUE);*/

		printf("unknow layer type in swf output");

		return FALSE;
	}

		// Free the memory used in this function
		g_string_free(as_gstring, TRUE);
		g_string_free(render_string, TRUE);
		g_string_free(message, TRUE);

	return TRUE;
}

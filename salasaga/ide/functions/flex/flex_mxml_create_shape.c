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


		return FALSE;
		// We're processing a text layer

		// Simplify pointers and work out element positioning info
		text_data = (layer_text *) this_layer_data->object_data;
		text_buffer = text_data->text_buffer;

		// Create a text view, not for display, but instead to retrieve attribute information from the layer's text buffer
		text_view = gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(text_buffer));

		// Retrieve the default attributes for the text view
		text_attributes = gtk_text_view_get_default_attributes(
				GTK_TEXT_VIEW(text_view));

		// Determine how many lines are in the text buffer, then loop through them
		num_lines = gtk_text_buffer_get_line_count(text_buffer);
		line_heights = g_new0(gfloat, num_lines);
		line_descents = g_new0(gfloat, num_lines);

	default:
		// Unknown type
		g_string_printf(message, "%s ED83: %s", _("Error"),
				_("Unknown layer type in swf output."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}

		// Free the memory used in this function
		g_string_free(as_gstring, TRUE);
		g_string_free(render_string, TRUE);
		g_string_free(message, TRUE);

	return TRUE;
}

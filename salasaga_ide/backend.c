/*
 * $Id$
 *
 * Flame Project: Source file for all general backend functions
 * 
 * Copyright (C) 2006 Justin Clift <justin@postgresql.org>
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
#include "flame-types.h"
#include "backend.h"
#include "callbacks.h"
#include "gui-functions.h"
#include "externs.h"

// Function to decode a Base64 encoded string
GString *base64_decode(GString *input_string, GString *output_string)
{
	// Local variables
	GString				*copied_string;			// A copy of the input string
	guint				counter;				// Counter of how many characters have been converted
	guchar				holding_byte;			// Holds the byte being worked on
	gdouble				max_loop;				// The maximum number of interations to run

	guchar				out_byte0;				// Used to hold the bytes being translated
	guchar				out_byte1;				// Used to hold the bytes being translated
	guchar				out_byte2;				// Used to hold the bytes being translated
	gint				string_offset;			// Used as an offset pointer while parsing

	GString				*tmp_gstring;			// Temporary GString


	// Initialise various things
	g_string_assign(output_string, "");
	tmp_gstring = g_string_new(NULL);

	// Make a copy of the input string so we can work on it in place
	copied_string = g_string_new(NULL);

	// Lookup each byte of the input string for it's numeric value
	for (counter = 0; counter < input_string->len - 1; counter++)
	{

// Notes:
//
//	base64_dictionary_offsets = "0 ABCDEFGHIJKLMNOPQRSTUVWXYZ"
// 								"26 abcdefghijklmnopqrstuvwxyz"
//								"52 0123456789"
//								"62 +"
//								"63 /"
//
// ASCII values
// + = 43
// / = 47
// 0 = 48
// 9 = 57
// = = 61
// A = 65
// Z = 90
// a = 97
// z = 122

		holding_byte = input_string->str[counter];

		if (48 <= holding_byte)
		{
			// * To get here, it must be in the ranges of 0-9, A-Z, or a-z *

			if (65 <= holding_byte)
			{
				// * To get here, it must be in the ranges of A-Z or a-z *

				if (97 <= holding_byte)
				{
					// * To get here, it must be in the range of a-z *
					copied_string = g_string_append_c(copied_string, (guchar) holding_byte - 71);  // a = ASCII 97.  97 - 71 = 26.  So a = 26, b = 27, etc
				} else
				{
					// * To get here, it must be in the range of A-Z *
					copied_string = g_string_append_c(copied_string, (guchar) holding_byte - 65);  // 65 Makes A = 0, B = 1, etc
				}
			} else
			{
				if (61 == holding_byte)
				{
					// * To get here it must be the character '=', which means "no value", so don't add to the output string *
				} else
				{
					// * To get here it must be in the range 0-9 *
					copied_string = g_string_append_c(copied_string, (guchar) holding_byte + 4);  // 0 = ASCII 48.  48 + 4 = 52.  So 0 = 52, 1 = 53, etc
				}
			}
		} else
		{
			// Must be either +, /, or \n
			switch (input_string->str[counter])
			{
				case '+':
					copied_string = g_string_append_c(copied_string, (guchar) 62);
					break;

				case '/':
					copied_string = g_string_append_c(copied_string, (guchar) 63);
					break;

				case '\n':
					break;

				default:
					// Should never get here
					g_string_printf(tmp_gstring, "ED64: Error in the Base64 decoding function, unrecognised input '%c'.", holding_byte);
					display_warning(tmp_gstring->str);
			}
		}
	}

	// * To get here, the copied string should have been converted to it's offset values
	counter = 0;
	max_loop = (copied_string->len / 4);
	while (counter != max_loop)
	{
		string_offset = counter * 4;

		out_byte0 = (guchar) (copied_string->str[string_offset] << 2) | (copied_string->str[string_offset + 1] >> 4);
		out_byte1 = (guchar) (copied_string->str[string_offset + 1] << 4) | (copied_string->str[string_offset + 2] >> 2);
		out_byte2 = (guchar) (copied_string->str[string_offset + 2] << 6) | (copied_string->str[string_offset + 3]);

		output_string = g_string_append_c(output_string, out_byte0);
		output_string = g_string_append_c(output_string, out_byte1);
		output_string = g_string_append_c(output_string, out_byte2);

		counter++;
	}

	switch ((gint) copied_string->len % 4)
	{
		case 0:
			// No remainder, nothing to do
			break;

		case 2:
			// We only need to process two bytes, producing one char
			out_byte0 = copied_string->str[copied_string->len - 3];
			out_byte0 = out_byte0 << 2;

			out_byte1 = copied_string->str[copied_string->len - 2];
			out_byte1 = out_byte1 >> 4;

			out_byte2 = out_byte0 | out_byte1;
			output_string = g_string_append_c(output_string, out_byte2);
			break;

		case 3:
			// We only need to process three bytes, producing two chars
			out_byte0 = (guchar) (copied_string->str[copied_string->len - 3] << 2) | (copied_string->str[copied_string->len - 2] >> 4);
			out_byte1 = (guchar) (copied_string->str[copied_string->len - 2] << 4) | (copied_string->str[copied_string->len - 1] >> 2);
			output_string = g_string_append_c(output_string, out_byte0);
			output_string = g_string_append_c(output_string, out_byte1);
			break;

		default:
			display_warning("ED73: Unknown Base64 decoding remainder (shouldn't happen)\n");
	}

	// Free the memory allocated in this function
	g_string_free(tmp_gstring, TRUE);
	g_string_free(copied_string, TRUE);

	return output_string;
}


// Function to create a Base64 encoded string from a block of data in memory
gboolean base64_encode(gpointer data, guint length, gchar **output_string)
{
	// Local variables
	gchar				*base64_dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	guint				buffer_length;
	guint				characters_out;			// Counter of how many characters have been output
	guint				counter;				// Counter of how many characters have been converted
	gchar				*input_buffer;
	gdouble				max_loop;				// The maximum number of interations to run
	guint				offset;					// Either 0, 1, or 2.  Used to calculate the end few output bytes
	gchar				*output_buffer;
	gint				output_counter;			// Counter used for positioning inside the output buffer
	gint				string_offset;			// Used as an offset pointer while parsing

	guchar				first_byte;
	guchar				fourth_byte;
	guchar				second_byte;
	guchar				third_byte;

	guchar				tmp_byte;


	// Initialise some things
	input_buffer = data;
	output_counter = 0;
	characters_out = 0;

	// Calculate the length of the Base64 buffer
	buffer_length = (guint) ((float) length * 1.5);  // Overestimate, to be on the safe side

	// Create the Base64 buffer
	output_buffer = g_new(gchar, buffer_length);

	// Calculate how many bytes are left hanging on the end of the input
	offset = length % 3;

	// Convert the data into Base64
	counter = 0;
	max_loop = length / 3;
	while (counter != max_loop)
	{
		string_offset = counter * 3;

		first_byte = input_buffer[string_offset];
		first_byte = first_byte >> 1;
		first_byte = first_byte >> 1;
		output_buffer[output_counter] = base64_dictionary[first_byte];
		output_counter++;

		second_byte = input_buffer[string_offset];
		second_byte = second_byte << 6;
		second_byte = second_byte >> 2;
		tmp_byte = input_buffer[string_offset + 1];
		tmp_byte = tmp_byte >> 4;
		second_byte = second_byte | tmp_byte;
		output_buffer[output_counter] = base64_dictionary[second_byte];
		output_counter++;

		third_byte = input_buffer[string_offset + 1];
		third_byte = third_byte << 4;
		third_byte = third_byte >> 2;
		tmp_byte = input_buffer[string_offset + 2];
		tmp_byte = tmp_byte >> 6;
		third_byte = third_byte | tmp_byte;
		output_buffer[output_counter] = base64_dictionary[third_byte];
		output_counter++;

		fourth_byte = input_buffer[string_offset + 2];
		fourth_byte = fourth_byte << 2;
		fourth_byte = fourth_byte >> 2;
		output_buffer[output_counter] = base64_dictionary[fourth_byte];
		output_counter++;

		characters_out = characters_out + 4;

		// Insert a newline into the output stream every 76 characters (as per spec)
		if (0 == (characters_out % 76))
		{
			output_buffer[output_counter] = '\n';
			output_counter++;
		}

		counter++;
	}

	// Process the last one or two bytes on the end of the input data (as per Base64 requirement)
	switch ((gint) length % 3)
	{
		case 0:
			// No remainder, nothing to do
			break;

		case 1:
			// We only need to process one byte, to make two characters
			first_byte = input_buffer[length - 1];
			first_byte = first_byte >> 2;
			output_buffer[output_counter] = base64_dictionary[first_byte];
			output_counter++;

			second_byte = input_buffer[length - 1];
			second_byte = second_byte << 6;
			second_byte = second_byte >> 2;
			output_buffer[output_counter] = base64_dictionary[second_byte];
			output_counter++;
			output_buffer[output_counter] = '=';
			output_counter++;
			output_buffer[output_counter] = '=';
			output_counter++;
			break;

		case 2:
			// We only need to process two bytes, to make three characters
			first_byte = input_buffer[length - 2];
			first_byte = first_byte >> 2;
			output_buffer[output_counter] = base64_dictionary[first_byte];
			output_counter++;

			second_byte = input_buffer[length - 2];
			second_byte = second_byte << 6;
			second_byte = second_byte >> 2;
			tmp_byte = input_buffer[length - 1];
			tmp_byte = tmp_byte >> 4;
			second_byte = second_byte | tmp_byte;
			output_buffer[output_counter] = base64_dictionary[second_byte];
			output_counter++;

			third_byte = input_buffer[length - 1];
			third_byte = third_byte << 4;
			third_byte = third_byte >> 2;
			output_buffer[output_counter] = base64_dictionary[third_byte];
			output_counter++;
			output_buffer[output_counter] = '=';
			output_counter++;
			break;

		default:
			display_warning("ED79: Unknown remainder amount when Base64 encoding (shouldn't happen)\n");
	}

	// Put a NULL at the end of the Base64 encoded string
	output_buffer[output_counter] = '\0';

	output_string[0] = output_buffer;
	return TRUE;
}


// Function to calculate collision detection boundaries
void calculate_object_boundaries(void)
{
	// Local variables
	GList				*layer_pointer;
	guint				num_layers;				// The number of layers in the slide
	guint				count_int;				// Counter

	boundary_box		*boundary;				// Boundary information

	guint				tmp_int;				//
	GdkRectangle		tmp_rectangle;			//


	// Only continue in this function if we have a slide structure available
	if (NULL == current_slide)
	{
		return;
	}

	// Work out how many layers we need to iterate through
	layer_pointer = g_list_first(((slide *) current_slide->data)->layers);
	num_layers = g_list_length(layer_pointer);

	// (Re-)Initialise the boundary list
	if (NULL != boundary_list)
	{
		boundary_list = g_list_first(boundary_list);
		g_list_foreach(boundary_list, (gpointer) g_free, NULL);  // I *think* this will free the memory allocated for the GdkRegions.
		g_list_free(boundary_list);
		boundary_list = NULL;
	}

	for (count_int = 0; count_int <= num_layers - 1; count_int++)
	{
		// Select the desired layer
		layer_pointer = g_list_first(layer_pointer);
		layer_pointer = g_list_nth(layer_pointer, count_int);

		// Determine the layer type then calculate its boundaries accordingly
		switch (((layer *) layer_pointer->data)->object_type)
		{
			case TYPE_EMPTY:
				// No boundaries to calculate
				tmp_rectangle.width = 0;  // Use this as a flag to indicate we're skipping this layer
				break;

			case TYPE_GDK_PIXBUF:
				// If we're processing the background layer, we skip it
				tmp_int = g_ascii_strncasecmp(((layer *) layer_pointer->data)->name->str, "Background", 10);
				if (0 == tmp_int)
				{
					tmp_rectangle.width = 0;  // Use this as a flag to indicate we're skipping this layer
					break;
				}

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_image *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (((layer_image *) ((layer *) layer_pointer->data)->object_data)->width * zoom) / 100;
				tmp_rectangle.y = (((layer_image *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_image *) ((layer *) layer_pointer->data)->object_data)->height * zoom) / 97;
				break;

			case TYPE_HIGHLIGHT:
				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_highlight *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (((layer_highlight *) ((layer *) layer_pointer->data)->object_data)->width * zoom) / 100;
				tmp_rectangle.y = (((layer_highlight *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_highlight *) ((layer *) layer_pointer->data)->object_data)->height * zoom) / 97;
				break;

			case TYPE_MOUSE_CURSOR:
				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_mouse *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (((layer_mouse *) ((layer *) layer_pointer->data)->object_data)->width * zoom) / 100;
				tmp_rectangle.y = (((layer_mouse *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_mouse *) ((layer *) layer_pointer->data)->object_data)->height * zoom) / 97;
				break;

			case TYPE_TEXT:
				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 102;
				tmp_rectangle.width = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->rendered_width * zoom) / 101;
				tmp_rectangle.y = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->rendered_height * zoom) / 97;
				break;

			default:
				display_warning("ED27: Unknown layer type\n");
		}

		// * Store the calculated boundary *

		// Skip the background layer
		if (0 != tmp_rectangle.width)
		{
			// Allocate memory for a new GdkRegion, then add it to the boundary list
			boundary = g_new(boundary_box, 1);
			boundary->region_ptr = gdk_region_rectangle(&tmp_rectangle);
			boundary->layer_ptr = layer_pointer;
			boundary_list = g_list_first(boundary_list);
			boundary_list = g_list_append(boundary_list, boundary);
		}
	}
}


// Function to free the memory for a given slide
void destroy_slide(gpointer element, gpointer user_data)
{
	// Local variables
	gint				layer_counter;			// Standard counter
	layer				*layer_data;			// Used for freeing the elements of a deleted slide
	gint				num_layers;				// Number of layers in a slide
	GList				*removed_layer;			// Used for freeing the elements of a deleted slide
	slide				*slide_data;			// Used for freeing the elements of a deleted slide


	// Initialise some things
	slide_data = element;

	// Remove the mouse click handler from the event box of the deleted slide
	g_signal_handler_disconnect(G_OBJECT(slide_data->event_box), slide_data->click_handler);

	// Free the memory allocated to the deleted slide
	if (NULL != slide_data->name)
	{
		g_string_free(slide_data->name, TRUE);
	}
	gtk_object_destroy(GTK_OBJECT(slide_data->tooltip));
	g_object_unref(slide_data->layer_store);
	g_object_unref(slide_data->timeline_widget);

	// Free the memory allocated to the deleted slides' layers
	slide_data->layers = g_list_first(slide_data->layers);
	num_layers = g_list_length(slide_data->layers);
	for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
	{
		// Point to the first remaining layer in the list
		slide_data->layers = g_list_first(slide_data->layers);
		layer_data = (slide_data->layers)->data;

		// Free its elements
		g_string_free(layer_data->name, TRUE);
		g_free(layer_data->row_iter);
		switch (layer_data->object_type)
		{
			case TYPE_GDK_PIXBUF:
				g_string_free(((layer_image *) layer_data->object_data)->image_path, TRUE);
				g_object_unref(((layer_image *) layer_data->object_data)->image_data);
				break;

			case TYPE_MOUSE_CURSOR:
				// Nothing here needs freeing
				break;

			case TYPE_EMPTY:
				// Nothing here needs freeing
				break;

			case TYPE_TEXT:
				g_object_unref(((layer_text *) layer_data->object_data)->text_buffer);
				break;

			case TYPE_HIGHLIGHT:
				// Nothing here needs freeing
				break;

			default:
				display_warning("ED57: Unknown layer type when destroying a slide.\n");
		}
		g_free(layer_data->object_data);

		// Remove the layer from the list
		removed_layer = slide_data->layers;
		slide_data->layers = g_list_remove_link(slide_data->layers, slide_data->layers);
		g_list_free(removed_layer);
	}
}


// Function to detect collisions between a given coordinate and a GList of boundary boxes
GList *detect_collisions(GList *collision_list, gdouble mouse_x, gdouble mouse_y)
{
	// Local variables
	guint				count_int;
	guint				num_boundaries;

	boundary_box			*boundary;


	// Only do this if we've been given a list of boundary boxes
	if (NULL == boundary_list)
	{
		return NULL;
	}

	// Check if there are any boundaries to detect collisions with
	boundary_list = g_list_first(boundary_list);
	num_boundaries = g_list_length(boundary_list);
	if (0 == num_boundaries)
	{
		//  No boundaries given, so return
		return NULL;
	}

	// Yes there are boundaries to check
	for (count_int = 0; count_int < num_boundaries; count_int++)
	{
		boundary_list = g_list_first(boundary_list);
		boundary = g_list_nth_data(boundary_list, count_int);
		if (TRUE == gdk_region_point_in(boundary->region_ptr, (guint) mouse_x, (guint) mouse_y))
		{
			// Collision detected, so add it to the collision list
			collision_list = g_list_first(collision_list);
			collision_list = g_list_append(collision_list, boundary);
		}
	}

	return collision_list;
}


// Function to parse the contents of a .flame file
gboolean flame_read(gchar *filename)
{
	// Local variables
	GError				*error = NULL;			// Pointer to error return structure
	GString				*error_string;			// Used to create error strings
	GdkPixbufLoader		*image_loader;			// Used for loading images embedded in project files
	gboolean			return_code;			// Boolean return code
	gfloat				save_version;			// The project file save version
	xmlDocPtr			document;				// Holds a pointer to the XML document
	xmlNodePtr			layer_ptr;				// Temporary pointer
	xmlNodePtr			meta_data_node = NULL;	// Points to the meta-data structure
	xmlNodePtr			preferences_node = NULL;// Points to the preferences structure
	xmlNodePtr			slides_node = NULL;		// Points to the slides structure
	xmlNodePtr			this_layer;				// Temporary pointer
	xmlNodePtr			this_node;				// Temporary pointer
	xmlNodePtr			this_slide;				// Temporary pointer

	xmlChar				*project_name_data = NULL;
	xmlChar				*output_folder_data = NULL;
	xmlChar				*output_width_data = NULL;
	xmlChar				*output_height_data = NULL;
	xmlChar				*output_quality_data = NULL;
	xmlChar				*project_width_data = NULL;
	xmlChar				*project_height_data = NULL;
	xmlChar				*save_format_data = NULL;
	xmlChar				*slide_length_data = NULL;

	xmlChar				*tmp_char;				// Temporary string pointer
	layer_empty			*tmp_empty_ob;			//
	GList				*tmp_glist;				//
	GString				*tmp_gstring;			// Temporary GString
	GString				*tmp_gstring2;			// Temporary GString
	layer_highlight		*tmp_highlight_ob;		// Temporary highlight layer object
	layer_image			*tmp_image_ob;			// Temporary image layer object
	gint				tmp_int;				// Temporary integer
	GtkTreeIter			*tmp_iter;				// Temporary GtkTreeIter
	layer				*tmp_layer;				// Temporary layer
	layer_mouse			*tmp_mouse_ob;			// Temporary mouse layer object
	GdkPixbuf			*tmp_pixbuf;			//
	slide				*tmp_slide;				// Temporary slide
	layer_text			*tmp_text_ob;			// Temporary text layer object

	gint				data_length;			// Number of image data bytes a layer says it stores


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	tmp_gstring2 = g_string_new(NULL);
	error_string = g_string_new(NULL);
	
	// Begin reading the file
	document = xmlParseFile(filename);
	if (NULL == document)
	{
		// The Flame file was unable to be parsed
		display_warning("ED43: Project file was unable to be loaded\n");
		return FALSE;
	}

	this_node = xmlDocGetRootElement(document);
	if (NULL == this_node)
	{
		display_warning("ED44: Project file is empty");
		xmlFreeDoc(document);
		return FALSE;
	}

	// Scan for the "meta-data" node
	this_node = this_node->xmlChildrenNode;
	while (NULL != this_node)
	{
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "meta-data")))
		{
			// Meta-data node found
			meta_data_node = this_node;
		}
		this_node = this_node->next;
	}

	// Scan for the "preferences" node
	this_node = xmlDocGetRootElement(document);
	this_node = this_node->xmlChildrenNode;
	while (NULL != this_node)
	{
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "preferences")))
		{
			// Preferences node found
			preferences_node = this_node;
		}
		this_node = this_node->next;
	}

	// Scan for the "slides" node
	this_node = xmlDocGetRootElement(document);
	this_node = this_node->xmlChildrenNode;
	while (NULL != this_node)
	{
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "slides")))
		{
			// slides node found
			slides_node = this_node;
		}
		this_node = this_node->next;
	}

	// Was there a meta-data structure in the save file?
	if (NULL == meta_data_node)
	{
		display_warning("ED63: Project meta-data missing, aborting load");
		return FALSE;
	}

	// Was there a preferences structure in the save file?
	if (NULL == preferences_node)
	{
		display_warning("ED45: Project preferences missing, aborting load");
		return FALSE;
	}

	// Was there a slides structure in the save file?
	if (NULL == slides_node)
	{
		display_warning("ED46: No slides in project, aborting load");
		return FALSE;
	}

	// Read in the project meta data
	this_node = meta_data_node->xmlChildrenNode;
	while (NULL != this_node)
	{
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "save_format")))
		{
			// Save format version found.  Extract and store it
			save_format_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		this_node = this_node->next;
	}

	// Read in the project preferences
	this_node = preferences_node->xmlChildrenNode;
	while (NULL != this_node)
	{
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "project_name")))
		{
			// Project Name found.  Extract and store it
			project_name_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "output_folder")))
		{
			// Output Folder found.  Extract and store it
			output_folder_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "output_width")))
		{
			// Output Width found.  Extract and store it
			output_width_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "output_height")))
		{
			// Output Height found.  Extract and store it
			output_height_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "output_quality")))
		{
			// Output Quality found.  Extract and store it
			output_quality_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "project_width")))
		{
			// Project Width found.  Extract and store it
			project_width_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "project_height")))
		{
			// Project Height found.  Extract and store it
			project_height_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "slide_length")))
		{
			// Slide Length found.  Extract and store it
			slide_length_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		this_node = this_node->next;
	}

	// If any of the critically needed preferences are missing, display a warning then abort
	if (NULL == project_name_data)
	{
		display_warning("ED47: Project Name missing, aborting project open");
		return FALSE;
	}
	if (NULL == project_width_data)
	{
		display_warning("ED48: Project Width missing, aborting project open");
		return FALSE;
	}
	if (NULL == project_height_data)
	{
		display_warning("ED49: Project Height missing, aborting project open");
		return FALSE;
	}

	// * All of the required meta-data and preferences are present, so we proceed *

	// If there's a project presently loaded in memory, we unload it
	if (NULL != slides)
	{
		// Free the resources presently allocated to slides
		g_list_foreach(slides, destroy_slide, NULL);

		// Re-initialise pointers
		slides = NULL;
		current_slide = NULL;
	}

	// Load project file format version
	save_version = atoi((const char *) save_format_data);
	xmlFree(save_format_data);

	// Load project name
	if (NULL == project_name)
		project_name = g_string_new(NULL);
	g_string_assign(project_name, (const gchar *) project_name_data);
	xmlFree(project_name_data);

	// Load output folder
	if (NULL == output_folder)
		output_folder = g_string_new(NULL);
	g_string_assign(output_folder, (const gchar *) output_folder_data);
	xmlFree(output_folder_data);

	// Load output width
	output_width = atoi((const char *) output_width_data);
	xmlFree(output_width_data);

	// Load output height
	output_height = atoi((const char *) output_height_data);
	xmlFree(output_height_data);

	// Load output quality
	output_quality = atoi((const char *) output_quality_data);
	xmlFree(output_quality_data);

	// Load project_width
	project_width = atoi((const char *) project_width_data);
	xmlFree(project_width_data);

	// Load project height
	project_height = atoi((const char *) project_height_data);
	xmlFree(project_height_data);

	// Load slide length
	slide_length = atoi((const char *) slide_length_data);
	xmlFree(slide_length_data);

	// * Preferences are loaded, so now load the slides *
	this_slide = slides_node->xmlChildrenNode;
	while (NULL != this_slide)
	{
		if ((!xmlStrcmp(this_slide->name, (const xmlChar *) "slide")))
		{
			// * We're in a slide *

			// Create a new slide in memory
			tmp_slide = g_new(slide, 1);
			tmp_slide->layers = NULL;
			tmp_slide->layer_store = gtk_list_store_new(TIMELINE_N_COLUMNS,  // TIMELINE_N_COLUMNS
										G_TYPE_STRING,  // TIMELINE_NAME
										G_TYPE_BOOLEAN,  // TIMELINE_VISIBILITY
										G_TYPE_UINT,  // TIMELINE_START
										G_TYPE_UINT,  // TIMELINE_FINISH
										G_TYPE_UINT,  // TIMELINE_X_OFF_START
										G_TYPE_UINT,  // TIMELINE_Y_OFF_START
										G_TYPE_UINT,  // TIMELINE_X_OFF_FINISH
										G_TYPE_UINT);  // TIMELINE_Y_OFF_FINISH

			// Process each layer
			this_layer = this_slide->xmlChildrenNode;
			while (NULL != this_layer)
			{
				if ((!xmlStrcmp(this_layer->name, (const xmlChar *) "layer")))
				{
					// We're in a layer, so determine the layer type and load it
					layer_ptr = this_layer->xmlChildrenNode;
					while (NULL != layer_ptr)
					{
						if ((!xmlStrcmp(layer_ptr->name, (const xmlChar *) "type")))
						{
							// Found the layer type attribute.  Process it
							tmp_char = xmlNodeListGetString(document, layer_ptr->xmlChildrenNode, 1);

							// Test if this layer is an empty layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "empty"))
							{
								// Construct a new image layer
								tmp_empty_ob = g_new(layer_empty, 1);
								tmp_layer = g_new(layer, 1);	
								tmp_layer->object_type = TYPE_EMPTY;
								tmp_layer->object_data = (GObject *) tmp_empty_ob;
								tmp_layer->external_link = g_string_new(NULL);

								// Load the empty layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "red")))
									{
										// Get the red value
										tmp_empty_ob->bg_color.red = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "green")))
									{
										// Get the green value
										tmp_empty_ob->bg_color.green = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "blue")))
									{
										// Get the blue value
										tmp_empty_ob->bg_color.blue = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										tmp_layer->start_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										tmp_layer->finish_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										tmp_layer->name = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										tmp_layer->external_link = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									this_node = this_node->next;	
								}

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_START, NULL,
										TIMELINE_FINISH, NULL,
										TIMELINE_X_OFF_START, NULL,
										TIMELINE_Y_OFF_START, NULL,
										TIMELINE_X_OFF_FINISH, NULL,
										TIMELINE_Y_OFF_FINISH, NULL,
										-1);

								// Add this (now completed) empty layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
							}

							// Test if this layer is an image layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "image"))
							{
								// Construct a new image layer
								tmp_image_ob = g_new(layer_image, 1);
								tmp_layer = g_new(layer, 1);	
								tmp_layer->object_type = TYPE_GDK_PIXBUF;
								tmp_layer->object_data = (GObject *) tmp_image_ob;
								tmp_layer->external_link = g_string_new(NULL);

								// Load the image layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										tmp_image_ob->x_offset_start = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_start")))
									{
										// Get the starting y offset
										tmp_image_ob->y_offset_start = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_finish")))
									{
										// Get the finishing x offset
										tmp_image_ob->x_offset_finish = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_finish")))
									{
										// Get the finishing y offset
										tmp_image_ob->y_offset_finish = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "width")))
									{
										// Get the width
										tmp_image_ob->width = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "height")))
									{
										// Get the height
										tmp_image_ob->height = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										tmp_layer->start_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										tmp_layer->finish_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										tmp_layer->name = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										tmp_layer->external_link = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}

									// If version 1.0 of file format, load the image path, otherwise load the embedded image data
									if (1.0 == save_version)
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "path")))
										{
											// Get the path to the image data
											tmp_image_ob->image_path = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));

											// Load the image data
											tmp_image_ob->image_data = gdk_pixbuf_new_from_file(tmp_image_ob->image_path->str, NULL);
										}
									} else
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "data")))
										{
											// Get the image data
											g_string_assign(tmp_gstring2, (const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "data_length")))
										{
											// Get the number of bytes the image should hold
											data_length = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										}
									}

									this_node = this_node->next;	
								}

								// Version 1.0 of the file format doesn't have embedded image data
								if (1.0 != save_version)
								{
									// * We should have all of the image details by this stage, so can process the image data *

									// Base64 decode the image data back into png format
									tmp_gstring = base64_decode(tmp_gstring2, tmp_gstring);

									// Convert the png data into a GdxPixbuf we can use
									image_loader = gdk_pixbuf_loader_new();
									return_code = gdk_pixbuf_loader_write(image_loader, (const guchar *) tmp_gstring->str, tmp_gstring->len, &error);
									if (TRUE != return_code)
									{
										g_string_printf(error_string, "ED66: Image data loading failed: '%s'", error->message);
										display_warning(error_string->str);
										g_string_free(error_string, TRUE);
									}
									return_code = gdk_pixbuf_loader_close(image_loader, &error);
									if (TRUE != return_code)
									{
										g_string_printf(error_string, "ED67: Image data loading failed: '%s'", error->message);
										display_warning(error_string->str);
										g_string_free(error_string, TRUE);
									}
									tmp_image_ob->image_data = gdk_pixbuf_loader_get_pixbuf(image_loader);
									if (NULL == tmp_image_ob->image_data)
									{
										display_warning("ED65: Error when loading image data");
									}
								}

								// Set the modified flag for this image to false
								tmp_image_ob->modified = FALSE;

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_START, tmp_layer->start_frame,
										TIMELINE_FINISH, tmp_layer->finish_frame,
										TIMELINE_X_OFF_START, tmp_image_ob->x_offset_start,
										TIMELINE_Y_OFF_START, tmp_image_ob->y_offset_start,
										TIMELINE_X_OFF_FINISH, tmp_image_ob->x_offset_finish,
										TIMELINE_Y_OFF_FINISH, tmp_image_ob->y_offset_finish,
										-1);

								// Add this (now completed) image layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
							}

							// Test if this layer is a highlight layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "highlight"))
							{
								// Construct a new highlight layer
								tmp_highlight_ob = g_new(layer_highlight, 1);
								tmp_layer = g_new(layer, 1);	
								tmp_layer->object_type = TYPE_HIGHLIGHT;
								tmp_layer->object_data = (GObject *) tmp_highlight_ob;
								tmp_layer->external_link = g_string_new(NULL);

								// Load the highlight layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										tmp_highlight_ob->x_offset_start = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_start")))
									{
										// Get the starting y offset
										tmp_highlight_ob->y_offset_start = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_finish")))
									{
										// Get the finishing x offset
										tmp_highlight_ob->x_offset_finish = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_finish")))
									{
										// Get the finishing y offset
										tmp_highlight_ob->y_offset_finish = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "width")))
									{
										// Get the width
										tmp_highlight_ob->width = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "height")))
									{
										// Get the height
										tmp_highlight_ob->height = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										tmp_layer->start_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										tmp_layer->finish_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										tmp_layer->name = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										tmp_layer->external_link = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									this_node = this_node->next;	
								}

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_START, tmp_layer->start_frame,
										TIMELINE_FINISH, tmp_layer->finish_frame,
										TIMELINE_X_OFF_START, tmp_highlight_ob->x_offset_start,
										TIMELINE_Y_OFF_START, tmp_highlight_ob->y_offset_start,
										TIMELINE_X_OFF_FINISH, tmp_highlight_ob->x_offset_finish,
										TIMELINE_Y_OFF_FINISH, tmp_highlight_ob->y_offset_finish,
										-1);

								// Add this (now completed) highlight layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
							}

							// Test if this layer is a mouse pointer layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "mouse"))
							{
								// Construct a new mouse pointer layer
								tmp_mouse_ob = g_new(layer_mouse, 1);
								tmp_layer = g_new(layer, 1);	
								tmp_layer->object_type = TYPE_MOUSE_CURSOR;
								tmp_layer->object_data = (GObject *) tmp_mouse_ob;
								tmp_layer->external_link = g_string_new(NULL);

								// Load the highlight layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										tmp_mouse_ob->x_offset_start = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_start")))
									{
										// Get the starting y offset
										tmp_mouse_ob->y_offset_start = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_finish")))
									{
										// Get the finishing x offset
										tmp_mouse_ob->x_offset_finish = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_finish")))
									{
										// Get the finishing y offset
										tmp_mouse_ob->y_offset_finish = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "width")))
									{
										// Get the width
										tmp_mouse_ob->width = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "height")))
									{
										// Get the height
										tmp_mouse_ob->height = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "click")))
									{
										// Get the mouse click type
										tmp_int = g_ascii_strncasecmp((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1), "none", 4);
										if (0 == tmp_int)
										{
											tmp_mouse_ob->click = MOUSE_NONE;
										} else
										{
											tmp_mouse_ob->click = MOUSE_LEFT_ONE;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										tmp_layer->start_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										tmp_layer->finish_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										tmp_layer->name = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										tmp_layer->external_link = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									this_node = this_node->next;	
								}

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_START, tmp_layer->start_frame,
										TIMELINE_FINISH, tmp_layer->finish_frame,
										TIMELINE_X_OFF_START, tmp_mouse_ob->x_offset_start,
										TIMELINE_Y_OFF_START, tmp_mouse_ob->y_offset_start,
										TIMELINE_X_OFF_FINISH, tmp_mouse_ob->x_offset_finish,
										TIMELINE_Y_OFF_FINISH, tmp_mouse_ob->y_offset_finish,
										-1);

								// Add this (now completed) mouse pointer layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
							}

							// Test if this layer is a text layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "text"))
							{
								// Construct a new text layer
								tmp_text_ob = g_new(layer_text, 1);
								tmp_layer = g_new(layer, 1);	
								tmp_layer->object_type = TYPE_TEXT;
								tmp_layer->object_data = (GObject *) tmp_text_ob;
								tmp_layer->external_link = g_string_new(NULL);

								// Load the text layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										tmp_text_ob->x_offset_start = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_start")))
									{
										// Get the starting y offset
										tmp_text_ob->y_offset_start = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_finish")))
									{
										// Get the finishing x offset
										tmp_text_ob->x_offset_finish = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_finish")))
									{
										// Get the finishing y offset
										tmp_text_ob->y_offset_finish = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "red")))
									{
										// Get the red value
										tmp_text_ob->text_color.red = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "green")))
									{
										// Get the green value
										tmp_text_ob->text_color.green = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "blue")))
									{
										// Get the blue value
										tmp_text_ob->text_color.blue = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "font_size")))
									{
										// Get the font size
										tmp_text_ob->font_size = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "text_value")))
									{
										// Get the text
										tmp_text_ob->text_buffer = gtk_text_buffer_new(NULL);
										gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), (const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1), -1);
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										tmp_layer->start_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										tmp_layer->finish_frame = atoi((const char *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										tmp_layer->name = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										tmp_layer->external_link = g_string_new((const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
									}
									this_node = this_node->next;
								}

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_START, tmp_layer->start_frame,
										TIMELINE_FINISH, tmp_layer->finish_frame,
										TIMELINE_X_OFF_START, tmp_text_ob->x_offset_start,
										TIMELINE_Y_OFF_START, tmp_text_ob->y_offset_start,
										TIMELINE_X_OFF_FINISH, tmp_text_ob->x_offset_finish,
										TIMELINE_Y_OFF_FINISH, tmp_text_ob->y_offset_finish,
										-1);

								// Add this (now completed) text layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
							}
						}
						layer_ptr = layer_ptr->next;
					}  // End of "We're in a layer" loop
				}
				this_layer = this_layer->next;
			}

			// Read the slide name from the save file
			tmp_slide->name = NULL;
			tmp_char = xmlGetProp(this_slide, (const xmlChar *) "name");
			if (NULL != tmp_char)
			{
				// A name for the slide is in the project file, so use that
				tmp_slide->name = g_string_new((const gchar *) tmp_char);
			}

			// Create the thumbnail for the slide
			tmp_glist = NULL;
			tmp_glist = g_list_append(tmp_glist, tmp_slide);
			tmp_pixbuf = compress_layers(tmp_glist, preview_width, 233);
			tmp_slide->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(GDK_PIXBUF(tmp_pixbuf)));

			// Create the event box for the slide
			tmp_slide->event_box = gtk_event_box_new();

			// Mark the tooltip as uncreated
			tmp_slide->tooltip = NULL;

			// Set the timeline widget for the slide to NULL, so we know to create it later on
			tmp_slide->timeline_widget = NULL;

			// Add the thumbnail to the event box
			gtk_container_add(GTK_CONTAINER(tmp_slide->event_box), GTK_WIDGET(tmp_slide->thumbnail));

			// Add a mouse click handler to the event box
			tmp_slide->click_handler = g_signal_connect(G_OBJECT(tmp_slide->event_box), "button_release_event", G_CALLBACK(film_strip_slide_clicked), tmp_slide);

			// To get here, we must have finished loading the present slide, so we add it to the working project
			slides = g_list_append(slides, tmp_slide);

		}  // End of "We're in a slide" loop

		this_slide = this_slide->next;
	}

	// We're finished with this XML document, so release its memory
	xmlFreeDoc(document);

	// Free the memory allocated in this function
	g_string_free(tmp_gstring2, TRUE);

	return TRUE;
}


void logger_simple(const gchar *str)
{
	// For now, do nothing with the logs
}


void logger_with_domain(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
	// For now, do nothing with the logs
}


// Function disable or enable a given menu
void menu_enable(const gchar *full_path, gboolean enable)
{
	// Local variables
	GtkWidget			*menu_item;

	menu_item = gtk_item_factory_get_item(GTK_ITEM_FACTORY(menu_bar), full_path);
	gtk_widget_set_sensitive(menu_item, enable);
}


// Function to convert a slide structure into a Flash output file
GByteArray *menu_export_flash_inner(GByteArray *swf_buffer)
//GByteArray *menu_export_flash_inner(GByteArray *swf_buffer, guint start_slide, guint finish_slide)
{
	// Local variables
	guint				char_counter;			// Counter of the number of characters in the animation
	guint				layer_counter;			// Holds the number of layers
	guint				max_frames = 0;			// The highest frame number in the slide
	guint				num_layers = 0;			// The number of layers in the slide
	guint				num_slides;				// The number of slides in the movie
	guint				slide_counter;			// Holds the number of slides
	swf_frame_element	*swf_timing_array = NULL;  // Used to coordinate the actions in each frame
	layer				*this_layer_data;		// Points to the data in the present layer
	slide				*this_slide_data;		// Points to the data in the present slide
	guint				total_frames;			// The total number of frames in the animation

	guint				element_counter;
	guint				element_max;

	gint				element_x_position_increment = 0;
	gint				element_x_position_start = 0;
	gint				element_y_position_increment = 0;
	gint				element_y_position_start = 0;

	guint				tmp_integer;			// Temporary integer


	// Initialise some things
	char_counter = 0;
	total_frames = 0;

	// For each slide, work out how many layers there are and how many frames the entire slide lasts for
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
//printf("Number of slides: %u\n", num_slides);
	for (slide_counter = 0; slide_counter <  num_slides; slide_counter++)
	{
		// Initialise things for this slide
		this_slide_data = g_list_nth_data(slides, slide_counter);
		max_frames = 0;

		// Work out how many layers there are in this slide
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		num_layers = g_list_length(this_slide_data->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			// Work out the maximum frame number for the slide
			this_layer_data = g_list_nth_data(this_slide_data->layers, layer_counter);
			if (this_layer_data->finish_frame > max_frames)
			{
				max_frames = this_layer_data->finish_frame;
			}
		}

		// * At this stage we should know both the maximum frame number and number of layers in the slide *
printf("Number of layers in slide %u is %u\n", slide_counter, num_layers);
printf("Maximum frame number in slide %u is %u\n", slide_counter, max_frames);

		// Add the frames for this slide to the total count of frames for the animation
		total_frames += max_frames;

		// Create an array that's layers x max number of frames
		swf_timing_array = g_new(swf_frame_element, num_layers * max_frames);

		// Point to the first layer again
		this_slide_data = g_list_nth_data(slides, slide_counter);

		// Process each layer in turn.  For every frame the layer is in, store in the array
		// whether the object in the layer is visible, it's position, transparency, and other details
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			this_layer_data = g_list_nth_data(this_slide_data->layers, layer_counter);

			// Mark that the element needs to be shown on this frame
			swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].add = TRUE;

			// Mark that the element needs to be removed on this frame
			swf_timing_array[(layer_counter * max_frames) + this_layer_data->finish_frame].remove = TRUE;

			// Calculate the starting positions, increments and other info for each frame of the layer
			switch (this_layer_data->object_type)
			{
				case TYPE_GDK_PIXBUF:
					// We're processing a image layer
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_type = TYPE_GDK_PIXBUF;
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_data = this_layer_data->object_data;
					element_x_position_start = ((layer_image *) this_layer_data->object_data)->x_offset_start;
					element_y_position_start = ((layer_image *) this_layer_data->object_data)->y_offset_start;
					element_x_position_increment = (((layer_image *) this_layer_data->object_data)->x_offset_finish - ((layer_image *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					element_y_position_increment = (((layer_image *) this_layer_data->object_data)->y_offset_finish - ((layer_image *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					break;

				case TYPE_HIGHLIGHT:
					// We're processing a highlight layer
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_type = TYPE_HIGHLIGHT;
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_data = this_layer_data->object_data;
					element_x_position_start = ((layer_highlight *) this_layer_data->object_data)->x_offset_start;
					element_y_position_start = ((layer_highlight *) this_layer_data->object_data)->y_offset_start;
					element_x_position_increment = (((layer_highlight *) this_layer_data->object_data)->x_offset_finish - ((layer_highlight *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					element_y_position_increment = (((layer_highlight *) this_layer_data->object_data)->y_offset_finish - ((layer_highlight *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					break;

				case TYPE_MOUSE_CURSOR:
					// We're processing a mouse layer
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_type = TYPE_MOUSE_CURSOR;
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_data = this_layer_data->object_data;
					element_x_position_start = ((layer_mouse *) this_layer_data->object_data)->x_offset_start;
					element_y_position_start = ((layer_mouse *) this_layer_data->object_data)->y_offset_start;
					element_x_position_increment = (((layer_mouse *) this_layer_data->object_data)->x_offset_finish - ((layer_mouse *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					element_y_position_increment = (((layer_mouse *) this_layer_data->object_data)->y_offset_finish - ((layer_mouse *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					break;

				case TYPE_TEXT:
					// We're processing a text layer
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_type = TYPE_TEXT;
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_data = this_layer_data->object_data;
					element_x_position_start = ((layer_text *) this_layer_data->object_data)->x_offset_start;
					element_y_position_start = ((layer_text *) this_layer_data->object_data)->y_offset_start;
					element_x_position_increment = (((layer_text *) this_layer_data->object_data)->x_offset_finish - ((layer_text *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					element_y_position_increment = (((layer_text *) this_layer_data->object_data)->y_offset_finish - ((layer_text *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					break;

				default:
					// Unknown type
					display_warning("ED83: Unknown layer type in swf output");
					break;
			}
			
			element_max = this_layer_data->finish_frame;
			for (element_counter = this_layer_data->start_frame; element_counter <= element_max; element_counter++)
			{
				// Mark that the element should be processed on this frame
				swf_timing_array[(layer_counter * max_frames) + element_counter].action_this = TRUE;

				// Give the object a unique character ID
				swf_timing_array[(layer_counter * max_frames) + element_counter].char_id = char_counter;

				// Store the x and y positions for each frame
				swf_timing_array[(layer_counter * max_frames) + element_counter].x_position = element_x_position_start + (element_counter * element_x_position_increment);
				swf_timing_array[(layer_counter * max_frames) + element_counter].y_position = element_y_position_start + (element_counter * element_y_position_increment);

				// Store the opacity setting for each frame
				// fixme2: Still need to calculate properly rather than hard code to 100% for the moment
				swf_timing_array[(layer_counter * max_frames) + element_counter].opacity = 65535;
			}

			// Increment the character counter
			char_counter++;
		}
	}
printf("The animation is %u frames long\n", total_frames);


	// * After all of the layers have been pre-processed, there remains an array *
	// * with the per frame info of what should be where in the output swf       *

	// Process the swf timing array, creating the swf
	for (tmp_integer = 0; tmp_integer < max_frames; tmp_integer++)  // This loops _frame_ number of times
	{
		for (element_counter = 0; element_counter < num_layers; element_counter++)  // This loops _num_layers_ of times
		{
			// For each frame, access all of the layers then move to the next frame
			if (TRUE == swf_timing_array[(tmp_integer * max_frames) + element_counter].action_this)
			{
				// * There is something to be done in this frame for this layer *

				if (TRUE == swf_timing_array[(tmp_integer * max_frames) + element_counter].add)
				{
					// We need to create the item in the swf dictionary, then add it to the swf display list
					switch (swf_timing_array[(tmp_integer * max_frames) + element_counter].layer_type)
					{
						case TYPE_GDK_PIXBUF:
							// We're processing a image layer

							// Create the swf with DefineShape3
							// fixme3: Needs to be written
							swf_buffer = g_byte_array_append(swf_buffer, (const guint8 *) "bits go here", 12);

// Header RECORDHEADER Tag type = 32
// ShapeId UI16 ID for this character
// ShapeBounds RECT Bounds of the shape
// Shapes SHAPEWITHSTYLE Shape information

							break;

						case TYPE_HIGHLIGHT:
							// We're processing a highlight layer

							// fixme3: Needs to be written

							break;

						case TYPE_MOUSE_CURSOR:
							// We're processing a mouse layer

							// fixme3: Needs to be written

							break;

						case TYPE_TEXT:
							// We're processing a text layer

							// fixme3: Needs to be written

							break;

						default:
							// Unknown type
							display_warning("ED84: Unknown layer type in swf output");
							break;
					}
				}
			}
		}		
	}

	// Process this array, first creating the dictionary of shapes


	return swf_buffer;
}


// Function to convert a slide structure into an output file
void menu_export_svg_animation_slide(gpointer element, gpointer user_data)
{
	// Local variables
	gchar				*base64_string;			// Pointer to an Base64 string
	gchar				*encoded_string;		// Pointer to an URI encoded Base64 string
	GError				*error = NULL;			// Pointer to error return structure
	GString				*file_name = NULL;		// Used to construct the file name of the output file
	guint				finish_frame;			// The finish frame in which the object appears
	gint				layer_counter;			// Simple counter
	layer				*layer_data;			// Points to the presently processing layer
	GList				*layer_pointer;			// Points to the layer structure in the slide
	gint				num_layers;				// Number of layers in the slide
	guint				object_type;			// The type of object in each layer
	gchar				*pixbuf_buffer;			// Gets given a pointer to a compressed jpeg image
	gsize				pixbuf_size;			// Gets given the size of a compressed jpeg image
	GIOStatus			return_value;			// Return value used in most GIOChannel functions
	slide				*slide_pointer;			// Points to the present slide
	guint				start_frame;			// The first frame in which the object appears
	GString				*string_to_write = NULL;// Holds SVG data to be written out
	GtkTextIter			text_end;				// The end position of the text buffer
	GtkTextIter			text_start;				// The start position of the text buffer
	gfloat				time_end;				// The second (or part of) in the animation, in which this slide disappears
	gfloat				time_start;				// The second (or part of) in the animation, in which this slide appears
	gfloat				x_scale;				// Width scale factor for the scene
	gfloat				y_scale;				// Height scale factor for the scene

	gboolean			tmp_bool;				// Temporary boolean value
	gfloat				tmp_gfloat;				// Temporary gfloat
	gsize				tmp_gsize;				// Temporary gsize
	GString				*tmp_gstring;			// Temporary GString
	gint				tmp_int;				// Temporary integer
	GdkPixbuf			*tmp_pixbuf;			// Temporary GDK pixbuf


	// Initialise some things
	slide_pointer = element;
	time_start = export_time_counter;
	time_end = 0;
	tmp_gstring = g_string_new(NULL);
	x_scale = (gfloat) output_width / project_width;
	y_scale = (gfloat) output_height / project_height;

	// Determine how many seconds this slide lasts for
	layer_pointer = g_list_first((GList *) slide_pointer->layers);
	num_layers = g_list_length(layer_pointer);
	for (layer_counter = num_layers - 1; layer_counter >= 0; layer_counter--)
	{
		layer_pointer = g_list_first((GList *) slide_pointer->layers);
		layer_data = g_list_nth_data(layer_pointer, layer_counter);

		// If the present layer lasts longer than the longest known thus far, adopt the present layers version
		if (layer_data->finish_frame > time_end)
			time_end = layer_data->finish_frame;
	}

	// * At this point, time_end should contain the highest "finish frame" that was in any of the layers *
	
	// Convert time_end into seconds
	time_end = (time_end / frames_per_second) + time_start;

	// * For each layer, write it to the output file *
	layer_pointer = g_list_first((GList *) slide_pointer->layers);
	num_layers = g_list_length(layer_pointer);
	for (layer_counter = num_layers - 1; layer_counter >= 0; layer_counter--)
	{
		layer_pointer = g_list_first((GList *) slide_pointer->layers);
		layer_data = g_list_nth_data(layer_pointer, layer_counter);

		// Create some useful pointers
		start_frame = layer_data->start_frame;
		finish_frame = layer_data->finish_frame;
		object_type = layer_data->object_type;

		// Determine the type of object present in the layer
		switch (object_type)
		{
			case TYPE_EMPTY:
				// fixme3: Needs writing
				break;

			case TYPE_GDK_PIXBUF:
				// * We're processing a GDK pixbuf *

				// Resize the pixbuf to the output resolution
				tmp_pixbuf = gdk_pixbuf_scale_simple(GDK_PIXBUF(((layer_image *) layer_data->object_data)->image_data), output_width, output_height, GDK_INTERP_BILINEAR);
				if (NULL == tmp_pixbuf)
				{
					// * Something went wrong when scaling down the pixbuf *

					// Display the warning message using our function
					if (NULL == slide_pointer->name)
					{
						g_string_printf(tmp_gstring, "Error ED13: An error '%s' occured when scaling a slide", error->message);
					} else
					{
						g_string_printf(tmp_gstring, "Error ED13: An error '%s' occured when scaling slide '%s'", error->message, slide_pointer->name->str);
					}
					display_warning(tmp_gstring->str);

					// Free the memory allocated in this function
					g_object_unref(tmp_pixbuf);
					g_string_free(tmp_gstring, TRUE);
					g_error_free(error);
					return;
				}

				// Convert the compressed image into jpeg data
				tmp_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(tmp_pixbuf),
								&pixbuf_buffer,  // Will come back filled out with location of jpeg data
								&pixbuf_size,  // Will come back filled out with size of jpeg data
								"jpeg",
								&error,
								"quality",
								"100",
								NULL);
				if (FALSE == tmp_bool)
				{
					// Something went wrong when encoding the image to jpeg format
					display_warning("ED51: Something went wrong when encoding a slide to jpeg format");

					// Free the memory allocated in this function
					g_object_unref(tmp_pixbuf);
					g_string_free(tmp_gstring, TRUE);
					g_error_free(error);
					return;
				}

				// Base64 encode the image data
				base64_encode(pixbuf_buffer, pixbuf_size, &base64_string);

				// URI encode the Base64 data
				uri_encode_base64(base64_string, strlen(base64_string), &encoded_string);

				// Create the temporary string to put everything into
				string_to_write = g_string_new(NULL);

				// If the layer has an external link, then create <a> tag around it
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "<a xlink:href=\"%s\" xlink:show=\"new\">\n", layer_data->external_link->str);
				}

				// We handle background images differently to other image types
				tmp_int = g_ascii_strncasecmp(layer_data->name->str, "Background", 10);
				if (0 == tmp_int)
				{
					// * We're processing a background layer *

					// Create a string to write to the output svg file
					g_string_append_printf(string_to_write, "<image id=\"%s-background\" width=\"%upx\" height=\"%upx\" opacity=\"0.0\" xlink:href=\"data:image/jpeg;base64,%s\">\n",
						layer_data->name->str,
						output_width,
						output_height,
						encoded_string);

					// Animate the image SVG properties so the background is only visible for as long as the slide
					g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n</image>\n",
						1 + time_start,
						time_end - time_start);
				} else
				{
					// * We're not processing a background layer *

					// Create a string to write to the output svg file
					g_string_append_printf(string_to_write, "<image id=\"%s-standard_image\" width=\"%.4fpx\" height=\"%.4fpx\" x=\"%.4fpx\" y=\"%.4fpx\" opacity=\"0.0\" xlink:href=\"data:image/jpeg;base64,%s\">\n",
						layer_data->name->str,
						x_scale * ((layer_image *) layer_data->object_data)->width,
						y_scale * ((layer_image *) layer_data->object_data)->height,
						x_scale * ((layer_image *) layer_data->object_data)->x_offset_start,
						y_scale * ((layer_image *) layer_data->object_data)->y_offset_start,
						encoded_string);

					// Animate the image SVG properties to fade it in over 1 second
					g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />\n",
						1 + time_start + (layer_data->start_frame / frames_per_second));

					// Animate the image SVG properties so it keeps visible after faded in
					g_string_append_printf(string_to_write,
						"\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n",
						1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
						(((gfloat) layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2);

					// Animate the image SVG properties so they fade out over 1 second
					g_string_append_printf(string_to_write,
						"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />\n",
						1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

					// Animate the SVG properties to move it to it's destination location
					g_string_append_printf(string_to_write,
						"\t<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
						1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
						time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
						x_scale * ((layer_image *) layer_data->object_data)->x_offset_start,
						x_scale * ((layer_image *) layer_data->object_data)->x_offset_finish);
					g_string_append_printf(string_to_write,
						"\t<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
						1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
						time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
						y_scale * ((layer_image *) layer_data->object_data)->y_offset_start,
						y_scale * ((layer_image *) layer_data->object_data)->y_offset_finish);

					// Remove the image from display after it's no longer needed
					g_string_append_printf(string_to_write,
						"\t<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n</image>\n",
						1 + time_start + (layer_data->finish_frame / frames_per_second));
				}

				// If the layer has an external link, then close the <a> tag
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "</a>\n");
				}

				// Free the allocated memory
				g_object_unref(tmp_pixbuf);
				g_free(encoded_string);
				g_free(base64_string);
				break;

			case TYPE_HIGHLIGHT:
				// We're processing a highlight layer
				string_to_write = g_string_new(NULL);

				// If the layer has an external link, then create <a> tag around it
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "<a xlink:href=\"%s\" xlink:show=\"new\">\n", layer_data->external_link->str);
				}

				// Add the SVG tag, but ensure the highlight box starts out invisible
				g_string_append_printf(string_to_write,
					"<rect id=\"%s-highlight\" class=\"highlight\" width=\"%.4fpx\" height=\"%.4fpx\" opacity=\"0.0\" x=\"%.4fpx\" y=\"%.4fpx\" stroke-width=\"%.4fpx\">\n",
					layer_data->name->str,
					x_scale * ((layer_highlight *) layer_data->object_data)->width,
					y_scale * ((layer_highlight *) layer_data->object_data)->height,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_start,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_start,
					y_scale * 2);

				// Animate the highlight box SVG properties to fade it in over 1 second
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />\n",
					1 + time_start + (layer_data->start_frame / frames_per_second));

				// Animate the highlight box SVG properties so it keeps visible after faded in
				tmp_gfloat = (((gfloat) layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2;
				if (0 > tmp_gfloat)
					tmp_gfloat = 0;
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					tmp_gfloat);

				// Animate the highlight box SVG properties so they fade out over 1 second
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

				// Animate the SVG properties to move it to it's destination location
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_start,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_finish);
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_start,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_finish);

				// Remove the rectangle from display after it's no longer needed
				g_string_append_printf(string_to_write,
					"\t<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n</rect>\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second));

				// If the layer has an external link, then close the <a> tag
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "</a>\n");
				}

				break;

			case TYPE_MOUSE_CURSOR:
				// We're processing a mouse pointer layer
				string_to_write = g_string_new(NULL);

				// Add the SVG tag, but ensure the mouse pointer starts out invisible
				g_string_append_printf(string_to_write,
					"<path d=\"M %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f z\""
					" fill=\"#ffffff\" opacity=\"0.0\" fill-rule=\"evenodd\" stroke=\"#000000\""
					" stroke-width=\"%.4f\" stroke-linecap=\"square\" stroke-miterlimit=\"4\""
					" stroke-dasharray=\"none\" stroke-dashoffset=\"0\">\n",
					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 20.875),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 19.1705),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 0.25),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 0.5623),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 0.25),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 26.2681),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 6.2604),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 20.2253),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 12.7899),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 32.8861),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 18.3869),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 32.8861),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 11.2354),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 19.1705),

					y_scale * 1.1776);  // Stroke width

				// Animate the mouse pointer SVG properties to fade it in over 1 second
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />\n",
					1 + time_start + (layer_data->start_frame / frames_per_second));

				// Animate the mouse pointer SVG properties so it keeps visible after faded in
				tmp_gfloat = (((gfloat) layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2;
				if (0 > tmp_gfloat)
					tmp_gfloat = 0;
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					tmp_gfloat);

				// Animate the mouse pointer SVG properties so they fade out over 1 second
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

				// Animate the SVG properties to move it to it's destination location
				g_string_append_printf(string_to_write,
					"\t<animateMotion attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"0px,0px\" to=\"%.4fpx,%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_finish - ((layer_highlight *) layer_data->object_data)->x_offset_start),  // X distance to travel
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_finish - ((layer_highlight *) layer_data->object_data)->y_offset_start));  // Y distance to travel

				// Remove the mouse pointer from display after it's no longer needed
				g_string_append_printf(string_to_write,
					"\t<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n</path>\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second));

				break;

			case TYPE_TEXT:
				// We're processing a text layer
				string_to_write = g_string_new(NULL);

				// Start the group enclosing the text layer elements
				g_string_append_printf(string_to_write, "<g id=\"%s\">\n", layer_data->name->str);

				// If the layer has an external link, then create <a> tag around it
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "<a xlink:href=\"%s\" xlink:show=\"new\">\n", layer_data->external_link->str);
				}

				// Create the SVG tag for the background box the text goes onto
				g_string_append_printf(string_to_write,
					"\t<rect id=\"%s-bg\" class=\"text\" width=\"%.4fpx\" height=\"%.4fpx\" opacity=\"0.0\" x=\"%.4fpx\" y=\"%.4fpx\" rx=\"%.4fpx\" ry=\"%.4fpx\" stroke-width=\"%.4fpx\">\n",
					layer_data->name->str,
					(x_scale * (((layer_text *) layer_data->object_data)->rendered_width + 10)) * 1.02,  // fixme5: Seems to need to be scaled up by 1.02 to look any good
					y_scale * (((layer_text *) layer_data->object_data)->rendered_height + 10),
					x_scale * ((layer_text *) layer_data->object_data)->x_offset_start,
					y_scale * ((layer_text *) layer_data->object_data)->y_offset_start,
					y_scale * 10,  // Relative X
					y_scale * 10,  // Relative Y
					y_scale * 2);  // Stroke width

				// Animate the background box SVG properties to fade it in over 1 second
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />\n",
					1 + time_start + (layer_data->start_frame / frames_per_second));

				// Animate the background box SVG properties so it keeps visible after faded in
				tmp_gfloat = (((gfloat) layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2;
				if (0 > tmp_gfloat)
					tmp_gfloat = 0;
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					tmp_gfloat);

				// Animate the background box SVG properties so they fade out over 1 second
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

				// Animate the SVG properties to move it to it's destination location
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_start,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_finish);
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_start,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_finish);

				// Remove the text background from display after it's no longer needed
				g_string_append_printf(string_to_write,
					"\t<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n\t</rect>\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second));

				// Create the text tag
				g_string_append_printf(string_to_write, "\t<text id=\"%s-text\" font-family=\"Bitstream Vera Sans svg\" class=\"text\" x=\"%.4fpx\" y=\"%.4fpx\" opacity=\"0.0\" font-size=\"%.4fpx\" textLength=\"%.4fpx\" lengthAdjust=\"spacingAndGlyphs\" dx=\"%.4fpx\" dy=\"%.4fpx\">",
					layer_data->name->str,
					x_scale * ((layer_text *) layer_data->object_data)->x_offset_start,  // X offset
					y_scale * ((layer_text *) layer_data->object_data)->y_offset_start,  // Y offset
					(y_scale * ((layer_text *) layer_data->object_data)->rendered_height - 2),  // Font size
					x_scale * (((layer_text *) layer_data->object_data)->rendered_width - 20),  // How wide to make the entire string
					x_scale * 10,  // Horizontal space between text background border and text start
					y_scale * ((((layer_text *) layer_data->object_data)->rendered_height + 54) / 2));  // Vertical space between text background border and text start

				// Add the text to the text layer
				gtk_text_buffer_get_bounds(((layer_text *) layer_data->object_data)->text_buffer, &text_start, &text_end);
				g_string_append_printf(string_to_write,
					"%s",
					gtk_text_buffer_get_text(((layer_text *) layer_data->object_data)->text_buffer, &text_start, &text_end, FALSE));  // Text to be rendered

				// Animate the text SVG properties to fade it in over 1 second
				g_string_append_printf(string_to_write,
					"<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />",
					1 + time_start + (layer_data->start_frame / frames_per_second));

				// Animate the text SVG properties so it keeps visible after faded in
				g_string_append_printf(string_to_write,
					"<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					tmp_gfloat);

				// Animate the text SVG properties so they fade out over 1 second
				g_string_append_printf(string_to_write,
					"<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />",
					1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

				// Animate the SVG properties to move it to it's destination location
				g_string_append_printf(string_to_write,
					"<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					x_scale * ((layer_text *) layer_data->object_data)->x_offset_start,
					x_scale * ((layer_text *) layer_data->object_data)->x_offset_finish);
				g_string_append_printf(string_to_write,
					"<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					(y_scale * (((layer_text *) layer_data->object_data)->y_offset_start)),  // Y start
					(y_scale * ((layer_text *) layer_data->object_data)->y_offset_finish));  // Y finish

				// Remove the text itself from display after it's no longer needed
				g_string_append_printf(string_to_write,
					"<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n</text>\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second));

				// If the layer has an external link, then close the <a> tag
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "</a>\n");
				}

				// Close the group
				g_string_append_printf(string_to_write, "</g>\n");

				break;

			default:
				g_string_printf(tmp_gstring, "ED29: Unknown object type found in layer '%s'\n", layer_data->name->str);
				display_warning(tmp_gstring->str);
		}

		// If there is a string to write to the output file, do so
		if (NULL != string_to_write)
		{
			return_value = g_io_channel_write_chars(output_file, string_to_write->str, string_to_write->len, &tmp_gsize, &error);
			if (G_IO_STATUS_ERROR == return_value)
			{
				// * An error occured when writing the SVG closing tags to the output file, so alert the user, and return to the calling routine indicating failure *

				// Display the warning message using our function
				g_string_printf(tmp_gstring, "Error ED12: An error '%s' occured when writing SVG data to the output file '%s'", error->message, file_name->str);
				display_warning(tmp_gstring->str);

				// Free the memory allocated in this function
				g_string_free(file_name, TRUE);
				g_string_free(tmp_gstring, TRUE);
				g_error_free(error);
				return;
			}

			// Reset the string to be written to the output file
			g_string_free(string_to_write, TRUE);
			string_to_write = NULL;
		}
	}

	// Update the time counter, so the next frame starts after the end of this one
	export_time_counter = time_end;
	return;
}


// Function that takes a layer structure pointer, and adds it's data to an in-memory XML document
void menu_file_save_layer(gpointer element, gpointer user_data)
{
	// Local variables
	gchar				*base64_string;			// Pointer to an Base64 string
	GError				*error = NULL;			// Pointer to error return structure
	guint				finish_frame;			// The finish frame in which the object appears
	GString				*layer_name;			// Name of the layer
	xmlNodePtr			layer_node;				// Pointer to the new layer node
	layer				*layer_pointer;			// Points to the presently processing layer
	guint				layer_type;				// The type of layer
	gchar				*pixbuf_buffer;			// Gets given a pointer to a compressed jpeg image
	gsize				pixbuf_size;			// Gets given the size of a compressed jpeg image
	xmlNodePtr			slide_node;				// Pointer to the slide node
	guint				start_frame;			// The first frame in which the object appears
	GtkTextIter			text_end;				// The end position of the text buffer
	GtkTextIter			text_start;				// The start position of the text buffer

	gboolean			tmp_bool;				// Temporary boolean value
	GString				*tmp_gstring;			// Temporary GString
	GString				*tmp_gstring2;			// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	tmp_gstring2 = g_string_new(NULL);
	layer_pointer = element;
	slide_node = user_data;

	// Create some useful pointers
	start_frame = layer_pointer->start_frame;
	finish_frame = layer_pointer->finish_frame;
	layer_name	= layer_pointer->name;
	layer_type = layer_pointer->object_type;

	// Create the layer container
	layer_node = xmlNewChild(slide_node, NULL, (const xmlChar *) "layer", NULL);
	if (NULL == layer_node)
	{
		display_warning("ED24: Error creating the layer elements\n");
		return;
	}

	// Add the layer data to the layer container
	xmlNewChild(layer_node, NULL, (const xmlChar *) "name", (const xmlChar *) layer_name->str);
	g_string_printf(tmp_gstring, "%u", start_frame);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "start_frame", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", finish_frame);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "finish_frame", (const xmlChar *) tmp_gstring->str);
	if (0 != layer_pointer->external_link->len)
	{
		xmlNewChild(layer_node, NULL, (const xmlChar *) "external_link", (const xmlChar *) layer_pointer->external_link->str);
	}
	switch (layer_type)
	{
		case TYPE_GDK_PIXBUF:

			// * We save the image data in the project file, in the same way we save image     *
			// * data in exported svg.  This way a project file is self-contained and portable *

			// Convert the compressed image into png data
			tmp_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(((layer_image *) layer_pointer->object_data)->image_data),
						&pixbuf_buffer,  // Will come back filled out with location of png data
						&pixbuf_size,  // Will come back filled out with size of png data
						"png",
						&error,
						NULL);
			if (FALSE == tmp_bool)
			{
				// Something went wrong when encoding the image to jpeg format
				display_warning("ED62: Something went wrong when encoding a slide to png format");

				// Free the memory allocated in this function
				g_string_free(tmp_gstring, TRUE);
				g_error_free(error);
				return;
			}

			// Store the count of image data bytes in the file for read back verification
			g_string_printf(tmp_gstring2, "%d", pixbuf_size);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "data_length", (const xmlChar *) tmp_gstring2->str);

			// Base64 encode the image data
			base64_encode(pixbuf_buffer, pixbuf_size, &base64_string);

			// Create a string to write to the output file
			g_string_printf(tmp_gstring, "%s", base64_string);

			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "image");
			xmlNewChild(layer_node, NULL, (const xmlChar *) "data", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			break;

		case TYPE_EMPTY:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "empty");
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "blue", (const xmlChar *) tmp_gstring->str);
			break;

		case TYPE_MOUSE_CURSOR:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "mouse");
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			if (MOUSE_NONE == ((layer_mouse *) layer_pointer->object_data)->click)
			{
				xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "none");
			} else
			{
				xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "left_one");
			}
			break;

		case TYPE_TEXT:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "text");
			gtk_text_buffer_get_bounds(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "text_value", (const xmlChar *) gtk_text_buffer_get_text(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end, FALSE));
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "blue", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%f", ((layer_text *) layer_pointer->object_data)->font_size);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "font_size", (const xmlChar *) tmp_gstring->str);
			break;

		case TYPE_HIGHLIGHT:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "highlight");
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			break;
	}

	// Free the memory used in this function
	g_string_free(tmp_gstring, TRUE);
	g_string_free(tmp_gstring2, TRUE);

	return;
}


// Function that takes a slide structure pointer, and adds it's data to an in-memory XML document
void menu_file_save_slide(gpointer element, gpointer user_data)
{
	// Local variables
	slide			*slide_pointer;				// Points to the present slide
	GList			*layer_pointer;				// Points to the presently processing layer

	xmlNodePtr		slide_root;					// Points to the root of the slide data
	xmlNodePtr		slide_node;					// Pointer to the new slide node


	// Initialise various things
	slide_pointer = element;
	slide_root = user_data;
	layer_pointer = slide_pointer->layers;

    // Create the slide container
	slide_node = xmlNewChild(slide_root, NULL, (const xmlChar *) "slide", NULL);
	if (NULL == slide_node)
	{
		display_warning("ED23: Error creating the slide node\n");
		return;
	}

	// Add the slide name to the slide container attributes
	if (NULL != slide_pointer->name)
	{
		xmlNewProp(slide_node, (const xmlChar *) "name", (const xmlChar *) slide_pointer->name->str);
	}

	// Add the layer data to the slide container
	layer_pointer = g_list_first(layer_pointer);
	g_list_foreach(layer_pointer, menu_file_save_layer, slide_node);

	return;
}


// Function to save the application preferences prior to exiting
void save_preferences_and_exit(void)
{
#ifndef _WIN32
	// * Non-windows code *

	// Local variables
	GConfEngine			*gconf_engine;			// GConf engine

	guint				tmp_int;					// Temporary integer
	GtkWindow			*tmp_gtk_window;			// Temporary GtkWindow
	GdkWindow			*tmp_gdk_window;			// Temporary GdkWindow


	// Save the application preferences
	gconf_engine = gconf_engine_get_default();
	gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/project_folder", default_project_folder->str, NULL);
	gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/screenshots_folder", screenshots_folder->str, NULL);
	gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/output_folder", default_output_folder->str, NULL);
	gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/project_name", project_name->str, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/project_width", project_width, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/project_height", project_height, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/output_width", default_output_width, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/output_height", default_output_height, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/slide_length", default_slide_length, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/output_quality", default_output_quality, NULL);
	gconf_engine_set_int(gconf_engine, "/apps/flame/defaults/frames_per_second", frames_per_second, NULL);
	switch (scaling_quality)
	{
		case GDK_INTERP_NEAREST:
			gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/scaling_quality", "Nearest", NULL);
			break;
			
		case GDK_INTERP_TILES:
			gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/scaling_quality", "Tiles", NULL);
			break;
		
		case GDK_INTERP_BILINEAR:
			gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/scaling_quality", "Bilinear", NULL);
			break;
		
		case GDK_INTERP_HYPER:
			gconf_engine_set_string(gconf_engine, "/apps/flame/defaults/scaling_quality", "Hyper", NULL);
	}

	// * Save the present window maximised state (i.e. if we're maximised or not) *

	// Get a pointer to the underlying GDK window
	tmp_gtk_window = GTK_WINDOW(main_window);
	if (NULL != tmp_gtk_window->frame)
	{
		tmp_gdk_window = tmp_gtk_window->frame;
	}
	else
	{
		tmp_gdk_window = main_window->window;
	}

	// If the main application window still exists, save the window maximisation state, else don't bother
	// (it doesn't exist if the application exits due to Alt-F4 or pressing the title bar close button)
	if (NULL != tmp_gdk_window)
	{
		// Find out if the window is presently maximised or not
		tmp_int = gdk_window_get_state(tmp_gdk_window);
		if (GDK_WINDOW_STATE_MAXIMIZED == tmp_int)
		{
			// The window is maximised, so save that info
			gconf_engine_set_bool(gconf_engine, "/apps/flame/defaults/window_maximised", TRUE, NULL);
		} else
		{
			// The window is not maximised, so save that info
			gconf_engine_set_bool(gconf_engine, "/apps/flame/defaults/window_maximised", FALSE, NULL);
		}
	}

	// Save a configuration structure version number
	gconf_engine_set_float(gconf_engine, "/apps/flame/defaults/config_version", 1.0, NULL);

	// fixme4: Should we save a list of recent projects worked on?

	// Notify the GConf engine that now is probably a good time to sync
	gconf_engine_suggest_sync(gconf_engine, NULL);

	// Free our GConf engine
	gconf_engine_unref(gconf_engine);

	// Shut down sound
	gnome_sound_shutdown();

#else

	// * Windows-only code (registry for now) *

	// Check if we have a saved configuration in the windows registry
	HKEY		hkey;
	glong		return_code;
	glong		string_size;

	GString		*tmp_gstring;


	// Initialise some things
	tmp_gstring = g_string_new(NULL);


	// Check if the base Flame Project registry key exists
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject", 0, KEY_QUERY_VALUE, &hkey))
	{
		// It doesn't, so create it
		return_code = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\FlameProject", 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL);
		if (ERROR_SUCCESS != return_code)
		{
			// Creating the base registry key failed, so display a warning then exit
			display_warning("ED54: Saving preferences in the registry failed\n");
			gtk_main_quit();
		}
	} else
	{
		// The base Flame Project registry key exists, so we don't need to create it
		RegCloseKey(hkey);
	}

	// Check if the defaults registry key exists
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey))
	{
		// It doesn't, so create it
		return_code = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL);
		if (ERROR_SUCCESS != return_code)
		{
			// Creating the defaults registry key failed, so display a warning then exit
			display_warning("ED55: Saving preferences in the registry failed\n");
			gtk_main_quit();
		}
	}

	// * At this point, the variable hkey should point to the opened defaults registry key *

	// Set the value for the project folder
	string_size = (default_project_folder->len) + 1;
	return_code = RegSetValueEx(hkey, "project_folder", 0, REG_SZ, default_project_folder->str, string_size);

	// Set the value for the screenshots folder
	string_size = (screenshots_folder->len) + 1;
	return_code = RegSetValueEx(hkey, "screenshots_folder", 0, REG_SZ, screenshots_folder->str, string_size);

	// Set the value for the output folder
	string_size = (default_output_folder->len) + 1;
	return_code = RegSetValueEx(hkey, "output_folder", 0, REG_SZ, default_output_folder->str, string_size);

	// Set the value for the project name
	string_size = (project_name->len) + 1;
	return_code = RegSetValueEx(hkey, "project_name", 0, REG_SZ, project_name->str, string_size);

	// Set the value for the project width
	g_string_printf(tmp_gstring, "%d", project_width);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "project_width", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the project height
	g_string_printf(tmp_gstring, "%d", project_height);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "project_height", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the default output width
	g_string_printf(tmp_gstring, "%d", default_output_width);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "output_width", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the default output height
	g_string_printf(tmp_gstring, "%d", default_output_height);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "output_height", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the default slide length
	g_string_printf(tmp_gstring, "%d", slide_length);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "slide_length", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the default output quality
	g_string_printf(tmp_gstring, "%d", output_quality);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "output_quality", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the frames per second
	g_string_printf(tmp_gstring, "%d", frames_per_second);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "frames_per_second", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the window maximisation to TRUE (hard coded for now)
	// fixme4: We should make this dynamic rather than hard coded on
	g_string_printf(tmp_gstring, "%d", TRUE);
	string_size = (tmp_gstring->len) + 1;
	return_code = RegSetValueEx(hkey, "window_maximised", 0, REG_SZ, tmp_gstring->str, string_size);

	// Set the value for the scaling quality
	switch (scaling_quality)
	{
		case GDK_INTERP_NEAREST:
			g_string_printf(tmp_gstring, "%s", "Nearest");
			string_size = (tmp_gstring->len) + 1;
			return_code = RegSetValueEx(hkey, "scaling_quality", 0, REG_SZ, tmp_gstring->str, string_size);
			break;

		case GDK_INTERP_TILES:
			g_string_printf(tmp_gstring, "%s", "Tiles");
			string_size = (tmp_gstring->len) + 1;
			return_code = RegSetValueEx(hkey, "scaling_quality", 0, REG_SZ, tmp_gstring->str, string_size);
			break;

		case GDK_INTERP_BILINEAR:
			g_string_printf(tmp_gstring, "%s", "Bilinear");
			string_size = (tmp_gstring->len) + 1;
			return_code = RegSetValueEx(hkey, "scaling_quality", 0, REG_SZ, tmp_gstring->str, string_size);
			break;

		case GDK_INTERP_HYPER:
			g_string_printf(tmp_gstring, "%s", "Hyper");
			string_size = (tmp_gstring->len) + 1;
			return_code = RegSetValueEx(hkey, "scaling_quality", 0, REG_SZ, tmp_gstring->str, string_size);
	}

	// All values saved in the windows registry
	RegCloseKey(hkey);

#endif

	// Exit the application
	gtk_main_quit();
}


// Function to sound a beep (on user error, etc)
void sound_beep(void)
{
#ifndef _WIN32
	// Local variables
	GString				*tmp_gstring;			// Temporary GString
	
	// fixme4: Need a non-gnome way of playing sound too, for windows
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "%s%s", "/usr/share/", "flame/sounds/generic.wav");
	gnome_sound_play(tmp_gstring->str);

	// Free local variables
	g_string_free(tmp_gstring, TRUE);
#endif
}


// Function to decode a URI encoded string
GString *uri_decode(GString *input_string, GString *output_string)
{
	// Local variables
	guint				input_counter;			// Counter for how many bytes have been decoded


	// Walk the input string looking for specific character strings, replacing them as needed
	for (input_counter = 0; input_counter <= input_string->len; input_counter++)
	{
		switch (input_string->str[input_counter])
		{
			case '%':
				// The input character is potentially the first of the "%2B" or "%2F" series
				// that we're watching for
				if ('2' == input_string->str[input_counter + 1])
				{
					switch (input_string->str[input_counter + 2])
					{
						case 'B':
							output_string = g_string_append_c(output_string, '+');
							input_counter = input_counter + 2;
							break;

						case 'F':
							output_string = g_string_append_c(output_string, '/');
							input_counter = input_counter + 2;
							break;

						default:
							// Turns out it's not one of the one's we're watching for,
							// so just copy it to the output string
							output_string = g_string_append_c(output_string, input_string->str[input_counter]);
							break;
					}					
				} else
				{
					// Turns out it's not one of the one's we're watching for,
					// so just copy it to the output string
					output_string = g_string_append_c(output_string, input_string->str[input_counter]);
				}
				break;
				
			default:
				// The input character isn't one of the one's we're watching for,
				// so just copy it to the output string
				output_string = g_string_append_c(output_string, input_string->str[input_counter]);
		}
	}

	return output_string;
}


// Function to create a URI encoded string from a Base64 encoded string
gboolean uri_encode_base64(gpointer data, guint length, gchar **output_string)
{
	// Local variables
	guint				buffer_length;
	gchar				*input_buffer;
	gint				input_counter;			// Counter used for positioning inside the input buffer
	gchar				*output_buffer;
	gint				output_counter;			// Counter used for positioning inside the output buffer


	// Initialise some things
	input_buffer = data;
	output_counter = 0;

	// Calculate the length of the URI encoded buffer
	buffer_length = (guint) ((float) length * 3);  // Overestimate, to be on the safe side

	// Create the URI encoded buffer
	output_buffer = g_new(gchar, buffer_length);

	// Do the URI encoding into the new buffer
	for (input_counter = 0; input_counter < length; input_counter++)
	{
		switch (input_buffer[input_counter])
		{
			case '+':
				output_buffer[output_counter] = '%';
				output_buffer[output_counter + 1] = '2';
				output_buffer[output_counter + 2] = 'B';
				output_counter = output_counter + 3;
				break;

			case '/':
				output_buffer[output_counter] = '%';
				output_buffer[output_counter + 1] = '2';
				output_buffer[output_counter + 2] = 'F';
				output_counter = output_counter + 3;
				break;

			case '\n':
				break;

			default:
				output_buffer[output_counter] = input_buffer[input_counter];
				output_counter++;
				break;
		}
	}

	// Put a NULL at the end of the URI encoded string
	output_buffer[output_counter] = '\0';
	output_string[0] = output_buffer;
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.77  2007/02/25 11:24:58  vapour
 * Updated the SVG output, so elements are removed by setting their display attribute rather than moving them to a large y offset.
 *
 * Revision 1.76  2007/01/05 06:46:47  vapour
 * Hard coded to expect icons and similar in /usr/share/.
 *
 * Revision 1.75  2006/12/30 12:59:58  vapour
 * Updated with new path info for the shared files, which should make packaging easier.
 *
 * Revision 1.74  2006/12/26 03:57:09  vapour
 * Added initial fixes for the gcc 4 warnings about pointer targets differing in signedness.  Main application still untested, needs to be verified as still fully functional.
 *
 * Revision 1.73  2006/09/21 13:23:23  vapour
 * Started working on the code to generate actual swf bytecode.  I need to better understand the bit coding of record headers before much will happen though.
 *
 * Revision 1.72  2006/09/20 12:36:36  vapour
 * Adding code to the swf output function, getting it ready to create elements in the swf dictionary.
 *
 * Revision 1.71  2006/09/19 13:25:12  vapour
 * Still adding initial code that works out what to display for each frame of the swf export.
 *
 * Revision 1.70  2006/09/13 11:46:48  vapour
 * Adding initial code to work out what needs to be displayed in swf for each slide.
 *
 * Revision 1.69  2006/09/12 12:13:28  vapour
 * Replaced old ming code with initial thoughts, in comments, for how to process slides.  Started adding stub GByteArray code too.
 *
 * Revision 1.68  2006/09/11 13:57:22  vapour
 * Small bug fixes to the Base64 encoding and decoding functions, rounding out the support for edge cases not initially tested.
 *
 * Revision 1.67  2006/09/03 10:19:42  vapour
 * Mostly re-wrote the Base64 encoding and decoding functions, and some of the flame_read function too.  Base64 decoding now works properly, and image data is stored in project files in png format rather than jpeg).
 *
 * Revision 1.66  2006/08/22 14:33:00  vapour
 * Adding initial code to verify image data being loaded is ok.  Needs more work.
 *
 * Revision 1.65  2006/08/21 12:51:03  vapour
 * Writing further code to round out the base64 decode routine.  Still needs more work, as multiple saves then loads ends up with invalid image data. Doh!
 *
 * Revision 1.64  2006/08/09 06:48:27  vapour
 * Added working code to save and read the external URLs associated with layers, for project files.
 *
 * Revision 1.63  2006/08/09 06:10:46  vapour
 * Added initial working code to make exported svg elements clickable if they have an associated URL.  Even works around bug #224129 in Opera 9.
 *
 * Revision 1.62  2006/08/07 16:40:38  vapour
 * Added initial working functions to load embedded image data from project files.
 *
 * Revision 1.61  2006/08/03 13:45:00  vapour
 * Started adding code to the project loading function so it loads embedded image data.
 *
 * Revision 1.60  2006/08/01 15:12:20  vapour
 * Fixed a small bug, where the x_offset_start for an image wasn't being saved into the project file correctly.
 *
 * Revision 1.59  2006/08/01 14:58:29  vapour
 * Added code to save image data in the project file, making it self contained and easier to work with.
 *
 * Revision 1.58  2006/07/30 04:58:50  vapour
 * Removed the ming includes, commented out the code needing ming, and added a notice to the user if they try and export flash that it's not  going to work as the code is being rewritten.
 *
 * Revision 1.57  2006/07/13 13:48:39  vapour
 * + Updated the individual svg elements to not wait for the .click event from the play button.
 * + Updated the mouse pointers to use the relative distance moved.
 *
 * Revision 1.56  2006/07/09 11:16:37  vapour
 * Export to SVG now includes mouse pointers.
 *
 * Revision 1.55  2006/07/09 08:45:52  vapour
 * Added code to save mouse pointer data into project files, and read it back out again properly when loading them.
 *
 * Revision 1.54  2006/07/06 14:56:01  vapour
 * Added code to calculate onscreen boundaries of mouse cursor.
 *
 * Revision 1.53  2006/06/27 13:47:16  vapour
 * + Moved some code from the flame_read function into the menu_file_open function, as it's more logical to have it there.
 * + Committed the scaling up of the exported svg text width, as I forgot to do it the other day.
 *
 * Revision 1.52  2006/06/25 13:16:18  vapour
 * Improved the vertical centering of text in it's background box.
 *
 * Revision 1.51  2006/06/25 12:57:25  vapour
 * Exported text layers now use the embedded Bitstream Vera font.
 *
 * Revision 1.50  2006/06/22 14:26:08  vapour
 * Updated to use the embedded italic Bitstream Vera font while validating the embedding of fonts.
 *
 * Revision 1.49  2006/06/19 14:51:57  vapour
 * + Small code cleanups in the svg export section, so freeing of memory is done a bit better.
 * + Improved the code that saves the window maximisation preference.
 *
 * Revision 1.48  2006/06/18 08:21:28  vapour
 * Updated so the exported svg animation starts playing after the play button has faded out.
 *
 * Revision 1.47  2006/06/12 11:03:16  vapour
 * Added code to the svg output so things will start when the play button is clicked.
 *
 * Revision 1.46  2006/06/12 04:11:31  vapour
 * Another tweak to ensure gnome isn't called on Windows.
 *
 * Revision 1.45  2006/06/12 03:49:55  vapour
 * Added functions for capturing log output, and updated many of the warning messages to go through the display_warning function.
 *
 * Revision 1.44  2006/06/10 16:03:50  vapour
 * Adjusted the text positioning properties when outputting svg.
 *
 * Revision 1.43  2006/06/10 15:31:52  vapour
 * Added code to free slide name memory when destroying slides.
 *
 * Revision 1.42  2006/06/10 15:10:59  vapour
 * + More visual realignment of variable names for my Linux system.
 * + Improved the code for saving slide names to a project file.
 * + Added working code for reading slide names from a project file.
 *
 * Revision 1.41  2006/06/08 12:16:14  vapour
 * Updated to use the new function for generating tooltips.
 *
 * Revision 1.40  2006/06/07 15:21:09  vapour
 * Created a function for freeing the resources allocated to a slide.
 *
 * Revision 1.39  2006/06/04 06:59:54  vapour
 * Updated to correctly size text in the SVG output.
 *
 * Revision 1.38  2006/06/04 06:12:33  vapour
 * Updated the svg output so element properties are initially defined in a <defs> section.
 *
 * Revision 1.37  2006/06/04 04:07:56  vapour
 * Updated the SVG output, so the elements of the text layer are grouped together logically.
 *
 * Revision 1.36  2006/05/31 11:30:54  vapour
 * Updated the stroke-linejoin attribute in the SVG output to be SVG 1.1 compliant.  Opera loads the output with 0 warnings now. :)
 *
 * Revision 1.35  2006/05/30 22:49:24  vapour
 * Fixed the keyTimes attribute being output for SVG, so Opera 9 beta 2 should be happy with it.  Also broke out the elements from the style attribute into their own component attributes.
 *
 * Revision 1.34  2006/05/30 12:43:38  vapour
 * Tweaked so new windows registry values will correctly overwrite old ones.
 *
 * Revision 1.33  2006/05/29 14:08:54  vapour
 * Added initial working code to save project preferences in the windows registry.
 *
 * Revision 1.32  2006/05/28 09:41:01  vapour
 * Added code so the output resolution selector is recreated each time a project is loaded, including whatever resolution is specified in the loaded project.
 *
 * Revision 1.31  2006/05/22 13:09:36  vapour
 * Fades (in and out) now work correctly in SVG output too (for Opera 9 at least).
 *
 * Revision 1.30  2006/05/22 12:51:17  vapour
 * Animation (i.e. movement) in the SVG output now occurs.  In Opera anyway.  Made it work by copying the W3C example at: http://www.w3.org/TR/SVG/animate.html#Animation.classExample, and adjusting to suit.
 *
 * Revision 1.29  2006/05/21 13:47:46  vapour
 * + Trying out the animateMotion keyword in the svg output.  Can't see much difference (i.e. needs more work), but it sounds like the right way forward.
 * + Trying out a new way of calculating font size in the svg output, and it's looking better already.
 *
 * Revision 1.28  2006/05/20 13:30:46  vapour
 * Fixed a comment.
 *
 * Revision 1.27  2006/05/20 12:52:11  vapour
 * Added svg output tags to move the highlight and text layers into their final positions over the course of each slide.
 *
 * Revision 1.26  2006/05/18 13:54:18  vapour
 * Added svg output properties to move image layers to their final positions during the animation.
 *
 * Revision 1.25  2006/05/17 11:20:26  vapour
 * Removed some crufty old code.
 *
 * Revision 1.24  2006/05/17 11:05:53  vapour
 * Corrected the output timings for the SVG export.  They seem good now.
 *
 * Revision 1.23  2006/05/15 13:35:12  vapour
 * Changed ifdefs so gnome functions aren't used on Windows.
 *
 * Revision 1.22  2006/05/14 12:34:59  vapour
 * + Adjusted images in svg output to be sized better (maybe even properly!). :)
 * + Added some explicit sizing info to the svg output.
 * + Added code to the SVG output, for fading in and out image layers.
 * + Added code to the SVG output, for fading in and out text layers.
 * + Changed the internal image encoding for SVG data from png to jpeg format, as Firefox has less issues with it.
 * + Improved the code for the SVG output highlight by switching to g_string_append_printf().
 * + Added working 1 second fade in for highlights for SVG output.  Works in ASV3, not Firefox.
 *
 * Revision 1.21  2006/05/06 19:36:18  vapour
 * Small tweaks to the font sizing for SVG export.
 *
 * Revision 1.20  2006/05/06 19:12:33  vapour
 * Added initial semi-working code for text output in the SVG export. Still needs work.
 *
 * Revision 1.19  2006/05/05 15:01:37  vapour
 * + Improved the SVG output code to draw layers in the correct order.
 * + Improved the SVG output code to scale the highlight and text background boxes to the rest of the scene.
 *
 * Revision 1.18  2006/05/02 13:06:38  vapour
 * Fixed a bug in the project loading code, where the values weren't being transferred through to the layer area.
 *
 * Revision 1.17  2006/05/01 13:53:21  vapour
 * Added code to output the background for the text layer, for the SVG output.
 *
 * Revision 1.16  2006/05/01 13:21:29  vapour
 * Improved output SVG highlight layer to include it's x and y starting offset positions.
 *
 * Revision 1.15  2006/05/01 09:20:01  vapour
 * Added initial code for output of highlight layers to SVG.
 *
 * Revision 1.14  2006/04/29 19:11:16  vapour
 * + Added a (very rough) function to URI encode Base64 data.
 * + Turned off the compression = 9 option for png data, as it appears to be causing a problem of images not displaying in FireFox.
 *
 * Revision 1.13  2006/04/29 17:41:31  vapour
 * Wrote a function to Base64 encode data.  Works ok.
 *
 * Revision 1.12  2006/04/27 19:53:33  vapour
 * Started adjusting the SVG slide output function, in preparation for further work.
 *
 * Revision 1.11  2006/04/27 16:03:34  vapour
 * Added support for empty layers to the project loading function.
 *
 * Revision 1.10  2006/04/26 18:28:41  vapour
 * + Fleshed out the flame_read function so it now works.  Had to adjust the entity names being saved in order to do so however.
 * + Removed the flame_write function, as it's function is done elsewhere.
 *
 * Revision 1.9  2006/04/25 10:53:05  vapour
 * Modified images now have their own file created when the project is saved.
 *
 * Revision 1.8  2006/04/25 09:56:24  vapour
 * Background color information is saved for empty layers.
 *
 * Revision 1.7  2006/04/22 08:36:54  vapour
 * + Replaced the text string display in the timeline (layer) widget area, with the x and y finish positions.
 * + Updated the entire project to use the word "finish" consistently, instead of "final".
 *
 * Revision 1.6  2006/04/21 17:44:51  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.5  2006/04/20 12:26:06  vapour
 * Updated to find the sound file in /(basepath)/share/sounds/flame/.
 *
 * Revision 1.4  2006/04/20 12:03:37  vapour
 * + Added a dialog box for the Export As Flash option, asking the user where they want to save.
 * + Quick fix to the flash exporting inner function, so it doesn't generate a warning message on empty layers.
 *
 * Revision 1.3  2006/04/18 18:00:52  vapour
 * Tweaks to allow compilation to succeed on both Windows and Solaris as well.
 * On Windows, the app will fire up as it only really required changes to not use GConf.
 * On Solaris however, a lot of stuff needed to be disabled, so it core dumps right away, prior to even displaying a window.
 * However, this *is* progress of a sort. :)
 *
 * Revision 1.2  2006/04/16 06:06:57  vapour
 * Added notes relevant to building on Windows (MinGW).
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

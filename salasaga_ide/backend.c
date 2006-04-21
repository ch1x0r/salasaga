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

// Ming includes
#include <ming.h>

// GTK includes
#include <gtk/gtk.h>

// GConf include (not for windows)
#ifndef _WIN32
	#include <gconf/gconf.h>
#endif

// Gnome include (for sound)
#include <libgnome/libgnome.h>

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Flame Edit includes
#include "flame-types.h"
#include "backend.h"
#include "callbacks.h"
#include "gui-functions.h"
#include "externs.h"


// Function to calculate collision detection boundaries
void calculate_object_boundaries(void)
{
	// Local variables
	GList				*layer_pointer;
	guint				num_layers;					// The number of layers in the slide
	guint				count_int;					// Counter

	boundary_box			*boundary;					// Boundary information

	guint				tmp_int;						//
	GdkRectangle			tmp_rectangle;				//


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
		g_list_foreach(boundary_list, (gpointer) g_free, NULL);  // I *think* this will free the memory allocated for the GdkRegions.  Disabled for now while debugging other stuff
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

			case TYPE_TEXT:
				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 102;
				tmp_rectangle.width = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->rendered_width * zoom) / 101;
				tmp_rectangle.y = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->rendered_height * zoom) / 97;
				break;

			default:
				g_printerr("ED27: Unknown layer type\n");
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
	xmlDocPtr	document;
//	xmlNodePtr	cur;
//	xmlChar		*key;
//	xmlChar		*language;


	// Begin reading the file
	document = xmlParseFile(filename);
	if (NULL == document)
	{
		// The Flame file was unable to be parsed
		display_warning("Flame file was unable to be loaded\n");
		return FALSE;
	}

/* This code copied straight from my Gentoo Application selector code
 * fixme2: Needs to be reworked to suit this
 * 
		cur = xmlDocGetRootElement(doc);
		if (NULL == cur)
		{
			fprintf(stderr, "Metadata document '%s' is empty.\n\n", tmp_string);
			xmlFreeDoc(doc);
			exit(16);
		}

		// Look for the "longdescription" node
		cur = cur->xmlChildrenNode;
		while (NULL != cur)
		{
			if ((!xmlStrcmp(cur->name, (const xmlChar *) "longdescription")))
			{
				// Find the correct language version of the string
				language = xmlGetProp(cur, (const xmlChar *) "lang");
				if (0 == xmlStrcmp(INTERFACE_LANGUAGE, language))
				{
						// Extract the text string and store it in the category info
						key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

						// Strip the tabs and newlines embedded in the string
						untaint_string((char *) &tmp_cat.description, key);

						// We're finished with these XML keys, so release their memory
						xmlFree(key);
						xmlFree(language);
						
						// Stop looping around, as we've finished the processing we need
						break;
				}
				// We're finished with this XML key, so release its memory
				xmlFree(language);
			}
			cur = cur->next;
		}
*/

		// We're finished with this XML document, so release its memory
		xmlFreeDoc(document);

	return FALSE;
}


// Function to create a .flame file
gboolean flame_write(gchar *filename)
{
	return FALSE;
}


// Function disable or enable a given menu
void menu_enable(const gchar *full_path, gboolean enable)
{
	// Local variables
	GtkWidget				*menu_item;

	menu_item = gtk_item_factory_get_item(GTK_ITEM_FACTORY(menu_bar), full_path);
	gtk_widget_set_sensitive(menu_item, enable);
}


// Function to convert a slide structure into a Flash output file
void menu_export_flash_inner(gchar *file_name, guint start_slide, guint finish_slide)
{
	// Local variables
	FILE					*file;
	slide				*processing_slide;
	layer				*processing_layer;

	GtkTextIter			text_end;				// The end position of the text buffer
	GtkTextIter			text_start;				// The start position of the text buffer

	guint				finish_x_offset;
	guint				finish_y_offset;
	gint					frame_counter;
	gint					height;
	guint				finish_frame;
	gfloat				font_size;
	gint					layer_counter;
	guint				num_layers;
	gint					num_steps;
	gint					slide_counter;
	guint				start_frame;
	guint				start_x_offset;
	guint				start_y_offset;
	guint				width;
	gfloat				x_step, y_step;

	SWFBitmap			bitmap;
	SWFDisplayItem		display_item;
	SWFInput				input;
	SWFFill				fill;
	SWFMovie				flash_movie;
	SWFFont				font;
	SWFMovieClip			movie_clip;
	SWFShape				shape;
	SWFText				text;
	SWFDisplayItem		text_item;
	SWFMovieClip			text_movie_clip;

	GString				*tmp_gstring;


	// Initialise various things
	tmp_gstring = g_string_new(NULL);

	// fixme4: At the time of coding, the only scale factor that works properly with all shape types in Ming is 1.0. :(
	//         That might have improved
	// Create the movie structure
	Ming_setScale(1.0);
	flash_movie = newSWFMovie();

	// Process the requested range of slides
	for (slide_counter = start_slide; slide_counter <= finish_slide; slide_counter++)
	{
		// Jump to the required slide
		slides = g_list_first(slides);
		processing_slide = g_list_nth_data(slides, slide_counter);

		// Process each layer in turn, in reverse order (so things are output visually correctly)
		processing_slide->layers = g_list_first(processing_slide->layers);
		num_layers = g_list_length(processing_slide->layers);
		for (layer_counter = num_layers - 1; 0 <= layer_counter; layer_counter--)
		{
			processing_layer = g_list_nth_data(processing_slide->layers, layer_counter);
			switch (processing_layer->object_type)
			{
				case TYPE_GDK_PIXBUF:
					start_x_offset = ((layer_image *) processing_layer->object_data)->x_offset_start;
					start_y_offset = ((layer_image *) processing_layer->object_data)->y_offset_start;
					finish_x_offset = ((layer_image *) processing_layer->object_data)->x_offset_finish;
					finish_y_offset = ((layer_image *) processing_layer->object_data)->y_offset_finish;
					width = (gint) ((layer_image *) processing_layer->object_data)->width;
					height = (gint) ((layer_image *) processing_layer->object_data)->height;
					start_frame = processing_layer->start_frame;
					finish_frame = processing_layer->final_frame;

					// Read the image file in again (forced to by Ming :-< )
					file = fopen(((layer_image *) processing_layer->object_data)->image_path->str, "r");
					input = newSWFInput_file(file);
					bitmap = newSWFBitmap_fromInput(input);
					fclose(file);

					// * Create the image layer *
					shape = newSWFShapeFromBitmap((SWFCharacter) bitmap, SWFFILL_TILED_BITMAP);

					// Create an instance of it
					movie_clip = newSWFMovieClip();
					display_item = SWFMovieClip_add(movie_clip, (SWFBlock) shape);
					SWFDisplayItem_setName(display_item, processing_layer->name->str);

					// Ensure things last for the required number of frames
					num_steps = finish_frame - start_frame;
					if (0 == num_steps) num_steps = 1;
					x_step = (finish_x_offset - start_x_offset) / num_steps;
					y_step = (finish_y_offset - start_y_offset) / num_steps;

					// Make the shape move to its finish position over time
					SWFDisplayItem_moveTo(display_item, start_x_offset, start_y_offset);
					for (frame_counter = start_frame; frame_counter <= finish_frame; frame_counter++)
					{
						SWFDisplayItem_move(display_item, (int) x_step, (int) y_step);
						SWFMovieClip_nextFrame(movie_clip);
					}

					// Attach the movie clip to the movie
					SWFMovie_add(flash_movie, (SWFBlock) movie_clip);
					break;

				case TYPE_TEXT:
					// Get the information for the layer
					start_x_offset = ((layer_text *) processing_layer->object_data)->x_offset_start;
					start_y_offset = ((layer_text *) processing_layer->object_data)->y_offset_start;
					finish_x_offset = ((layer_text *) processing_layer->object_data)->x_offset_finish;
					finish_y_offset = ((layer_text *) processing_layer->object_data)->y_offset_finish;
					width = (gint) ((layer_text *) processing_layer->object_data)->rendered_width;
					height = (gint) ((layer_text *) processing_layer->object_data)->rendered_height;
					font_size = ((layer_text *) processing_layer->object_data)->font_size;
					start_frame = processing_layer->start_frame;
					finish_frame = processing_layer->final_frame;

					// Create the background for the text to go on
					shape = newSWFShape();
					fill = SWFShape_addSolidFill(shape, 0xff, 0xff, 0xcc, 0xff); // 0xFFFFCCFF
					SWFShape_setLine(shape, 1, 0x0, 0, 0, 0xff);
					SWFShape_setRightFill(shape, fill);
					SWFShape_movePenTo(shape, 0, -height + 20);
					SWFShape_drawLineTo(shape, width, -height + 20);
					SWFShape_drawLineTo(shape, width, 0 + 20);
					SWFShape_drawLineTo(shape, 0, 0 + 20);
					SWFShape_drawLineTo(shape, 0, -height + 20);

					// * Create the text *

					// Read the font file
					file = fopen("/home/jc/workspace3/flame-edit/fonts/BitstreamVera/fdb/Bitstream Vera Sans.fdb", "r");
					if (NULL == file)
					{
						display_warning("ED22: Something went wrong opening the Bitstream Vera Sans font file\n");
						return;
					}
					font = loadSWFFontFromFile(file);
					fclose(file);

					// Create the text object
					text = newSWFText2();
					SWFText_setFont(text, font);
					SWFText_setColor(text,
						((layer_text *) processing_layer->object_data)->text_color.red >> 8,  // Red
						((layer_text *) processing_layer->object_data)->text_color.green >> 8,  // Green
						((layer_text *) processing_layer->object_data)->text_color.blue >> 8,  // Blue
						0xff);  // Alpha
					SWFText_setHeight(text, font_size);
					SWFText_setSpacing(text, 1.0);
					gtk_text_buffer_get_bounds(((layer_text *) processing_layer->object_data)->text_buffer, &text_start, &text_end);
					g_string_assign(tmp_gstring, gtk_text_buffer_get_text(((layer_text *) processing_layer->object_data)->text_buffer, &text_start, &text_end, FALSE));
					SWFText_addString(text, tmp_gstring->str, NULL);

					// Attach the background and text to sprites/movie-clips
					movie_clip = newSWFMovieClip();
					display_item = SWFMovieClip_add(movie_clip, (SWFBlock) shape);
					SWFDisplayItem_setName(display_item, processing_layer->name->str);

					text_movie_clip = newSWFMovieClip();
					text_item = SWFMovieClip_add(text_movie_clip, (SWFBlock) text);
					SWFDisplayItem_setName(text_item, processing_layer->name->str);

					// Ensure things last for the required number of frames
					num_steps = finish_frame - start_frame;
					if (0 == num_steps) num_steps = 1;
					x_step = (finish_x_offset - start_x_offset) / num_steps;
					y_step = (finish_y_offset - start_y_offset) / num_steps;

					// Make the shape move to its finish position over time
					SWFDisplayItem_moveTo(display_item, start_x_offset, start_y_offset);
					SWFDisplayItem_moveTo(text_item, start_x_offset + 10, start_y_offset);
					for (frame_counter = start_frame; frame_counter <= finish_frame; frame_counter++)
					{
						SWFDisplayItem_move(display_item, (int) x_step, (int) y_step);
						SWFDisplayItem_move(text_item, (int) x_step, (int) y_step);
						SWFMovieClip_nextFrame(movie_clip);
						SWFMovieClip_nextFrame(text_movie_clip);
					}
					SWFDisplayItem_moveTo(display_item, finish_x_offset, finish_y_offset);
					SWFDisplayItem_moveTo(text_item, finish_x_offset + 10, finish_y_offset);
					SWFMovieClip_nextFrame(movie_clip);
					SWFMovieClip_nextFrame(text_movie_clip);

					// Attach the movie clips to the movie
					SWFMovie_add(flash_movie, (SWFBlock) movie_clip);
					SWFMovie_add(flash_movie, (SWFBlock) text_movie_clip);
					break;

				case TYPE_HIGHLIGHT:
					// Get the information for the layer
					start_x_offset = ((layer_highlight *) processing_layer->object_data)->x_offset_start;
					start_y_offset = ((layer_highlight *) processing_layer->object_data)->y_offset_start;
					finish_x_offset = ((layer_highlight *) processing_layer->object_data)->x_offset_finish;
					finish_y_offset = ((layer_highlight *) processing_layer->object_data)->y_offset_finish;
					width = (gint) ((layer_highlight *) processing_layer->object_data)->width;
					height = (gint) ((layer_highlight *) processing_layer->object_data)->height;
					start_frame = processing_layer->start_frame;
					finish_frame = processing_layer->final_frame;

					// Create the highlight layer
					shape = newSWFShape();
					fill = SWFShape_addSolidFill(shape, 0x00, 0xFF, 0x00, 0x40); // 0x00FF0040 : Good fill color
					SWFShape_setLine(shape, 1, 0x00, 0xFF, 0x00, 0xCC);  // 0x00FF00CC : Good border color
					SWFShape_setRightFill(shape, fill);
					SWFShape_movePenTo(shape, 0, 0);
					SWFShape_drawLineTo(shape, width, 0);
					SWFShape_drawLineTo(shape, width, height);
					SWFShape_drawLineTo(shape, 0, height);
					SWFShape_drawLineTo(shape, 0, 0);

					// Create an instance of it
					movie_clip = newSWFMovieClip();
					display_item = SWFMovieClip_add(movie_clip, (SWFBlock) shape);
					SWFDisplayItem_setName(display_item, processing_layer->name->str);

					// Ensure things last for the required number of frames
					num_steps = finish_frame - start_frame;
					if (0 == num_steps) num_steps = 1;
					x_step = (finish_x_offset - start_x_offset) / num_steps;
					y_step = (finish_y_offset - start_y_offset) / num_steps;

					// Make the shape move to its finish position over time
					SWFDisplayItem_moveTo(display_item, start_x_offset, start_y_offset);
					for (frame_counter = start_frame; frame_counter <= finish_frame; frame_counter++)
					{
						SWFDisplayItem_move(display_item, (int) x_step, (int) y_step);
						SWFMovieClip_nextFrame(movie_clip);
					}

					// Attach the movie clip to the movie
					SWFMovie_add(flash_movie, (SWFBlock) movie_clip);
					break;

				case TYPE_EMPTY:
					// Empty layer, skip it
					break;

				default:
					g_printerr("ED28: Unknown layer type\n");

			}  // Switch statement

		}  // Layer counter

	}  // Slide counter

	// Save the flash file
	SWFMovie_setDimension(flash_movie, project_width, project_height);
	SWFMovie_setBackground(flash_movie, 0x0, 0x0, 0x0);  // Background color
	SWFMovie_setRate(flash_movie, frames_per_second);
	SWFMovie_save(flash_movie, file_name);

	// Add a message to the status bar about the successful flash export
	g_string_printf(tmp_gstring, "Flash '%s' exported successfully.", file_name);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// Free the memory allocated in this function
	g_string_free(tmp_gstring, TRUE);
}


// Function to convert a slide structure into an output file
void menu_export_svg_animation_slide(gpointer element, gpointer user_data)
{
	// Local variables
	GError			*error = NULL;				// Pointer to error return structure
	GIOStatus		return_value;				// Return value used in most GIOChannel functions

	slide			*slide_pointer;				// Points to the present slide
	layer			*layer_pointer;				// Points to the presently processing layer
	guint			slide_number;				// The number of the presently processing slide
	guint			object_type;					// The type of object in each layer
	guint			start_frame;					// The first frame in which the object appears
	guint			final_frame;					// The final frame in which the object appears
	GString			*file_name = NULL;			// Used to construct the file name of the output file
	GString			*string_to_write = NULL;		// Holds SVG data to be written out
	GString			*new_file_name = NULL;		// Holds the filename for any output files
	guint			second_counter = 0;			// Holds the starting second (in the whole animation) for the present layer
	guint			max_frame_second;			// For any frame, holds the last frame number visible
	guint			*image_counter = user_data;	// Counter for how many images have been written out

	guint			recent_message;				// Message identifier, for newest status bar message

	GdkRectangle		status_bar_rect =			// Rectangle covering the status bar area
						{0,
						 0,
						 status_bar->allocation.width,
						 status_bar->allocation.height};

	gboolean			tmp_bool;					// Temporary boolean value
	gsize			tmp_gsize;					// Temporary gsize
	GdkPixbuf		*tmp_pixbuf;					// Temporary GDK pixbuf
	GString			*tmp_gstring;				// Temporary GString


	// Initialise the temporary gstring
	tmp_gstring = g_string_new(NULL);

	// Reset the counter for the last frame number visible in this slide
	max_frame_second = 0;

	// * For each layer, write it to the output file *
	// fixme2: Assume each slide only has one layer for now
	slide_pointer = element;
	layer_pointer = ((GList *) slide_pointer->layers)->data;

	// Create some useful pointers
	slide_number = slide_pointer->number;
	start_frame = layer_pointer->start_frame;
	final_frame = layer_pointer->final_frame;
	object_type = layer_pointer->object_type;

//g_printerr("Slide number: %u\n", slide_number);
//g_printerr("Start frame: %u\n", start_frame);
//g_printerr("Final frame: %u\n", final_frame);
//g_printerr("Object type: %u\n", object_type);

		// Work out if the present layer is visible longer than any known present
		if (final_frame > max_frame_second)
		{
			max_frame_second = final_frame;
		}

		// Determine the type of object present in the layer
		switch (object_type)
		{
			case TYPE_GDK_PIXBUF:
					// * We're processing a GDK pixbuf *

					// Resize the pixbuf to the output resolution
					tmp_pixbuf = gdk_pixbuf_scale_simple(GDK_PIXBUF(layer_pointer->object_data), output_width, output_height, GDK_INTERP_BILINEAR);
					if (NULL == tmp_pixbuf)
					{
						// * Something went wrong when scaling down the pixbuf *

						// Display the warning message using our function
						g_string_printf(tmp_gstring, "Error ED13: An error '%s' occured when scaling slide %u", error->message, slide_number);
						display_warning(tmp_gstring->str);

						// Free the memory allocated in this function
						g_object_unref(tmp_pixbuf);
						g_string_free(tmp_gstring, TRUE);
						g_error_free(error);
						// fixme5: Probably needs some work, as there would likely be further allocated memory by this stage

						return;
					}

					// Write the pixbuf to the output file
					new_file_name = g_string_new(NULL);
					g_string_printf(new_file_name, "%s%coutput%u.png", output_folder->str, G_DIR_SEPARATOR, *image_counter);
					tmp_bool = gdk_pixbuf_save(tmp_pixbuf, new_file_name->str, "png", &error, "compression", "9", NULL); // fixme2: Should use output_quality variable rather than "9"
					if (FALSE == tmp_bool)
					{
						// * Some kind of error occured when saving the file *

						// Display a warning message using our function
						g_string_printf(tmp_gstring, "Error ED14: An error '%s' occured when saving the file '%s'", error->message, new_file_name->str);
						display_warning(tmp_gstring->str);
						g_string_free(tmp_gstring, TRUE);

						return;
					}

					// Increment the image counter
					(*image_counter)++;

					// Free the allocated pixbuf memory
					g_object_unref(tmp_pixbuf);

					// Small update to the status bar, to show progress to the user
					// fixme5: This works, but a progress bar might be the way to go
					g_string_printf(tmp_gstring, "Wrote component file '%s'.", new_file_name->str);
					recent_message = gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
					gtk_widget_draw(status_bar, &status_bar_rect);
					gdk_flush();

					// Create a string to write to the output svg file
					string_to_write = g_string_new(NULL);
					g_string_printf(string_to_write, "<image xlink:href=\"%s\" width=\"%u\" height=\"0\">\n\t<animate attributeName=\"height\" from=\"%upx\" to=\"%upx\" begin=\"%us\" dur=\"%us\" />\n</image>\n", new_file_name->str, output_width, output_height, output_height, second_counter + start_frame, second_counter + final_frame);
					g_string_free(new_file_name, TRUE);
					new_file_name = NULL;

					break;

			case TYPE_MOUSE_CURSOR:
					g_printerr("Mouse cursor found in layer %u\n", slide_pointer->number);
					break;

			default:
					g_printerr("ED29: Unknown object type found in layer %u\n", slide_pointer->number);
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
				g_string_free(string_to_write, TRUE);
				g_string_free(file_name, TRUE);
				g_string_free(tmp_gstring, TRUE);
				g_error_free(error);

				return;
			}

			// Reset the string to be written to the output file
			g_string_free(string_to_write, TRUE);
		}

		// Increment the overall frame counter
		second_counter = second_counter + max_frame_second;

	return;
}


// Function that takes a layer structure pointer, and adds it's data to an in-memory XML document
void menu_file_save_layer(gpointer element, gpointer user_data)
{
	// Local variables
	layer			*layer_pointer;				// Points to the presently processing layer

	xmlNodePtr		slide_node;					// Pointer to the slide node
	xmlNodePtr		layer_node;					// Pointer to the new layer node

	guint			layer_type;					// The type of layer
	guint			start_frame;					// The first frame in which the object appears
	guint			final_frame;					// The final frame in which the object appears
	GString			*layer_name;					// Name of the layer

	GtkTextIter		text_start;					// The start position of the text buffer
	GtkTextIter		text_end;					// The end position of the text buffer

	GString			*tmp_gstring;				// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	layer_pointer = element;
	slide_node = user_data;

	// Create some useful pointers
	start_frame = layer_pointer->start_frame;
	final_frame = layer_pointer->final_frame;
	layer_type = layer_pointer->object_type;
	layer_name	= layer_pointer->name;

    // Create the layer container
	layer_node = xmlNewChild(slide_node, NULL, "layer", NULL);
	if (NULL == layer_node)
	{
		display_warning("ED24: Error creating the layer elements\n");
		return;
	}

	// Add the layer data to the layer container
	xmlNewChild(layer_node, NULL, "name", layer_name->str);
	g_string_printf(tmp_gstring, "%u", start_frame);
	xmlNewChild(layer_node, NULL, "start frame", tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", final_frame);
	xmlNewChild(layer_node, NULL, "final frame", tmp_gstring->str);
	switch (layer_type)
	{
		case TYPE_GDK_PIXBUF:
			xmlNewChild(layer_node, NULL, "type", "image");
			xmlNewChild(layer_node, NULL, "path", ((layer_image *) layer_pointer->object_data)->image_path->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, "x offset start", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, "y offset start", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, "x offset finish", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, "y offset finish", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, "width", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, "height", tmp_gstring->str);
			break;

		case TYPE_EMPTY:
			xmlNewChild(layer_node, NULL, "type", "empty");
			break;

		case TYPE_TEXT:
			xmlNewChild(layer_node, NULL, "type", "text");
			gtk_text_buffer_get_bounds(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end);
			xmlNewChild(layer_node, NULL, "text", gtk_text_buffer_get_text(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end, FALSE));
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, "x offset start", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, "y offset start", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, "x offset finish", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, "y offset finish", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.red);
			xmlNewChild(layer_node, NULL, "red", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.green);
			xmlNewChild(layer_node, NULL, "green", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.blue);
			xmlNewChild(layer_node, NULL, "blue", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%f", ((layer_text *) layer_pointer->object_data)->font_size);
			xmlNewChild(layer_node, NULL, "font size", tmp_gstring->str);
			break;

		case TYPE_HIGHLIGHT:
			xmlNewChild(layer_node, NULL, "type", "highlight");
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, "x offset start", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, "y offset start", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, "x offset finish", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, "y offset finish", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, "width", tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, "height", tmp_gstring->str);
			break;
	}

	return;
}


// Function that takes a slide structure pointer, and adds it's data to an in-memory XML document
void menu_file_save_slide(gpointer element, gpointer user_data)
{
	// Local variables
	slide			*slide_pointer;				// Points to the present slide
	GList			*layer_pointer;				// Points to the presently processing layer

	guint			slide_number;				// The number of the presently processing slide

	xmlNodePtr		slide_root;					// Points to the root of the slide data
	xmlNodePtr		slide_node;					// Pointer to the new slide node

	GString			*tmp_gstring;				// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	slide_pointer = element;
	slide_root = user_data;
	layer_pointer = slide_pointer->layers;

	// Create some useful pointers
	slide_number = slide_pointer->number;

    // Create the slide container
	slide_node = xmlNewChild(slide_root, NULL, "slide", NULL);
	if (NULL == slide_node)
	{
		display_warning("ED23: Error creating the slide node\n");
		return;
	}

	// Add the slide number to the slide container attributes
	g_string_printf(tmp_gstring, "%u", slide_number);
	xmlNewProp(slide_node, "number", tmp_gstring->str);

	// Add the layer data to the slide container
	layer_pointer = g_list_first(layer_pointer);
	g_list_foreach(layer_pointer, menu_file_save_layer, slide_node);

	return;
}


// Function to save the application preferences prior to exiting
// fixme3: This doesn't seem to be getting called when the user presses Alt-F4
void save_preferences_and_exit(void)
{
	// Local variables
#ifndef _WIN32
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

	// Save a configuration structure version number
	gconf_engine_set_float(gconf_engine, "/apps/flame/defaults/config_version", 1.0, NULL);

	// fixme4: Should we save a list of recent projects worked on?

	// Notify the GConf engine that now is probably a good time to sync
	gconf_engine_suggest_sync(gconf_engine, NULL);

	// Free our GConf engine
	gconf_engine_unref(gconf_engine);
#endif

	// Shut down sound
	gnome_sound_shutdown();

	// Exit the application
	gtk_main_quit();
}


// Function to sound a beep (on user error, etc)
void sound_beep(void)
{
	gnome_sound_play("../share/sounds/flame/generic.wav");
}


/*
 * History
 * +++++++
 * 
 * $Log$
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

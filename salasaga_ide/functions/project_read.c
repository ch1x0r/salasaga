/*
 * $Id$
 *
 * Flame Project: Function to parse the contents of a .flame file
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
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
#include <math.h>

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
#include "../flame-types.h"
#include "../externs.h"
#include "base64_decode.h"
#include "compress_layers.h"
#include "create_timeline_slider.h"
#include "destroy_slide.h"
#include "display_warning.h"


gboolean flame_read(gchar *filename)
{
	// Local variables
	GError				*error = NULL;				// Pointer to error return structure
	GString				*error_string;				// Used to create error strings
	GdkPixbufLoader			*image_loader;				// Used for loading images embedded in project files
	gboolean			return_code;				// Boolean return code
	gfloat				save_version;				// The project file save version
	xmlDocPtr			document;				// Holds a pointer to the XML document
	xmlNodePtr			layer_ptr;				// Temporary pointer
	xmlNodePtr			meta_data_node = NULL;			// Points to the meta-data structure
	xmlNodePtr			preferences_node = NULL;		// Points to the preferences structure
	xmlNodePtr			slides_node = NULL;			// Points to the slides structure
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
	layer_empty			*tmp_empty_ob;				//
	GList				*tmp_glist;				//
	GString				*tmp_gstring;				// Temporary GString
	GString				*tmp_gstring2;				// Temporary GString
	layer_highlight			*tmp_highlight_ob;			// Temporary highlight layer object
	layer_image			*tmp_image_ob;				// Temporary image layer object
	gint				tmp_int;				// Temporary integer
	GtkTreeIter			*tmp_iter;				// Temporary GtkTreeIter
	layer				*tmp_layer;				// Temporary layer
	layer_mouse			*tmp_mouse_ob;				// Temporary mouse layer object
	GdkPixbuf			*tmp_pixbuf;				//
	slide				*tmp_slide;				// Temporary slide
	layer_text			*tmp_text_ob;				// Temporary text layer object

	gint				data_length;				// Number of image data bytes a layer says it stores

	guint				num_layers;				// Number of layers in a slide (used for a loop)
	guint				layer_counter;				// Counter used when processing layers
	layer				*layer_data;				// Pointer to the layer data we're working on
	GdkPixbuf			*layer_pixbuf;				// Pointer used when creating duration images for layers
	guint				start_frame;				// Used when working out a layer's start frame
	guint				finish_frame;				// Used when working out a layer's finish frame
	gfloat				start_pixel;				// Starting slider pixel to fill in
	gfloat				finish_pixel;				// Ending slider pixel to fill in
	gfloat				pixel_width;				// Width of pixels to fill

	GtkTreeIter			film_strip_iter;

	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	tmp_gstring2 = g_string_new(NULL);
	error_string = g_string_new(NULL);
	
	// Begin reading the file
	document = xmlParseFile(filename);
	if (NULL == document)
	{
		// The Flame file was unable to be parsed
		display_warning("Error ED43: Project file was unable to be loaded\n");
		return FALSE;
	}

	this_node = xmlDocGetRootElement(document);
	if (NULL == this_node)
	{
		display_warning("Error ED44: Project file is empty");
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
		display_warning("Error ED63: Project meta-data missing, aborting load");
		return FALSE;
	}

	// Was there a preferences structure in the save file?
	if (NULL == preferences_node)
	{
		display_warning("Error ED45: Project preferences missing, aborting load");
		return FALSE;
	}

	// Was there a slides structure in the save file?
	if (NULL == slides_node)
	{
		display_warning("Error ED46: No slides in project, aborting load");
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
		display_warning("Error ED47: Project Name missing, aborting project open");
		return FALSE;
	}
	if (NULL == project_width_data)
	{
		display_warning("Error ED48: Project Width missing, aborting project open");
		return FALSE;
	}
	if (NULL == project_height_data)
	{
		display_warning("Error ED49: Project Height missing, aborting project open");
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

	// If there's an existing film strip, we unload it
	gtk_list_store_clear(GTK_LIST_STORE(film_strip_store));

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
			tmp_slide->duration = slide_length;
			tmp_slide->layer_store = gtk_list_store_new(TIMELINE_N_COLUMNS,  // TIMELINE_N_COLUMNS
										G_TYPE_STRING,  // TIMELINE_NAME
										G_TYPE_BOOLEAN,  // TIMELINE_VISIBILITY
										GDK_TYPE_PIXBUF,  // TIMELINE_DURATION
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

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
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
										g_string_printf(error_string, "Error ED66: Image data loading failed: '%s'", error->message);
										display_warning(error_string->str);
										g_string_free(error_string, TRUE);
									}
									return_code = gdk_pixbuf_loader_close(image_loader, &error);
									if (TRUE != return_code)
									{
										g_string_printf(error_string, "Error ED67: Image data loading failed: '%s'", error->message);
										display_warning(error_string->str);
										g_string_free(error_string, TRUE);
									}
									tmp_image_ob->image_data = gdk_pixbuf_loader_get_pixbuf(image_loader);
									if (NULL == tmp_image_ob->image_data)
									{
										display_warning("Error ED65: Error when loading image data");
									}
								}

								// Set the modified flag for this image to false
								tmp_image_ob->modified = FALSE;

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
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

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
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

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
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

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
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

			// Create the duration slider images for the timeline area
			num_layers = g_list_length(tmp_slide->layers);
			for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
			{
				// Work out the start and ending frames for this layer
				layer_data = g_list_nth_data(tmp_slide->layers, layer_counter);
				start_frame = layer_data->start_frame;
				finish_frame = layer_data->finish_frame;

				// Calculate the duration of the layer for drawing inside the slider
				start_pixel = 180 * ((gfloat) start_frame / (gfloat) tmp_slide->duration);
				finish_pixel = 180 * ((gfloat) finish_frame / (gfloat) tmp_slide->duration);
				pixel_width = finish_pixel - start_pixel;

				// Create duration image
				layer_pixbuf = NULL;
				layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, start_pixel, pixel_width);

				// Update the timeline with the duration image
				gtk_list_store_set(tmp_slide->layer_store, layer_data->row_iter,
							TIMELINE_DURATION, layer_pixbuf,
							-1);
			}

			// Create the thumbnail for the slide
			tmp_glist = NULL;
			tmp_glist = g_list_append(tmp_glist, tmp_slide);
			tmp_pixbuf = compress_layers(tmp_glist, preview_width, (guint) preview_width * 0.75);
			tmp_slide->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(GDK_PIXBUF(tmp_pixbuf)));

			// Mark the tooltip as uncreated, so we know to create it later on
			tmp_slide->tooltip = NULL;

			// Set the timeline widget for the slide to NULL, so we know to create it later on
			tmp_slide->timeline_widget = NULL;

			// Add the thumbnail to the film strip
			gtk_list_store_append(film_strip_store, &film_strip_iter);
			gtk_list_store_set(film_strip_store, &film_strip_iter, 0, gtk_image_get_pixbuf(tmp_slide->thumbnail), -1);

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


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/01/19 06:37:32  vapour
 * Tweaked some error messages for clarity.
 *
 * Revision 1.5  2008/01/15 16:18:58  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.4  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.3  2007/09/29 04:22:11  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:39:55  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

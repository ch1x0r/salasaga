/*
 * $Id$
 *
 * Salasaga: Function to parse the contents of a project file
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "base64_decode.h"
#include "compress_layers.h"
#include "create_timeline_slider.h"
#include "display_warning.h"
#include "regenerate_timeline_duration_images.h"
#include "slide_free.h"
#include "validate_value.h"


gboolean project_read(gchar *filename)
{
	// Local variables
	xmlChar				*control_bar_data = NULL;	// Temporarily holds incoming data prior to validation
	gint				data_length;				// Number of image data bytes a layer says it stores
	xmlDocPtr			document;					// Holds a pointer to the XML document
	xmlChar				*end_behaviour_data = NULL;
	GError				*error = NULL;				// Pointer to error return structure
	GString				*error_string;				// Used to create error strings
	GtkTreeIter			film_strip_iter;
	xmlChar				*fps_data = NULL;
	guint				guint_val;					// Temporary guint value used for validation
	GdkPixbufLoader		*image_loader;				// Used for loading images embedded in project files
	xmlNodePtr			layer_ptr;					// Temporary pointer
	xmlNodePtr			meta_data_node = NULL;		// Points to the meta-data structure
	GList				*new_slides = NULL;			// Linked list holding the new slide info
	guint				num_slides;					// Holds the number of slides we've processed
	xmlNodePtr			preferences_node = NULL;	// Points to the preferences structure
	xmlChar				*project_name_data = NULL;
	xmlChar				*output_folder_data = NULL;
	xmlChar				*output_width_data = NULL;
	xmlChar				*output_height_data = NULL;
	xmlChar				*project_width_data = NULL;
	xmlChar				*project_height_data = NULL;
	gboolean			return_code;				// Boolean return code
	xmlChar				*save_format_data = NULL;
	guint				slide_counter;				// Counter used for looping
	xmlChar				*slide_length_data = NULL;
	xmlNodePtr			slides_node = NULL;			// Points to the slides structure
	xmlChar				*start_behaviour_data = NULL;
	xmlNodePtr			this_layer;					// Temporary pointer
	xmlNodePtr			this_node;					// Temporary pointer
	xmlNodePtr			this_slide;					// Temporary pointer
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	gboolean			valid_control_bar_behaviour;  // Receives the new control bar display behaviour
	guint				valid_end_behaviour;		// Receives the new end behaviour once validated
	guint				valid_fps;					// Receives the new fps once validated
	guint				valid_output_height;		// Receives the new output height once validated
	GString				*valid_output_folder;		// Receives the new output folder once validated
	guint				valid_output_width;			// Receives the new output width once validated
	GString				*valid_project_name;		// Receives the new project name once validated
	guint				valid_project_height;		// Receives the new project height once validated
	guint				valid_project_width;		// Receives the new project width once validated
	gfloat				valid_save_format;			// Receives the project file format version once validated
	guint				valid_slide_length;			// Receives the new slide length once validated
	guint				valid_start_behaviour;		// Receives the new start behaviour once validated
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function

	xmlChar				*tmp_char;					// Temporary string pointer
	layer_empty			*tmp_empty_ob;				//
	GList				*tmp_glist;					//
	GString				*tmp_gstring;				// Temporary GString
	GString				*tmp_gstring2;				// Temporary GString
	layer_highlight		*tmp_highlight_ob;			// Temporary highlight layer object
	layer_image			*tmp_image_ob;				// Temporary image layer object
	gint				tmp_int;					// Temporary integer
	GtkTreeIter			*tmp_iter;					// Temporary GtkTreeIter
	layer				*tmp_layer;					// Temporary layer
	layer_mouse			*tmp_mouse_ob;				// Temporary mouse layer object
	GdkPixbuf			*tmp_pixbuf;				//
	slide				*tmp_slide;					// Temporary slide
	layer_text			*tmp_text_ob;				// Temporary text layer object


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	tmp_gstring2 = g_string_new(NULL);
	error_string = g_string_new(NULL);
	valid_output_folder = g_string_new(NULL);
	valid_project_name = g_string_new(NULL);

	// Begin reading the file
	document = xmlParseFile(filename);
	if (NULL == document)
	{
		// The project file was unable to be parsed
		display_warning("Error ED43: The selected project file was not loaded successfully.  Please choose a different project file.\n");
		return FALSE;
	}

	this_node = xmlDocGetRootElement(document);
	if (NULL == this_node)
	{
		display_warning("Error ED44: Project file is empty.  Please choose a different project file.");
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
		display_warning("Error ED63: Project meta data missing, aborting load.  Please choose a different project file.");
		return FALSE;
	}

	// Was there a preferences structure in the save file?
	if (NULL == preferences_node)
	{
		display_warning("Error ED45: Project preferences missing, aborting load.  Please choose a different project file.");
		return FALSE;
	}

	// Was there a slides structure in the save file?
	if (NULL == slides_node)
	{
		display_warning("Error ED46: No slides in project, aborting load.  Please choose a different project file.");
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
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "frames_per_second")))
		{
			// Frames per second found.  Extract and store it
			fps_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_behaviour")))
		{
			// End behaviour found.  Extract and store it
			start_behaviour_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "end_behaviour")))
		{
			// End behaviour found.  Extract and store it
			end_behaviour_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "show_control_bar")))
		{
			// Show control bar behaviour found.  Extract and store it
			control_bar_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		this_node = this_node->next;
	}

	// If any of the critically needed preferences are missing, display a warning then abort
	if (NULL == project_name_data)
	{
		display_warning("Error ED47: Project Name missing, aborting load.  Please choose a different project file.");
		return FALSE;
	}
	if (NULL == project_width_data)
	{
		display_warning("Error ED48: Project Width missing, aborting load.  Please choose a different project file.");
		return FALSE;
	}
	if (NULL == project_height_data)
	{
		display_warning("Error ED49: Project Height missing, aborting load.  Please choose a different project file.");
		return FALSE;
	}

	// * All of the required meta-data and preferences are present, so we proceed *

	// Reset the useable input flag
	useable_input = TRUE;

	// Retrieve the save format input
	validated_gfloat = validate_value(PROJECT_VERSION, V_CHAR, save_format_data);
	if (NULL == validated_guint)
	{
		display_warning("Error ED210: There was something wrong with the file format version value in the project file.");
		useable_input = FALSE;
	} else
	{
		valid_save_format = *validated_gfloat;
		g_free(validated_gfloat);
		xmlFree(save_format_data);
	}

	// Validate the project name input
	validated_string = validate_value(PROJECT_NAME, V_CHAR, project_name_data);
	if (NULL == validated_string)
	{
		display_warning("Error ED202: There was something wrong with the project name value in the project file.");
		useable_input = FALSE;
	} else
	{
		valid_project_name = g_string_assign(valid_project_name, validated_string->str);
		g_string_free(validated_string, TRUE);
		xmlFree(project_name_data);
		validated_string = NULL;
	}

	// Retrieve the new output folder input
	validated_string = validate_value(FOLDER_PATH, V_CHAR, output_folder_data);
	if (NULL == validated_string)
	{
		display_warning("Error ED203: There was something wrong with the output folder value in the project file.");
		useable_input = FALSE;
	} else
	{
		valid_output_folder = g_string_assign(valid_output_folder, validated_string->str);
		g_string_free(validated_string, TRUE);
		xmlFree(output_folder_data);
		validated_string = NULL;
	}

	// Retrieve the new output width input
	guint_val = atoi((const char *) output_width_data);
	validated_guint = validate_value(PROJECT_WIDTH, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		display_warning("Error ED204: There was something wrong with the output width value in the project file.");
		useable_input = FALSE;
	} else
	{
		valid_output_width = *validated_guint;
		g_free(validated_guint);
		xmlFree(output_width_data);
	}

	// Retrieve the new output height input
	guint_val = atoi((const char *) output_height_data);
	validated_guint = validate_value(PROJECT_HEIGHT, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		display_warning("Error ED205: There was something wrong with the output height value in the project file.");
		useable_input = FALSE;
	} else
	{
		valid_output_height = *validated_guint;
		g_free(validated_guint);
		xmlFree(output_height_data);
	}

	// Retrieve the new project width input
	guint_val = atoi((const char *) project_width_data);
	validated_guint = validate_value(PROJECT_WIDTH, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		display_warning("Error ED206: There was something wrong with the project width value in the project file.");
		useable_input = FALSE;
	} else
	{
		valid_project_width = *validated_guint;
		g_free(validated_guint);
		xmlFree(project_width_data);
	}

	// Retrieve the new project height input
	guint_val = atoi((const char *) project_height_data);
	validated_guint = validate_value(PROJECT_HEIGHT, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		display_warning("Error ED207: There was something wrong with the project width value in the project file.");
		useable_input = FALSE;
	} else
	{
		valid_project_height = *validated_guint;
		g_free(validated_guint);
		xmlFree(project_height_data);
	}

	// Retrieve the new slide length input
	guint_val = atoi((const char *) slide_length_data);
	validated_guint = validate_value(SLIDE_LENGTH, V_INT_UNSIGNED, &guint_val);
	if (NULL == validated_guint)
	{
		display_warning("Error ED208: There was something wrong with the slide length value in the project file.");
		useable_input = FALSE;
	} else
	{
		valid_slide_length = *validated_guint;
		g_free(validated_guint);
		xmlFree(slide_length_data);
	}

	// Retrieve the new frames per second input
	if (NULL != fps_data)
	{
		guint_val = atoi((const char *) fps_data);
		validated_guint = validate_value(PROJECT_FPS, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			display_warning("Error ED209: There was something wrong with the frames per second value in the project file.");
			useable_input = FALSE;
		} else
		{
			valid_fps = *validated_guint;
			g_free(validated_guint);
			xmlFree(fps_data);
		}
	}

	// Retrieve the new start behaviour input
	if (NULL != start_behaviour_data)
	{
		validated_string = validate_value(START_BEHAVIOUR, V_CHAR, start_behaviour_data);
		if (NULL == validated_string)
		{
			display_warning("Error ED282: There was something wrong with the start behaviour value in the project file.");
			useable_input = FALSE;
		} else
		{
			if (0 == g_ascii_strncasecmp(validated_string->str, "play", 4))
			{
				valid_start_behaviour = START_BEHAVIOUR_PLAY;
			} else
			{
				valid_start_behaviour = START_BEHAVIOUR_PAUSED;
			}
			g_string_free(validated_string, TRUE);
			xmlFree(start_behaviour_data);
			validated_string = NULL;
		}
	} else
	{
		// If no start behaviour value was present in the project file, we default to "Paused"
		valid_start_behaviour = START_BEHAVIOUR_PAUSED;
	}

	// Retrieve the new end behaviour input
	if (NULL != end_behaviour_data)
	{
		validated_string = validate_value(END_BEHAVIOUR, V_CHAR, end_behaviour_data);
		if (NULL == validated_string)
		{
			display_warning("Error ED279: There was something wrong with the end behaviour value in the project file.");
			useable_input = FALSE;
		} else
		{
			if (0 == g_ascii_strncasecmp(validated_string->str, "loop_play", 9))
			{
				valid_end_behaviour = END_BEHAVIOUR_LOOP_PLAY;
			} else
			{
				if (0 == g_ascii_strncasecmp(validated_string->str, "loop_stop", 9))
				{
					valid_end_behaviour = END_BEHAVIOUR_LOOP_STOP;
				} else
				{
					valid_end_behaviour = END_BEHAVIOUR_STOP;
				}
			}
			g_string_free(validated_string, TRUE);
			xmlFree(end_behaviour_data);
			validated_string = NULL;
		}
	} else
	{
		// If no end behaviour value was present in the project file, we default to "Stop"
		valid_end_behaviour = END_BEHAVIOUR_STOP;
	}

	// Retrieve the new control bar display input
	if (NULL != control_bar_data)
	{
		validated_string = validate_value(SHOW_CONTROL_BAR, V_CHAR, control_bar_data);
		if (NULL == validated_string)
		{
			display_warning("Error ED283: There was something wrong with the control bar display value in the project file.");
			useable_input = FALSE;
		} else
		{
			if (0 == g_ascii_strncasecmp(validated_string->str, "true", 4))
			{
				valid_control_bar_behaviour = TRUE;
			} else
			{
				valid_control_bar_behaviour = FALSE;
			}
			g_string_free(validated_string, TRUE);
			xmlFree(control_bar_data);
			validated_string = NULL;
		}
	} else
	{
		// If no control bar display value was present in the project file, we default to "True"
		valid_control_bar_behaviour = TRUE;
	}


	// * Preferences are loaded, so now load the slides *

	this_slide = slides_node->xmlChildrenNode;
	while (NULL != this_slide)
	{
		if ((!xmlStrcmp(this_slide->name, (const xmlChar *) "slide")))
		{
			// * We're in a slide *

			// Create a new slide in memory
			tmp_slide = g_new0(slide, 1);
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
			tmp_slide->scaled_cached_pixbuf = NULL;
			tmp_slide->cached_pixbuf_valid = FALSE;

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
								// Construct a new empty layer
								tmp_empty_ob = g_new0(layer_empty, 1);
								tmp_layer = g_new0(layer, 1);	
								tmp_layer->object_type = TYPE_EMPTY;
								tmp_layer->object_data = (GObject *) tmp_empty_ob;
								tmp_layer->external_link = g_string_new(NULL);
								tmp_layer->visible = TRUE;
								tmp_layer->background = TRUE;
								tmp_layer->external_link_window = g_string_new("_self");
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;

								// Load the empty layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "red")))
									{
										// Get the red value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED211: There was something wrong with a red component colour value in the project file.");
											useable_input = FALSE;
											tmp_empty_ob->bg_color.red = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_empty_ob->bg_color.red = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "green")))
									{
										// Get the green value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED212: There was something wrong with a green component colour value in the project file.");
											useable_input = FALSE;
											tmp_empty_ob->bg_color.green = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_empty_ob->bg_color.green = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "blue")))
									{
										// Get the blue value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED213: There was something wrong with a blue component colour value in the project file.");
											useable_input = FALSE;
											tmp_empty_ob->bg_color.blue = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_empty_ob->bg_color.blue = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED214: There was something wrong with a start frame value in the project file.");
											useable_input = FALSE;
											tmp_layer->start_frame = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->start_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED215: There was something wrong with a finish frame value in the project file.");
											useable_input = FALSE;
											tmp_layer->finish_frame = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->finish_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED216: There was something wrong with a layer visibility value in the project file.");
											useable_input = FALSE;
											tmp_layer->visible = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->visible = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										validated_string = validate_value(LAYER_NAME, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED217: There was something wrong with a layer name value in the project file.");
											useable_input = FALSE;
											tmp_layer->name = g_string_new("Empty");  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->name = validated_string;  // We keep the validated string instead of copying then freeing it
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED218: There was something wrong with an external link value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link = g_string_assign(tmp_layer->external_link, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link_window")))
									{
										// Get the window to open the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED219: There was something wrong with an external link target window value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									this_node = this_node->next;	
								}

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new0(GtkTreeIter, 1);
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
								tmp_image_ob = g_new0(layer_image, 1);
								tmp_layer = g_new0(layer, 1);	
								tmp_layer->object_type = TYPE_GDK_PIXBUF;
								tmp_layer->object_data = (GObject *) tmp_image_ob;
								tmp_layer->external_link = g_string_new(NULL);
								tmp_layer->visible = TRUE;
								tmp_layer->background = FALSE;
								tmp_layer->external_link_window = g_string_new("_self");
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;

								// Load the image layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED220: There was something wrong with an x offset start value in the project file.");
											useable_input = FALSE;
											tmp_layer->x_offset_start = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->x_offset_start = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_start")))
									{
										// Get the starting y offset
										validated_guint = validate_value(SCREENSHOT_Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED221: There was something wrong with a y offset start value in the project file.");
											useable_input = FALSE;
											tmp_layer->y_offset_start = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->y_offset_start = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_finish")))
									{
										// Get the finishing x offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED222: There was something wrong with an x offset finish value in the project file.");
											useable_input = FALSE;
											tmp_layer->x_offset_finish = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->x_offset_finish = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_finish")))
									{
										// Get the finishing y offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED223: There was something wrong with a y offset finish value in the project file.");
											useable_input = FALSE;
											tmp_layer->y_offset_finish = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->y_offset_finish = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "width")))
									{
										// Get the width
										validated_guint = validate_value(LAYER_WIDTH, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED224: There was something wrong with a layer width value in the project file.");
											useable_input = FALSE;
											tmp_image_ob->width = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_image_ob->width = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "height")))
									{
										// Get the height
										validated_guint = validate_value(LAYER_HEIGHT, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED225: There was something wrong with a layer height value in the project file.");
											useable_input = FALSE;
											tmp_image_ob->height = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_image_ob->height = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "background")))
									{
										// Is this the background layer?
										validated_guint = validate_value(LAYER_BACKGROUND, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED226: There was something wrong with a layer background value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->background = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED227: There was something wrong with a start frame value in the project file.");
											tmp_layer->start_frame = 0;  // Fill in the value, just to be safe
											useable_input = FALSE;
										} else
										{
											tmp_layer->start_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED228: There was something wrong with a finish frame value in the project file.");
											tmp_layer->finish_frame = 0;  // Fill in the value, just to be safe
											useable_input = FALSE;
										} else
										{
											tmp_layer->finish_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED229: There was something wrong with a layer visibility value in the project file.");
											useable_input = FALSE;
											tmp_layer->visible = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->visible = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										validated_string = validate_value(LAYER_NAME, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED230: There was something wrong with a layer name value in the project file.");
											useable_input = FALSE;
											tmp_layer->name = g_string_new("Empty");  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->name = validated_string;  // We keep the validated string instead of copying then freeing it
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED231: There was something wrong with an external link value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link = g_string_assign(tmp_layer->external_link, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link_window")))
									{
										// Get the window to open the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED232: There was something wrong with an external link target window value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}

									// If version 1.0 of file format, load the image path, otherwise load the embedded image data
									if (1.0 == valid_save_format)
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "path")))
										{
											// Get the path to the image data
											validated_string = validate_value(FILE_PATH, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_string)
											{
												display_warning("Error ED233: There was something wrong with a file name path value in the project file.");
												tmp_image_ob->image_path = g_string_new(NULL);  // Fill in the value, just to be safe
												useable_input = FALSE;
											} else
											{
												tmp_image_ob->image_path = validated_string;  // We keep the validated string instead of copying then freeing it
												validated_string = NULL;

												// Load the image data
												tmp_image_ob->image_data = gdk_pixbuf_new_from_file(tmp_image_ob->image_path->str, NULL);
											}
										}
									} else
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "data")))
										{
											// Get the image data
											validated_string = validate_value(IMAGE_DATA, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_string)
											{
												display_warning("Error ED234: There was something wrong with image data in the project file.");
												tmp_gstring2 = NULL;
												useable_input = FALSE;
											} else
											{
												tmp_gstring2 = validated_string;
												validated_string = NULL;
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "data_length")))
										{
											// Get the number of bytes the image should hold
											validated_guint = validate_value(IMAGE_DATA_LENGTH, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												display_warning("Error ED235: There was something wrong with an image data length value in the project file.");
												data_length = 0;  // Fill in the value, just to be safe
												useable_input = FALSE;
											} else
											{
												data_length = *validated_guint;
												g_free(validated_guint);
											}
										}

										// Create an empty image path string
										tmp_image_ob->image_path = g_string_new(NULL);
									}

									this_node = this_node->next;	
								}

								// Version 1.0 of the file format doesn't have embedded image data
								if ((1.0 != valid_save_format) && (FALSE != useable_input))
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
								tmp_iter = g_new0(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
										TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
										TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
										TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
										TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
										-1);

								// Add this (now completed) image layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
							}

							// Test if this layer is a highlight layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "highlight"))
							{
								// Construct a new highlight layer
								tmp_highlight_ob = g_new0(layer_highlight, 1);
								tmp_layer = g_new0(layer, 1);	
								tmp_layer->object_type = TYPE_HIGHLIGHT;
								tmp_layer->object_data = (GObject *) tmp_highlight_ob;
								tmp_layer->external_link = g_string_new(NULL);
								tmp_layer->visible = TRUE;
								tmp_layer->background = FALSE;
								tmp_layer->external_link_window = g_string_new("_self");
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;

								// Load the highlight layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED236: There was something wrong with an x offset start value in the project file.");
											useable_input = FALSE;
											tmp_layer->x_offset_start = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->x_offset_start = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_start")))
									{
										// Get the starting y offset
										validated_guint = validate_value(SCREENSHOT_Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED237: There was something wrong with a y offset start value in the project file.");
											useable_input = FALSE;
											tmp_layer->y_offset_start = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->y_offset_start = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_finish")))
									{
										// Get the finishing x offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED238: There was something wrong with an x offset finish value in the project file.");
											useable_input = FALSE;
											tmp_layer->x_offset_finish = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->x_offset_finish = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_finish")))
									{
										// Get the finishing y offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED239: There was something wrong with a y offset finish value in the project file.");
											useable_input = FALSE;
											tmp_layer->y_offset_finish = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->y_offset_finish = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "width")))
									{
										// Get the width
										validated_guint = validate_value(HIGHLIGHT_WIDTH, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED240: There was something wrong with a layer width value in the project file.");
											useable_input = FALSE;
											tmp_highlight_ob->width = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_highlight_ob->width = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "height")))
									{
										// Get the height
										validated_guint = validate_value(HIGHLIGHT_HEIGHT, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED241: There was something wrong with a layer height value in the project file.");
											useable_input = FALSE;
											tmp_highlight_ob->height = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_highlight_ob->height = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED242: There was something wrong with a start frame value in the project file.");
											tmp_layer->start_frame = 0;  // Fill in the value, just to be safe
											useable_input = FALSE;
										} else
										{
											tmp_layer->start_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED243: There was something wrong with a finish frame value in the project file.");
											tmp_layer->finish_frame = 0;  // Fill in the value, just to be safe
											useable_input = FALSE;
										} else
										{
											tmp_layer->finish_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED244: There was something wrong with a layer visibility value in the project file.");
											useable_input = FALSE;
											tmp_layer->visible = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->visible = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										validated_string = validate_value(LAYER_NAME, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED245: There was something wrong with a layer name value in the project file.");
											useable_input = FALSE;
											tmp_layer->name = g_string_new("Empty");  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->name = validated_string;  // We keep the validated string instead of copying then freeing it
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED246: There was something wrong with an external link value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link = g_string_assign(tmp_layer->external_link, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link_window")))
									{
										// Get the window to open the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED247: There was something wrong with an external link target window value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									this_node = this_node->next;	
								}

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new0(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
										TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
										TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
										TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
										TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
										-1);

								// Add this (now completed) highlight layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
							}

							// Test if this layer is a mouse pointer layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "mouse"))
							{
								// Construct a new mouse pointer layer
								tmp_mouse_ob = g_new0(layer_mouse, 1);
								tmp_layer = g_new0(layer, 1);	
								tmp_layer->object_type = TYPE_MOUSE_CURSOR;
								tmp_layer->object_data = (GObject *) tmp_mouse_ob;
								tmp_layer->external_link = g_string_new(NULL);
								tmp_layer->visible = TRUE;
								tmp_layer->background = FALSE;
								tmp_layer->external_link_window = g_string_new("_self");
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;

								// Load the highlight layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED248: There was something wrong with an x offset start value in the project file.");
											useable_input = FALSE;
											tmp_layer->x_offset_start = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->x_offset_start = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_start")))
									{
										// Get the starting y offset
										validated_guint = validate_value(SCREENSHOT_Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED249: There was something wrong with a y offset start value in the project file.");
											useable_input = FALSE;
											tmp_layer->y_offset_start = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->y_offset_start = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_finish")))
									{
										// Get the finishing x offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED250: There was something wrong with an x offset finish value in the project file.");
											useable_input = FALSE;
											tmp_layer->x_offset_finish = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->x_offset_finish = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_finish")))
									{
										// Get the finishing y offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED251: There was something wrong with a y offset finish value in the project file.");
											useable_input = FALSE;
											tmp_layer->y_offset_finish = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->y_offset_finish = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "width")))
									{
										// Get the width
										validated_guint = validate_value(LAYER_WIDTH, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED252: There was something wrong with a layer width value in the project file.");
											useable_input = FALSE;
											tmp_mouse_ob->width = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_mouse_ob->width = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "height")))
									{
										// Get the height
										validated_guint = validate_value(LAYER_HEIGHT, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED253: There was something wrong with a layer height value in the project file.");
											useable_input = FALSE;
											tmp_mouse_ob->height = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_mouse_ob->height = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "click")))
									{
										// Get the mouse click type
										validated_string = validate_value(MOUSE_CLICK, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED254: There was something wrong with a mouse click value in the project file.");
											useable_input = FALSE;
											tmp_mouse_ob->click = MOUSE_NONE;  // Fill in the value, just to be safe
										} else
										{
											tmp_int = g_ascii_strncasecmp(validated_string->str, "left_one", 8);
											if (0 == tmp_int)
											{
												tmp_mouse_ob->click = MOUSE_LEFT_ONE;
											} else
											{
												tmp_mouse_ob->click = MOUSE_NONE;
											}
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED255: There was something wrong with a start frame value in the project file.");
											tmp_layer->start_frame = 0;  // Fill in the value, just to be safe
											useable_input = FALSE;
										} else
										{
											tmp_layer->start_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED256: There was something wrong with a finish frame value in the project file.");
											tmp_layer->finish_frame = 0;  // Fill in the value, just to be safe
											useable_input = FALSE;
										} else
										{
											tmp_layer->finish_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED257: There was something wrong with a layer visibility value in the project file.");
											useable_input = FALSE;
											tmp_layer->visible = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->visible = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										validated_string = validate_value(LAYER_NAME, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED258: There was something wrong with a layer name value in the project file.");
											useable_input = FALSE;
											tmp_layer->name = g_string_new("Empty");  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->name = validated_string;  // We keep the validated string instead of copying then freeing it
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED259: There was something wrong with an external link value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link = g_string_assign(tmp_layer->external_link, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link_window")))
									{
										// Get the window to open the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED260: There was something wrong with an external link target window value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									this_node = this_node->next;	
								}

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new0(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
										TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
										TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
										TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
										TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
										-1);

								// Add this (now completed) mouse pointer layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
							}

							// Test if this layer is a text layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "text"))
							{
								// Construct a new text layer
								tmp_text_ob = g_new0(layer_text, 1);
								tmp_layer = g_new0(layer, 1);	
								tmp_layer->object_type = TYPE_TEXT;
								tmp_layer->object_data = (GObject *) tmp_text_ob;
								tmp_layer->external_link = g_string_new(NULL);
								tmp_layer->visible = TRUE;
								tmp_layer->background = FALSE;
								tmp_layer->external_link_window = g_string_new("_self");
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;

								// Load the text layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED261: There was something wrong with an x offset start value in the project file.");
											useable_input = FALSE;
											tmp_layer->x_offset_start = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->x_offset_start = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_start")))
									{
										// Get the starting y offset
										validated_guint = validate_value(SCREENSHOT_Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED262: There was something wrong with a y offset start value in the project file.");
											useable_input = FALSE;
											tmp_layer->y_offset_start = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->y_offset_start = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_finish")))
									{
										// Get the finishing x offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED263: There was something wrong with an x offset finish value in the project file.");
											useable_input = FALSE;
											tmp_layer->x_offset_finish = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->x_offset_finish = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "y_offset_finish")))
									{
										// Get the finishing y offset
										validated_guint = validate_value(SCREENSHOT_X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED264: There was something wrong with a y offset finish value in the project file.");
											useable_input = FALSE;
											tmp_layer->y_offset_finish = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->y_offset_finish = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "red")))
									{
										// Get the red value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED265: There was something wrong with a red component colour value in the project file.");
											useable_input = FALSE;
											tmp_text_ob->text_color.red = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->text_color.red = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "green")))
									{
										// Get the green value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED266: There was something wrong with a green component colour value in the project file.");
											useable_input = FALSE;
											tmp_text_ob->text_color.green = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->text_color.green = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "blue")))
									{
										// Get the blue value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED267: There was something wrong with a blue component colour value in the project file.");
											useable_input = FALSE;
											tmp_text_ob->text_color.blue = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->text_color.blue = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "font_size")))
									{
										// Get the font size
										validated_gfloat = validate_value(FONT_SIZE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											display_warning("Error ED268: There was something wrong with a font size value in the project file.");
											useable_input = FALSE;
											tmp_text_ob->font_size = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->font_size = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "text_value")))
									{
										// Get the text
										// fixme5: Unsure if we should validate this or not... it's supposed to be free-form. (?)
										tmp_text_ob->text_buffer = gtk_text_buffer_new(NULL);
										gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), (const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1), -1);
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
									{
										// Get the start frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED269: There was something wrong with a start frame value in the project file.");
											useable_input = FALSE;
											tmp_layer->start_frame = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->start_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
									{
										// Get the finish frame
										validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED270: There was something wrong with a finish frame value in the project file.");
											useable_input = FALSE;
											tmp_layer->finish_frame = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->finish_frame = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											display_warning("Error ED271: There was something wrong with a layer visibility value in the project file.");
											useable_input = FALSE;
											tmp_layer->visible = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->visible = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "name")))
									{
										// Get the name of the layer
										validated_string = validate_value(LAYER_NAME, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED272: There was something wrong with a layer name value in the project file.");
											useable_input = FALSE;
											tmp_layer->name = g_string_new("Empty");  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->name = validated_string;  // We keep the validated string instead of copying then freeing it
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link")))
									{
										// Get the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED273: There was something wrong with an external link value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link = g_string_assign(tmp_layer->external_link, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "external_link_window")))
									{
										// Get the window to open the URL associated with the layer
										validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											display_warning("Error ED274: There was something wrong with an external link target window value in the project file.");
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									this_node = this_node->next;
								}

								// If the finish_frame is longer than the slide duration, we increase the slide duration
								if (tmp_layer->finish_frame > tmp_slide->duration) tmp_slide->duration = tmp_layer->finish_frame;

								// Add the layer to the slide list store
								tmp_iter = g_new0(GtkTreeIter, 1);
								tmp_layer->row_iter = tmp_iter;
								gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
								gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
										TIMELINE_NAME, tmp_layer->name->str,
										TIMELINE_VISIBILITY, TRUE,
										TIMELINE_DURATION, NULL,
										TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
										TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
										TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
										TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
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
				// A name for the slide is in the project file, so we use it if it validates
				validated_string = validate_value(SLIDE_NAME, V_CHAR, tmp_char);
				if (NULL == validated_string)
				{
					display_warning("Error ED275: There was something wrong with a slide name value in the project file.");
					useable_input = FALSE;
				} else
				{
					tmp_slide->name = validated_string;
					validated_string = NULL;
					xmlFree(tmp_char);
				}
			}

			// Read the slide duration from the save file
			tmp_slide->duration = default_slide_length;
			tmp_char = xmlGetProp(this_slide, (const xmlChar *) "duration");
			if (NULL != tmp_char)
			{
				// A duration for the slide is in the project file, so we use it if it validates
				validated_guint = validate_value(SLIDE_LENGTH, V_CHAR, tmp_char);
				if (NULL == validated_guint)
				{
					display_warning("Error ED276: There was something wrong with a slide duration value in the project file.");
					useable_input = FALSE;
				} else
				{
					tmp_slide->duration = *validated_guint;
					g_free(validated_guint);
					xmlFree(tmp_char);
				}
			}

			// To get here, we must have finished loading the present slide, so we add it to the new project
			new_slides = g_list_append(new_slides, tmp_slide);

		}  // End of "We're in a slide" loop

		this_slide = this_slide->next;
	}

	// Abort if any of the input isn't valid
	if (TRUE != useable_input)
	{
		// At least one of the values in the project file is invalid, so abort the load
		display_warning("Aborting load of project file.");  // Individual error messages will already have been displayed

		// fixme4: We should probably free the memory allocated for any slides thus far too
		
		return FALSE;
	}

	// ** We only get here if all the input is considered valid **

	// If there's a project presently loaded in memory, we unload it
	if (NULL != slides)
	{
		// Free the resources presently allocated to slides
		g_list_foreach(slides, slide_free, NULL);
		g_list_free(slides);

		// Re-initialise pointers
		slides = NULL;
		current_slide = NULL;
	}

	// If there's an existing film strip, we unload it
	gtk_list_store_clear(GTK_LIST_STORE(film_strip_store));

	// Load project name
	if (NULL == project_name)
		project_name = g_string_new(NULL);
	g_string_assign(project_name, valid_project_name->str);
	g_string_free(valid_project_name, TRUE);

	// Load output folder
	if (NULL == output_folder)
		output_folder = g_string_new(NULL);
	if (TRUE == g_file_test(valid_output_folder->str, G_FILE_TEST_IS_DIR))  // Test if the directory given in the project file exists
	{
		// Yes, it does
		g_string_assign(output_folder, valid_output_folder->str);
	} else
	{
		// No, it doesn't, so use the default output folder
		g_string_assign(output_folder, default_output_folder->str);
	}
	g_string_free(valid_output_folder, TRUE);

	// Load output width
	output_width = valid_output_width;

	// Load output height
	output_height = valid_output_height;

	// Load project_width
	project_width = valid_project_width;

	// Load project height
	project_height = valid_project_height;

	// Load slide length
	slide_length = valid_slide_length;

	// Load frames per second
	if (0 != valid_fps)
	{
		frames_per_second = valid_fps;
	}

	// Load Start behaviour
	start_behaviour = valid_start_behaviour;

	// Load End behaviour
	end_behaviour = valid_end_behaviour;

	// Control bar display
	show_control_bar = valid_control_bar_behaviour;

	// Make the new slides active, and update them to fill in their remaining pieces
	slides = g_list_first(new_slides);
	num_slides = g_list_length(slides);
	for (slide_counter = 0; slide_counter < num_slides; slide_counter++)
	{
		// Select the desired slide
		slides = g_list_first(slides);
		tmp_slide = g_list_nth_data(slides, slide_counter);

		// Regenerate the timeline duration images for all layers in the slide
		regenerate_timeline_duration_images(tmp_slide);

		// Set the timeline widget for the slide to NULL, so we know to create it later on
		tmp_slide->timeline_widget = NULL;

		// Create the thumbnail for the slide
		tmp_glist = NULL;
		tmp_glist = g_list_append(tmp_glist, tmp_slide);
		tmp_pixbuf = compress_layers(tmp_glist, preview_width, (guint) preview_width * 0.75);
		tmp_slide->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(GDK_PIXBUF(tmp_pixbuf)));

		// Add the thumbnail to the film strip
		gtk_list_store_append(film_strip_store, &film_strip_iter);
		gtk_list_store_set(film_strip_store, &film_strip_iter, 0, gtk_image_get_pixbuf(tmp_slide->thumbnail), -1);
	}

	// We're finished with this XML document, so release its memory
	xmlFreeDoc(document);

	// Free the memory allocated in this function
	g_string_free(tmp_gstring2, TRUE);

	return TRUE;
}

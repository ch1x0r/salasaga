/*
 * $Id$
 *
 * Salasaga: Function to parse the contents of a project file
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard includes
#include <stdlib.h>

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
#include "validate_value.h"
#include "cairo/create_cairo_pixbuf_pattern.h"
#include "conversion/base64_decode.h"
#include "dialog/display_warning.h"
#include "slide/slide_free.h"
#include "layer/compress_layers.h"


gboolean project_read(gchar *filename)
{
	// Local variables
	xmlChar				*control_bar_data = NULL;	// Temporarily holds incoming data prior to validation
	gint				data_length;				// Number of image data bytes a layer says it stores
	xmlDocPtr			document;					// Holds a pointer to the XML document
	xmlChar				*end_behaviour_data = NULL;
	gfloat				end_time;					// Used to calculate the end time in seconds of a layer
	GError				*error = NULL;				// Pointer to error return structure
	GString				*error_string;				// Used to create error strings
	GtkTreeIter			film_strip_iter;
	xmlChar				*fps_data = NULL;
	guint				guint_val;					// Temporary guint value used for validation
	GdkPixbufLoader		*image_loader;				// Used for loading images embedded in project files
	xmlChar				*info_display_data = NULL;
	xmlChar				*info_link_data = NULL;
	xmlChar				*info_link_target_data = NULL;
	xmlChar				*info_text_data = NULL;
	guint				layer_counter;				// Counter used in loops
	xmlNodePtr			layer_ptr;					// Temporary pointer
	GString				*message;					// Used to construct message strings
	xmlNodePtr			meta_data_node = NULL;		// Points to the meta-data structure
	GList				*new_slides = NULL;			// Linked list holding the new slide info
	guint				num_layers;					// Holds the number of layers in a slide
	guint				num_slides;					// Holds the number of slides we've processed
	gfloat				potential_duration;			// Receives a slide duration from the project file that we may or may not use
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
	xmlChar				*slide_duration_data = NULL;
	xmlChar				*slide_length_data = NULL;
	xmlNodePtr			slides_node = NULL;			// Points to the slides structure
	xmlChar				*start_behaviour_data = NULL;
	GtkTextIter			text_end;					// End position of text buffer
	GtkTextIter			text_start;					// Start position of text buffer
	xmlNodePtr			this_layer;					// Temporary pointer
	layer				*this_layer_ptr;			// Pointer into a layer structure
	xmlNodePtr			this_node;					// Temporary pointer
	xmlNodePtr			this_slide;					// Temporary pointer
	gboolean			useable_input;				// Used as a flag to indicate if all validation was successful
	gboolean			valid_control_bar_behaviour = TRUE;// Receives the new control bar display behaviour
	guint				valid_end_behaviour = 0;	// Receives the new end behaviour once validated
	guint				valid_fps = 0;				// Receives the new fps once validated
	gboolean			valid_info_display = TRUE;	// Receives the new information button display state once validated
	GString				*valid_info_link;			//
	GString				*valid_info_link_target;	//
	GtkTextBuffer		*valid_info_text;			// Text to be shown in the information button in swf output
	guint				valid_output_height = 0;	// Receives the new output height once validated
	GString				*valid_output_folder;		// Receives the new output folder once validated
	guint				valid_output_width = 0;		// Receives the new output width once validated
	GString				*valid_project_name;		// Receives the new project name once validated
	guint				valid_project_height = 0;	// Receives the new project height once validated
	guint				valid_project_width = 0;	// Receives the new project width once validated
	gfloat				valid_save_format;			// Receives the project file format version once validated
	guint				valid_start_behaviour = 0;	// Receives the new start behaviour once validated
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
	layer				*tmp_layer;					// Temporary layer
	layer_mouse			*tmp_mouse_ob;				// Temporary mouse layer object
	GdkPixmap			*tmp_pixmap;				//
	slide				*tmp_slide;					// Temporary slide
	layer_text			*tmp_text_ob;				// Temporary text layer object


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	tmp_gstring2 = g_string_new(NULL);
	error_string = g_string_new(NULL);
	message = g_string_new(NULL);
	valid_output_folder = g_string_new(NULL);
	valid_project_name = g_string_new(NULL);

	// Set sensible defaults for the swf information button
	valid_info_link = g_string_new(_("http://www.salasaga.org"));
	valid_info_link_target = g_string_new(_("_blank"));
	valid_info_text = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(valid_info_text), _("Created using Salasaga"), -1);
	valid_info_display = TRUE;

	// Begin reading the file
	document = xmlParseFile(filename);
	if (NULL == document)
	{
		// The project file was unable to be parsed
		g_string_printf(message, "%s ED43: %s", _("Error"), _("The selected project file was not loaded successfully.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}

	this_node = xmlDocGetRootElement(document);
	if (NULL == this_node)
	{
		g_string_printf(message, "%s ED44: %s", _("Error"), _("Project file is empty.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
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
		g_string_printf(message, "%s ED63: %s", _("Error"), _("Project meta data missing, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}

	// Was there a preferences structure in the save file?
	if (NULL == preferences_node)
	{
		g_string_printf(message, "%s ED45: %s", _("Error"), _("Project preferences missing, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}

	// Was there a slides structure in the save file?
	if (NULL == slides_node)
	{
		g_string_printf(message, "%s ED46: %s", _("Error"), _("No slides in project, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
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
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "slide_duration")))
		{
			// Slide Duration found.  Extract and store it
			slide_duration_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
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
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "info_display")))
		{
			// Show information button found.  Extract and store it
			info_display_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "info_link")))
		{
			// Show information button link found.  Extract and store it
			info_link_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "info_link_target")))
		{
			// Show information button link target found.  Extract and store it
			info_link_target_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "info_text")))
		{
			// Show information button text found.  Extract and store it
			info_text_data = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
		}

		this_node = this_node->next;
	}

	// If any of the critically needed preferences are missing, display a warning then abort
	if (NULL == project_name_data)
	{
		g_string_printf(message, "%s ED47: %s", _("Error"), _("Project Name missing, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}
	if (NULL == project_width_data)
	{
		g_string_printf(message, "%s ED48: %s", _("Error"), _("Project Width missing, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}
	if (NULL == project_height_data)
	{
		g_string_printf(message, "%s ED49: %s", _("Error"), _("Project Height missing, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}

	// Retrieve the save format input
	validated_gfloat = validate_value(PROJECT_VERSION, V_CHAR, save_format_data);
	if (NULL == validated_gfloat)
	{
		g_string_printf(message, "%s ED210: %s", _("Error"), _("The file format value in the project file is not recognised.  This project file can't be used."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	} else
	{
		valid_save_format = *validated_gfloat;
		g_free(validated_gfloat);
		xmlFree(save_format_data);
	}

	// Reset the useable input flag
	useable_input = TRUE;

	// * All of the required meta-data and preferences are present, so we proceed *

	// Validate the project name input
	validated_string = validate_value(PROJECT_NAME, V_CHAR, project_name_data);
	if (NULL == validated_string)
	{
		g_string_printf(message, "%s ED202: %s", _("Error"), _("There was something wrong with the project name value in the project file."));
		display_warning(message->str);
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
		g_string_printf(message, "%s ED203: %s", _("Error"), _("There was something wrong with the output folder value in the project file."));
		display_warning(message->str);
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
		g_string_printf(message, "%s ED204: %s", _("Error"), _("There was something wrong with the output width value in the project file."));
		display_warning(message->str);
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
		g_string_printf(message, "%s ED205: %s", _("Error"), _("There was something wrong with the output height value in the project file."));
		display_warning(message->str);
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
		g_string_printf(message, "%s ED206: %s", _("Error"), _("There was something wrong with the project width value in the project file."));
		display_warning(message->str);
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
		g_string_printf(message, "%s ED207: %s", _("Error"), _("There was something wrong with the project width value in the project file."));
		display_warning(message->str);
		useable_input = FALSE;
	} else
	{
		valid_project_height = *validated_guint;
		g_free(validated_guint);
		xmlFree(project_height_data);
	}

	// Retrieve the new frames per second input
	if (NULL != fps_data)
	{
		guint_val = atoi((const char *) fps_data);
		validated_guint = validate_value(PROJECT_FPS, V_INT_UNSIGNED, &guint_val);
		if (NULL == validated_guint)
		{
			g_string_printf(message, "%s ED209: %s", _("Error"), _("There was something wrong with the frames per second value in the project file."));
			display_warning(message->str);
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
			g_string_printf(message, "%s ED282: %s", _("Error"), _("There was something wrong with the start behaviour value in the project file."));
			display_warning(message->str);
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
			g_string_printf(message, "%s ED279: %s", _("Error"), _("There was something wrong with the end behaviour value in the project file."));
			display_warning(message->str);
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
			g_string_printf(message, "%s ED283: %s", _("Error"), _("There was something wrong with the control bar display value in the project file."));
			display_warning(message->str);
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

	// Retrieve the new information button display state
	if (NULL != info_display_data)
	{
		validated_string = validate_value(SHOW_INFO_BUTTON, V_CHAR, info_display_data);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED409: %s", _("Error"), _("There was something wrong with the information button display value in the project file."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			if (0 == g_ascii_strncasecmp(validated_string->str, "true", 4))
			{
				valid_info_display = TRUE;
			} else
			{
				valid_info_display = FALSE;
			}
			g_string_free(validated_string, TRUE);
			xmlFree(info_display_data);
			validated_string = NULL;
		}
	}

	// Retrieve the new information button link input
	if (NULL != info_link_data)
	{
		validated_string = validate_value(EXTERNAL_LINK, V_CHAR, info_link_data);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED410: %s", _("Error"), _("There was something wrong with the information button link value in the project file."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_info_link = g_string_assign(valid_info_link, validated_string->str);
			g_string_free(validated_string, TRUE);
			xmlFree(info_link_data);
			validated_string = NULL;
		}
	}

	// Retrieve the new information button link target input
	if (NULL != info_link_target_data)
	{
		validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, info_link_target_data);
		if (NULL == validated_string)
		{
			g_string_printf(message, "%s ED411: %s", _("Error"), _("There was something wrong with the information button link value in the project file."));
			display_warning(message->str);
			useable_input = FALSE;
		} else
		{
			valid_info_link_target = g_string_assign(valid_info_link_target, validated_string->str);
			g_string_free(validated_string, TRUE);
			xmlFree(info_link_target_data);
			validated_string = NULL;
		}
	}

	// Retrieve the new information button text input
	if (NULL != info_text_data)
	{
		gtk_text_buffer_set_text(GTK_TEXT_BUFFER(valid_info_text), (const gchar *) info_text_data, -1);
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
			tmp_slide->duration = 0;  // 0 on purpose instead of default_slide_duration, so we can calculate things correctly!
			tmp_slide->scaled_cached_pixmap = NULL;
			tmp_slide->cached_pixmap_valid = FALSE;
			tmp_slide->num_layers = 0;

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
								tmp_layer->start_time = 0.0;
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->duration = default_slide_duration;
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
											g_string_printf(message, "%s ED211: %s", _("Error"), _("There was something wrong with a red component colour value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED212: %s", _("Error"), _("There was something wrong with a green component colour value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED213: %s", _("Error"), _("There was something wrong with a blue component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_empty_ob->bg_color.blue = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_empty_ob->bg_color.blue = *validated_guint;
											g_free(validated_guint);
										}
									}
									// If the file format is less the version 4.0, then it has layers with frame based input rather than time based input
									if (4.0 > valid_save_format)
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
										{
											// Get the start frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED214: %s", _("Error"), _("There was something wrong with a start frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_guint / valid_fps;
												g_free(validated_guint);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
										{
											// Get the finish frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED215: %s", _("Error"), _("There was something wrong with a finish frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = (*validated_guint / valid_fps) - tmp_layer->start_time;
												g_free(validated_guint);
											}
										}
									} else
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_time")))
										{
											// Get the start time
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED336: %s", _("Error"), _("There was something wrong with a layer start time value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "duration")))
										{
											// Get the finish frame
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED337: %s", _("Error"), _("There was something wrong with a layer duration value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED216: %s", _("Error"), _("There was something wrong with a layer visibility value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED217: %s", _("Error"), _("There was something wrong with a layer name value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED218: %s", _("Error"), _("There was something wrong with an external link value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED219: %s", _("Error"), _("There was something wrong with an external link target window value in the project file."));
											display_warning(message->str);
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

								// Work out the end time in seconds of the presently selected layer
								end_time = tmp_layer->start_time + tmp_layer->duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_in_type)
									end_time += tmp_layer->transition_in_duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_out_type)
									end_time += tmp_layer->transition_out_duration;

								// If the new layer end time is longer than the slide duration, then extend the slide duration
								if (end_time > tmp_slide->duration)
								{
									tmp_slide->duration = end_time;
								}

								// Add this (now completed) empty layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
								tmp_slide->num_layers++;
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
								tmp_layer->start_time = 0.0;
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->duration = default_layer_duration;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;

								// Load the image layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										validated_guint = validate_value(X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED220: %s", _("Error"), _("There was something wrong with an image x offset start value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED221: %s", _("Error"), _("There was something wrong with an image y offset start value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED222: %s", _("Error"), _("There was something wrong with an image x offset finish value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED223: %s", _("Error"), _("There was something wrong with an image y offset finish value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED224: %s", _("Error"), _("There was something wrong with an image layer width value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED225: %s", _("Error"), _("There was something wrong with an image layer height value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED226: %s", _("Error"), _("There was something wrong with an image layer background value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_layer->background = *validated_guint;
											g_free(validated_guint);
										}
									}
									// If the file format is less the version 4.0, then it has layers with frame based input rather than time based input
									if (4.0 > valid_save_format)
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
										{
											// Get the start frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED227: %s", _("Error"), _("There was something wrong with an image start frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_guint / valid_fps;
												g_free(validated_guint);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
										{
											// Get the finish frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED228: %s", _("Error"), _("There was something wrong with an image finish frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = (*validated_guint / valid_fps) - tmp_layer->start_time;
												g_free(validated_guint);
											}
										}
									} else
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_time")))
										{
											// Get the start time
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED340: %s", _("Error"), _("There was something wrong with an image layer start time value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "duration")))
										{
											// Get the finish frame
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED341: %s", _("Error"), _("There was something wrong with an image layer duration value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED229: %s", _("Error"), _("There was something wrong with an image layer visibility value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED230: %s", _("Error"), _("There was something wrong with an image layer name value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED231: %s", _("Error"), _("There was something wrong with an image layer external link value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED232: %s", _("Error"), _("There was something wrong with an image layer external link target window value in the project file."));
											display_warning(message->str);
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
												g_string_printf(message, "%s ED233: %s", _("Error"), _("There was something wrong with a file name path value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
											} else
											{
												// Load the image data
												tmp_image_ob->image_data = gdk_pixbuf_new_from_file(validated_string->str, NULL);
												validated_string = NULL;
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
												g_string_printf(message, "%s ED234: %s", _("Error"), _("There was something wrong with image data in the project file."));
												display_warning(message->str);
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
												g_string_printf(message, "%s ED235: %s", _("Error"), _("There was something wrong with an image data length value in the project file."));
												display_warning(message->str);
												data_length = 0;  // Fill in the value, just to be safe
												useable_input = FALSE;
											} else
											{
												data_length = *validated_guint;
												g_free(validated_guint);
											}
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_type")))
									{
										// Get the type of transition in
										validated_string = validate_value(TRANSITION_TYPE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											g_string_printf(message, "%s ED313: %s", _("Error"), _("There was something wrong with an image transition in type value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "fade", 4))
											{
												tmp_layer->transition_in_type = TRANS_LAYER_FADE;
											} else
											{
												tmp_layer->transition_in_type = TRANS_LAYER_NONE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_duration")))
									{
										// Get the transition in duration
										validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED314: %s", _("Error"), _("There was something wrong with an image transition in duration value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_layer->transition_in_duration = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->transition_in_duration = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_out_type")))
									{
										// Get the type of transition out
										validated_string = validate_value(TRANSITION_TYPE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											g_string_printf(message, "%s ED315: %s", _("Error"), _("There was something wrong with an image transition out type value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "fade", 4))
											{
												tmp_layer->transition_out_type = TRANS_LAYER_FADE;
											} else
											{
												tmp_layer->transition_out_type = TRANS_LAYER_NONE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_out_duration")))
									{
										// Get the transition out duration
										validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED316: %s", _("Error"), _("There was something wrong with an image transition out duration value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_layer->transition_out_duration = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->transition_out_duration = *validated_gfloat;
											g_free(validated_gfloat);
										}
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
										g_string_printf(error_string, "%s ED66: %s: '%s'", _("Error"), _("Image data loading failed"), error->message);
										display_warning(error_string->str);
										g_string_free(error_string, TRUE);
									}
									return_code = gdk_pixbuf_loader_close(image_loader, &error);
									if (TRUE != return_code)
									{
										g_string_printf(error_string, "%s ED67: %s: '%s'", _("Error"), _("Image data loading failed"), error->message);
										display_warning(error_string->str);
										g_string_free(error_string, TRUE);
									}
									tmp_image_ob->image_data = gdk_pixbuf_loader_get_pixbuf(image_loader);
									if (NULL == tmp_image_ob->image_data)
									{
										g_string_printf(message, "%s ED65: %s", _("Error"), _("Error when loading image data"));
										display_warning(message->str);
									}
								}

								// Create a cairo pattern from the image data
								tmp_image_ob->cairo_pattern = create_cairo_pixbuf_pattern(tmp_image_ob->image_data);
								if (NULL == tmp_image_ob->cairo_pattern)
								{
									// Something went wrong when creating the image pattern
									g_string_printf(message, "%s ED373: %s", _("Error"), _("Couldn't create an image pattern"));
									display_warning(message->str);
								}

								// Set the modified flag for this image to false
								tmp_image_ob->modified = FALSE;

								// Work out the end time in seconds of the presently selected layer
								end_time = tmp_layer->start_time + tmp_layer->duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_in_type)
									end_time += tmp_layer->transition_in_duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_out_type)
									end_time += tmp_layer->transition_out_duration;

								// If the new layer end time is longer than the slide duration, then extend the slide duration
								if (end_time > tmp_slide->duration)
								{
									tmp_slide->duration = end_time;
								}

								// Add this (now completed) image layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
								tmp_slide->num_layers++;
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
								tmp_layer->external_link_window = g_string_new(_("_self"));
								tmp_layer->start_time = 0.0;
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->duration = default_layer_duration;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;
								tmp_highlight_ob->border_colour.red = 0;
								tmp_highlight_ob->border_colour.green = 65535;
								tmp_highlight_ob->border_colour.blue = 0;
								tmp_highlight_ob->border_width = 2.0;
								tmp_highlight_ob->fill_colour.red = 0;
								tmp_highlight_ob->fill_colour.green = 40000;
								tmp_highlight_ob->fill_colour.blue = 0;
								tmp_highlight_ob->opacity = 50;  // Sensible default

								// Load the highlight layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "border_colour_red")))
									{
										// Get the border colour red value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED397: %s", _("Error"), _("There was something wrong with a red component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_highlight_ob->border_colour.red = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "border_colour_green")))
									{
										// Get the border colour green value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED398: %s", _("Error"), _("There was something wrong with a green component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_highlight_ob->border_colour.green = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "border_colour_blue")))
									{
										// Get the border colour blue value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED399: %s", _("Error"), _("There was something wrong with a blue component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_highlight_ob->border_colour.blue = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "border_width")))
									{
										// Get the border width
										validated_gfloat = validate_value(LINE_WIDTH, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED400: %s", _("Error"), _("There was something wrong with a highlight border width value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_highlight_ob->border_width = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "fill_colour_red")))
									{
										// Get the fill colour red value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED401: %s", _("Error"), _("There was something wrong with a red component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_highlight_ob->fill_colour.red = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "fill_colour_green")))
									{
										// Get the fill colour green value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED402: %s", _("Error"), _("There was something wrong with a green component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_highlight_ob->fill_colour.green = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "fill_colour_blue")))
									{
										// Get the fill colour blue value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED403: %s", _("Error"), _("There was something wrong with a blue component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_highlight_ob->fill_colour.blue = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "opacity")))
									{
										// Get the highlight opacity value
										validated_gfloat = validate_value(OPACITY, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED404: %s", _("Error"), _("There was something wrong with a highlight opacity value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_highlight_ob->opacity = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										validated_guint = validate_value(X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED236: %s", _("Error"), _("There was something wrong with an x offset start value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED237: %s", _("Error"), _("There was something wrong with a y offset start value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED238: %s", _("Error"), _("There was something wrong with an x offset finish value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED239: %s", _("Error"), _("There was something wrong with a y offset finish value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED240: %s", _("Error"), _("There was something wrong with a layer width value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED241: %s", _("Error"), _("There was something wrong with a layer height value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_highlight_ob->height = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_highlight_ob->height = *validated_guint;
											g_free(validated_guint);
										}
									}
									// If the file format is less the version 4.0, then it has layers with frame based input rather than time based input
									if (4.0 > valid_save_format)
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
										{
											// Get the start frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED242: %s", _("Error"), _("There was something wrong with a start frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_guint / valid_fps;
												g_free(validated_guint);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
										{
											// Get the finish frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED243: %s", _("Error"), _("There was something wrong with a finish frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = (*validated_guint / valid_fps) - tmp_layer->start_time;
												g_free(validated_guint);
											}
										}
									} else
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_time")))
										{
											// Get the start time
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED342: %s", _("Error"), _("There was something wrong with a layer start time value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "duration")))
										{
											// Get the finish frame
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED343: %s", _("Error"), _("There was something wrong with a layer duration value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED244: %s", _("Error"), _("There was something wrong with a layer visibility value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED245: %s", _("Error"), _("There was something wrong with a layer name value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED246: %s", _("Error"), _("There was something wrong with an external link value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED247: %s", _("Error"), _("There was something wrong with an external link target window value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_type")))
									{
										// Get the type of transition in
										validated_string = validate_value(TRANSITION_TYPE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											g_string_printf(message, "%s ED317: %s", _("Error"), _("There was something wrong with a transition in type value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "fade", 4))
											{
												tmp_layer->transition_in_type = TRANS_LAYER_FADE;
											} else
											{
												tmp_layer->transition_in_type = TRANS_LAYER_NONE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_duration")))
									{
										// Get the transition in duration
										validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED318: %s", _("Error"), _("There was something wrong with a transition in duration value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_layer->transition_in_duration = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->transition_in_duration = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_out_type")))
									{
										// Get the type of transition out
										validated_string = validate_value(TRANSITION_TYPE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											g_string_printf(message, "%s ED319: %s", _("Error"), _("There was something wrong with a transition out type value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "fade", 4))
											{
												tmp_layer->transition_out_type = TRANS_LAYER_FADE;
											} else
											{
												tmp_layer->transition_out_type = TRANS_LAYER_NONE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_out_duration")))
									{
										// Get the transition out duration
										validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED320: %s", _("Error"), _("There was something wrong with a transition out duration value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_layer->transition_out_duration = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->transition_out_duration = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									this_node = this_node->next;	
								}

								// Work out the end time in seconds of the presently selected layer
								end_time = tmp_layer->start_time + tmp_layer->duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_in_type)
									end_time += tmp_layer->transition_in_duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_out_type)
									end_time += tmp_layer->transition_out_duration;

								// If the new layer end time is longer than the slide duration, then extend the slide duration
								if (end_time > tmp_slide->duration)
								{
									tmp_slide->duration = end_time;
								}

								// Add this (now completed) highlight layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
								tmp_slide->num_layers++;
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
								tmp_layer->start_time = 0.0;
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->duration = default_layer_duration;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;

								// Load the highlight layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										validated_guint = validate_value(X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED248: %s", _("Error"), _("There was something wrong with an x offset start value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED249: %s", _("Error"), _("There was something wrong with a y offset start value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED250: %s", _("Error"), _("There was something wrong with an x offset finish value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED251: %s", _("Error"), _("There was something wrong with a y offset finish value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED252: %s", _("Error"), _("There was something wrong with a layer width value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED253: %s", _("Error"), _("There was something wrong with a layer height value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED254: %s", _("Error"), _("There was something wrong with a mouse click value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_mouse_ob->click = MOUSE_NONE;  // Fill in the value, just to be safe
										} else
										{
											tmp_mouse_ob->click = MOUSE_NONE;  // Set the default
											if (0 == g_ascii_strncasecmp(validated_string->str, "left_one", 8))
												tmp_mouse_ob->click = MOUSE_LEFT_ONE;
											if (0 == g_ascii_strncasecmp(validated_string->str, "left_double", 11))
												tmp_mouse_ob->click = MOUSE_LEFT_DOUBLE;
											if (0 == g_ascii_strncasecmp(validated_string->str, "left_triple", 11))
												tmp_mouse_ob->click = MOUSE_LEFT_TRIPLE;
											if (0 == g_ascii_strncasecmp(validated_string->str, "right_one", 9))
												tmp_mouse_ob->click = MOUSE_RIGHT_ONE;
											if (0 == g_ascii_strncasecmp(validated_string->str, "right_double", 12))
												tmp_mouse_ob->click = MOUSE_RIGHT_DOUBLE;
											if (0 == g_ascii_strncasecmp(validated_string->str, "right_triple", 12))
												tmp_mouse_ob->click = MOUSE_RIGHT_TRIPLE;
											if (0 == g_ascii_strncasecmp(validated_string->str, "middle_one", 10))
												tmp_mouse_ob->click = MOUSE_MIDDLE_ONE;
											if (0 == g_ascii_strncasecmp(validated_string->str, "middle_double", 13))
												tmp_mouse_ob->click = MOUSE_MIDDLE_DOUBLE;
											if (0 == g_ascii_strncasecmp(validated_string->str, "middle_triple", 13))
												tmp_mouse_ob->click = MOUSE_MIDDLE_TRIPLE;
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									// If the file format is less the version 4.0, then it has layers with frame based input rather than time based input
									if (4.0 > valid_save_format)
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
										{
											// Get the start frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED255: %s", _("Error"), _("There was something wrong with a start frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_guint / valid_fps;
												g_free(validated_guint);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
										{
											// Get the finish frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED256: %s", _("Error"), _("There was something wrong with a finish frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = (*validated_guint / valid_fps) - tmp_layer->start_time;
												g_free(validated_guint);
											}
										}
									} else
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_time")))
										{
											// Get the start time
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED344: %s", _("Error"), _("There was something wrong with a layer start time value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "duration")))
										{
											// Get the finish frame
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED345: %s", _("Error"), _("There was something wrong with a layer duration value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED257: %s", _("Error"), _("There was something wrong with a layer visibility value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED258: %s", _("Error"), _("There was something wrong with a layer name value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED259: %s", _("Error"), _("There was something wrong with an external link value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED260: %s", _("Error"), _("There was something wrong with an external link target window value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_type")))
									{
										// Get the type of transition in
										validated_string = validate_value(TRANSITION_TYPE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											g_string_printf(message, "%s ED321: %s", _("Error"), _("There was something wrong with a transition in type value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "fade", 4))
											{
												tmp_layer->transition_in_type = TRANS_LAYER_FADE;
											} else
											{
												tmp_layer->transition_in_type = TRANS_LAYER_NONE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_duration")))
									{
										// Get the transition in duration
										validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED322: %s", _("Error"), _("There was something wrong with a transition in duration value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_layer->transition_in_duration = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->transition_in_duration = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_out_type")))
									{
										// Get the type of transition out
										validated_string = validate_value(TRANSITION_TYPE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											g_string_printf(message, "%s ED323: %s", _("Error"), _("There was something wrong with a transition out type value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "fade", 4))
											{
												tmp_layer->transition_out_type = TRANS_LAYER_FADE;
											} else
											{
												tmp_layer->transition_out_type = TRANS_LAYER_NONE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_out_duration")))
									{
										// Get the transition out duration
										validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED324: %s", _("Error"), _("There was something wrong with a transition out duration value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_layer->transition_out_duration = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->transition_out_duration = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									this_node = this_node->next;	
								}

								// Work out the end time in seconds of the presently selected layer
								end_time = tmp_layer->start_time + tmp_layer->duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_in_type)
									end_time += tmp_layer->transition_in_duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_out_type)
									end_time += tmp_layer->transition_out_duration;

								// If the new layer end time is longer than the slide duration, then extend the slide duration
								if (end_time > tmp_slide->duration)
								{
									tmp_slide->duration = end_time;
								}

								// Add this (now completed) mouse pointer layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
								tmp_slide->num_layers++;
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
								tmp_layer->external_link_window = g_string_new(_("_self"));
								tmp_layer->start_time = 0.0;
								tmp_layer->transition_in_type = TRANS_LAYER_NONE;
								tmp_layer->transition_in_duration = 0.0;
								tmp_layer->duration = default_layer_duration;
								tmp_layer->transition_out_type = TRANS_LAYER_NONE;
								tmp_layer->transition_out_duration = 0.0;
								tmp_text_ob->rendered_height = 0;
								tmp_text_ob->rendered_width = 0;
								tmp_text_ob->show_bg = TRUE;
								tmp_text_ob->bg_border_width = 2.0;
								tmp_text_ob->bg_border_colour.red = 0;
								tmp_text_ob->bg_border_colour.green = 0;
								tmp_text_ob->bg_border_colour.blue = 0;
								tmp_text_ob->bg_fill_colour.red = 65535;
								tmp_text_ob->bg_fill_colour.green = 65535;
								tmp_text_ob->bg_fill_colour.blue = 52428;  // Sensible default

								// Load the text layer values
								this_node = this_layer->xmlChildrenNode;
								while (NULL != this_node)
								{
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
									{
										// Get the starting x offset
										validated_guint = validate_value(X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED261: %s", _("Error"), _("There was something wrong with an x offset start value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED262: %s", _("Error"), _("There was something wrong with a y offset start value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(X_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED263: %s", _("Error"), _("There was something wrong with an x offset finish value in the project file."));
											display_warning(message->str);
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
										validated_guint = validate_value(Y_OFFSET, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED264: %s", _("Error"), _("There was something wrong with a y offset finish value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED265: %s", _("Error"), _("There was something wrong with a red component colour value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED266: %s", _("Error"), _("There was something wrong with a green component colour value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED267: %s", _("Error"), _("There was something wrong with a blue component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_text_ob->text_color.blue = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->text_color.blue = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_border_colour_red")))
									{
										// Get the red value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED388: %s", _("Error"), _("There was something wrong with a red component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_text_ob->bg_border_colour.red = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->bg_border_colour.red = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_border_colour_green")))
									{
										// Get the green value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED389: %s", _("Error"), _("There was something wrong with a green component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_text_ob->bg_border_colour.green = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->bg_border_colour.green = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_border_colour_blue")))
									{
										// Get the blue value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED390: %s", _("Error"), _("There was something wrong with a blue component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_text_ob->bg_border_colour.blue = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->bg_border_colour.blue = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_fill_colour_red")))
									{
										// Get the red value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED391: %s", _("Error"), _("There was something wrong with a red component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_text_ob->bg_fill_colour.red = 0;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->bg_fill_colour.red = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_fill_colour_green")))
									{
										// Get the green value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED392: %s", _("Error"), _("There was something wrong with a green component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_text_ob->bg_fill_colour.green = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_fill_colour_blue")))
									{
										// Get the blue value
										validated_guint = validate_value(COLOUR_COMP16, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED393: %s", _("Error"), _("There was something wrong with a blue component colour value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_text_ob->bg_fill_colour.blue = *validated_guint;
											g_free(validated_guint);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "font_size")))
									{
										// Get the font size
										validated_gfloat = validate_value(FONT_SIZE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED268: %s", _("Error"), _("There was something wrong with a font size value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_text_ob->font_size = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_text_ob->font_size = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_border_width")))
									{
										// Get the background border width
										validated_gfloat = validate_value(LINE_WIDTH, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED394: %s", _("Error"), _("There was something wrong with a background border width value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_text_ob->bg_border_width = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "show_bg")))
									{
										// Get the background visibility
										validated_string = validate_value(SHOW_TEXT_BG, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED395: %s", _("Error"), _("There was something wrong with a text background visibility value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "true", 4))
											{
												tmp_text_ob->show_bg = TRUE;
											} else
											{
												tmp_text_ob->show_bg = FALSE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "text_value")))
									{
										// Get the text
										// fixme5: Unsure if we should validate this or not... it's supposed to be free-form. (?)
										tmp_text_ob->text_buffer = gtk_text_buffer_new(NULL);
										gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), (const gchar *) xmlNodeListGetString(document, this_node->xmlChildrenNode, 1), -1);
									}
									// If the file format is less the version 4.0, then it has layers with frame based input rather than time based input
									if (4.0 > valid_save_format)
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_frame")))
										{
											// Get the start frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED269: %s", _("Error"), _("There was something wrong with a start frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_guint / valid_fps;
												g_free(validated_guint);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "finish_frame")))
										{
											// Get the finish frame
											validated_guint = validate_value(FRAME_NUMBER, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_guint)
											{
												g_string_printf(message, "%s ED270: %s", _("Error"), _("There was something wrong with a finish frame value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = (*validated_guint / valid_fps) - tmp_layer->start_time;
												g_free(validated_guint);
											}
										}
									} else
									{
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_time")))
										{
											// Get the start time
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED346: %s", _("Error"), _("There was something wrong with a layer start time value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->start_time = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->start_time = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
										if ((!xmlStrcmp(this_node->name, (const xmlChar *) "duration")))
										{
											// Get the finish frame
											validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
											if (NULL == validated_gfloat)
											{
												g_string_printf(message, "%s ED347: %s", _("Error"), _("There was something wrong with a layer duration value in the project file."));
												display_warning(message->str);
												useable_input = FALSE;
												tmp_layer->duration = 0;  // Fill in the value, just to be safe
											} else
											{
												tmp_layer->duration = *validated_gfloat;
												g_free(validated_gfloat);
											}
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
									{
										// Get the visibility
										validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_guint)
										{
											g_string_printf(message, "%s ED271: %s", _("Error"), _("There was something wrong with a layer visibility value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED272: %s", _("Error"), _("There was something wrong with a layer name value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED273: %s", _("Error"), _("There was something wrong with an external link value in the project file."));
											display_warning(message->str);
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
											g_string_printf(message, "%s ED274: %s", _("Error"), _("There was something wrong with an external link target window value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
											g_string_free(validated_string,TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_type")))
									{
										// Get the type of transition in
										validated_string = validate_value(TRANSITION_TYPE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											g_string_printf(message, "%s ED325: %s", _("Error"), _("There was something wrong with a transition in type value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "fade", 4))
											{
												tmp_layer->transition_in_type = TRANS_LAYER_FADE;
											} else
											{
												tmp_layer->transition_in_type = TRANS_LAYER_NONE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_duration")))
									{
										// Get the transition in duration
										validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED326: %s", _("Error"), _("There was something wrong with a transition in duration value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_layer->transition_in_duration = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->transition_in_duration = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_out_type")))
									{
										// Get the type of transition out
										validated_string = validate_value(TRANSITION_TYPE, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_string)
										{
											g_string_printf(message, "%s ED327: %s", _("Error"), _("There was something wrong with a transition out type value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
										} else
										{
											if (0 == g_ascii_strncasecmp(validated_string->str, "fade", 4))
											{
												tmp_layer->transition_out_type = TRANS_LAYER_FADE;
											} else
											{
												tmp_layer->transition_out_type = TRANS_LAYER_NONE;
											}
											g_string_free(validated_string, TRUE);
											validated_string = NULL;
										}
									}
									if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_out_duration")))
									{
										// Get the transition out duration
										validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, xmlNodeListGetString(document, this_node->xmlChildrenNode, 1));
										if (NULL == validated_gfloat)
										{
											g_string_printf(message, "%s ED328: %s", _("Error"), _("There was something wrong with a transition out duration value in the project file."));
											display_warning(message->str);
											useable_input = FALSE;
											tmp_layer->transition_out_duration = 1;  // Fill in the value, just to be safe
										} else
										{
											tmp_layer->transition_out_duration = *validated_gfloat;
											g_free(validated_gfloat);
										}
									}
									this_node = this_node->next;
								}

								// Work out the end time in seconds of the presently selected layer
								end_time = tmp_layer->start_time + tmp_layer->duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_in_type)
									end_time += tmp_layer->transition_in_duration;
								if (TRANS_LAYER_NONE != tmp_layer->transition_out_type)
									end_time += tmp_layer->transition_out_duration;

								// If the new layer end time is longer than the slide duration, then extend the slide duration
								if (end_time > tmp_slide->duration)
								{
									tmp_slide->duration = end_time;
								}

								// Add this (now completed) text layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
								tmp_slide->num_layers++;
							}
						}
						layer_ptr = layer_ptr->next;
					}  // End of "We're in a layer" loop
				}
				this_layer = this_layer->next;
			}

			// Read the slide name from the project file
			tmp_slide->name = NULL;
			tmp_char = xmlGetProp(this_slide, (const xmlChar *) "name");
			if (NULL != tmp_char)
			{
				// A name for the slide is in the project file, so we use it if it validates
				validated_string = validate_value(SLIDE_NAME, V_CHAR, tmp_char);
				if (NULL == validated_string)
				{
					g_string_printf(message, "%s ED275: %s", _("Error"), _("There was something wrong with a slide name value in the project file."));
					display_warning(message->str);
					useable_input = FALSE;
				} else
				{
					tmp_slide->name = validated_string;
					validated_string = NULL;
					xmlFree(tmp_char);
				}
			}

			// Read the slide duration from the project file
			tmp_char = xmlGetProp(this_slide, (const xmlChar *) "duration");
			if (NULL != tmp_char)
			{
				// A duration for the slide is in the project file, so we use it if it validates and is longer than the one we worked out
				validated_gfloat = validate_value(SLIDE_DURATION, V_CHAR, tmp_char);
				if (NULL == validated_gfloat)
				{
					g_string_printf(message, "%s ED276: %s", _("Error"), _("There was something wrong with a slide duration value in the project file."));
					display_warning(message->str);
					useable_input = FALSE;
				} else
				{
					// If the file format is less the version 4.0, then it has slides with frame based input rather than time based input
					if (4.0 > valid_save_format)
					{
						potential_duration = *validated_gfloat / valid_fps;
					} else
					{
						potential_duration = *validated_gfloat;
					}
					g_free(validated_gfloat);
					xmlFree(tmp_char);

					// Use the larger of either the duration in the project file, or the one we calculated while loading frames
					if (potential_duration > tmp_slide->duration)
						tmp_slide->duration = potential_duration;
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
		display_warning(_("Aborting load of project file."));  // Individual error messages will already have been displayed

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

	// Information button variables
	info_link = valid_info_link;
	info_link_target = valid_info_link_target;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(valid_info_text), &text_start, &text_end);
	info_text = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(info_text), gtk_text_buffer_get_slice(GTK_TEXT_BUFFER(valid_info_text), &text_start, &text_end, TRUE), -1);
	info_display = valid_info_display;

	// Make the new slides active, and update them to fill in their remaining pieces
	slides = g_list_first(new_slides);
	num_slides = g_list_length(slides);
	for (slide_counter = 0; slide_counter < num_slides; slide_counter++)
	{
		// Select the desired slide
		slides = g_list_first(slides);
		tmp_slide = g_list_nth_data(slides, slide_counter);

		// As a workaround for potentially incorrectly saved (old) project files,
		// we scan through all of the layers in each slide, setting the duration
		// of background layers to match that of the slide
		num_layers = tmp_slide->num_layers;
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			tmp_slide->layers = g_list_first(tmp_slide->layers);
			this_layer_ptr = g_list_nth_data(tmp_slide->layers, layer_counter);

			if (TRUE == this_layer_ptr->background)
			{
				// This is a background layer, so we set its duration to match the slide
				this_layer_ptr->duration = tmp_slide->duration;
			}
		}

		// Set the timeline widget for the slide to NULL, so we know to create it later on
		tmp_slide->timeline_widget = NULL;

		// Create the thumbnail for the slide
		tmp_glist = NULL;
		tmp_glist = g_list_append(tmp_glist, tmp_slide);
		tmp_pixmap = compress_layers(tmp_glist, 0, preview_width, (guint) preview_width * 0.75);
		tmp_slide->thumbnail = gdk_pixbuf_get_from_drawable(NULL, GDK_PIXMAP(tmp_pixmap), NULL, 0, 0, 0, 0, -1, -1);

		// Add the thumbnail to the film strip
		gtk_list_store_append(film_strip_store, &film_strip_iter);
		gtk_list_store_set(film_strip_store, &film_strip_iter, 0, tmp_slide->thumbnail, -1);
	}

	// We're finished with this XML document, so release its memory
	xmlFreeDoc(document);

	// Free the memory allocated in this function
	g_string_free(message, TRUE);
	g_string_free(tmp_gstring2, TRUE);

	return TRUE;
}

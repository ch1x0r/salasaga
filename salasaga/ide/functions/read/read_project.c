/*
 * $Id$
 *
 * Salasaga: Function to parse the contents of a project file
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
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../validate_value.h"
#include "../dialog/display_warning.h"
#include "../layer/compress_layers.h"
#include "../read/read_empty_layer.h"
#include "../read/read_highlight_layer.h"
#include "../read/read_image_layer.h"
#include "../read/read_mouse_layer.h"
#include "../read/read_text_layer.h"
#include "../slide/slide_free.h"
#include "../text_tags/reset_global_text_tags_table.h"
#include "../text_tags/text_layer_create_colour_tag.h"
#include "../text_tags/text_layer_create_font_size_tag.h"


gboolean read_project(gchar *filename, guint *total_num_slides)
{
	// Local variables
	GdkColor			fg_colour;					// Foreground colours are constructed into this
	xmlChar				*control_bar_data = NULL;	// Temporarily holds incoming data prior to validation
	xmlDocPtr			document;					// Holds a pointer to the XML document
	xmlChar				*end_behaviour_data = NULL;	//
	gfloat				end_time;					// Used to calculate the end time in seconds of a layer
	GString				*error_string;				// Used to create error strings
	GtkTreeIter			film_strip_iter;
	gdouble				font_size;					// Font size
	xmlChar				*fps_data = NULL;
	guint				guint_val;					// Temporary guint value used for validation
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
	guint				preview_height;				// The height we calculate a film strip thumbnail should be
	xmlChar				*project_name_data = NULL;
	gfloat				project_ratio;				// Ratio of project height to width
	xmlChar				*output_folder_data = NULL;
	xmlChar				*output_width_data = NULL;
	xmlChar				*output_height_data = NULL;
	xmlChar				*project_width_data = NULL;
	xmlChar				*project_height_data = NULL;
	xmlChar				*save_format_data = NULL;
	guint				slide_counter;				// Counter used for looping
	xmlChar				*slide_duration_data = NULL;
	xmlChar				*slide_length_data = NULL;
	xmlNodePtr			slides_node = NULL;			// Points to the slides structure
	xmlChar				*start_behaviour_data = NULL;
	xmlNodePtr			tag_node = NULL;			// Temporary pointer
	xmlNodePtr			tags_node = NULL;			// Points to the text tags structure
	xmlNodePtr			tags_structure = NULL;		// Temporary pointer
	xmlNodePtr			this_layer;					// Temporary pointer
	layer				*this_layer_ptr;			// Pointer into a layer structure
	xmlNodePtr			this_node;					// Temporary pointer
	xmlNodePtr			this_slide;					// Temporary pointer
	xmlChar				*tmp_char;					// Temporary string pointer
	GList				*tmp_glist;					//
	GString				*tmp_gstring;				// Temporary GString
	layer				*tmp_layer = NULL;			// Temporary layer
	GdkPixbuf			*tmp_pixbuf;				// Used to convert from a pixmap to a pixbuf
	GdkPixmap			*tmp_pixmap;				//
	GdkRectangle		tmp_rect = {0, 0, get_status_bar()->allocation.width, get_status_bar()->allocation.height};  // Temporary rectangle covering the area of the status bar
	slide				*tmp_slide;					// Temporary slide
	gboolean			usable_input;				// Used as a flag to indicate if all validation was successful
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


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	error_string = g_string_new(NULL);
	message = g_string_new(NULL);
	valid_output_folder = g_string_new(NULL);
	valid_project_name = g_string_new(NULL);

	// Set sensible defaults for the swf information button
	valid_info_link = g_string_new(_("http://www.salasaga.org"));
	valid_info_link_target = g_string_new(_("_blank"));
	valid_info_text = gtk_text_buffer_new(text_tags_table);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(valid_info_text), _("Created using Salasaga"), -1);
	valid_info_display = TRUE;

	// Update the status bar
	g_string_printf(tmp_gstring, "%s - %s", _("Loading file"), filename);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_gstring->str);

	// Begin reading the file
	document = xmlParseFile(filename);
	if (NULL == document)
	{
		// The project file was unable to be parsed
		g_string_printf(message, "%s ED43: %s", _("Error"), _("The selected project file was not loaded successfully.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
		return FALSE;
	}

	this_node = xmlDocGetRootElement(document);
	if (NULL == this_node)
	{
		g_string_printf(message, "%s ED44: %s", _("Error"), _("Project file is empty.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
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

	// Scan for the "text tags" node
	this_node = xmlDocGetRootElement(document);
	this_node = this_node->xmlChildrenNode;
	while (NULL != this_node)
	{
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "text-tags")))
		{
			// Text tags node found
			tags_node = this_node;
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
			// Slides node found
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
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
		return FALSE;
	}

	// Was there a preferences structure in the save file?
	if (NULL == preferences_node)
	{
		g_string_printf(message, "%s ED45: %s", _("Error"), _("Project preferences missing, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
		return FALSE;
	}

	// Was there a slides structure in the save file?
	if (NULL == slides_node)
	{
		g_string_printf(message, "%s ED46: %s", _("Error"), _("No slides in project, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
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
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
		return FALSE;
	}
	if (NULL == project_width_data)
	{
		g_string_printf(message, "%s ED48: %s", _("Error"), _("Project Width missing, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
		return FALSE;
	}
	if (NULL == project_height_data)
	{
		g_string_printf(message, "%s ED49: %s", _("Error"), _("Project Height missing, aborting load.  Please choose a different project file."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
		return FALSE;
	}

	// Retrieve the save format input
	validated_gfloat = validate_value(PROJECT_VERSION, V_CHAR, save_format_data);
	if (NULL == validated_gfloat)
	{
		g_string_printf(message, "%s ED210: %s", _("Error"), _("The file format value in the project file is not recognized.  This project file can't be used."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);
		return FALSE;
	} else
	{
		valid_save_format = *validated_gfloat;
		g_free(validated_gfloat);
		xmlFree(save_format_data);
	}

	// Reset the usable input flag
	usable_input = TRUE;

	// * All of the required meta-data and preferences are present, so we proceed *

	// Reset the global text tag table
	// fixme4: We should update this to make a copy of the text table first,
	//         in case we need to abort the load of the project file later on
	reset_global_text_tags_table();

	// Load the text tags from the project file if they're present
	if (NULL != tags_node)
	{
		// Descend into the text tags structure
		tags_structure = tags_node->xmlChildrenNode;
		while (NULL != tags_structure)
		{
			// Check if this structure holds font size tags
			if ((!xmlStrcmp(tags_structure->name, (const xmlChar *) "size-tags")))
			{
				// Yes, this holds font size tags
				tag_node = tags_structure->xmlChildrenNode;
				while (NULL != tag_node)
				{
					// If this is an element node, we process it
					if (XML_ELEMENT_NODE == tag_node->type)
					{
						// Extract the tag name
						tmp_char = xmlGetProp(tag_node, (const xmlChar *) "label");

						// Extract the font size component value
						tmp_char = xmlGetProp(tag_node, (const xmlChar *) "size");
						font_size = g_strtod((const char *) tmp_char, NULL);

						// Create the new font size tag
						text_layer_create_font_size_tag(font_size);
					}
					// Move to the next tag
					tag_node = tag_node->next;
				}
			}

			// Check if this structure holds foreground colour tags
			if ((!xmlStrcmp(tags_structure->name, (const xmlChar *) "fg-colour-tags")))
			{
				// Yes, this holds foreground colour tags, so we process them and create the tags
				tag_node = tags_structure->xmlChildrenNode;
				while (NULL != tag_node)
				{
					// If this is an element node, we process it
					if (XML_ELEMENT_NODE == tag_node->type)
					{
						// Reset the temporary colour
						fg_colour.red = 0;
						fg_colour.green = 0;
						fg_colour.blue = 0;

						// Extract the colour values
						tmp_char = xmlGetProp(tag_node, (const xmlChar *) "red");
						fg_colour.red = atoi((const char *) tmp_char);
						tmp_char = xmlGetProp(tag_node, (const xmlChar *) "green");
						fg_colour.green = atoi((const char *) tmp_char);
						tmp_char = xmlGetProp(tag_node, (const xmlChar *) "blue");
						fg_colour.blue = atoi((const char *) tmp_char);

						// Extract the tag name
						tmp_char = xmlGetProp(tag_node, (const xmlChar *) "label");

						// Create the new font foreground colour tag
						text_layer_create_colour_tag(&fg_colour);
					}

					// Move to the next tag
					tag_node = tag_node->next;
				}
			}

			// Point to the next structure
			tags_structure = tags_structure->next;
		}
	}

	// Validate the project name input
	validated_string = validate_value(PROJECT_NAME, V_CHAR, project_name_data);
	if (NULL == validated_string)
	{
		g_string_printf(message, "%s ED202: %s", _("Error"), _("There was something wrong with the project name value in the project file."));
		display_warning(message->str);
		usable_input = FALSE;
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
		usable_input = FALSE;
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
		usable_input = FALSE;
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
		usable_input = FALSE;
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
		usable_input = FALSE;
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
		usable_input = FALSE;
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
			usable_input = FALSE;
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
			g_string_printf(message, "%s ED282: %s", _("Error"), _("There was something wrong with the start behavior value in the project file."));
			display_warning(message->str);
			usable_input = FALSE;
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
			g_string_printf(message, "%s ED279: %s", _("Error"), _("There was something wrong with the end behavior value in the project file."));
			display_warning(message->str);
			usable_input = FALSE;
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
			usable_input = FALSE;
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
			usable_input = FALSE;
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
			usable_input = FALSE;
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
			usable_input = FALSE;
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
		xmlFree(info_text_data);
	}

	// * Preferences are loaded, so now load the slides *

	this_slide = slides_node->xmlChildrenNode;
	while (NULL != this_slide)
	{
		if ((!xmlStrcmp(this_slide->name, (const xmlChar *) "slide")))
		{
			// * We're in a slide *

			// Increment the slide counter
			*total_num_slides = *total_num_slides + 1;

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
								// Yes it is, so load its data
								tmp_layer = read_empty_layer(document, this_layer->xmlChildrenNode);
							}
							// Test if this layer is an image layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "image"))
							{
								// Yes it is, so load its data
								tmp_layer = read_image_layer(document, this_layer->xmlChildrenNode);
							}
							// Test if this layer is a highlight layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "highlight"))
							{
								// Yes it is, so load its data
								tmp_layer = read_highlight_layer(document, this_layer->xmlChildrenNode);
							}
							// Test if this layer is a mouse pointer layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "mouse"))
							{
								// Yes it is, so load its data
								tmp_layer = read_mouse_layer(document, this_layer->xmlChildrenNode);
							}
							// Test if this layer is a text layer
							if (!xmlStrcmp(tmp_char, (const xmlChar *) "text"))
							{
								// Yes it is, so load its data
								tmp_layer = read_text_layer(document, this_layer->xmlChildrenNode, valid_save_format);
							}

							// Check if the layer information was read ok, and process it if it was
							if (NULL != tmp_layer)
							{
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

								// Add this (now completed) layer to the slide
								tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);
								tmp_slide->num_layers++;
							} else
							{
								// The layer wasn't read back ok
								usable_input = FALSE;
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
					usable_input = FALSE;
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
					usable_input = FALSE;
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

			// Update the status bar
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(get_status_bar()));
			gtk_widget_draw(get_status_bar(), &tmp_rect);
			gdk_flush();

		}  // End of "We're in a slide" loop

		this_slide = this_slide->next;
	}

	// Abort if any of the input isn't valid
	if (TRUE != usable_input)
	{
		// At least one of the values in the project file is invalid, so abort the load
		display_warning(_("Aborting load of project file."));  // Individual error messages will already have been displayed
		g_string_free(message, TRUE);
		g_string_free(tmp_gstring, TRUE);
		g_string_free(error_string, TRUE);
		g_string_free(valid_output_folder, TRUE);
		g_string_free(valid_project_name, TRUE);
		g_string_free(valid_info_link, TRUE);
		g_string_free(valid_info_link_target, TRUE);
		g_object_unref(valid_info_text);

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
		set_frames_per_second(valid_fps);
	}

	// Load Start behaviour
	set_start_behaviour(valid_start_behaviour);

	// Load End behaviour
	set_end_behaviour(valid_end_behaviour);

	// Control bar display
	set_show_control_bar(valid_control_bar_behaviour);

	// Information button variables
	info_link = valid_info_link;
	info_link_target = valid_info_link_target;
	info_text = valid_info_text;
	set_info_display(valid_info_display);

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

		// Determine the proper thumbnail height
		project_ratio = (gfloat) project_height / (gfloat) project_width;
		preview_height = preview_width * project_ratio;

		// Create the thumbnail for the slide
		tmp_glist = NULL;
		tmp_glist = g_list_append(tmp_glist, tmp_slide);
		tmp_pixmap = compress_layers(tmp_glist, 0, project_width, project_height);
		tmp_pixbuf = gdk_pixbuf_get_from_drawable(NULL, GDK_PIXMAP(tmp_pixmap), NULL, 0, 0, 0, 0, -1, -1);
		tmp_slide->thumbnail = gdk_pixbuf_scale_simple(GDK_PIXBUF(tmp_pixbuf), preview_width, preview_height, GDK_INTERP_TILES);
		g_object_unref(GDK_PIXBUF(tmp_pixbuf));

		// Add the thumbnail to the film strip
		gtk_list_store_append(film_strip_store, &film_strip_iter);
		gtk_list_store_set(film_strip_store, &film_strip_iter, 0, tmp_slide->thumbnail, -1);
	}

	// We're finished with this XML document, so release its memory
	xmlFreeDoc(document);

	// Free the memory allocated in this function
	g_string_free(message, TRUE);
	g_string_free(tmp_gstring, TRUE);
	g_string_free(error_string, TRUE);

	return TRUE;
}

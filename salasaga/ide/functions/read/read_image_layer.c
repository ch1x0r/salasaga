/*
 * $Id$
 *
 * Salasaga: Function to create the read image layer data from a project file
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

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../cairo/create_cairo_pixbuf_pattern.h"
#include "../dialog/display_warning.h"
#include "../layer/layer_free.h"
#include "../other/validate_value.h"


layer *read_image_layer(xmlDocPtr document, xmlNodePtr this_node)
{
	// Local variables
	gint				data_length;				// Number of image data bytes a layer says it stores
	GError				*error = NULL;				// Pointer to error return structure
	GString				*error_string;				// Used to create error strings
	GString				*image_decode_gstring;		// Temporary GString used for base64 decoding image data
	GdkPixbufLoader		*image_loader;				// Used for loading images embedded in project files
	GString				*message;					// Used to construct message strings
	gboolean			return_code;				// Boolean return code
	guchar				*decoded_string;			// Used for holding a decoded base64 strings
	gsize				decoded_string_length;		// Holds the length of a decoded base64 string
	layer				*tmp_layer;					// Temporary layer
	layer_image			*tmp_image_ob;				// Temporary image layer object
	xmlChar				*tmp_xmlChar;				// Temporary xmlChar pointer
	gboolean			usable_input = TRUE;		// Used as a flag to indicate if all validation was successful
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function


	// Initialisation
	error_string = g_string_new(NULL);
	image_decode_gstring = g_string_new(NULL);
	message = g_string_new(NULL);

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
	while (NULL != this_node)
	{
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "x_offset_start")))
		{
			// Get the starting x offset
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(X_OFFSET, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED220: %s", _("Error"), _("There was something wrong with an image x offset start value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(Y_OFFSET, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED221: %s", _("Error"), _("There was something wrong with an image y offset start value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(X_OFFSET, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED222: %s", _("Error"), _("There was something wrong with an image x offset finish value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(Y_OFFSET, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED223: %s", _("Error"), _("There was something wrong with an image y offset finish value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(LAYER_WIDTH, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED224: %s", _("Error"), _("There was something wrong with an image layer width value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(LAYER_HEIGHT, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED225: %s", _("Error"), _("There was something wrong with an image layer height value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(LAYER_BACKGROUND, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED226: %s", _("Error"), _("There was something wrong with an image layer background value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
			} else
			{
				tmp_layer->background = *validated_guint;
				g_free(validated_guint);
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "start_time")))
		{
			// Get the start time
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_gfloat)
			{
				g_string_printf(message, "%s ED340: %s", _("Error"), _("There was something wrong with an image layer start time value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_gfloat = validate_value(LAYER_DURATION, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_gfloat)
			{
				g_string_printf(message, "%s ED341: %s", _("Error"), _("There was something wrong with an image layer duration value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
				tmp_layer->duration = 0;  // Fill in the value, just to be safe
			} else
			{
				tmp_layer->duration = *validated_gfloat;
				g_free(validated_gfloat);
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "visible")))
		{
			// Get the visibility
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(LAYER_VISIBLE, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED229: %s", _("Error"), _("There was something wrong with an image layer visibility value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(LAYER_NAME, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_string)
			{
				g_string_printf(message, "%s ED230: %s", _("Error"), _("There was something wrong with an image layer name value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(EXTERNAL_LINK, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_string)
			{
				g_string_printf(message, "%s ED231: %s", _("Error"), _("There was something wrong with an image layer external link value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(EXTERNAL_LINK_WINDOW, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_string)
			{
				g_string_printf(message, "%s ED232: %s", _("Error"), _("There was something wrong with an image layer external link target window value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
			} else
			{
				tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
				g_string_free(validated_string,TRUE);
				validated_string = NULL;
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "data")))
		{
			// Get the image data
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(IMAGE_DATA, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_string)
			{
				g_string_printf(message, "%s ED234: %s", _("Error"), _("There was something wrong with image data in the project file."));
				display_warning(message->str);
				image_decode_gstring = NULL;
				usable_input = FALSE;
			} else
			{
				image_decode_gstring = validated_string;
				validated_string = NULL;
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "data_length")))
		{
			// Get the number of bytes the image should hold
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(IMAGE_DATA_LENGTH, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED235: %s", _("Error"), _("There was something wrong with an image data length value in the project file."));
				display_warning(message->str);
				data_length = 0;  // Fill in the value, just to be safe
				usable_input = FALSE;
			} else
			{
				data_length = *validated_guint;
				g_free(validated_guint);
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "transition_in_type")))
		{
			// Get the type of transition in
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(TRANSITION_TYPE, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_string)
			{
				g_string_printf(message, "%s ED313: %s", _("Error"), _("There was something wrong with an image transition in type value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_gfloat)
			{
				g_string_printf(message, "%s ED314: %s", _("Error"), _("There was something wrong with an image transition in duration value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(TRANSITION_TYPE, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_string)
			{
				g_string_printf(message, "%s ED315: %s", _("Error"), _("There was something wrong with an image transition out type value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_gfloat = validate_value(TRANSITION_DURATION, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_gfloat)
			{
				g_string_printf(message, "%s ED316: %s", _("Error"), _("There was something wrong with an image transition out duration value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
				tmp_layer->transition_out_duration = 1;  // Fill in the value, just to be safe
			} else
			{
				tmp_layer->transition_out_duration = *validated_gfloat;
				g_free(validated_gfloat);
			}
		}
		this_node = this_node->next;
	}

	// * We should have all of the image details by this stage, so can process the image data *

	// Base64 decode the image data back into png format
	decoded_string = g_base64_decode(image_decode_gstring->str, &decoded_string_length);
	image_decode_gstring = g_string_set_size(image_decode_gstring, 0);  // Remove the data now that it's been used

	// Convert the png data into a GdxPixbuf we can use
	image_loader = gdk_pixbuf_loader_new();
	return_code = gdk_pixbuf_loader_write(image_loader, decoded_string, decoded_string_length, &error);
	if (TRUE != return_code)
	{
		g_string_printf(error_string, "%s ED66: %s: '%s'", _("Error"), _("Image data loading failed"), error->message);
		display_warning(error_string->str);
	}
	tmp_image_ob->image_data = gdk_pixbuf_loader_get_pixbuf(image_loader);
	if (NULL == tmp_image_ob->image_data)
	{
		g_string_printf(message, "%s ED65: %s", _("Error"), _("Error when loading image data"));
		display_warning(message->str);
	}
	return_code = gdk_pixbuf_loader_close(image_loader, &error);
	if (TRUE != return_code)
	{
		g_string_printf(error_string, "%s ED67: %s: '%s'", _("Error"), _("Image data loading failed"), error->message);
		display_warning(error_string->str);
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

	// Free memory allocated in this function
	g_string_free(error_string, TRUE);
	g_string_free(image_decode_gstring, TRUE);
	g_string_free(message, TRUE);

	// Return the validated mouse layer, or an indicator of failure
	if (TRUE == usable_input)
	{
		return tmp_layer;
	} else
	{
		// Free the newly allocated mouse layer data, as it didn't pass validation, so we won't use it
		layer_free(tmp_layer);
		return NULL;
	}
}

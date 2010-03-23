/*
 * $Id$
 *
 * Salasaga: Function to create the read empty layer data from a project file
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../layer/layer_free.h"
#include "../other/validate_value.h"
#include "../preference/application_preferences.h"


layer *read_empty_layer(xmlDocPtr document, xmlNodePtr this_node)
{
	// Local variables
	GString				*message;					// Used to construct message strings
	layer				*tmp_layer;					// Temporary layer
	layer_empty			*tmp_empty_ob;				// Temporary empty layer object
	xmlChar				*tmp_xmlChar;				// Temporary xmlChar pointer
	gboolean			usable_input = TRUE;		// Used as a flag to indicate if all validation was successful
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function


	// Initialisation
	message = g_string_new(NULL);

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
	tmp_layer->duration = get_default_slide_duration();
	tmp_layer->transition_out_type = TRANS_LAYER_NONE;
	tmp_layer->transition_out_duration = 0.0;

	// Load the empty layer values
	while (NULL != this_node)
	{
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "red")))
		{
			// Get the red value
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED211: %s", _("Error"), _("There was something wrong with a red component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED212: %s", _("Error"), _("There was something wrong with a green component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED213: %s", _("Error"), _("There was something wrong with a blue component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
				tmp_empty_ob->bg_color.blue = 0;  // Fill in the value, just to be safe
			} else
			{
				tmp_empty_ob->bg_color.blue = *validated_guint;
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
				g_string_printf(message, "%s ED336: %s", _("Error"), _("There was something wrong with a layer start time value in the project file."));
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
				g_string_printf(message, "%s ED337: %s", _("Error"), _("There was something wrong with a layer duration value in the project file."));
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
				g_string_printf(message, "%s ED216: %s", _("Error"), _("There was something wrong with a layer visibility value in the project file."));
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
				g_string_printf(message, "%s ED217: %s", _("Error"), _("There was something wrong with a layer name value in the project file."));
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
				g_string_printf(message, "%s ED218: %s", _("Error"), _("There was something wrong with an external link value in the project file."));
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
				g_string_printf(message, "%s ED219: %s", _("Error"), _("There was something wrong with an external link target window value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
			} else
			{
				tmp_layer->external_link_window = g_string_assign(tmp_layer->external_link_window, validated_string->str);
				g_string_free(validated_string,TRUE);
				validated_string = NULL;
			}
		}
		this_node = this_node->next;
	}

	// Free memory allocated in this function
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

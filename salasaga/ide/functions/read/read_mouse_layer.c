/*
 * $Id$
 *
 * Salasaga: Function to create the read mouse layer data from a project file
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
#include "../dialog/display_warning.h"
#include "../layer/layer_free.h"
#include "../other/validate_value.h"
#include "../preference/application_preferences.h"


layer *read_mouse_layer(xmlDocPtr document, xmlNodePtr this_node)
{
	// Local variables
	GString				*message;					// Used to construct message strings
	layer				*tmp_layer;					// Temporary layer
	layer_mouse			*tmp_mouse_ob;				// Temporary mouse layer object
	xmlChar				*tmp_xmlChar;				// Temporary xmlChar pointer
	gboolean			usable_input = TRUE;		// Used as a flag to indicate if all validation was successful
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint;			// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function


	// Initialisation
	message = g_string_new(NULL);

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
	tmp_layer->duration = get_default_layer_duration();
	tmp_layer->transition_out_type = TRANS_LAYER_NONE;
	tmp_layer->transition_out_duration = 0.0;

	// Load the highlight layer values
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
				g_string_printf(message, "%s ED248: %s", _("Error"), _("There was something wrong with an x offset start value in the project file."));
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
				g_string_printf(message, "%s ED249: %s", _("Error"), _("There was something wrong with a y offset start value in the project file."));
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
				g_string_printf(message, "%s ED250: %s", _("Error"), _("There was something wrong with an x offset finish value in the project file."));
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
				g_string_printf(message, "%s ED251: %s", _("Error"), _("There was something wrong with a y offset finish value in the project file."));
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
				g_string_printf(message, "%s ED252: %s", _("Error"), _("There was something wrong with a layer width value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(LAYER_HEIGHT, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED253: %s", _("Error"), _("There was something wrong with a layer height value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(MOUSE_CLICK, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_string)
			{
				g_string_printf(message, "%s ED254: %s", _("Error"), _("There was something wrong with a mouse click value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
				g_string_free(validated_string, TRUE);
				validated_string = NULL;
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
				g_string_printf(message, "%s ED344: %s", _("Error"), _("There was something wrong with a layer start time value in the project file."));
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
				g_string_printf(message, "%s ED345: %s", _("Error"), _("There was something wrong with a layer duration value in the project file."));
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
				g_string_printf(message, "%s ED257: %s", _("Error"), _("There was something wrong with a layer visibility value in the project file."));
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
				g_string_printf(message, "%s ED258: %s", _("Error"), _("There was something wrong with a layer name value in the project file."));
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
				g_string_printf(message, "%s ED259: %s", _("Error"), _("There was something wrong with an external link value in the project file."));
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
				g_string_printf(message, "%s ED260: %s", _("Error"), _("There was something wrong with an external link target window value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(TRANSITION_TYPE, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_string)
			{
				g_string_printf(message, "%s ED321: %s", _("Error"), _("There was something wrong with a transition in type value in the project file."));
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
				g_string_printf(message, "%s ED322: %s", _("Error"), _("There was something wrong with a transition in duration value in the project file."));
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
				g_string_printf(message, "%s ED323: %s", _("Error"), _("There was something wrong with a transition out type value in the project file."));
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
				g_string_printf(message, "%s ED324: %s", _("Error"), _("There was something wrong with a transition out duration value in the project file."));
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

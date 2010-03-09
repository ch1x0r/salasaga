/*
 * $Id$
 *
 * Salasaga: Function to create the read text layer data from a project file
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
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../layer/layer_free.h"
#include "../other/validate_value.h"
#include "../preference/application_preferences.h"
#include "../text_tags/text_layer_create_colour_tag.h"
#include "../text_tags/text_layer_create_font_size_tag.h"


layer *read_text_layer(xmlDocPtr document, xmlNodePtr this_node, gfloat valid_save_format)
{
	// Local variables
	xmlNodePtr			character_node;				// Used for pointing at the start of the character nodes
	gchar				*conversion_buffer;			// Used for converting from UTF-32 to UTF-8
	GtkTextIter			end_iter;					// Used to point to the end of a Gtk text buffer
	GtkTextIter			end_iter_minus_one;			// Used to point to the character right before the end of a Gtk text buffer
	GdkColor			fg_colour;					// Text foreground colour values, used when converting old project files to the newer (v5.0+) format
	GtkTextTag			*fg_colour_tag;				// Text tag used when converting old project files to the newer (v5.0+) format
	guint				font_face;					// Used for loading old format project files
	gdouble				font_size;					// Used for loading old format project files
	guint				loop_counter;				// Simple counter used for loops
	guint				max_chars;					// Max characters
	GString				*message;					// Used to construct message strings
	gunichar			temp_char;					// Used when converting between character sets
	GString				*text_buffer_decode_gstring;  // Temporary GString used for base64 decoding
	GtkTextIter			text_end;					// End position of text buffer
	xmlNodePtr			text_node;					// Used for pointing at the top of a text layer branch
	GtkTextTag			*text_size_text_tag;		// Text tag used when converting old project files to the newer (v5.0+) format
	GtkTextIter			text_start;					// Start position of text buffer
	layer				*tmp_layer;					// Temporary layer
	layer_text			*tmp_text_ob;				// Temporary text layer object
	xmlChar				*tmp_xmlChar;				// Temporary xmlChar pointer
	gboolean			usable_input = TRUE;		// Used as a flag to indicate if all validation was successful
	gfloat				*validated_gfloat;			// Receives known good gfloat values from the validation function
	guint				*validated_guint = NULL;	// Receives known good guint values from the validation function
	GString				*validated_string;			// Receives known good strings from the validation function


	// Initialisation
	font_face = FONT_DEJAVU_SANS;  // Default font
	font_size = 10.0;  // Default font size
	fg_colour.red = 0;    // Default foreground colour of black
	fg_colour.green = 0;  //
	fg_colour.blue = 0;   //
	message = g_string_new(NULL);
	text_buffer_decode_gstring = g_string_new(NULL);

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
	tmp_layer->duration = get_default_layer_duration();
	tmp_layer->transition_out_type = TRANS_LAYER_NONE;
	tmp_layer->transition_out_duration = 0.0;
	tmp_text_ob->text_buffer = NULL;
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
				g_string_printf(message, "%s ED261: %s", _("Error"), _("There was something wrong with an x offset start value in the project file."));
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
				g_string_printf(message, "%s ED262: %s", _("Error"), _("There was something wrong with a y offset start value in the project file."));
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
				g_string_printf(message, "%s ED263: %s", _("Error"), _("There was something wrong with an x offset finish value in the project file."));
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
				g_string_printf(message, "%s ED264: %s", _("Error"), _("There was something wrong with a y offset finish value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
				tmp_layer->y_offset_finish = 0;  // Fill in the value, just to be safe
			} else
			{
				tmp_layer->y_offset_finish = *validated_guint;
				g_free(validated_guint);
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_border_colour_red")))
		{
			// Get the red value
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED388: %s", _("Error"), _("There was something wrong with a red component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED389: %s", _("Error"), _("There was something wrong with a green component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED390: %s", _("Error"), _("There was something wrong with a blue component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED391: %s", _("Error"), _("There was something wrong with a red component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED392: %s", _("Error"), _("There was something wrong with a green component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
			} else
			{
				tmp_text_ob->bg_fill_colour.green = *validated_guint;
				g_free(validated_guint);
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_fill_colour_blue")))
		{
			// Get the blue value
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED393: %s", _("Error"), _("There was something wrong with a blue component color value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
			} else
			{
				tmp_text_ob->bg_fill_colour.blue = *validated_guint;
				g_free(validated_guint);
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "bg_border_width")))
		{
			// Get the background border width
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_gfloat = validate_value(LINE_WIDTH, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_gfloat)
			{
				g_string_printf(message, "%s ED394: %s", _("Error"), _("There was something wrong with a background border width value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
			} else
			{
				tmp_text_ob->bg_border_width = *validated_gfloat;
				g_free(validated_gfloat);
			}
		}
		if ((!xmlStrcmp(this_node->name, (const xmlChar *) "show_bg")))
		{
			// Get the background visibility
			tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
			validated_string = validate_value(SHOW_TEXT_BG, V_CHAR, tmp_xmlChar);
			xmlFree(tmp_xmlChar);
			if (NULL == validated_guint)
			{
				g_string_printf(message, "%s ED395: %s", _("Error"), _("There was something wrong with a text background visibility value in the project file."));
				display_warning(message->str);
				usable_input = FALSE;
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

		// Determine if we're using a project file version that stores the text here in XML (v5.0+),
		// or an older version ( < v5.0)
		if (5.0 <= valid_save_format)
		{
			// We're using a project file that stores the text here in XML
			if ((NULL == tmp_text_ob->text_buffer) && (!xmlStrcmp(this_node->name, (const xmlChar *) "text_buffer")))
			{
				// Store this pointer for later, and move into the XML child node for now
				text_node = this_node;

				// Create a new text buffer for putting text in
				tmp_text_ob->text_buffer = gtk_text_buffer_new(get_text_tags_table());
				gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), &text_start);

				// Get the number of characters to process
				tmp_xmlChar = xmlGetProp(this_node, (const xmlChar *) "num_characters");

				validated_guint = validate_value(NUM_TEXT_CHARS, V_CHAR, tmp_xmlChar);
				xmlFree(tmp_xmlChar);
				if (NULL == validated_guint)
				{
					g_string_printf(message, "%s ED446: %s", _("Error"), _("There was something wrong with the count of text layer values given in the project file."));
					display_warning(message->str);
					usable_input = FALSE;
					max_chars = 0;  // Fill in the value, just to be safe
				} else
				{
					max_chars = *validated_guint;
					g_free(validated_guint);
				}

				// Search for the required characters one by one
				character_node = this_node->xmlChildrenNode;
				for (loop_counter = 0; loop_counter < max_chars; loop_counter++)
				{
					// Move to the required character node
					g_string_printf(message, "character_%u", loop_counter);
					this_node = character_node;
					while (0 != xmlStrcmp(this_node->name, (const xmlChar *) message->str))
					{
						// Move to the next node
						this_node = this_node->next;
					}

					// Retrieve the character value
					tmp_xmlChar = xmlGetProp(this_node, (const xmlChar *) "character");
					temp_char = g_utf8_get_char_validated((const gchar *) tmp_xmlChar, -1);

					// Validate the retrieved character
					if (TRUE != g_unichar_validate(temp_char))
					{
						// Something other than a unicode character was retrieved
						g_string_printf(message, "%s ED445: %s", _("Error"), _("Invalid unicode character found in text layer input."));
						display_warning(message->str);
						continue;
					}

					// Insert the text character into the Gtk text buffer
					conversion_buffer = g_ucs4_to_utf8(&temp_char, 1, NULL, NULL, NULL);
					gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), conversion_buffer, -1);
					xmlFree(tmp_xmlChar);

					// Set up the TextIter pointers for the Gtk text buffer
					gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), &end_iter);
					end_iter_minus_one = end_iter;
					gtk_text_iter_backward_char(&end_iter_minus_one);

					// Process the text tags from the XML and apply them to this character in the Gtk text buffer
					this_node = this_node->xmlChildrenNode;
					while (NULL != this_node)
					{
						if ((!xmlStrcmp(this_node->name, (const xmlChar *) "tag")))
						{
							// Retrieve the tag string value
							// fixme3: Should probably add validation of the tag names at some point, to be on the safe side
							tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);

							// If the tag is a text size tag, we replace any comma characters ',' with a full stop '.'
							if (0 == xmlStrncmp(tmp_xmlChar, (const xmlChar *) "text size", 9))
							{
								g_strcanon((gchar *) tmp_xmlChar, "tex siz0123456789", '.');
							}

							// Apply the tag to this character in the Gtk text buffer
							gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), (const gchar *) tmp_xmlChar, &end_iter_minus_one, &end_iter);
							xmlFree(tmp_xmlChar);
						}

						// Move to the next node
						this_node = this_node->next;
					}

					// End of the loop that retrieves characters
				}

				// Let the program continue on it's merry way
				this_node = text_node;
			}

		} else
		{
			// * We're using an older project file version, that has text stored in multiple fields *

			if ((!xmlStrcmp(this_node->name, (const xmlChar *) "text_value")))
			{
				// Get the text
				// fixme5: Unsure how to validate this... it's supposed to be free-form. (?)
				tmp_text_ob->text_buffer = gtk_text_buffer_new(get_text_tags_table());
				tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
				gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), (const gchar *) tmp_xmlChar, -1);
				xmlFree(tmp_xmlChar);
			}

			if ((!xmlStrcmp(this_node->name, (const xmlChar *) "font_face")))
			{
				// Get the font face for the text
				tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
				validated_guint = validate_value(FONT_FACE, V_CHAR, tmp_xmlChar);
				xmlFree(tmp_xmlChar);
				if (NULL == validated_guint)
				{
					g_string_printf(message, "%s ED425: %s", _("Error"), _("There was something wrong with a font face value in the project file."));
					display_warning(message->str);
					usable_input = FALSE;
					font_face = FONT_DEJAVU_SANS;  // Safe default
				} else
				{
					// Store the requested font face in a temporary variable, so we can apply it to the whole text buffer later on
					font_face = *validated_guint;
					g_free(validated_guint);
				}
			}
			if ((!xmlStrcmp(this_node->name, (const xmlChar *) "font_size")))
			{
				// Get the font size
				tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
				validated_gfloat = validate_value(FONT_SIZE, V_CHAR, tmp_xmlChar);
				xmlFree(tmp_xmlChar);
				if (NULL == validated_gfloat)
				{
					g_string_printf(message, "%s ED268: %s", _("Error"), _("There was something wrong with a font size value in the project file."));
					display_warning(message->str);
					usable_input = FALSE;
					font_size = 10.0;  // Safe default
				} else
				{
					// Store the requested font size in a temporary variable, so we can apply it to the whole text buffer later on
					font_size = *validated_gfloat;
					g_free(validated_gfloat);
				}
			}
			if ((!xmlStrcmp(this_node->name, (const xmlChar *) "red")))
			{
				// Get the red value
				tmp_xmlChar = xmlNodeListGetString(document, this_node->xmlChildrenNode, 1);
				validated_guint = validate_value(COLOUR_COMP16, V_CHAR, tmp_xmlChar);
				xmlFree(tmp_xmlChar);
				if (NULL == validated_guint)
				{
					g_string_printf(message, "%s ED265: %s", _("Error"), _("There was something wrong with a red component color value in the project file."));
					display_warning(message->str);
					usable_input = FALSE;
					fg_colour.red = 0;  // Safe default
				} else
				{
					// Store this colour component in a temporary GdkColor, so we can apply it to the whole text buffer later on
					fg_colour.red = *validated_guint;
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
					g_string_printf(message, "%s ED266: %s", _("Error"), _("There was something wrong with a green component color value in the project file."));
					display_warning(message->str);
					usable_input = FALSE;
					fg_colour.green = 0;  // Safe default
				} else
				{
					// Store this colour component in a temporary GdkColor, so we can apply it to the whole text buffer later on
					fg_colour.green = *validated_guint;
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
					g_string_printf(message, "%s ED267: %s", _("Error"), _("There was something wrong with a blue component color value in the project file."));
					display_warning(message->str);
					usable_input = FALSE;
					fg_colour.blue = 0;  // Safe default
				} else
				{
					// Store this colour component in a temporary GdkColor, so we can apply it to the whole text buffer later on
					fg_colour.blue = *validated_guint;
					g_free(validated_guint);
				}
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
				g_string_printf(message, "%s ED346: %s", _("Error"), _("There was something wrong with a layer start time value in the project file."));
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
				g_string_printf(message, "%s ED347: %s", _("Error"), _("There was something wrong with a layer duration value in the project file."));
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
				g_string_printf(message, "%s ED271: %s", _("Error"), _("There was something wrong with a layer visibility value in the project file."));
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
				g_string_printf(message, "%s ED272: %s", _("Error"), _("There was something wrong with a layer name value in the project file."));
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
				g_string_printf(message, "%s ED273: %s", _("Error"), _("There was something wrong with an external link value in the project file."));
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
				g_string_printf(message, "%s ED274: %s", _("Error"), _("There was something wrong with an external link target window value in the project file."));
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
				g_string_printf(message, "%s ED325: %s", _("Error"), _("There was something wrong with a transition in type value in the project file."));
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
				g_string_printf(message, "%s ED326: %s", _("Error"), _("There was something wrong with a transition in duration value in the project file."));
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
				g_string_printf(message, "%s ED327: %s", _("Error"), _("There was something wrong with a transition out type value in the project file."));
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
				g_string_printf(message, "%s ED328: %s", _("Error"), _("There was something wrong with a transition out duration value in the project file."));
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

	if (5.0 > valid_save_format)
	{
		// * Old style project file, with text data in multiple values *

		// The text buffer already has the text layer strings loaded into it, so we just apply
		// the foreground colour, size, and font face information to the entire text buffer

		// Retrieve the start and end points for the entire text buffer
		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), &text_start, &text_end);

		// Apply the font face (via a tag) to the entire text buffer
		gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), get_salasaga_font_name(font_face), &text_start, &text_end);

		// Apply the font size (via a tag) to the entire text buffer
		text_size_text_tag = text_layer_create_font_size_tag(font_size);
		gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), GTK_TEXT_TAG(text_size_text_tag), &text_start, &text_end);

		// Apply the font foreground colour (via a tag) to the entire text buffer
		fg_colour_tag = text_layer_create_colour_tag(&fg_colour);
		gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), GTK_TEXT_TAG(fg_colour_tag), &text_start, &text_end);
	}

	// Free memory allocated in this function
	g_string_free(message, TRUE);

	// Return the validated text layer, or an indicator of failure
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

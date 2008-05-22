/*
 * $Id$
 *
 * Salasaga: Function called when the user selects File -> Save from the top menu 
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


// GTK includes
#include <gtk/gtk.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../validate_value.h"
#include "../dialog/display_warning.h"
#include "menu_file_save_as.h"
#include "menu_file_save_slide.h"


void menu_file_save(void)
{
	// Local variables
	xmlDocPtr			document_pointer;			// Points to the XML document structure in memory
	xmlNodePtr			slide_root;					// Points to the root of the slide data
	xmlNodePtr			meta_pointer;				// Points to the meta-data node
	xmlNodePtr			pref_pointer;				// Points to the preferences node
	xmlNodePtr			root_node;					// Points to the root node
	xmlSaveCtxt			*save_context;				// Points to the save context
	GtkTextIter			text_end;					// End position of text buffer
	GtkTextIter			text_start;					// Start position of text buffer
	GString				*tmp_gstring;				// Temporary GString
	gint				tmp_int;					// Temporary integer
	glong				tmp_long;					// Temporary long integer


	// If there's no project active, we just beep and return
	if (FALSE == project_active)
	{
		gdk_beep();
		return;
	}

	// If we don't have a valid file name, we open the Save As dialog instead
	if (NULL == file_name)
	{
		// Call the function which gets the filename (then does it's own save)
		menu_file_save_as();
		return;
	}

	// Initialise some things
	tmp_gstring = g_string_new(NULL);

	// Create an empty document pointer
	document_pointer = xmlNewDoc((const xmlChar *) "1.0");
	if (NULL == document_pointer)
	{
		display_warning("Error ED19: Error creating the XML save document.");
		return;
	}

    // Create the root node
	root_node = xmlNewDocRawNode(document_pointer, NULL, (const xmlChar *) "salasaga_project", NULL);
	if (NULL == root_node)
	{
		display_warning("Error ED21: Error creating the root node.");
		return;
	}

	// Anchor the root node to the document
	xmlDocSetRootElement(document_pointer, root_node);

    // Create the meta information container
	meta_pointer = xmlNewChild(root_node, NULL, (const xmlChar *) "meta-data", NULL);
	if (NULL == meta_pointer)
	{
		display_warning("Error ED25: Error creating the meta-data container.");
		return;
	}

	// Add the save format version number to the XML document
	xmlNewChild(meta_pointer, NULL, (const xmlChar *) "save_format", (const xmlChar *) "4.2");

    // Create the preferences container
	pref_pointer = xmlNewChild(root_node, NULL, (const xmlChar *) "preferences", NULL);
	if (NULL == pref_pointer)
	{
		display_warning("Error ED20: Error creating the preferences container.");
		return;
	}

	// Add the project preferences to the XML document
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "project_name", (const xmlChar *) project_name->str);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "output_folder", (const xmlChar *) output_folder->str);
	g_string_printf(tmp_gstring, "%u", output_width);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "output_width", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", output_height);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "output_height", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", project_width);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "project_width", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", project_height);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "project_height", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", frames_per_second);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "frames_per_second", (const xmlChar *) tmp_gstring->str);
	switch (start_behaviour)
	{
		case START_BEHAVIOUR_PAUSED:
			xmlNewChild(pref_pointer, NULL, (const xmlChar *) "start_behaviour", (const xmlChar *) "paused");
			break;

		case START_BEHAVIOUR_PLAY:
			xmlNewChild(pref_pointer, NULL, (const xmlChar *) "start_behaviour", (const xmlChar *) "play");
			break;

		default:
			display_warning("Error ED281: Error creating the start behaviour value.");
			return;
	}
	switch (end_behaviour)
	{
		case END_BEHAVIOUR_STOP:
			xmlNewChild(pref_pointer, NULL, (const xmlChar *) "end_behaviour", (const xmlChar *) "stop");
			break;

		case END_BEHAVIOUR_LOOP_PLAY:
			xmlNewChild(pref_pointer, NULL, (const xmlChar *) "end_behaviour", (const xmlChar *) "loop_play");
			break;

		case END_BEHAVIOUR_LOOP_STOP:
			xmlNewChild(pref_pointer, NULL, (const xmlChar *) "end_behaviour", (const xmlChar *) "loop_stop");
			break;

		default:
			display_warning("Error ED278: Error creating the end behaviour value.");
			return;
	}
	if (TRUE == show_control_bar)
	{
		xmlNewChild(pref_pointer, NULL, (const xmlChar *) "show_control_bar", (const xmlChar *) "true");
	} else
	{
		xmlNewChild(pref_pointer, NULL, (const xmlChar *) "show_control_bar", (const xmlChar *) "false");
	}
	if (TRUE == info_display)
	{
		xmlNewChild(pref_pointer, NULL, (const xmlChar *) "info_display", (const xmlChar *) "true");
	} else
	{
		xmlNewChild(pref_pointer, NULL, (const xmlChar *) "info_display", (const xmlChar *) "false");
	}
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "info_link", (const xmlChar *) info_link->str);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "info_link_target", (const xmlChar *) info_link_target->str);
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(info_text), &text_start, &text_end);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "info_text", (const xmlChar *) gtk_text_buffer_get_slice(GTK_TEXT_BUFFER(info_text), &text_start, &text_end, TRUE));

    // Create a container for the slides
	slide_root = xmlNewChild(root_node, NULL, (const xmlChar *) "slides", NULL);
	if (NULL == slide_root)
	{
		display_warning("Error ED22: Error creating the slides container.");
		return;
	}

	// Add the slide data to the XML structure
	slides = g_list_first(slides);
	g_list_foreach(slides, menu_file_save_slide, slide_root);

	// Create a saving context
	save_context = xmlSaveToFilename(file_name->str, "utf8", 1);  // XML_SAVE_FORMAT == 1

	// Flush the saving context
	tmp_long = xmlSaveDoc(save_context, document_pointer);

	// Close the saving context
	tmp_int = xmlSaveClose(save_context);

	// Clear the changes made variable
	changes_made = FALSE;

	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(tmp_gstring, " Project saved - %s", file_name->str);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// * Function clean up area *

	// Free the memory allocated in this function
//	g_free(filename);
//	g_string_free(validated_string, TRUE);
	g_string_free(tmp_gstring, TRUE);
}

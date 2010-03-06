/*
 * $Id$
 *
 * Salasaga: Function called when the user selects File -> Save from the top menu
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

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../../config.h"
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../validate_value.h"
#include "../dialog/display_warning.h"
#include "../save/save_slide.h"
#include "../save/save_text_tags.h"
#include "menu_file_save_as.h"


void menu_file_save(void)
{
	// Local variables
	xmlDocPtr			document_pointer;			// Points to the XML document structure in memory
	GString				*message;					// Used to construct message strings
	xmlNodePtr			meta_pointer;				// Points to the meta-data node
	xmlNodePtr			pref_pointer;				// Points to the preferences node
	xmlNodePtr			root_node;					// Points to the root node
	xmlSaveCtxt			*save_context;				// Points to the save context
	xmlNodePtr			slide_root;					// Points to the root of the slide data
	xmlNodePtr			tag_root;					// Points to the root of the text tag data
	GtkTextIter			text_end;					// End position of text buffer
	GtkTextIter			text_start;					// Start position of text buffer
	GString				*tmp_gstring;				// Temporary GString
	gint				tmp_int;					// Temporary integer
	glong				tmp_long;					// Temporary long integer


	// If there's no project active, we just beep and return
	if (FALSE == get_project_active())
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
	message = g_string_new(NULL);
	tmp_gstring = g_string_new(NULL);

	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(tmp_gstring, " %s - %s", _("Saving project"), file_name->str);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_gstring->str);

	// Create an empty document pointer
	document_pointer = xmlNewDoc((const xmlChar *) "1.0");
	if (NULL == document_pointer)
	{
		g_string_printf(message, "%s ED19: %s", _("Error"), _("Error creating the XML save document."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

    // Create the root node
	root_node = xmlNewDocRawNode(document_pointer, NULL, (const xmlChar *) "salasaga_project", NULL);
	if (NULL == root_node)
	{
		g_string_printf(message, "%s ED21: %s", _("Error"), _("Error creating the root node."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Anchor the root node to the document
	xmlDocSetRootElement(document_pointer, root_node);

    // Create the meta information container
	meta_pointer = xmlNewChild(root_node, NULL, (const xmlChar *) "meta-data", NULL);
	if (NULL == meta_pointer)
	{
		g_string_printf(message, "%s ED25: %s", _("Error"), _("Error creating the meta-data container."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Add the save format version number to the XML document
	xmlNewChild(meta_pointer, NULL, (const xmlChar *) "save_format", (const xmlChar *) "5.0");

    // Create the preferences container
	pref_pointer = xmlNewChild(root_node, NULL, (const xmlChar *) "preferences", NULL);
	if (NULL == pref_pointer)
	{
		g_string_printf(message, "%s ED20: %s", _("Error"), _("Error creating the preferences container."));
		display_warning(message->str);
		g_string_free(message, TRUE);
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
	g_string_printf(tmp_gstring, "%u", get_frames_per_second());
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "frames_per_second", (const xmlChar *) tmp_gstring->str);
	switch (get_start_behaviour())
	{
		case START_BEHAVIOUR_PAUSED:
			xmlNewChild(pref_pointer, NULL, (const xmlChar *) "start_behaviour", (const xmlChar *) "paused");
			break;

		case START_BEHAVIOUR_PLAY:
			xmlNewChild(pref_pointer, NULL, (const xmlChar *) "start_behaviour", (const xmlChar *) "play");
			break;

		default:
			g_string_printf(message, "%s ED281: %s", _("Error"), _("Error creating the start behavior value."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return;
	}
	switch (get_end_behaviour())
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
			g_string_printf(message, "%s ED278: %s", _("Error"), _("Error creating the end behavior value."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return;
	}
	if (TRUE == get_show_control_bar())
	{
		xmlNewChild(pref_pointer, NULL, (const xmlChar *) "show_control_bar", (const xmlChar *) "true");
	} else
	{
		xmlNewChild(pref_pointer, NULL, (const xmlChar *) "show_control_bar", (const xmlChar *) "false");
	}
	if (TRUE == get_info_display())
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

    // Create a container for all of the text tags
	tag_root = xmlNewChild(root_node, NULL, (const xmlChar *) "text-tags", NULL);
	if (NULL == tag_root)
	{
		g_string_printf(message, "%s ED434: %s", _("Error"), _("Error creating the text tag container."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Add the text tags to the project file
	save_text_tags(tag_root);

    // Create a container for the slides
	slide_root = xmlNewChild(root_node, NULL, (const xmlChar *) "slides", NULL);
	if (NULL == slide_root)
	{
		g_string_printf(message, "%s ED22: %s", _("Error"), _("Error creating the slides container."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Update the status bar to indicate we're exporting the swf file
	g_string_printf(tmp_gstring, " %s - %s", _("Saving project"), file_name->str);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_gstring->str);

	// Add the slide data to the XML structure
	slides = g_list_first(slides);
	g_list_foreach(slides, save_slide, slide_root);

	// Create a saving context
	save_context = xmlSaveToFilename(file_name->str, "utf8", 1);  // XML_SAVE_FORMAT == 1

	// Flush the saving context
	tmp_long = xmlSaveDoc(save_context, document_pointer);

	// Close the saving context
	tmp_int = xmlSaveClose(save_context);

	// Clear the changes made variable
	set_changes_made(FALSE);

	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(tmp_gstring, " %s - %s", _("Project saved"), file_name->str);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), tmp_gstring->str);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);
	gdk_flush();

	// Change the title bar to include the file name
	g_string_printf(message, "%s v%s - %s", APP_NAME, APP_VERSION, g_path_get_basename(file_name->str));
	gtk_window_set_title(GTK_WINDOW(get_main_window()), message->str);

	// * Function clean up area *

	// Free the memory allocated in this function
	g_string_free(message, TRUE);
	g_string_free(tmp_gstring, TRUE);
}

/*
 * $Id$
 *
 * Salasaga: Function called when the user selects File -> Save from the top menu 
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

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "menu_file_save_slide.h"
#include "validate_value.h"


void menu_file_save(void)
{
	// Local variables
	GtkFileFilter		*all_filter;				// Filter for *.*
	gchar				*dir_name_part;				// Briefly used for holding a directory name
	gchar				*filename;					// Pointer to the chosen file name
	gchar				*file_name_part;			// Briefly used for holding a file name
	GtkFileFilter		*salasaga_filter;				// Filter for *.salasaga
	GtkWidget 			*save_dialog;				// Dialog widget
	gboolean			useable_input;				// Used to control loop flow
	GString				*validated_string;			// Receives known good strings from the validation function
	GtkWidget			*warn_dialog;				// Widget for overwrite warning dialog

	xmlDocPtr			document_pointer;			// Points to the XML document structure in memory
	xmlNodePtr			slide_root;					// Points to the root of the slide data
	xmlNodePtr			meta_pointer;				// Points to the meta-data node
	xmlNodePtr			pref_pointer;				// Points to the preferences node
	xmlNodePtr			root_node;					// Points to the root node
	xmlSaveCtxt			*save_context;				// Points to the save context

	GString				*tmp_gstring;				// Temporary GString
	gint				tmp_int;					// Temporary integer
	glong				tmp_long;					// Temporary long integer


	// Initialise some things
	tmp_gstring = g_string_new(NULL);

	// Create the dialog asking the user for the name to save as
	save_dialog = gtk_file_chooser_dialog_new("Save As",
											  GTK_WINDOW(main_window),
											  GTK_FILE_CHOOSER_ACTION_SAVE,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
											  NULL);

	// Create the filter so only *.salasaga files are displayed
	salasaga_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(salasaga_filter, "*.salasaga");
	gtk_file_filter_set_name(salasaga_filter, "Salasaga Project file (*.salasaga)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), salasaga_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, "All files (*.*)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), all_filter);

	// Set the path and name of the file to save as.  Use project_name as a default
	if (NULL != file_name)
	{
		// Work out the directory and file name components
		dir_name_part = g_path_get_dirname(file_name->str);
		file_name_part = g_path_get_basename(file_name->str);

		// Set the default directory and file name for the dialog		
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog), dir_name_part);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_dialog), file_name_part);

		// Free the string
		g_free(dir_name_part);
		g_free(file_name_part);
	} else
	{
		// Nothing has been established, so use project_name
		g_string_printf(tmp_gstring, "%s.salasaga", project_name->str);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_dialog), tmp_gstring->str);

		// Change to the default project directory
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog), default_project_folder->str);
	}

	// Loop around until we have a valid filename or the user cancels out
	useable_input = FALSE;
	validated_string = NULL;
	do
	{
		// Get a filename to save as
		if (gtk_dialog_run(GTK_DIALOG(save_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so free the memory allocated in this function, destroy the dialog, and return to the caller
			g_string_free(tmp_gstring, TRUE);
			gtk_widget_destroy(save_dialog);
			return;
		}

		// Retrieve the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_dialog));

		// Free the validated_string variable before recreating it
		if (NULL != validated_string)
		{
			g_string_free(validated_string, TRUE);
			validated_string = NULL;
		}

		// Validate the filename input
		validated_string = validate_value(FILE_PATH, V_CHAR, filename);
		if (NULL == validated_string)
		{
			// Invalid file name
			display_warning("Error ED125: There was something wrong with the file name given.  Please try again.");
		} else
		{
			// * Valid file name, so check if there's an existing file of this name, and give an Overwrite? type prompt if there is
			if (TRUE == g_file_test(validated_string->str, G_FILE_TEST_EXISTS))
			{
				// Something with this name already exists
				warn_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
									GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_QUESTION,
									GTK_BUTTONS_YES_NO,
									"Overwrite existing file?");
				if (GTK_RESPONSE_YES == gtk_dialog_run(GTK_DIALOG(warn_dialog)))
				{
					// We've been told to overwrite the existing file
					useable_input = TRUE;
				}
				gtk_widget_destroy(warn_dialog);
			} else
			{
				// The indicated file name is unique, we're fine to save
				useable_input = TRUE;
			}
		}
	} while (FALSE == useable_input);

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(save_dialog);

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
	xmlNewChild(meta_pointer, NULL, (const xmlChar *) "save_format", (const xmlChar *) "3.1");

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
	g_string_printf(tmp_gstring, "%u", slide_length);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "slide_length", (const xmlChar *) tmp_gstring->str);
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
	save_context = xmlSaveToFilename(validated_string->str, "utf8", 1);  // XML_SAVE_FORMAT == 1

	// Flush the saving context
	tmp_long = xmlSaveDoc(save_context, document_pointer);

	// Close the saving context
	tmp_int = xmlSaveClose(save_context);

	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(tmp_gstring, " Project saved - %s", validated_string->str);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// Keep the full file name around for future reference
	if (NULL == file_name)
	{
		file_name = g_string_new(NULL);
	}
	file_name = g_string_assign(file_name, validated_string->str);

	// * Function clean up area *

	// Free the memory allocated in this function
	g_free(filename);
	g_string_free(validated_string, TRUE);
	g_string_free(tmp_gstring, TRUE);
}

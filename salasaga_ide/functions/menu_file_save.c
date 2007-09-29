/*
 * $Id$
 *
 * Flame Project: Function called when the user selects File -> Save from the top menu 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void menu_file_save(void)
{
	// Local variables
	GtkFileFilter			*all_filter;				// Filter for *.*
	gchar				*dir_name_part;				// Briefly used for holding a directory name
	gchar				*filename;				// Pointer to the chosen file name
	gchar				*file_name_part;			// Briefly used for holding a file name
	GtkFileFilter			*flame_filter;				// Filter for *.flame
	GtkWidget 			*save_dialog;				// Dialog widget
	gboolean			unique_name;				// Switch used to mark when we have a valid filename
	GtkWidget			*warn_dialog;				// Widget for overwrite warning dialog

	xmlDocPtr			document_pointer;			// Points to the XML document structure in memory
	xmlNodePtr			slide_root;				// Points to the root of the slide data
	xmlNodePtr			meta_pointer;				// Points to the meta-data node
	xmlNodePtr			pref_pointer;				// Points to the preferences node
	xmlNodePtr			root_node;				// Points to the root node
	xmlSaveCtxt			*save_context;				// Points to the save context

	GString				*tmp_gstring;				// Temporary GString
	gint				tmp_int;				// Temporary integer
	glong				tmp_long;				// Temporary long integer


	// Initialise some things
	tmp_gstring = g_string_new(NULL);

	// Create the dialog asking the user for the name to save as
	save_dialog = gtk_file_chooser_dialog_new("Save As",
											  GTK_WINDOW(main_window),
											  GTK_FILE_CHOOSER_ACTION_SAVE,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
											  NULL);

	// Create the filter so only *.flame files are displayed
	flame_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(flame_filter, "*.flame");
	gtk_file_filter_set_name(flame_filter, "Flame Project file (*.flame)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), flame_filter);

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
		g_string_printf(tmp_gstring, "%s.flame", project_name->str);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_dialog), tmp_gstring->str);

		// Change to the default project directory
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog), default_project_folder->str);
	}

	// Run the dialog and wait for user input
	unique_name = FALSE;
	while (TRUE != unique_name)
	{
		// Get a filename to save as
		if (gtk_dialog_run(GTK_DIALOG(save_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(save_dialog);
			return;
		}

		// Retrieve the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_dialog));

		// Check if there's an existing file of this name, and give an Overwrite? type prompt if there is
		if (TRUE == g_file_test(filename, G_FILE_TEST_EXISTS))
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
				unique_name = TRUE;
			}
			gtk_widget_destroy(warn_dialog);
		} else
		{
			// The indicated file name is unique, we're fine to save
			unique_name = TRUE;
		}
	}

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(save_dialog);

	// Create an empty document pointer
	document_pointer = xmlNewDoc((const xmlChar *) "1.0");
	if (NULL == document_pointer)
	{
		display_warning("ED19: Error creating the XML save document\n");
		return;
	}

    // Create the root node
	root_node = xmlNewDocRawNode(document_pointer, NULL, (const xmlChar *) "flame_project", NULL);
	if (NULL == root_node)
	{
		display_warning("ED21: Error creating the root node\n");
		return;
	}

	// Anchor the root node to the document
	xmlDocSetRootElement(document_pointer, root_node);

    // Create the meta information container
	meta_pointer = xmlNewChild(root_node, NULL, (const xmlChar *) "meta-data", NULL);
	if (NULL == meta_pointer)
	{
		display_warning("ED25: Error creating the meta-data container\n");
		return;
	}

	// Add the save format version number to the XML document
	xmlNewChild(meta_pointer, NULL, (const xmlChar *) "save_format", (const xmlChar *) "2.0");

    // Create the preferences container
	pref_pointer = xmlNewChild(root_node, NULL, (const xmlChar *) "preferences", NULL);
	if (NULL == pref_pointer)
	{
		display_warning("ED20: Error creating the preferences container\n");
		return;
	}

	// Add the preferences to the XML document
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "project_name", (const xmlChar *) project_name->str);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "output_folder", (const xmlChar *) output_folder->str);
	g_string_printf(tmp_gstring, "%u", output_width);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "output_width", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", output_height);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "output_height", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", output_quality);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "output_quality", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", project_width);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "project_width", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", project_height);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "project_height", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", slide_length);
	xmlNewChild(pref_pointer, NULL, (const xmlChar *) "slide_length", (const xmlChar *) tmp_gstring->str);

    // Create a container for the slides
	slide_root = xmlNewChild(root_node, NULL, (const xmlChar *) "slides", NULL);
	if (NULL == slide_root)
	{
		display_warning("ED22: Error creating the slides container\n");
		return;
	}

	// Add the slide data to the XML structure
	slides = g_list_first(slides);
	g_list_foreach(slides, menu_file_save_slide, slide_root);

	// Create a saving context
	save_context = xmlSaveToFilename(filename, "utf8", 1);  // XML_SAVE_FORMAT == 1

	// Flush the saving context
	tmp_long = xmlSaveDoc(save_context, document_pointer);

	// Close the saving context
	tmp_int = xmlSaveClose(save_context);

	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(tmp_gstring, "Saved project as '%s'.", filename);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// Keep the full file name around for future reference
	file_name = g_string_new(NULL);
	file_name = g_string_assign(file_name, filename);

	// * Function clean up area *

	// Frees the memory holding the file name
	g_free(filename);

	// Free the temporary gstring
	g_string_free(tmp_gstring, TRUE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

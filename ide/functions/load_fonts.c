/*
 * $Id$
 *
 * Salasaga: Loads all of the ttf fonts for use by the rendering engine
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Cairo include
#include <cairo/cairo-ft.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "dialog/display_warning.h"


gboolean load_fonts()
{
	// Local variables
	cairo_status_t		cairo_status;				// Receives return status from cairo functions
	FILE				*font_file;					// A file we load an fdb font shape from
	gchar				*font_name;					// Points to a font name (string) we're about to use
	gchar				*font_pathname;				// Full pathname to a font file to load is constructed in this
	guint				ft_error;					// Receives error codes from FreeType functions
	FT_Library			ft_library_handle;			// FreeType library handle
	guint 				loop_counter;				// A standard counter used in for loops
	GString				*message;					// Used to construct message strings


	// Initialisation
	message = g_string_new(NULL);

	// Initialise FreeType
	ft_error = FT_Init_FreeType(&ft_library_handle);
	if (ft_error)
	{
		g_string_printf(message, "%s ED421: %s", _("Error"), _("Failed to initialize FreeType."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}

	// * Load the font faces *
	for (loop_counter = 0; loop_counter <= FONT_DEJAVU_SERIF_I; loop_counter++)
	{
		switch (loop_counter)
		{
			case FONT_DEJAVU_SANS:
				font_name = "DejaVuSans";
				break;

			case FONT_DEJAVU_SANS_B:
				font_name = "DejaVuSans-Bold";
				break;

			case FONT_DEJAVU_SANS_B_O:
				font_name = "DejaVuSans-BoldOblique";
				break;

			case FONT_DEJAVU_SANS_C:
				font_name = "DejaVuSansCondensed";
				break;

			case FONT_DEJAVU_SANS_C_B:
				font_name = "DejaVuSansCondensed-Bold";
				break;

			case FONT_DEJAVU_SANS_C_B_O:
				font_name = "DejaVuSansCondensed-BoldOblique";
				break;

			case FONT_DEJAVU_SANS_C_O:
				font_name = "DejaVuSansCondensed-Oblique";
				break;

			case FONT_DEJAVU_SANS_EL:
				font_name = "DejaVuSans-ExtraLight";
				break;

			case FONT_DEJAVU_SANS_MONO:
				font_name = "DejaVuSansMono";
				break;

			case FONT_DEJAVU_SANS_MONO_B:
				font_name = "DejaVuSansMono-Bold";
				break;

			case FONT_DEJAVU_SANS_MONO_B_O:
				font_name = "DejaVuSansMono-BoldOblique";
				break;

			case FONT_DEJAVU_SANS_MONO_O:
				font_name = "DejaVuSansMono-Oblique";
				break;

			case FONT_DEJAVU_SANS_O:
				font_name = "DejaVuSans-Oblique";
				break;

			case FONT_DEJAVU_SERIF:
				font_name = "DejaVuSerif";
				break;

			case FONT_DEJAVU_SERIF_B:
				font_name = "DejaVuSerif-Bold";
				break;

			case FONT_DEJAVU_SERIF_B_I:
				font_name = "DejaVuSerif-BoldItalic";
				break;

			case FONT_DEJAVU_SERIF_C:
				font_name = "DejaVuSerifCondensed";
				break;

			case FONT_DEJAVU_SERIF_C_B:
				font_name = "DejaVuSerifCondensed-Bold";
				break;

			case FONT_DEJAVU_SERIF_C_B_I:
				font_name = "DejaVuSerifCondensed-BoldItalic";
				break;

			case FONT_DEJAVU_SERIF_C_I:
				font_name = "DejaVuSerifCondensed-Italic";
				break;

			case FONT_DEJAVU_SERIF_I:
				font_name = "DejaVuSerif-Italic";
				break;
		}

		// Create the full path to the ttf font we want
		g_string_printf(message, "%s.ttf", font_name);
		font_pathname = g_build_path(G_DIR_SEPARATOR_S, FONT_TTF_DIR, message->str, NULL);

		// Load the FreeType font face
		ft_error = FT_New_Face(ft_library_handle, font_pathname, 0, &ft_font_face[loop_counter]);
		if (FT_Err_Unknown_File_Format == ft_error)
		{
			g_string_printf(message, "%s ED422: %s '%s' %s", _("Error"), _("Font file"), font_pathname, _("is not in a format FreeType recognizes."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}
		else if (ft_error)
		{
			g_string_printf(message, "%s ED423: %s '%s' %s", _("Error"), _("Font file"), font_pathname, _("could not be opened by FreeType."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}

		// Load the font face into Cairo
		cairo_font_face[loop_counter] = cairo_ft_font_face_create_for_ft_face(ft_font_face[loop_counter], 0);

		// Check if the font face was successfully loaded
		cairo_status = cairo_font_face_status(cairo_font_face[loop_counter]);
		if (CAIRO_STATUS_SUCCESS != cairo_status)
		{
			message = g_string_new(NULL);
			g_string_printf(message, "%s", "cairo_ft_font_face_create_for_ft_face() gave an error");
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}

		// Create the full path to the fdb font we want
		g_string_printf(message, "%s.fdb", font_name);
		font_pathname = g_build_path(G_DIR_SEPARATOR_S, FONT_OUTLINE_DIR, message->str, NULL);

		// Open the fdb font face file
		font_file = fopen(font_pathname, "r");
		if (NULL == font_file)
		{
			// Something went wrong when loading the font file, so return
			g_string_printf(message, "%s ED380: %s '%s'", _("Error"), _("Something went wrong when opening the font file"), font_pathname);
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}

		fdb_font_object[loop_counter] = loadSWFFontFromFile(font_file);
		if (NULL == fdb_font_object[loop_counter])
		{
			// Something went wrong when loading the font file, so return
			g_string_printf(message, "%s ED96: %s: '%s'", _("Error"), _("Something went wrong when loading the font file"), font_pathname);
			display_warning(message->str);

			// Free the memory allocated in this function
			g_string_free(message, TRUE);
			g_free(font_pathname);

			return FALSE;
		}
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);

	return TRUE;
}

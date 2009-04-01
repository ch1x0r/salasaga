/*
 * $Id$
 *
 * Salasaga: Loads a given ttf font, for use by the rendering engine
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

// Turn on C99 compatibility
#define _ISOC99_SOURCE

// GTK includes
#include <gtk/gtk.h>

// Freetype includes
#include <ft2build.h>
#include FT_FREETYPE_H

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"


FT_Face load_font(FT_Face *ft_font_face, gchar *font_pathname)
{
	// Local variables
	guint				ft_error;					// Receives error codes from FreeType functions
	FT_Library			ft_library_handle;			// FreeType library handle
	GString				*message;					// Used to construct message strings


	// Initialisation
	message = g_string_new(NULL);

	// Initialise FreeType
	ft_error = FT_Init_FreeType(&ft_library_handle);
	if (ft_error)
	{
		g_string_printf(message, "%s ED421: %s", _("Error"), _("Failed to initialize FreeType."));
		display_warning(message->str);
		return FALSE;
	}

	// Load the font face
	ft_error = FT_New_Face(ft_library_handle, font_pathname, 0, ft_font_face);
	if (FT_Err_Unknown_File_Format == ft_error)
	{
		g_string_printf(message, "%s ED422: %s '%s' %s", _("Error"), _("Font file"), font_pathname, _("is not in a format FreeType recognizes."));
		display_warning(message->str);
		return FALSE;
	}
	else if (ft_error)
	{
		g_string_printf(message, "%s ED423: %s '%s' %s", _("Error"), _("Font file"), font_pathname, _("could not be opened by FreeType."));
		display_warning(message->str);
		return FALSE;
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);

	return ft_font_face;
}

/*
 * $Id$
 *
 * Salasaga: Include file for global variables
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

#ifndef __EXTERNS_H__
#define __EXTERNS_H__

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H

// Ming include
#include <ming.h>

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Variables defined elsewhere
extern GList				*current_slide;
extern SWFFont				fdb_font_object[FONT_COUNT];
extern GString				*file_name;
extern GtkTreeViewColumn	*film_strip_column;
extern GtkScrolledWindow	*film_strip_container;
extern GtkListStore			*film_strip_store;
extern GdkPixmap			*front_store;
extern FT_Face				ft_font_face[FONT_COUNT];
extern GString				*icon_extension;
extern GString				*icon_path;
extern GtkTextBuffer		*info_text;
extern GString				*last_folder;
extern GtkItemFactory		*menu_bar;
extern GtkTable				*message_bar;
extern GdkPixbuf			*mouse_ptr_pixbuf;
extern GString				*mouse_ptr_string;
extern GIOChannel			*output_file;
extern GtkComboBox			*resolution_selector;
extern GdkRectangle			resize_handles_rect[8];
extern GList				*slides;
extern GtkStatusIcon		*status_icon;
extern GSList				*text_tags_fg_colour_slist;
extern GtkTextTag			*text_tags_fonts[FONT_COUNT];
extern GSList				*text_tags_size_slist;
extern GtkTextTagTable		*text_tags_table;
extern GtkComboBox			*zoom_selector;

// Field descriptions used for bounds and validation
extern validatation_entry	valid_fields[];

extern gchar				*salasaga_font_names[FONT_COUNT];

#ifdef _WIN32
// Windows only variables
extern HHOOK				win32_keyboard_hook_handle;
#endif

// Include the global functions
#include "functions/global_functions.h"


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EXTERNS_H__

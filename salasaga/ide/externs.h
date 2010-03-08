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
extern GList				*slides;

// Field descriptions used for bounds and validation
extern validatation_entry	valid_fields[];

extern gchar				*salasaga_font_names[FONT_COUNT];

#ifdef _WIN32
// Windows only global variable
extern HHOOK				win32_keyboard_hook_handle;
#endif

// Include the global functions
#include "functions/global_functions.h"


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EXTERNS_H__

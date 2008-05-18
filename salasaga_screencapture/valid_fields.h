/*
 * $Id$
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

#ifndef VALIDATE_FIELDS_H_
#define VALIDATE_FIELDS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// The validation entries for each input field in the application
validatation_entry		valid_fields[] =
{
	{ FOLDER_PATH,			"Full path to a folder",				V_CHAR,				V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ PROJECT_NAME,			"Name of this project",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		20 },
	{ SCREENSHOT_HEIGHT,	"Height for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_WIDTH,		"Width for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_X_OFFSET,	"X offset for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_Y_OFFSET,	"Y offset for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 }
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*VALIDATE_FIELDS_H_*/

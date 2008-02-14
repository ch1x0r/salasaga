/*
 * $Id$
 *
 * Copyright (C) 2008 Justin Clift <justin@postgresql.org>
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

#ifndef VALIDATE_FIELDS_H_
#define VALIDATE_FIELDS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// The validation entries for each input field in the application
validatation_entry		valid_fields[] =
{
	{ COLOUR_COMP8,			"8 Bit Colour Value Component",			V_INT_UNSIGNED,		V_NONE,		0,		255 },
	{ COLOUR_COMP16, 		"16 Bit Colour Value Component",		V_INT_UNSIGNED,		V_NONE,		0,		65535 },
	{ PROJECT_FPS,			"Frames per Second for this project",	V_INT_UNSIGNED,		V_NONE,		0,		120 },
	{ PROJECT_HEIGHT,		"Height of this project",				V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ PROJECT_NAME,			"Name of this project",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		255 },
	{ PROJECT_WIDTH,		"Width of this project",				V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_FOLDER,	"Folder for screenshots",				V_CHAR,				V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		255 },
	{ SCREENSHOT_HEIGHT,	"Height for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_WIDTH,		"Width for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_X_OFFSET,	"X offset for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_Y_OFFSET,	"Y offset for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 }
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*VALIDATE_FIELDS_H_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2008/02/14 13:37:10  vapour
 * Added new header file, with the field names and their capabilities for validation in the application.
 *
 */

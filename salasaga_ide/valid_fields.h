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
	{ FOLDER_PATH,			"Full path to a folder",				V_CHAR,				V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ ICON_HEIGHT,			"Height of the tool bar icons",			V_INT_UNSIGNED,		V_NONE,		0,		80 },
	{ PREVIEW_WIDTH,		"Width of film strip thumbnails",		V_INT_UNSIGNED,		V_NONE,		0,		800 },
	{ PROJECT_FPS,			"Frames per Second for this project",	V_INT_UNSIGNED,		V_NONE,		0,		120 },
	{ PROJECT_HEIGHT,		"Height of this project",				V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ PROJECT_NAME,			"Name of this project",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		20 },
	{ PROJECT_PATH,			"Full path to a project file",			V_CHAR,				V_FULL_STOP | V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ PROJECT_WIDTH,		"Width of this project",				V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ RESOLUTION,			"An X * Y resolution value",			V_RESOLUTION,		V_NONE,		0,		16 },
	{ SCREENSHOT_HEIGHT,	"Height for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_WIDTH,		"Width for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_X_OFFSET,	"X offset for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_Y_OFFSET,	"Y offset for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SLIDE_LENGTH,			"Number of frames per slide",			V_INT_UNSIGNED,		V_NONE,		0,		1200 },
	{ SLIDE_NAME,			"Name of this slide",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		20 },
	{ ZOOM_LEVEL,			"Magnification level of working area",	V_ZOOM,				V_NONE,		0,		12 }
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
 * Revision 1.6  2008/02/20 09:09:18  vapour
 * Added the slide name validation field type.
 *
 * Revision 1.5  2008/02/19 12:34:41  vapour
 * Added validation field entries for icon height, film strip width, default slide length, and default zoom level.
 *
 * Revision 1.4  2008/02/19 06:38:24  vapour
 * Added a new resolution field for validation, renamed the screenshot folder field to a more generic folder type of field.
 *
 * Revision 1.3  2008/02/18 07:06:04  vapour
 * Added project path validation entry.
 *
 * Revision 1.2  2008/02/14 16:57:45  vapour
 * Shortened project name to 20 characters wide.
 *
 * Revision 1.1  2008/02/14 13:37:10  vapour
 * Added new header file, with the field names and their capabilities for validation in the application.
 *
 */

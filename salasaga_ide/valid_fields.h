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
	{ END_BEHAVIOUR, 		"Animation behaviour after playing",	V_CHAR,				V_UNDERSCORES,		0,		9 },
	{ EXTERNAL_LINK,		"External link",						V_CHAR,				V_HYPENS | V_PATH_SEP | V_PERCENT | V_SPACES | V_UNDERSCORES,		0,		200 },
	{ EXTERNAL_LINK_WINDOW,	"Window target for an external link",	V_CHAR,				V_UNDERSCORES,		0,		20 },
	{ FILE_PATH,			"Full path to a file",					V_CHAR,				V_FULL_STOP | V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ FOLDER_PATH,			"Full path to a folder",				V_CHAR,				V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ FONT_SIZE,			"Font size",							V_FLOAT_UNSIGNED,	V_NONE,		0,		200 },
	{ FRAME_NUMBER,			"A frame number",						V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ HIGHLIGHT_HEIGHT,		"Height of a highlight layer",			V_INT_UNSIGNED,		V_NONE,		5,		4096 },
	{ HIGHLIGHT_WIDTH,		"Width of a highlight layer",			V_INT_UNSIGNED,		V_NONE,		5,		4096 },
	{ ICON_HEIGHT,			"Height of the tool bar icons",			V_INT_UNSIGNED,		V_NONE,		0,		80 },
	{ IMAGE_DATA,			"Base64 encoded image data",			V_CHAR,				V_EQUALS | V_FORWARD_SLASHES | V_NEW_LINES | V_PLUSES,		0,		-1 },
	{ IMAGE_DATA_LENGTH,	"Length in bytes of image data",		V_INT_UNSIGNED,		V_NONE,		0,		10485760 },
	{ LAYER_BACKGROUND,		"Layer background switch",				V_INT_UNSIGNED,		V_NONE,		0,		1 },
	{ LAYER_HEIGHT,			"Height of this layer",					V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ LAYER_NAME,			"Name of this layer",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		20 },
	{ LAYER_VISIBLE,		"Layer visibility switch",				V_INT_UNSIGNED,		V_NONE,		0,		1 },
	{ LAYER_WIDTH,			"Width of this layer",					V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ MOUSE_CLICK,			"Mouse click",							V_CHAR,				V_UNDERSCORES,		0,		8 },
	{ PREVIEW_WIDTH,		"Width of film strip thumbnails",		V_INT_UNSIGNED,		V_NONE,		0,		800 },
	{ PROJECT_FPS,			"Frames per Second for this project",	V_INT_UNSIGNED,		V_NONE,		0,		120 },
	{ PROJECT_HEIGHT,		"Height of this project",				V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ PROJECT_NAME,			"Name of this project",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		20 },
	{ PROJECT_VERSION,		"Version number of the project file",	V_FLOAT_UNSIGNED,	V_NONE,		0,		3.0 },
	{ PROJECT_WIDTH,		"Width of this project",				V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ RESOLUTION,			"An X * Y resolution value",			V_RESOLUTION,		V_NONE,		0,		16 },
	{ SCREENSHOT_HEIGHT,	"Height for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_WIDTH,		"Width for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_X_OFFSET,	"X offset for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SCREENSHOT_Y_OFFSET,	"Y offset for next screenshot",			V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ SLIDE_LENGTH,			"Number of frames per slide",			V_INT_UNSIGNED,		V_NONE,		0,		1200 },
	{ SLIDE_NAME,			"Name of this slide",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		20 },
	{ START_BEHAVIOUR, 		"Animation behaviour after loading",	V_CHAR,				V_UNDERSCORES,		0,		6 },
	{ X_OFFSET,				"Y Offset",								V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ Y_OFFSET,				"X Offset"			,					V_INT_UNSIGNED,		V_NONE,		0,		4096 },
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
 * Revision 1.16  2008/03/09 14:52:24  vapour
 * Added validation field details for start behaviour.
 *
 * Revision 1.15  2008/03/07 07:25:14  vapour
 * Added validation field entries for highlight width and highlight height.
 *
 * Revision 1.14  2008/03/06 00:14:27  vapour
 * Added validation field entry to support the new end behaviour project preference.
 *
 * Revision 1.13  2008/03/05 02:55:49  vapour
 * Updated the font size and mouse click field types.
 *
 * Revision 1.12  2008/03/04 11:35:08  vapour
 * Added mouse click validation field value.
 *
 * Revision 1.11  2008/03/04 11:05:45  vapour
 * Added field types for image data, image data length, and the layer background toggle.
 *
 * Revision 1.10  2008/03/04 09:54:14  vapour
 * Added layer visibility field.
 *
 * Revision 1.9  2008/03/04 08:50:09  vapour
 * Added the new project file format version validation field.
 *
 * Revision 1.8  2008/02/20 21:59:13  vapour
 * Added font size and layer name validation fields.
 *
 * Revision 1.7  2008/02/20 18:39:32  vapour
 * Added validation field values for the external link, external link target window, and x and y offsets.
 *
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

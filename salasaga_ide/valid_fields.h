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
	{ COLOUR_COMP8,			"8 Bit Colour Value Component",			V_INT_UNSIGNED,		V_NONE,		0,		255 },
	{ COLOUR_COMP16, 		"16 Bit Colour Value Component",		V_INT_UNSIGNED,		V_NONE,		0,		65535 },
	{ END_BEHAVIOUR, 		"Animation behaviour after playing",	V_CHAR,				V_UNDERSCORES,		0,		9 },
	{ EXTERNAL_LINK,		"External link",						V_CHAR,				V_AMPERSAND | V_AT | V_COLON | V_EQUALS | V_FORWARD_SLASHES | V_FULL_STOP | V_HYPENS | V_PATH_SEP | V_PERCENT | V_QUESTION | V_SPACES | V_UNDERSCORES,		0,		200 },
	{ EXTERNAL_LINK_WINDOW,	"Window target for an external link",	V_CHAR,				V_UNDERSCORES,		0,		20 },
	{ FILE_PATH,			"Full path to a file",					V_CHAR,				V_FULL_STOP | V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ FOLDER_PATH,			"Full path to a folder",				V_CHAR,				V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ FONT_SIZE,			"Font size",							V_FLOAT_UNSIGNED,	V_NONE,		0,		200 },
	{ FRAME_NUMBER,			"A frame number",						V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ HIGHLIGHT_HEIGHT,		"Height of a highlight layer",			V_INT_UNSIGNED,		V_NONE,		5,		4096 },
	{ HIGHLIGHT_WIDTH,		"Width of a highlight layer",			V_INT_UNSIGNED,		V_NONE,		5,		4096 },
	{ ICON_HEIGHT,			"Height of the tool bar icons",			V_INT_UNSIGNED,		V_NONE,		20,		80 },
	{ IMAGE_DATA,			"Base64 encoded image data",			V_CHAR,				V_EQUALS | V_FORWARD_SLASHES | V_NEW_LINES | V_PLUSES,		0,		-1 },
	{ IMAGE_DATA_LENGTH,	"Length in bytes of image data",		V_INT_UNSIGNED,		V_NONE,		200,	10485760 },
	{ LAYER_BACKGROUND,		"Layer background switch",				V_INT_UNSIGNED,		V_NONE,		0,		1 },
	{ LAYER_DURATION,		"Layer duration in seconds",			V_FLOAT_UNSIGNED,	V_NONE,		0,		300.0 },
	{ LAYER_HEIGHT,			"Height of this layer",					V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ LAYER_NAME,			"Name of this layer",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		20 },
	{ LAYER_VISIBLE,		"Layer visibility switch",				V_INT_UNSIGNED,		V_NONE,		0,		1 },
	{ LAYER_WIDTH,			"Width of this layer",					V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ LINE_WIDTH,			"Line width",							V_FLOAT_UNSIGNED,	V_NONE,		0.05,	20 },
	{ MOUSE_CLICK,			"Mouse click",							V_CHAR,				V_UNDERSCORES,		0,		13 },
	{ OPACITY,				"Opacity level",						V_FLOAT_UNSIGNED,	V_NONE,		0,		100 },
	{ PREVIEW_WIDTH,		"Width of film strip thumbnails",		V_INT_UNSIGNED,		V_NONE,		20,		800 },
	{ PROJECT_FPS,			"Frames per Second for this project",	V_INT_UNSIGNED,		V_NONE,		1,		120 },
	{ PROJECT_HEIGHT,		"Height of this project",				V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ PROJECT_NAME,			"Name of this project",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		120 },
	{ PROJECT_VERSION,		"Version number of the project file",	V_FLOAT_UNSIGNED,	V_NONE,		0,		4.2 },
	{ PROJECT_WIDTH,		"Width of this project",				V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ RESOLUTION,			"An X * Y resolution value",			V_RESOLUTION,		V_NONE,		0,		16 },
	{ SCREENSHOT_HEIGHT,	"Height for next screenshot",			V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ SCREENSHOT_WIDTH,		"Width for next screenshot",			V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ SHOW_CONTROL_BAR,		"Include control bar in exported swf",	V_CHAR,				V_NONE,		0,		5 },
	{ SHOW_INFO_BUTTON,		"Include info button in exported swf",	V_CHAR,				V_NONE,		0,		5 },
	{ SHOW_TEXT_BG,			"Show the background for text layers",	V_CHAR,				V_NONE,		0,		5 },
	{ SLIDE_LENGTH,			"Number of frames per slide",			V_INT_UNSIGNED,		V_NONE,		1,		1200 },
	{ SLIDE_DURATION,		"Number of seconds in a slide",			V_FLOAT_UNSIGNED,	V_NONE,		0.1,	300 },
	{ SLIDE_NAME,			"Name of this slide",					V_CHAR,				V_HYPENS | V_SPACES | V_UNDERSCORES,					0,		20 },
	{ START_BEHAVIOUR, 		"Animation behaviour after loading",	V_CHAR,				V_UNDERSCORES,		0,		6 },
	{ TRANSITION_DURATION,	"Transition duration in seconds",		V_FLOAT_UNSIGNED,	V_NONE,		0,		20 },
	{ TRANSITION_TYPE, 		"Type of transition",					V_CHAR,				V_UNDERSCORES,		0,		4 },
	{ X_OFFSET,				"X Offset",								V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ Y_OFFSET,				"Y Offset"			,					V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ ZOOM_LEVEL,			"Magnification level of working area",	V_ZOOM,				V_NONE,		0,		12 }
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*VALIDATE_FIELDS_H_*/

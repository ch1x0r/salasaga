/*
 * $Id$
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

#ifndef VALID_FIELDS_H_
#define VALID_FIELDS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// The validation entries for each input field in the application
validatation_entry		valid_fields[] =
{
	{ COLOUR_COMP8,			N_("8 Bit Color Value Component"),				V_INT_UNSIGNED,		V_NONE,		0,		255 },
	{ COLOUR_COMP16, 		N_("16 Bit Color Value Component"),				V_INT_UNSIGNED,		V_NONE,		0,		65535 },
	{ END_BEHAVIOUR, 		N_("Animation behavior after playing"),			V_CHAR,				V_UNDERSCORES,		0,		9 },
	{ EXTERNAL_LINK,		N_("External link"),							V_CHAR,				V_AMPERSAND | V_AT | V_COLON | V_EQUALS | V_FORWARD_SLASHES | V_FULL_STOP | V_HYPENS | V_PATH_SEP | V_PERCENT | V_QUESTION | V_SPACES | V_UNDERSCORES,		0,		200 },
	{ EXTERNAL_LINK_WINDOW,	N_("Window target for an external link"),		V_CHAR,				V_UNDERSCORES,		0,		20 },
	{ FILE_PATH,			N_("Full path to a file"),						V_CHAR,				V_ANY_UNICHAR | V_FULL_STOP | V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ FOLDER_PATH,			N_("Full path to a folder"),					V_CHAR,				V_ANY_UNICHAR | V_HYPENS | V_PATH_SEP | V_SPACES | V_UNDERSCORES,		0,		-1 },
	{ FONT_FACE,			N_("Font face"),								V_INT_UNSIGNED,		V_NONE,		0,		FONT_COUNT - 1 },
	{ FONT_SIZE,			N_("Font size"),								V_FLOAT_UNSIGNED,	V_NONE,		1.0,	140 },
	{ FRAME_NUMBER,			N_("A frame number"),							V_INT_UNSIGNED,		V_NONE,		0,		4096 },
	{ HIGHLIGHT_HEIGHT,		N_("Height of a highlight layer"),				V_INT_UNSIGNED,		V_NONE,		5,		4096 },
	{ HIGHLIGHT_WIDTH,		N_("Width of a highlight layer"),				V_INT_UNSIGNED,		V_NONE,		5,		4096 },
	{ ICON_HEIGHT,			N_("Height of the tool bar icons"),				V_INT_UNSIGNED,		V_NONE,		20,		80 },
	{ IMAGE_DATA,			N_("Base64 encoded image data"),				V_CHAR,				V_EQUALS | V_FORWARD_SLASHES | V_NEW_LINES | V_PLUSES,		0,		-1 },
	{ IMAGE_DATA_LENGTH,	N_("Length in bytes of image data"),			V_INT_UNSIGNED,		V_NONE,		200,	10485760 },
	{ LAYER_BACKGROUND,		N_("Layer background switch"),					V_INT_UNSIGNED,		V_NONE,		0,		1 },
	{ LAYER_DURATION,		N_("Layer duration in seconds"),				V_FLOAT_UNSIGNED,	V_NONE,		0,		120.0 },
	{ LAYER_HEIGHT,			N_("Height of this layer"),						V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ LAYER_NAME,			N_("Name of this layer"),						V_CHAR,				V_ANY_UNICHAR | V_COLON | V_FULL_STOP | V_HYPENS | V_SPACES | V_UNDERSCORES,	0,		20 },
	{ LAYER_VISIBLE,		N_("Layer visibility switch"),					V_INT_UNSIGNED,		V_NONE,		0,		1 },
	{ LAYER_WIDTH,			N_("Width of this layer"),						V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ LINE_WIDTH,			N_("Line width"),								V_FLOAT_UNSIGNED,	V_NONE,		0.00,	20 },
	{ MOUSE_CLICK,			N_("Mouse click"),								V_CHAR,				V_UNDERSCORES,		0,		13 },
	{ NUM_TEXT_CHARS,		N_("Characters in a text layer"),				V_INT_UNSIGNED,		V_NONE,		0,		2000 },
	{ OPACITY,				N_("Opacity level"),							V_FLOAT_UNSIGNED,	V_NONE,		0,		100 },
	{ PREVIEW_WIDTH,		N_("Width of film strip thumbnails"),			V_INT_UNSIGNED,		V_NONE,		20,		800 },
	{ PROJECT_FPS,			N_("Frames per Second for this project"),		V_INT_UNSIGNED,		V_NONE,		1,		120 },
	{ PROJECT_HEIGHT,		N_("Height of this project"),					V_INT_UNSIGNED,		V_NONE,		10,		4096 },
	{ PROJECT_NAME,			N_("Name of this project"),						V_CHAR,				V_ANY_UNICHAR | V_FULL_STOP | V_HYPENS | V_SPACES | V_UNDERSCORES,	0,		120 },
	{ PROJECT_VERSION,		N_("Version number of the project file"),		V_FLOAT_UNSIGNED,	V_NONE,		0,		5.0 },
	{ PROJECT_WIDTH,		N_("Width of this project"),					V_INT_UNSIGNED,		V_NONE,		10,		4096 },
	{ RESOLUTION,			N_("An X * Y resolution value"),				V_RESOLUTION,		V_NONE,		0,		30 },
	{ SCREENSHOT_DELAY,		N_("Number of seconds to delay a screenshot"),	V_INT_UNSIGNED,		V_NONE,		0,		60 },
	{ SCREENSHOT_HEIGHT,	N_("Height for next screenshot"),				V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ SCREENSHOT_WIDTH,		N_("Width for next screenshot"),				V_INT_UNSIGNED,		V_NONE,		1,		4096 },
	{ SHOW_CONTROL_BAR,		N_("Include control bar in exported swf"),		V_CHAR,				V_NONE,		0,		5 },
	{ SHOW_INFO_BUTTON,		N_("Include info button in exported swf"),		V_CHAR,				V_NONE,		0,		5 },
	{ SHOW_TEXT_BG,			N_("Show the background for text layers"),		V_CHAR,				V_NONE,		0,		5 },
	{ SLIDE_LENGTH,			N_("Number of frames per slide"),				V_INT_UNSIGNED,		V_NONE,		1,		1200 },
	{ SLIDE_DURATION,		N_("Number of seconds in a slide"),				V_FLOAT_UNSIGNED,	V_NONE,		0.1,	300.0 },
	{ SLIDE_NAME,			N_("Name of this slide"),						V_CHAR,				V_ANY_UNICHAR | V_HYPENS | V_SPACES | V_UNDERSCORES,	0,		20 },
	{ START_BEHAVIOUR, 		N_("Animation behavior after loading"),			V_CHAR,				V_UNDERSCORES,		0,		6 },
	{ TEXT_DATA,			N_("Base64 encoded text buffer data"),			V_CHAR,				V_EQUALS | V_FORWARD_SLASHES | V_NEW_LINES | V_PLUSES,		0,		-1 },
	{ TRANSITION_DURATION,	N_("Transition duration in seconds"),			V_FLOAT_UNSIGNED,	V_NONE,		0,		5 },
	{ TRANSITION_TYPE, 		N_("Type of transition"),						V_CHAR,				V_UNDERSCORES,		0,		4 },
	{ X_OFFSET,				N_("X Offset"),									V_INT_SIGNED,		V_HYPENS,	-4096,	4096 },
	{ Y_OFFSET,				N_("Y Offset"),									V_INT_SIGNED,		V_HYPENS,	-4096,	4096 },
	{ ZOOM_LEVEL,			N_("Magnification level of working area"),		V_ZOOM,				V_NONE,		0,		30 }
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*VALID_FIELDS_H_*/

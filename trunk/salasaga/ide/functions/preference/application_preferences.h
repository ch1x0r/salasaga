/*
 * $Id$
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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

#ifndef APPLICATION_PREFERENCES_
#define APPLICATION_PREFERENCES_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


GdkColor *get_default_bg_colour();
guint32 get_default_bg_colour_pixel();
guint16 get_default_bg_colour_red();
guint16 get_default_bg_colour_green();
guint16 get_default_bg_colour_blue();
guint get_default_fps();
gfloat get_default_layer_duration();
gchar *get_default_output_folder();
gsize get_default_output_folder_length();
guint get_default_output_height();
guint get_default_output_width();
gchar *get_default_project_folder();
gsize get_default_project_folder_length();
gfloat get_default_slide_duration();
gchar *get_default_zoom_level();
gsize get_default_zoom_level_length();
guint get_icon_height();
guint get_preview_width();
gchar *get_screenshots_folder();
gsize get_screenshots_folder_length();
gchar *get_default_browser_folder();
void set_default_bg_colour(GdkColor *new_default_bg_colour);
void set_default_bg_colour_pixel(guint32 new_pixel);
void set_default_bg_colour_red(guint16 new_red);
void set_default_bg_colour_green(guint16 new_green);
void set_default_bg_colour_blue(guint16 new_blue);
void set_default_fps(guint new_default_fps);
void set_default_layer_duration(gfloat new_default_layer_duration);
void set_default_output_folder(gchar *new_default_output_folder);
void set_default_output_height(guint new_default_output_height);
void set_default_output_width(guint new_default_output_width);
void set_default_project_folder(gchar *new_default_project_folder);
void set_default_slide_duration(gfloat new_default_slide_duration);
void set_default_zoom_level(gchar *new_default_zoom_level);
void set_icon_height(guint new_icon_height);
void set_preview_width(guint new_preview_width);
void set_screenshots_folder(gchar *new_screenshots_folder);
void set_default_browser_folder(gchar* default_browser_path);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* APPLICATION_PREFERENCES_ */

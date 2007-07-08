/*
 * $Id$
 *
 * Flame Project: Header file for all general backend functions
 * 
 * Copyright (C) 2006 Justin Clift <justin@postgresql.org>
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

#ifndef __BACKEND_H__
#define __BACKEND_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Functions included in this file and its corresponding source file
GString *base64_decode(GString *input_string, GString *output_string);
gboolean base64_encode(gpointer data, guint length, gchar ** output_string);
void calculate_object_boundaries(void);
void destroy_slide(gpointer element, gpointer user_data);
GList *detect_collisions(GList *collision_list, gdouble mouse_x, gdouble mouse_y);
gboolean flame_read(gchar *filename);
void logger_simple(const gchar *str);
void logger_with_domain(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);
void menu_enable(const gchar *full_path, gboolean enable);
GByteArray *menu_export_flash_inner(GByteArray *swf_buffer);
void menu_export_svg_animation_slide(gpointer element, gpointer user_data);
void menu_file_save_layer(gpointer element, gpointer user_data);
void menu_file_save_slide(gpointer element, gpointer user_data);
void regenerate_film_strip_thumbnails(void);
void save_preferences_and_exit(void);
void sound_beep(void);
GString *uri_decode(GString *input_string, GString *output_string);
gboolean uri_encode_base64(gpointer data, guint length, gchar **output_string);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __BACKEND_H__


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.11  2007/07/08 14:38:21  vapour
 * Added a function to regenerate the film strip thumbnails when the film strip seperator handle is moved.
 *
 * Revision 1.10  2006/09/12 12:13:28  vapour
 * Replaced old ming code with initial thoughts, in comments, for how to process slides.  Started adding stub GByteArray code too.
 *
 * Revision 1.9  2006/08/07 16:40:38  vapour
 * Added initial working functions to load embedded image data from project files.
 *
 * Revision 1.8  2006/06/12 03:47:09  vapour
 * Added functions for capturing log output.
 *
 * Revision 1.7  2006/06/07 15:21:09  vapour
 * Created a function for freeing the resources allocated to a slide.
 *
 * Revision 1.6  2006/04/29 19:10:04  vapour
 * Added a function to URI encode Base64 data.
 *
 * Revision 1.5  2006/04/29 17:03:59  vapour
 * Added the base64_encode function declaration.
 *
 * Revision 1.4  2006/04/26 18:29:57  vapour
 * Removed the flame_write function.
 *
 * Revision 1.3  2006/04/21 17:45:31  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.2  2006/04/16 06:06:14  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

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
void calculate_object_boundaries(void);
GList *detect_collisions(GList *collision_list, gdouble mouse_x, gdouble mouse_y);
gboolean flame_read(gchar *filename);
void menu_enable(const gchar *full_path, gboolean enable);
void menu_export_flash_inner(gchar *file_name, guint start_slide, guint finish_slide);
void menu_export_svg_animation_slide(gpointer element, gpointer user_data);
void menu_file_save_layer(gpointer element, gpointer user_data);
void menu_file_save_slide(gpointer element, gpointer user_data);
void save_preferences_and_exit(void);
void sound_beep(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __BACKEND_H__


/*
 * History
 * +++++++
 * 
 * $Log$
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

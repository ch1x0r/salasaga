/*
 * $Id$
 * 
 * Flame Project: Header file for all general backend functions
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2006/04/16 06:06:14  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
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
gboolean flame_write(gchar *filename);
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

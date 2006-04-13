/*
 * $Id$
 * 
 * Flame Project: Header file for all general backend functions
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 *
 * Revision 1.9  2006/04/13 14:57:47  jc
 * Added a (no code) function stub for sound_beep.
 *
 * Revision 1.8  2006/04/05 17:20:56  jc
 * Added a menu_enable function, to disable/greyscale and enable menu items.
 *
 * Revision 1.7  2006/03/26 23:43:58  jc
 * Added a definition for the initial flash export inner function.  Likely to be changed significantly.
 *
 * Revision 1.6  2006/03/24 03:17:48  jc
 * Moved the calculation of layer object boundaries into its own function.
 *
 * Revision 1.5  2006/03/23 07:15:51  jc
 * Adding further collision detection callbacks and code.
 *
 * Revision 1.4  2006/03/12 10:12:57  jc
 * Updated to save application defaults via GConf at shutdown, and restore them at startup.
 *
 * Revision 1.3  2006/03/12 04:17:17  jc
 * Added a function to save the slide layers.
 *
 * Revision 1.2  2006/03/09 14:28:45  jc
 * + Began adding code for parsing in an XML .flame file.
 * + Added a empty stub backend function for saving an XML .flame file.
 *
 * Revision 1.1  2006/03/04 11:52:09  jc
 * Started moving backend functions into new source backend.c and header backend.h files.
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

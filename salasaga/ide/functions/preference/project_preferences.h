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

#ifndef PROJECT_PREFERENCES_
#define PROJECT_PREFERENCES_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


gchar *get_info_link();
gsize get_info_link_length();
gchar *get_info_link_target();
gsize get_info_link_target_length();
gchar *get_output_folder();
gsize get_output_folder_length();
guint get_output_height();
guint get_output_width();
gchar *get_project_folder();
gsize get_project_folder_length();
guint get_project_height();
gchar *get_project_name();
gsize get_project_name_length();
guint get_project_width();
void set_info_link(gchar *new_info_link);
void set_info_link_target(gchar *new_info_link_target);
void set_output_folder(gchar *new_output_folder);
void set_output_height(guint new_output_height);
void set_output_width(guint new_output_width);
void set_project_folder(gchar *new_project_folder);
void set_project_height(guint new_project_height);
void set_project_name(gchar *new_project_name);
void set_project_width(guint new_project_width);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* PROJECT_PREFERENCES_ */

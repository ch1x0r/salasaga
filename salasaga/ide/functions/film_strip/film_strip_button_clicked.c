/*
 * $Id$
 *
 * Salasaga: Callback function to update the delete focus when the film strip is clicked
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../other/widget_focus.h"


gboolean film_strip_button_clicked(GtkWidget *widget, gpointer data)
{
	// Only do this function if we have a front store available and a project loaded
	if ((NULL == front_store) || (FALSE == get_project_active()))
	{
		return FALSE;
	}

	// Set the internal delete focus to be the film strip
	set_delete_focus(FOCUS_SLIDE);

	return FALSE;
}

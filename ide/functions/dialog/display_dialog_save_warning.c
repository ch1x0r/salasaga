/*
 * $Id$
 *
 * Salasaga: Displays a dialog box warning the user of unsaved changes
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../menu/menu_file_save.h"


gboolean display_dialog_save_warning(void)
{
	// Local variables
	gint				return_code_int;			// Status code returned from the dialog box
	GtkWidget			*tmp_dialog;				// Temporary dialog box


	// Display the warning dialog
	tmp_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _("Project has been modified.  Save changes?"));
	gtk_dialog_add_buttons(GTK_DIALOG(tmp_dialog),
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_NO, GTK_RESPONSE_NO,
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			NULL);

	return_code_int = gtk_dialog_run(GTK_DIALOG(tmp_dialog));
	gtk_widget_destroy(tmp_dialog);

	// If the user wants to save then do so
	if (GTK_RESPONSE_YES == return_code_int)
	{
		menu_file_save();
		changes_made = FALSE;
	}

	// If the user wants to cancel, then indicate this
	if (GTK_RESPONSE_CANCEL == return_code_int)
	{
		return FALSE;
	}

	// The user is ok to proceed
	return TRUE;
}

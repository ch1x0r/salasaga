/*
 * $Id$
 *
 * Flame Project: Writes the given number of bits to a swf output stream 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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


// GTK includes
#include <gtk/gtk.h>

// Flame Edit includes
#include "../../../flame-types.h"
#include "../../../externs.h"


GByteArray *flash_write_bits(GByteArray *swf_output, GByteArray *swf_input, guint num_bits)
{
	// Local variables

	return NULL;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/31 11:43:32  vapour
 * Fixed a bug in the paths to the include files.  Stub function now compiles.
 *
 * Revision 1.1  2007/10/30 09:53:10  vapour
 * Added an initial stub function for writing a given number of bits to a swf output stream.
 *
 */

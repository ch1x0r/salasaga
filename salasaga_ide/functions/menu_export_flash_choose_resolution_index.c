/*
 * $Id$
 *
 * Salasaga: 
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"

gint menu_export_flash_choose_resolution_index()
{
	// Local variables
	switch (output_width)
	{
		case 1920:
			
			if (1200 == output_height)
			{
				return 0;
			}
			if (1080 == output_height)
			{
				return 1;
			}

			// We're using an unknown output resolution
			return -1;

		case 1600:

			if (1200 == output_height)
			{
				return 2;
			}

			// We're using an unknown output resolution
			return -1;

		case 1280:
			
			if (1024 == output_height)
			{
				return 3;
			}
			if (720 == output_height)
			{
				return 4;
			}

			// We're using an unknown output resolution
			return -1;

		case 1024:

			if (768 == output_height)
			{
				return 5;
			}

			// We're using an unknown output resolution
			return -1;

		case 800:

			if (600 == output_height)
			{
				return 6;
			}

			// We're using an unknown output resolution
			return -1;

		case 720:

			if (480 == output_height)
			{
				return 7;
			}

			// We're using an unknown output resolution
			return -1;

		case 640:

			if (480 == output_height)
			{
				return 8;
			}

			// We're using an unknown output resolution
			return -1;

		case 352:

			if (288 == output_height)
			{
				return 9;
			}

			// We're using an unknown output resolution
			return -1;

		case 320:

			if (240 == output_height)
			{
				return 10;
			}

			// We're using an unknown output resolution
			return -1;

		case 176:

			if (144 == output_height)
			{
				return 11;
			}

			// We're using an unknown output resolution
			return -1;

		case 160:

			if (120 == output_height)
			{
				return 12;
			}

			// We're using an unknown output resolution
			return -1;

		case 128:

			if (96 == output_height)
			{
				return 13;
			}

			// We're using an unknown output resolution
			return -1;

		default:

			// We're using an unknown output resolution
			return -1;
	}
}

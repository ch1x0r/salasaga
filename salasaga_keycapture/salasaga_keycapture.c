/*
 * $Id$
 *
 * Flame Project: Source file for the Control-Printscreen key capturing dll
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

#include <stdio.h>
#include <windows.h>
#include "flame-keycapture.h"

#ifdef _WIN32

// Function to trigger flame-capture whenever the Control-Printscreen key combination is pressed
EXPORT LRESULT win32_keyboard_press_hook(int hook_code, WPARAM message_id, LPARAM message_data_ptr)
{
	// Local variables
	SHORT				key_state = 0;
	KBDLLHOOKSTRUCT		*message_data_pointer;


	// Simplify pointer name and type
	message_data_pointer = (KBDLLHOOKSTRUCT *) message_data_ptr;

	// Check if we can expect a keyboard message
	if (HC_ACTION == hook_code)
	{
		// Values will be in message_id and message_data_pointer

		// Check for the Control-Printscreen key
		if (WM_KEYUP == message_id)
		{
			// * A keyboard key was released *

			// Is the released key the Print screen key?
			if (VK_SNAPSHOT == message_data_pointer->vkCode)
			{
				// Yes, the Print screen key was pressed, so check if the Control key was down as well
				key_state = GetAsyncKeyState(VK_CONTROL);
				if (0x8000 & key_state)
				{
					// * Yes, they've been pressed together *

					// Launch flame-capture in the background
					// Fixme1: Needs to be written
				}
			}
		}
	}

	// Check if CallNextHookEx must be called
	if (0 > hook_code)
	{
		return CallNextHookEx((HHOOK) NULL, hook_code, message_id, message_data_ptr);
	}

	// Pass the message along the chain
	return 0;
}
#endif


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/19 13:11:47  vapour
 * Initial version.  Detection of Control Printscreen works, but it still needs to launch flame-capture.
 *
 */

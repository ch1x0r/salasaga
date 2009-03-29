/*
 * $Id$
 *
 * Salasaga: Source file for the Control-Printscreen key capturing dll
 * 
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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

#ifdef _WIN32

#include <stdio.h>
#include <windows.h>
#include "salasaga_keycapture.h"

// Function to trigger salasaga_keycapture whenever the Control-Printscreen key combination is pressed
EXPORT LRESULT win32_keyboard_press_hook(int hook_code, WPARAM message_id, LPARAM message_data_ptr)
{
	// Local variables
	SHORT				key_state = 0;
	DWORD				last_error;
	LPVOID				message_buffer_pointer;
	KBDLLHOOKSTRUCT		*message_data_pointer;
	PROCESS_INFORMATION	process_info;
	BOOL				return_code_bool;
	STARTUPINFO			startup_info;
	TCHAR				text_buffer[120];


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
					// * Yes, they've been pressed together, so launch salasaga_keycapture in the background
					ZeroMemory(&process_info, sizeof(process_info));					
					ZeroMemory(&startup_info, sizeof(startup_info));
					startup_info.cb = sizeof(startup_info);
					return_code_bool = CreateProcess(
											TEXT("salasaga_keycapture.exe"),  // Application name
											NULL,  // Command line
											NULL,  // Process Attributes
											NULL,  // Thread attributes
											FALSE,  // Inherit handles?
											0,  // Process creation flags
											NULL,  // Environment
											NULL,  // Working directory
											&startup_info,  // Startup info
											&process_info);  // Process information
					if (0 == return_code_bool)
					{
						// Launch of salasaga_keycapture failed
						last_error = GetLastError();
						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, last_error,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &message_buffer_pointer, 0, NULL);
						wsprintf(text_buffer, "Launch of salasaga_keycapture failed with error %d: %s", last_error, message_buffer_pointer); 
						MessageBox(NULL, text_buffer, "Error", MB_OK);
						LocalFree(message_buffer_pointer);
					}
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

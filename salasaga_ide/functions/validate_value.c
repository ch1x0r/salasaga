/*
 * $Id$
 *
 * Flame Project: Function to validate a given value
 * 
 * Copyright (C) 2008 Justin Clift <justin@postgresql.org>
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


// Standard includes
#include <stdlib.h>
#include <string.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../valid_fields.h"
#include "../externs.h"
#include "display_warning.h"

void *validate_value(gint value_id, gint input_type, void *value)
{
	// Local variables
	guint				capabilities;
	guint				base_type;
	GString				*error_string;
	gchar				input_char;
	gboolean			match_found;
	GString				*output_gstring;
	guint				output_guint;
	guint				*output_guint_ptr;
	guint				string_counter;
	gint				string_length;
	guint				string_max;
	guint				string_min;
	guint				value_max;
	guint				value_min;


	// Initialise various things
	base_type = valid_fields[value_id].base_type;
	capabilities = valid_fields[value_id].capabilities;
	output_gstring = g_string_new(NULL);

	switch (base_type)
	{
		case V_BOOLEAN:

			break;

		case V_CHAR:

			// * We're validating a char or string *

			// We can only validate string input for this type of value
			if (V_CHAR != input_type)
			{
				return NULL;
			}

			// Get the length of the input string
			string_max = valid_fields[value_id].max_value;
			string_min = valid_fields[value_id].min_value;
			string_length = strlen((gchar *) value);

			// If the length of the string isn't in the acceptable range, return NULL
			if ((string_length < string_min) || (string_length > string_max))
				return NULL;

			// Sanitise each character of the input string
			for (string_counter = 0; string_counter < string_length; string_counter++)
			{
				input_char = ((gchar *) value)[string_counter]; 
				if (TRUE == g_ascii_isalnum(input_char))
				{
					output_gstring = g_string_append_c(output_gstring, input_char);
				}
				else
				{
					// * The input wasn't a standard alphanumic character, so check if it *
					// * is one of the characters in the capabilites list for this field *
					match_found = FALSE;
					if (TRUE == (V_SPACES && capabilities))
					{
						// This field is allowed to have spaces.  Is this character a space?
						if (0 == g_ascii_strncasecmp(" ", &input_char, 1))
						{
							// Yes, this is a space character
							output_gstring = g_string_append_c(output_gstring, input_char);
							match_found = TRUE;
							continue;
						}
					}
					if (TRUE == (V_HYPENS && capabilities))
					{
						// This field is allowed to have hypens.  Is this character a hyphen?
						if (0 == g_ascii_strncasecmp("-", &input_char, 1))
						{
							// Yes, this is a hypen character
							output_gstring = g_string_append_c(output_gstring, input_char);
							match_found = TRUE;
							continue;
						}
					}
					if (TRUE == (V_UNDERSCORES && capabilities))
					{
						// This field is allowed to have underscores.  Is this character an underscore?
						if (0 == g_ascii_strncasecmp("_", &input_char, 1))
						{
							// Yes, this is an underscore character
							output_gstring = g_string_append_c(output_gstring, input_char);
							match_found = TRUE;
							continue;
						}
					}
					if (TRUE == (V_PATH_SEP && capabilities))
					{
						// This field is allowed to have path seperator charachers ('/', '\') .  Is this character one of those?
						if ((0 == g_ascii_strncasecmp("/", &input_char, 1)) || (0 == g_ascii_strncasecmp("\\", &input_char, 1)))
						{
							// Yes, this is a path seperator character
							output_gstring = g_string_append_c(output_gstring, G_DIR_SEPARATOR);  // Output the OS correct version
							match_found = TRUE;
							continue;
						}
					}

					// The character we are checking is not in the list of valid inputs for this field
					if (FALSE == match_found)
					{
						g_string_free(output_gstring, TRUE);
						return NULL;
					}
				}
			}

			// Remove any leading and/or trailing white space
			output_gstring->str = g_strstrip(output_gstring->str);
			output_gstring->len = strlen(output_gstring->str);

			// Recheck the length of the output string
			if ((output_gstring->len < string_min) || (output_gstring->len > string_max))
			{
				g_string_free(output_gstring, TRUE);
				return NULL;
			}

			// The string seems to be valid, so return it for use
			return output_gstring;

			break;

		case V_FLOAT:

			break;

		case V_INT_SIGNED:

			// fixme2:
			// Check if the first character is a hypen
			
			break;

		case V_INT_UNSIGNED:

			// * We're validating an unsigned integer *

			// If we're working with string input, we need to convert it to an integer first
			if (V_CHAR == input_type)
			{
				// * We're working with string input *

				// Get the length of the input string
				string_length = strlen((gchar *) value);
	
				// Sanitise each character of the input string
				for (string_counter = 0; string_counter < string_length; string_counter++)
				{
					input_char = ((gchar *) value)[string_counter]; 
					if (TRUE == g_ascii_isdigit(input_char))
					{
						output_gstring = g_string_append_c(output_gstring, input_char);
					}
					else
					{
						// This wasn't a valid character
						g_string_free(output_gstring, TRUE);
						return NULL;
					}
				}
	
				// Convert the string to an integer
				output_guint = atoi(output_gstring->str);
			}
			else
			{
				// We're working with integer input, so just copy the value directly
				output_guint = *((guint *) value);
				//output_guint = (guint) ((guint *) value);
			}

			// Is the integer value within the defined bounds?
			value_max = valid_fields[value_id].max_value;
			value_min = valid_fields[value_id].min_value;
			if ((output_guint < value_min) || (output_guint > value_max))
			{
				// Value is out of bounds, so fail
				g_string_free(output_gstring, TRUE);
				return NULL;
			}

			// The value looks ok, so we copy it to newly allocated memory, to pass it back
			output_guint_ptr = g_try_new0(guint, 1);
			if (NULL == output_guint_ptr)
			{
				// Unable to allocate memory for the new value, so fail
				g_string_free(output_gstring, TRUE);
				return NULL;
			}
			*output_guint_ptr = output_guint;

			// Free the string memory allocated in this function
			g_string_free(output_gstring, TRUE);

			return output_guint_ptr;

			break;

		default:

			// Unknown value type, we should never get here
			error_string = g_string_new(NULL);
			g_string_printf(error_string, "Error ED119: Unknown value '%s' passed to validation function!", valid_fields[value_id].name_string);
			display_warning(error_string->str);
			g_string_free(error_string, TRUE);

			return NULL;
	}

	// If we get here, then something went wrong!
	return NULL;
}

/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/02/14 16:52:10  vapour
 * Updated validation function with an additional parameter for the type of input.  Also added working code to validate unsigned integers.
 *
 * Revision 1.2  2008/02/14 13:43:45  vapour
 * Updated to reference the new valid fields header file directly.
 *
 * Revision 1.1  2008/02/14 13:28:17  vapour
 * Added new function to validate input.  Only works on strings thus far.
 *
 */

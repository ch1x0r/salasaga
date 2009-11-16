/*
 * $Id$
 *
 * Salasaga: Function to validate a given value
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


// Standard includes
#include <stdlib.h>
#include <string.h>
#include <locale.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "salasaga_types.h"
#include "valid_fields.h"
#include "display_warning.h"

void *validate_value(gint value_id, gint input_type, void *value)
{
	// Local variables
	guint				base_type;
	guint				capabilities;
	gboolean			capability_check;
	gchar				*conversion_buffer;			// Used when converting between unicode character types
	gchar				*decimal_point;
	GString				*error_string;
	gchar				input_char;
	gunichar			input_char_unicode;
	gchar				*input_ptr;
	struct lconv		*locale_info;
	gboolean			match_found;
	gfloat				output_gfloat;
	gfloat				*output_gfloat_ptr;
	GString				*output_gstring;
	guint				output_guint;
	guint				*output_guint_ptr;
	guint				string_counter;
	gint				string_length;
	gint				string_max;
	guint				string_min;
	gfloat				value_max;
	gfloat				value_min;


	// Initialise various things
	base_type = valid_fields[value_id].base_type;
	capabilities = valid_fields[value_id].capabilities;
	input_ptr = (gchar *) value;
	output_gstring = g_string_new(NULL);
	locale_info = localeconv();
	decimal_point = locale_info->decimal_point;

	switch (base_type)
	{
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
			string_length = g_utf8_strlen(input_ptr, -1);

			// If the length of the string isn't in the acceptable range, return NULL
			if (string_length < string_min)
				return NULL;
			if ((string_length > string_max) && (-1 != string_max))  // -1 for string_max means "no maximum limit"
				return NULL;

			// Sanitise each character of the input string
			for (string_counter = 0; string_counter < string_length; string_counter++)
			{
				// Get the next character
				input_char_unicode = g_utf8_get_char_validated(input_ptr, -1);
				if ((gunichar)-1 == input_char_unicode)
				{
					// The returned character was not a valid unicode character, so we indicate failure
					return NULL;
				}
				if ((gunichar)-2 == input_char_unicode)
				{
					// The returned character was not a valid unicode character, so we indicate failure
					return NULL;
				}

				// Convert the character to UTF-8 so we can process it
				conversion_buffer = g_ucs4_to_utf8(&input_char_unicode, 1, NULL, NULL, NULL);

				// Determine which character we're examining
				if (TRUE == g_unichar_isalnum(input_char_unicode))
				{
					// It's a standard unicode alphanumeric character, so we accept it as is
					g_string_append_printf(output_gstring, "%s", conversion_buffer);
					input_ptr = g_utf8_find_next_char(input_ptr, NULL);
				}
				else
				{
					// * The input wasn't a standard alphanumic character, so check if it *
					// * is one of the characters in the capabilities list for this field *
					match_found = FALSE;
					capability_check = V_SPACES & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have spaces
						if (0 == g_strcmp0(" ", conversion_buffer))
						{
							// Yes, this is a space character
							match_found = TRUE;
							g_string_append_printf(output_gstring, "%s", conversion_buffer);
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_FULL_STOP & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have full stops
						if (0 == g_strcmp0(".", conversion_buffer))
						{
							// Yes, this is a full stop character
							match_found = TRUE;
							g_string_append_printf(output_gstring, "%s", conversion_buffer);
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_HYPENS & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have hypens
						if (0 == g_strcmp0("-", conversion_buffer))
						{
							// Yes, this is a hypen character
							match_found = TRUE;
							g_string_append_printf(output_gstring, "%s", conversion_buffer);
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_UNDERSCORES & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have underscores
						if (0 == g_strcmp0("_", conversion_buffer))
						{
							// Yes, this is an underscore character
							match_found = TRUE;
							g_string_append_printf(output_gstring, "%s", conversion_buffer);
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_PATH_SEP & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have path separator characters ('/', '\')
						if ((0 == g_strcmp0("/", conversion_buffer)) || (0 == g_strcmp0("\\", conversion_buffer)))
						{
							// Yes, this is a path separator character
							match_found = TRUE;
							output_gstring = g_string_append_c(output_gstring, G_DIR_SEPARATOR);  // Output the OS correct version
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_EQUALS & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have equals signs
						if (0 == g_strcmp0("=", conversion_buffer))
						{
							// Yes, this is an equals sign character
							match_found = TRUE;
							g_string_append_printf(output_gstring, "%s", conversion_buffer);
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_FORWARD_SLASHES & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have forward slashes
						if (0 == g_strcmp0("/", conversion_buffer))
						{
							// Yes, this is a forward slash character
							match_found = TRUE;
							g_string_append_printf(output_gstring, "%s", conversion_buffer);
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_NEW_LINES & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have new line characters
						if (0 == g_strcmp0("\n", conversion_buffer))
						{
							// Yes, this is a new line character
							match_found = TRUE;
							output_gstring = g_string_append_c(output_gstring, '\n');
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_PLUSES & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have pluses
						if (0 == g_strcmp0("+", conversion_buffer))
						{
							// Yes, this is a plus character
							match_found = TRUE;
							g_string_append_printf(output_gstring, "%s", conversion_buffer);
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}
					capability_check = V_PERCENT & capabilities;
					if (FALSE != capability_check)
					{
						// This field is allowed to have the percent sign
						if (0 == g_strcmp0("%", conversion_buffer))
						{
							// Yes, this is a percent sign
							match_found = TRUE;
							g_string_append_printf(output_gstring, "%s", conversion_buffer);
							input_ptr = g_utf8_find_next_char(input_ptr, NULL);
							continue;
						}
					}

					// The character we are checking is not in the list of valid inputs for this field
					if (FALSE == match_found)
					{
						g_free(conversion_buffer);
						g_string_free(output_gstring, TRUE);
						return NULL;
					}
				}
			}

			// Remove any leading and/or trailing white space
			output_gstring->str = g_strstrip(output_gstring->str);
			output_gstring->len = strlen(output_gstring->str);

			// Recheck the length of the output string
			if (output_gstring->len < string_min)
			{
				g_free(conversion_buffer);
				g_string_free(output_gstring, TRUE);
				return NULL;
			}
			if ((output_gstring->len > string_max) && (-1 != string_max))  // -1 for string_max means "no maximum limit"
			{
				g_free(conversion_buffer);
				g_string_free(output_gstring, TRUE);
				return NULL;
			}

			// Free the memory used so far
			g_free(conversion_buffer);

			// The string seems to be valid, so return it for use
			return output_gstring;

			break;

		case V_FLOAT_UNSIGNED:

			// * We're validating an unsigned float *

			// If we're working with string input, we need to convert it to a float first
			if (V_CHAR == input_type)
			{
				// * We're working with string input *

				// Get the length of the input string
				string_length = strlen((gchar *) value);
	
				// Sanitise each character of the input string
				for (string_counter = 0; string_counter < string_length; string_counter++)
				{
					input_char = ((gchar *) value)[string_counter];

					// Check for decimal digits
					if (TRUE == g_ascii_isdigit(input_char))
					{
						output_gstring = g_string_append_c(output_gstring, input_char);
					}
					else
					{
						// This field is allowed to have full stops.  Is this character a full stop?
						if (0 == g_ascii_strncasecmp(".", &input_char, 1))
						{
							// Yes, this is a full stop character
							output_gstring = g_string_append_c(output_gstring, *decimal_point);
							match_found = TRUE;
							continue;
						}

						// This field is allowed to have commas (equiv to full stop in some locales).  Is this character a comma?
						if (0 == g_ascii_strncasecmp(",", &input_char, 1))
						{
							// Yes, this is a comma character
							output_gstring = g_string_append_c(output_gstring, *decimal_point);
							match_found = TRUE;
							continue;
						}

						// The character we are checking is not in the list of valid inputs for this field
						if (FALSE == match_found)
						{
							g_string_free(output_gstring, TRUE);
							return NULL;
						}
					}
				}

				// Convert the string to a float
				output_gfloat = (gfloat) g_strtod(output_gstring->str, NULL);
			}
			else
			{
				// We're working with a float input, so just copy the value directly
				output_gfloat = *((gfloat *) value);
			}

			// Is the float value within the defined bounds?
			value_max = valid_fields[value_id].max_value;
			value_min = valid_fields[value_id].min_value;
			if ((output_gfloat < value_min) || (output_gfloat > value_max))
			{
				// Value is out of bounds, so fail
				g_string_free(output_gstring, TRUE);
				return NULL;
			}

			// The value looks ok, so we copy it to newly allocated memory, to pass it back
			output_gfloat_ptr = g_try_new0(gfloat, 1);
			if (NULL == output_gfloat_ptr)
			{
				// Unable to allocate memory for the new value, so fail
				g_string_free(output_gstring, TRUE);
				return NULL;
			}
			*output_gfloat_ptr = output_gfloat;

			// Free the string memory allocated in this function
			g_string_free(output_gstring, TRUE);

			return output_gfloat_ptr;

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

					// Check for decimal digits
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

		case V_RESOLUTION:

				// * We're working with a resolution (text string) input.  i.e. '1920x1200 pixels' *

				// Get the length of the input string
				string_length = strlen((gchar *) value);
				string_max = valid_fields[value_id].max_value;
				string_min = valid_fields[value_id].min_value;

				// If the length of the string isn't in the acceptable range, return NULL
				if ((string_length < string_min) || (string_length > string_max))
					return NULL;

				// Sanitise each character of the input string
				for (string_counter = 0; string_counter < string_length; string_counter++)
				{
					input_char = ((gchar *) value)[string_counter];

					// Check for decimal digits
					if (TRUE == g_ascii_isdigit(input_char))
					{
						output_gstring = g_string_append_c(output_gstring, input_char);
					}
					else
					{
						match_found = FALSE;
						// This field is allowed to have the ' ', 'p', 'i', 'x', 'e', 'l', 's' characters .  Is this character one of those?
						if (0 == g_ascii_strncasecmp(" ", &input_char, 1))
						{
							// Yes, this is a space character
							output_gstring = g_string_append_c(output_gstring, input_char);
							match_found = TRUE;
							continue;
						}
						if (0 == g_ascii_strncasecmp("p", &input_char, 1))
						{
							// Yes, this is a 'p' character
							output_gstring = g_string_append_c(output_gstring, 'p');
							match_found = TRUE;
							continue;
						}
						if (0 == g_ascii_strncasecmp("i", &input_char, 1))
						{
							// Yes, this is a 'i' character
							output_gstring = g_string_append_c(output_gstring, 'i');
							match_found = TRUE;
							continue;
						}
						if (0 == g_ascii_strncasecmp("x", &input_char, 1))
						{
							// Yes, this is a 'x' character
							output_gstring = g_string_append_c(output_gstring, 'x');
							match_found = TRUE;
							continue;
						}
						if (0 == g_ascii_strncasecmp("e", &input_char, 1))
						{
							// Yes, this is a 'e' character
							output_gstring = g_string_append_c(output_gstring, 'e');
							match_found = TRUE;
							continue;
						}
						if (0 == g_ascii_strncasecmp("l", &input_char, 1))
						{
							// Yes, this is a 'l' character
							output_gstring = g_string_append_c(output_gstring, 'l');
							match_found = TRUE;
							continue;
						}
						if (0 == g_ascii_strncasecmp("s", &input_char, 1))
						{
							// Yes, this is a 's' character
							output_gstring = g_string_append_c(output_gstring, 's');
							match_found = TRUE;
							continue;
						}
						if (FALSE == match_found)
						{
							// This wasn't a valid character
							g_string_free(output_gstring, TRUE);
							return NULL;
						}
					}
				}

				// Remove any leading and/or trailing white space
				output_gstring->str = g_strstrip(output_gstring->str);
				output_gstring->len = strlen(output_gstring->str);

				// Recheck the length of the output string
				if ((string_length < string_min) || (string_length > string_max))
					return NULL;

				// The string seems to be valid, so return it for use
				return output_gstring;

		case V_ZOOM:

			// * We're working with a zoom level.  i.e. "100%" or "Fit to width" *

			// Get the length of the input string
			string_length = g_utf8_strlen((gchar *) value, -1);
			string_max = valid_fields[value_id].max_value;
			string_min = valid_fields[value_id].min_value;

			// If the length of the string isn't in the acceptable range, return NULL
			if ((string_length < string_min) || (string_length > string_max))
				return NULL;

			// If the string is "Fit to width" or a localised version of it
			if (0 == g_strcmp0(_("Fit to width"), (gchar *) value))
			{
				// Yes, this is the "Fit to width" value
				output_gstring = g_string_assign(output_gstring, value);
				return output_gstring;
			}

			// * The incoming string isn't the "Fit to width" value, *
			// * so should only consist of decimal characters and '%' *

			// Sanitise each character of the input string
			for (string_counter = 0; string_counter < string_length; string_counter++)
			{
				input_char = ((gchar *) value)[string_counter];

				// Check for decimal digits
				if (TRUE == g_ascii_isdigit(input_char))
				{
					output_gstring = g_string_append_c(output_gstring, input_char);
					continue;
				}
				// Check for '%' character
				if (0 == g_ascii_strncasecmp("%", &input_char, 1))
				{
					// Yes, this is a '%' character
					output_gstring = g_string_append_c(output_gstring, '%');
					continue;
				}
				// This wasn't a valid character
				g_string_free(output_gstring, TRUE);
				return NULL;
			}

			return output_gstring;

		default:

			// Unknown value type, we should never get here
			error_string = g_string_new(NULL);
			g_string_printf(error_string, "%s CA28: %s '%s'", _("Error"), _("Unknown value passed to validation function"), valid_fields[value_id].name_string);
			display_warning(error_string->str);
			g_string_free(error_string, TRUE);

			return NULL;
	}

	// If we get here, then something went wrong!
	return NULL;
}

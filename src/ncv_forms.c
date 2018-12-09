/*
 * MIT License
 *
 * Copyright (c) 2018 dead-end
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ncv_common.h"

#include <string.h>
#include <ncursesw/form.h>

/***************************************************************************
 * Definition of the checkbox checked char.
 **************************************************************************/

#define CHECK_CHAR 'x'

#define CHECK_WCHAR L'x'

/***************************************************************************
 * The function is a simple wrapper around form_driver_w that provides error
 * handling.
 **************************************************************************/

void forms_driver(FORM *form, const int key_type, const wint_t chr) {

	const int result = form_driver_w(form, key_type, chr);

	//
	// The form driver returns E_REQUEST_DENIED if you what to write before
	// the start or after the end of the field.
	//
	if (result != E_OK && result != E_REQUEST_DENIED) {
		print_exit("forms_driver() Unable to process key request for key code: %d key: %lc result: %d\n", key_type, chr, result);
	}
}

/***************************************************************************
 * The function reads the input from the field and stores it in the buffer.
 * The buffers size has to include the terminating \0. The input string is
 * a multi byte string, which will be trimmed and converted to wchar_t.
 *
 * If the resulting string is longer than the buffer it is an error.
 **************************************************************************/

void forms_get_input_str(FIELD *field, wchar_t *buffer, const int buffer_size) {

	//
	// The first step is to get the raw field input data.
	//
	char *raw_field = field_buffer(field, 0);

	//
	// If the string contains multi byte chars, the length can be greater
	// than the buffer size.
	//
	size_t raw_len = strlen(raw_field);
	char raw_str[raw_len + 1];

	//
	// Create a copy of the field content that can be modified (trimmed).
	// The string has by construction the same size as the raw string, so
	// strncpy will add a \0
	//
	strncpy(raw_str, raw_field, raw_len);
	print_debug("forms_get_input_str() raw len: '%zu'\n", raw_len);

	//
	// The field content is filled with blanks, which had to be trimmed
	// before the conversion.
	//
	const char *trimed = trim(raw_str);

	//
	// Convert the trimmed input string to a wchar_t string. The buffer size
	// has to include the \0.
	//
	mbs_2_wchars(trimed, buffer, buffer_size);
}

/***************************************************************************
 * The functions returns true if the checkbox field is checked which means
 * that the buffer is 'x'. If the checkbox is not checked it is ' '.
 **************************************************************************/

bool forms_checkbox_is_checked(FIELD *field) {

	//
	// Get the buffer string.
	//
	const char *buffer = field_buffer(field, 0);
	if (buffer == NULL) {
		print_exit_str("forms_checkbox_is_checked() Unable to get field buffer!\n");
	}

	return buffer[0] == CHECK_CHAR;
}

/***************************************************************************
 * The function stores the state of a checkbox in the function argument. It
 * returns true if the value changed.
 **************************************************************************/

bool forms_get_checkbox_value(FIELD *field, bool *checked) {

	//
	// Get the new value from the checkbox field.
	//
	const bool tmp = forms_checkbox_is_checked(field);

	//
	// Compare the old and the new value
	//
	const bool result = tmp != *checked;

	//
	// Update the checkbox value and return the result.
	//
	*checked = tmp;
	return result;
}

/***************************************************************************
 * The function processes checkbox input. A checkbox is an input field with
 * one char. The only valid values are 'x' and ' ', which represent checked
 * and unchecked. The status is toggled with the space key or a 'x'.
 **************************************************************************/

void forms_process_checkbox(FORM *form, FIELD *field, const int key_type, const wint_t chr) {

	//
	// We are only processing spaces.
	//
	if (key_type == OK && ((wchar_t) chr == W_SPACE || (wchar_t) chr == CHECK_WCHAR)) {

		if (forms_checkbox_is_checked(field)) {
			print_debug_str("forms_process_checkbox() Unchecking checkbox!\n");

			//
			// If the checkbox is checked, the buffer (with one char) is
			// full and does not acccept any input, so the char has to be
			// deleted.
			//
			forms_driver(form, KEY_CODE_YES, REQ_DEL_CHAR);

		} else {
			print_debug_str("forms_process_checkbox() Checking checkbox!\n");

			//
			// Set the first and only char to 'x'. The field cursor is at
			// the end and does no accept any input.
			//
			forms_driver(form, OK, CHECK_WCHAR);
		}

		//
		// Request validation to do an update
		//
		forms_driver(form, KEY_CODE_YES, REQ_VALIDATION);
	}
}

/***************************************************************************
 * The function creates, configures and returns a field.
 **************************************************************************/

FIELD *forms_create_field(const int rows, const int cols, const int start_row, const int start_col, const chtype attr) {
	FIELD *field;
	int result;

	//
	// Create the field with a size and a position.
	//
	field = new_field(rows, cols, start_row, start_col, 0, 0);
	if (field == NULL) {
		print_exit_str("forms_create_field() Unable to create filter field!\n");
	}

	//
	// Switch off O_AUTOSKIP and O_BLANK which deletes the content of the
	// field if the first char is changed.
	//
	if ((result = field_opts_off(field, O_AUTOSKIP | O_BLANK)) != E_OK) {
		print_exit("forms_create_field() Unable to set option: O_AUTOSKIP and O_BLANK result: %d\n", result);
	}

	//
	// Set the background of the field.
	//
	if ((result = set_field_back(field, attr)) != E_OK) {
		print_exit("forms_create_field() Unable to set field background result: %d\n", result);
	}

	return field;
}

/***************************************************************************
 * The function sets the associated windows of the form and posts the form.
 **************************************************************************/

void forms_post_form(FORM *form, WINDOW *win, WINDOW *win_sub) {
	int result;

	//
	// Set the form to the window and the sub window.
	//
	if ((result = set_form_win(form, win)) != E_OK) {
		print_exit("forms_set_win_and_post() Unable to set form to the window! (result: %d)\n", result);
	}

	if ((result = set_form_sub(form, win_sub)) != E_OK) {
		print_exit("forms_set_win_and_post() Unable to set form to the sub window! (result: %d)\n", result);
	}

	//
	// Post the form. (E_NO_ROOM is returned if the window is too small)
	//
	if ((result = post_form(form)) != E_OK) {
		print_exit("forms_set_win_and_post() Unable to post filter form! (result: %d)\n", result);
	}
}

/***************************************************************************
 * The function creates a form with some fields and ensures that appropriate
 * options are set.
 **************************************************************************/

FORM *forms_create_form(FIELD **fields) {
	FORM *form;

	//
	// Create the filter form.
	//
	form = new_form(fields);
	if (form == NULL) {
		print_exit_str("forms_create_form() Unable to create filter form!\n");
	}

	//
	// Switch off special treatment of REQ_DEL_PREV at the beginning of the buffer.
	//
	if (form_opts_off(form, O_BS_OVERLOAD) != E_OK) {
		print_exit_str("forms_create_form() Unable to switch off O_BS_OVERLOAD\n");
	}

	return form;
}

/***************************************************************************
 * The function does an unpost and free of a form and its fields.
 **************************************************************************/

void forms_free(FORM *form, FIELD **fields) {

	//
	// Ensure that there is a form
	//
	if (form != NULL) {

		//
		// Ensure that the form was posted at all.
		//
		const int result = unpost_form(form);
		if (result != E_OK && result != E_NOT_POSTED) {
			print_exit_str("forms_free() Unable to unpost form!\n");
		}

		if (free_form(form) != E_OK) {
			print_exit_str("forms_free() Unable to free form!\n");
		}

		for (int i = 0; fields[i] != NULL; i++) {
			if (free_field(fields[i]) != E_OK) {
				print_exit("forms_free() Unable to free field: %d\n", i);
			}
		}
	}
}


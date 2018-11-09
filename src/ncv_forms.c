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

#include <ncursesw/form.h>

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
 * The function processes checkbox input. A checkbox is an input field with
 * one char. The only valid values are 'x' and ' ', which represent checked
 * and unchecked. The status is toggled with the space key, so the only key
 * that is processed is the space key. Then the state of the field is read
 * and toggled.
 **************************************************************************/

void forms_process_checkbox(FORM *form, FIELD *field, const int key_type, const wint_t chr) {

	//
	// We are only processing spaces.
	//
	if (key_type == OK && (wchar_t) chr == L' ') {

		//
		// Get the buffer string.
		//
		const char *buffer = field_buffer(field, 0);
		if (buffer == NULL) {
			print_exit_str("forms_process_checkbox() Unable to get field buffer!\n");
		}

		if (buffer[0] == 'x') {
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
			forms_driver(form, OK, L'x');
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


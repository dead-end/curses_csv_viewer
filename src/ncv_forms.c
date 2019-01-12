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
#include <ncursesw/menu.h>

/******************************************************************************
 * Definition of the checkbox checked char.
 *****************************************************************************/

#define CHECK_CHAR 'x'

#define CHECK_WCHAR L'x'

/******************************************************************************
 * The function is a simple wrapper around form_driver_w that provides error
 * handling.
 *****************************************************************************/

void forms_driver(FORM *form, const int key_type, const wint_t chr) {

	const int result = form_driver_w(form, key_type, chr);

	//
	// The form driver returns E_REQUEST_DENIED if you what to write before the
	// start or after the end of the field.
	//
	if (result != E_OK && result != E_REQUEST_DENIED) {
		print_exit("forms_driver() Unable to process key request for key code: %d key: %lc result: %d\n", key_type, chr, result);
	}
}

/******************************************************************************
 * The function is a simple wrapper around menu_driver that provides error
 * handling.
 *****************************************************************************/

void menus_driver(MENU *menu, const int chr) {

	const int result = menu_driver(menu, chr);

	//
	// The menu driver returns E_REQUEST_DENIED if it could not process the
	// request.
	//
	if (result != E_OK && result != E_REQUEST_DENIED) {
		print_exit("menus_driver() Unable to process request for key: %d result: %d\n", chr, result);
	}
}

/******************************************************************************
 * The method returns the index of the current field of the form.
 *****************************************************************************/

int forms_get_index(const FORM *form) {

	//
	// Get the current field.
	//
	const FIELD *field = current_field(form);
	if (field == NULL) {
		print_exit_str("forms_get_index() Unable to get current form field\n");
	}

	//
	// Get the index of the current field.
	//
	const int idx = field_index(field);
	if (idx == ERR) {
		print_exit_str("forms_get_index() Unable to get the index of a field\n");
	}

	print_debug("forms_get_index() Form field has index: %d\n", idx);

	return idx;
}

/******************************************************************************
 * The method returns the index of the current item of the menu.
 *****************************************************************************/

int menus_get_index(const MENU *menu) {

	//
	// Get the current item.
	//
	const ITEM *item = current_item(menu);
	if (item == NULL) {
		print_exit_str("menus_get_index() Unable to get current menu item\n");
	}

	//
	// Get the index of the current item.
	//
	const int idx = item_index(item);
	if (idx == ERR) {
		print_exit_str("menus_get_index() Unable to get the index of an item\n");
	}

	print_debug("menus_get_index() Menu item has index: %d\n", idx);

	return idx;
}

/******************************************************************************
 * The function checks if the current field of a form is the last one.
 *****************************************************************************/

bool forms_is_last(const FORM *form) {

	//
	// Get the number of items of the menu.
	//
	const int num_fields = field_count(form);
	if (num_fields == ERR) {
		print_exit_str("forms_is_last() Unable to get the number of fields from the form!\n");
	}

	const int idx = forms_get_index(form);

	print_debug("forms_is_last() current idx: %d last idx: %d\n", idx, num_fields - 1);

	return idx == num_fields - 1;
}

/******************************************************************************
 * The function checks if the current item of a menu is the last one.
 *****************************************************************************/

bool menus_is_last(const MENU *menu) {

	//
	// Get the number of items of the menu.
	//
	const int num_items = item_count(menu);
	if (num_items == ERR) {
		print_exit_str("menus_is_last() Unable to get the number of fields from the form!\n");
	}

	const int idx = menus_get_index(menu);

	print_debug("menus_is_last() current idx: %d last idx: %d\n", idx, num_items - 1);

	return idx == num_items - 1;
}

/******************************************************************************
 * The function creates, configures and returns a field.
 *****************************************************************************/

FIELD *forms_create_field(const int rows, const int cols, const int start_row, const int start_col, const chtype attr) {
	FIELD *field;
	int result;

	//
	// Create the field with a size and a position.
	//
	if ((field = new_field(rows, cols, start_row, start_col, 0, 0)) == NULL) {
		print_exit_str("forms_create_field() Unable to create filter field!\n");
	}

	//
	// Switch off O_AUTOSKIP and O_BLANK which deletes the content of the field
	// if the first char is changed.
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

/******************************************************************************
 * The function creates the items of a menu by a list of labels. The function
 * is called with the number of items, not including the terminating NULL. The
 * terminating NULL is set by the function.
 *****************************************************************************/

// TODO: Add function pointer / enum ???
void menus_create_items(ITEM **items, const int num_items, const char **labels) {

	for (int i = 0; i < num_items; i++) {

		if ((items[i] = new_item(labels[i], "")) == NULL) {
			print_exit("menus_create_items() Unable to create item: %d\n", i);
		}
	}

	//
	// Set the terminating NULL
	//
	items[num_items] = NULL;
}

/******************************************************************************
 * The function creates a form with some fields and ensures that appropriate
 * options are set.
 *****************************************************************************/

FORM *forms_create_form(FIELD **fields) {
	FORM *form;

	//
	// Create the form.
	//
	if ((form = new_form(fields)) == NULL) {
		print_exit_str("forms_create_form() Unable to create filter form!\n");
	}

	//
	// Switch off special treatment of REQ_DEL_PREV at the beginning of the
	// buffer.
	//
	if (form_opts_off(form, O_BS_OVERLOAD) != E_OK) {
		print_exit_str("forms_create_form() Unable to switch off O_BS_OVERLOAD\n");
	}

	return form;
}

/******************************************************************************
 * The function creates a menu with its items and ensures that appropriate
 * options are set. The method is called with the number of items not including
 * the terminating NULL. All items of the menu have the same attributes, so
 * they are set here.
 *****************************************************************************/

MENU *menus_create_menu(ITEM **items, const int num_items, const chtype attr) {
	MENU *menu;

	//
	// Create the menu.
	//
	if ((menu = new_menu(items)) == NULL) {
		print_exit_str("menus_create_menu() Unable to create menu!\n");
	}

	//
	// Create a horizontal menu.
	//
	if (set_menu_format(menu, 1, num_items) != E_OK) {
		print_exit_str("menus_create_menu() Unable to set menu format!\n");
	}

	//
	// Do not mark the current item.
	//
	if (set_menu_mark(menu, "") != E_OK) {
		print_exit_str("menus_create_menu() Unable to set menu mark!\n");
	}

	//
	// Set the background of the menu.
	//
	if (set_menu_back(menu, attr) != E_OK) {
		print_exit_str("menus_create_menu() Unable to set menu background!\n");
	}

	return menu;
}

/******************************************************************************
 * The function sets the associated windows of the form and posts the form.
 *****************************************************************************/

void forms_set_win_and_post(FORM *form, WINDOW *win, WINDOW *win_sub) {
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

/******************************************************************************
 * The function sets the associated windows of the menu and posts the menu.
 *****************************************************************************/

void menus_set_win_and_post(MENU *menu, WINDOW *win, WINDOW *win_sub) {
	int result;

	//
	// Set the menu to the window and the sub window.
	//
	if ((result = set_menu_win(menu, win)) != E_OK) {
		print_exit("menus_set_win_and_post() Unable to set menu to the window! (result: %d)\n", result);
	}

	if ((result = set_menu_sub(menu, win_sub)) != E_OK) {
		print_exit("menus_set_win_and_post() Unable to set menu to the sub window! (result: %d)\n", result);
	}

	//
	// Post the menu. (E_NO_ROOM is returned if the window is too small)
	//
	if ((result = post_menu(menu)) != E_OK) {
		print_exit("menus_set_win_and_post() Unable to post menu! (result: %d)\n", result);
	}
}

/******************************************************************************
 * The function does an unpost and free of a form and its fields.
 *****************************************************************************/

void forms_free(FORM *form) {

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

		//
		// The form has to be freed to be able to free the fields. Otherwise
		// the fields were connected.
		//
		if (free_form(form) != E_OK) {
			print_exit_str("forms_free() Unable to free form!\n");
		}

		//
		// Get the field array from the form and ensure that it is not null.
		//
		FIELD **field_ptr = form_fields(form);
		if (field_ptr != NULL) {

			//
			// Loop through the field array, which is NULL terminated.
			//
			for (; *field_ptr != NULL; field_ptr++) {

				if (free_field(*field_ptr) != E_OK) {
					print_exit_str("forms_free() Unable to free field!\n");
				}
			}
		}
	}
}

/******************************************************************************
 * The function does an unpost and free of a menu and its items.
 *****************************************************************************/

void menus_free(MENU *menu) {

	//
	// Ensure that there is a menu
	//
	if (menu != NULL) {

		//
		// Ensure that the menu was posted at all.
		//
		const int result = unpost_menu(menu);
		if (result != E_OK && result != E_NOT_POSTED) {
			print_exit_str("menus_free() Unable to unpost menu!\n");
		}

		//
		// The menu has to be freed to be able to free the items. Otherwise the
		// items were connected.
		//
		if (free_menu(menu) != E_OK) {
			print_exit_str("menus_free() Unable to free menu!\n");
		}

		//
		// Get the item array from the menu and ensure that it is not null.
		//
		ITEM **item_ptr = menu_items(menu);
		if (item_ptr != NULL) {

			//
			// Loop through the item array, which is NULL terminated.
			//
			for (; *item_ptr != NULL; item_ptr++) {

				if (free_item(*item_ptr) != E_OK) {
					print_exit_str("menus_free() Unable to free item!\n");
				}
			}
		}
	}
}

/******************************************************************************
 * The function reads the input from the field and stores it in the buffer. The
 * buffers size has to include the terminating \0. The input string is a multi
 * byte string, which will be trimmed and converted to wchar_t.
 *
 * If the resulting string is longer than the buffer it is an error.
 *****************************************************************************/

void forms_get_input_str(FIELD *field, wchar_t *buffer, const int buffer_size) {

	//
	// The first step is to get the raw field input data.
	//
	char *raw_field = field_buffer(field, 0);
	if (raw_field == NULL) {
		print_exit_str("forms_get_input_str() Unable to get field buffer!\n");
	}

	//
	// If the string contains multi byte chars, the length can be greater than
	// the buffer size.
	//
	size_t raw_len = strlen(raw_field);
	char raw_str[raw_len + 1];

	//
	// Create a copy of the field content that can be modified (trimmed). The
	// string has by construction the same size as the raw string, so strncpy
	// will add a \0
	//
	strncpy(raw_str, raw_field, raw_len);
	print_debug("forms_get_input_str() raw len: '%zu'\n", raw_len);

	//
	// The field content is filled with blanks, which had to be trimmed before
	// the conversion.
	//
	const char *trimed = trim(raw_str);

	//
	// Convert the trimmed input string to a wchar_t string. The buffer size
	// has to include the \0.
	//
	mbs_2_wchars(trimed, buffer, buffer_size);
}

/******************************************************************************
 * The functions returns true if the checkbox field is checked which means that
 * the buffer is 'x'. If the checkbox is not checked it is ' '.
 *****************************************************************************/

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

/******************************************************************************
 * The function stores the state of a checkbox in the function argument. It
 * returns true if the value changed.
 *****************************************************************************/

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

/******************************************************************************
 * The function processes checkbox input. A checkbox is an input field with one
 * char. The only valid values are 'x' and ' ', which represent checked and
 * unchecked. The status is toggled with the space key or a 'x'.
 *****************************************************************************/

void forms_process_checkbox(FORM *form, FIELD *field, const int key_type, const wint_t chr) {

	//
	// We are only processing spaces.
	//
	if (key_type == OK && ((wchar_t) chr == W_SPACE || (wchar_t) chr == CHECK_WCHAR)) {

		if (forms_checkbox_is_checked(field)) {
			print_debug_str("forms_process_checkbox() Unchecking checkbox!\n");

			//
			// If the checkbox is checked, the buffer (with one char) is full
			// and does not acccept any input, so the char has to be deleted.
			//
			forms_driver(form, KEY_CODE_YES, REQ_DEL_CHAR);

		} else {
			print_debug_str("forms_process_checkbox() Checking checkbox!\n");

			//
			// Set the first and only char to 'x'. The field cursor is at the
			// end and does no accept any input.
			//
			forms_driver(form, OK, CHECK_WCHAR);
		}

		//
		// Request validation to do an update
		//
		forms_driver(form, KEY_CODE_YES, REQ_VALIDATION);
	}
}

/******************************************************************************
 * The function does the processing of the input to a normal input field. If
 * the field needs special processing for certain keys, this has to be done
 * before and for the other keys the processing can be delegated to this
 * function.
 *
 * The parameter FIELD is not strictly necessary but allows a common function
 * pointer for "forms_process_checkbox" and "forms_process_input_field".
 *****************************************************************************/

void forms_process_input_field(FORM *form, FIELD *field, const int key_type, const wint_t chr) {

	switch (key_type) {

	case KEY_CODE_YES:

		//
		// Process function keys
		//
		switch (chr) {

		case KEY_DC:
			forms_driver(form, KEY_CODE_YES, REQ_DEL_CHAR);

			//
			// If the field content has changed, do an update.
			//
			forms_driver(form, KEY_CODE_YES, REQ_VALIDATION);
			break;

		case KEY_BACKSPACE:
			forms_driver(form, KEY_CODE_YES, REQ_DEL_PREV);

			//
			// If the field content has changed, do an update.
			//
			forms_driver(form, KEY_CODE_YES, REQ_VALIDATION);
			break;

		case KEY_LEFT:
			forms_driver(form, KEY_CODE_YES, REQ_LEFT_CHAR);
			break;

		case KEY_RIGHT:
			forms_driver(form, KEY_CODE_YES, REQ_RIGHT_CHAR);
			break;

		case KEY_HOME:
			forms_driver(form, KEY_CODE_YES, REQ_BEG_FIELD);
			break;

		case KEY_END:
			forms_driver(form, KEY_CODE_YES, REQ_END_FIELD);
			break;

		default:
			print_debug("forms_process_input_field() Found key code: %d\n", chr);
			break;
		}

		break; // case KEY_CODE_YES

	case OK:

		//
		// Process char keys
		//
		forms_driver(form, OK, (wchar_t) chr);
		forms_driver(form, KEY_CODE_YES, REQ_VALIDATION);
		print_debug("forms_process_input_field() Found char: %d field content: %s\n", chr, field_buffer(field, 0));

		break; // case OK

	default:
		print_exit("forms_process_input_field() Unknown key code: %d key: %lc for field: %d\n", key_type, chr, field_index(field))
		;
		break;
	}
}

/******************************************************************************
 * The function computes the size of the menu. It is called with the menu,
 * which contains a NULL terminated array of ITEM's. Each item has the size of
 * the max item name length. The items are delimited by strings, so the size
 * is:
 *
 * num-items * max(item-name-len) + num-items -1
 *****************************************************************************/

int menus_get_size(const MENU *menu) {
	size_t size;
	size_t max = 0;
	int num_items = 0;

	//
	// Get the item array from the menu.
	//
	ITEM **item_ptr = menu_items(menu);
	if (item_ptr == NULL) {
		print_exit_str("menus_get_size() Unable to get menu items!\n");
	}

	//
	// Loop through the item array, which is NULL terminated.
	//
	for (; *item_ptr != NULL; item_ptr++) {

		size = strlen(item_name(*item_ptr));

		if (size > max) {
			max = size;
		}
		num_items++;
	}

	return max * num_items + (num_items - 1);
}

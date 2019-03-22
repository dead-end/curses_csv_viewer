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
 * The function is called with a NULL terminated array of item labels. For each
 * label an item is created. With the items the menu is created. For the items
 * an array is allocated. To free this array the function menu_items(menu) is
 * called to get a reference to it.
 *****************************************************************************/

MENU *menus_create_menu(char **labels) {
	MENU *menu;
	ITEM **items;
	int num_items = 0;

	//
	// Count the number of labels.
	//
	for (char **ptr = labels; *ptr != NULL; ptr++, num_items++) {
		print_debug("menus_create_menu() Item label: '%s'\n", *ptr);
	}

	//
	// Allocate memory for the item array (which is NULL terminated). The items
	// can be determined with menu_items(menu). This is used for freeing the
	// array.
	//
	items = xmalloc(sizeof(ITEM *) * (num_items + 1));

	//
	// Set the terminating NULL
	//
	items[num_items] = NULL;

	//
	// Create the items.
	//
	for (int i = 0; i < num_items; i++) {
		print_debug("menus_create_menu() Creating item: '%s'\n", labels[i]);

		if ((items[i] = new_item(labels[i], "")) == NULL) {
			print_exit("menus_create_menu() Unable to create item: '%s'\n", labels[i]);
		}
	}

	//
	// After the items are created, we can create and return the menu.
	//
	if ((menu = new_menu(items)) == NULL) {
		print_exit_str("menus_create_menu() Unable to create menu!\n");
	}

	return menu;
}

/******************************************************************************
 * The function formats the menu as horizontal or vertical, sets the background
 * and the menu marker.
 *****************************************************************************/

void menus_format_menu(MENU *menu, const chtype attr, const bool horizontal) {
	int num_items;

	//
	// Get the number of item from the menu.
	//
	if ((num_items = item_count(menu)) == ERR) {
		print_exit_str("menus_format_menu() to get the number of items!\n");
	}

	//
	// Format the menu as horizontal or vertical.
	//
	int rows, cols;

	if (horizontal) {
		rows = 1;
		cols = num_items;
	} else {
		rows = num_items;
		cols = 1;
	}

	if (set_menu_format(menu, rows, cols) != E_OK) {
		print_exit("menus_format_menu() Unable to set menu format with rows: 1 cols: %d\n", num_items);
	}

	//
	// Do not mark the current item.
	//
	if (set_menu_mark(menu, "") != E_OK) {
		print_exit_str("menus_format_menu() Unable to set menu mark!\n");
	}

	//
	// Set the background of the menu.
	//
	if (set_menu_back(menu, attr) != E_OK) {
		print_exit_str("menus_format_menu() Unable to set menu background!\n");
	}
}

/******************************************************************************
 * The function creates the field array, which is NULL terminated. The NULL is
 * not part of the num_fiels parameter.
 *****************************************************************************/

FIELD **forms_create_fields(const int num_fields) {

	FIELD **fields = xmalloc(sizeof(FIELD *) * (num_fields + 1));

	fields[num_fields] = NULL;

	return fields;
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

MENU *menus_create_menu_old(ITEM **items, const int num_items, const chtype attr) {
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
		print_exit("menus_create_menu() Unable to set menu format with rows: 1 cols: %d\n", num_items);
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
 * The function set the form windows.
 *****************************************************************************/

void menus_set_wins(MENU *menu, WINDOW *win, WINDOW *win_menu) {
	int result;

	//
	// Set the menu to the window and the sub window.
	//
	if ((result = set_menu_win(menu, win)) != E_OK) {
		print_exit("menus_set_wins() Unable to set menu to the window! (result: %d)\n", result);
	}

	if ((result = set_menu_sub(menu, win_menu)) != E_OK) {
		print_exit("menus_set_wins() Unable to set menu to the sub window! (result: %d)\n", result);
	}
}

/******************************************************************************
 * The function set the form windows.
 *****************************************************************************/

void forms_set_wins(FORM *form, WINDOW *win, WINDOW *win_form) {
	int result;

	//
	// Set the form to the window and the sub window.
	//
	if ((result = set_form_win(form, win)) != E_OK) {
		print_exit("forms_set_wins() Unable to set form to the window! (result: %d)\n", result);
	}

	if ((result = set_form_sub(form, win_form)) != E_OK) {
		print_exit("forms_set_wins() Unable to set form to the sub window! (result: %d)\n", result);
	}
}

/******************************************************************************
 * The function frees the user pointer data of all fields of a form.
 *****************************************************************************/

void forms_user_ptr_free(const FORM *form) {

	//
	// The freeing functions should be fault tolerant.
	//
	if (form == NULL) {
		return;
	}

	//
	// Due to man page form_fields can return null, so be fault tolerant again.
	//
	FIELD **field_ptr = form_fields(form);
	if (field_ptr == NULL) {
		return;
	}

	//
	// Loop through the field array, which is NULL terminated.
	//
	for (; *field_ptr != NULL; field_ptr++) {
		void *ptr = field_userptr(*field_ptr);

		//
		// If a pointer is present, free it.
		//
		if (ptr != NULL) {
			free(ptr);
		}
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
		// Get a pointer to the fields and the number of fields. Calling:
		// free_form(form) disconnects the fields from the form, so after that,
		// the data cannot be retrieved.
		//
		FIELD **field_ptr = form_fields(form);

		const int num_fields = field_count(form);

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
		// Ensure that the field pointer is not NULL
		//
		if (field_ptr != NULL) {

			//
			// Ensure that the form has field.
			//
			if (num_fields > 0) {

				//
				// Loop through the field array, which is NULL terminated.
				//
				for (FIELD **ptr = field_ptr; *ptr != NULL; ptr++) {
					print_debug_str("forms_free() Freeing field!\n");

					if (free_field(*ptr) != E_OK) {
						print_exit_str("forms_free() Unable to free field!\n");
					}
				}
			}

			//
			// The last step is to free the fields array.
			//
			print_debug_str("forms_free() Freeing the fields array!\n");
			free(field_ptr);
		}
	}
}

/******************************************************************************
 * The function does an unpost and the freeing of the menu and its items.
 *****************************************************************************/

void menus_free(MENU *menu) {

	//
	// Ensure that there is a menu
	//
	if (menu != NULL) {

		//
		// Get a pointer to the items and the number of items. Calling:
		// free_menu(menu) disconnects the items from the menu, so after that,
		// the data cannot be retrieved.
		//
		ITEM **item_ptr = menu_items(menu);

		const int num_items = item_count(menu);

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
		// Ensure that the item pointer is not NULL
		//
		if (item_ptr != NULL) {

			//
			// Ensure that the form has field.
			//
			if (num_items > 0) {

				//
				// Loop through the item array, which is NULL terminated.
				//
				for (ITEM **ptr = item_ptr; *ptr != NULL; ptr++) {
					print_debug("menus_free() Freeing item: '%s'\n", item_name(*ptr));

					if (free_item(*ptr) != E_OK) {
						print_exit_str("menus_free() Unable to free item!\n");
					}
				}
			}

			//
			// The last step is to free the items array.
			//
			print_debug_str("menus_free() Freeing the items array!\n");
			free(item_ptr);
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
 * The function switches on or of the menu and the cursor
 *****************************************************************************/

void menus_switch_on_off(MENU *menu, const bool on) {

	const chtype type = on ? A_NORMAL : A_REVERSE;

	if (set_menu_fore(menu, type) != E_OK) {
		print_exit_str("menus_switch_curser() Unable to change the menu forground!\n");
	}

	if (curs_set(!on) == ERR) {
		print_exit_str("menus_switch_curser() Unable to switch on/off the cursor!\n");
	}
}

/******************************************************************************
 * The function does a post of the menu. If necessary it does an unpost before.
 * The repost is necessary if the size of the win_menu changes.
 *****************************************************************************/

void menus_unpost_post(MENU *menu, const bool unpost) {

	int result;

	if (unpost && (result = unpost_menu(menu)) != E_OK) {
		print_exit("menus_unpost_post() Unable to unpost menu! (result: %d)\n", result);
	}

	if ((result = post_menu(menu)) != E_OK) {
		print_exit("menus_unpost_post() Unable to post menu! (result: %d)\n", result);
	}
}


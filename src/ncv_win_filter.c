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

#include "ncv_popup.h"
#include "ncv_filter.h"
#include "ncv_win_filter.h"
#include "ncv_ncurses.h"
#include "ncv_forms.h"
#include "ncv_common.h"

#include <string.h>
#include <ncursesw/form.h>
#include <ncursesw/menu.h>

/******************************************************************************
 * Some definitions for the fields.
 *****************************************************************************/

//
// The popup has a border of 2 chars, one char for the box and a space char
// (padding).
//
#define BOX 1

#define PADDING 1

//
// The row of the field in the sub window
//
#define FILTER_ROW 0

#define CASE_ROW 2

#define SEARCH_ROW 4

//
// The length and the heights of the fields
//
#define CKBOX_FIELD_LEN 1

#define FIELD_HIGHT 1

//
// The struc holds the main data for the popup.
//
static s_popup popup;

/******************************************************************************
 * The struct contains data specific to a field.
 *****************************************************************************/

//
// Type definition for the function pointer to the input processing function of
// a field.
//
typedef void (*fp_driver)(FORM *form, FIELD *field, const int key_type, const wint_t chr);

//
// There is a user data struct associate with every form field.
//
typedef struct s_field_user_ptr {

	//
	// The label of the field, which can be an empty string but not null.
	//
	char *label;

	//
	// The type of the field (input field or checkbox)
	//
	enum FIELD_TYPE type;

	//
	// A driver, which is processing the input specific to the field type.
	// (forms_process_input_field or forms_process_checkbix)
	//
	fp_driver driver;

} s_field_user_ptr;

/******************************************************************************
 * The s_popup_sizes struct contains all sizes that are necessary to print a
 * popup.
 *****************************************************************************/

typedef struct s_popup_sizes {

	//
	// The popup window cols and rows.
	//
	int win_rows;

	int win_cols;

	//
	// The size and the position of the form.
	//
	int form_rows;

	int form_cols;

	int form_start_row;

	int form_start_col;

	//
	// The offset of the label (box and padding chars)
	//
	int form_start_label;

	//
	// The size and the position of the button menu.
	//
	int menu_rows;

	int menu_cols;

	int menu_start_row;

	int menu_start_col;

} s_popup_sizes;

//
// An instance of the popup sizes struct for the filter popup.
//
s_popup_sizes popup_sizes;

/******************************************************************************
 * The function create a s_field_user_ptr struct and adds it to the associated
 * field.
 *
 * Freeing can be done with the generic function:
 *
 * void forms_user_ptr_free(const FORM *form);
 *****************************************************************************/

static s_field_user_ptr *field_user_ptr_create(FIELD *field, const enum FIELD_TYPE type, char *label, const fp_driver driver) {

	//
	// NULL labels are not allowed, use empty strings instead.
	//
	if (label == NULL) {
		print_exit_str("field_user_ptr_create() Label is NULL!\n");
	}

	//
	// Allocate the struct.
	//
	s_field_user_ptr *field_user_ptr = xmalloc(sizeof(s_field_user_ptr));

	//
	// Set the struct values.
	//
	field_user_ptr->type = type;
	field_user_ptr->label = label;
	field_user_ptr->driver = driver;

	//
	// Add the struct to the field.
	//
	if (set_field_userptr(field, field_user_ptr) != E_OK) {
		print_exit_str("field_user_ptr_create() Unable to set user ptr for field!\n");
	}

	return field_user_ptr;
}

/******************************************************************************
 * The function computes various sizes of the fields of a form:
 *
 * - The number of fields of the form.
 * - The maximum label size of the fields.
 * - The number of rows of the fields. (A field can have more than one row).
 * - The maximum column size of the form fields.
 *****************************************************************************/

static void form_get_field_sizes(const FORM *form, int *num_fields, int *max_label_size, int *num_rows, int *max_col_size) {
	int tmp_label_size, tmp_rows, tmp_cols, dummy;

	//
	// Initialize the parameter.
	//
	*num_fields = 0;
	*max_label_size = 0;
	*num_rows = 0;
	*max_col_size = 0;

	//
	// Get the field array from the form.
	//
	FIELD **field_ptr = form_fields(form);
	if (field_ptr == NULL) {
		print_exit_str("form_get_field_sizes() Unable to get form fields!\n");
	}

	//
	// Loop through the field array, which is NULL terminated.
	//
	for (; *field_ptr != NULL; field_ptr++) {

		//
		// Get the user ptr from the field, which contains the label.
		//
		s_field_user_ptr *user_ptr = (s_field_user_ptr *) field_userptr(*field_ptr);
		if (user_ptr == NULL) {
			print_exit_str("form_get_field_sizes() Field has no user ptr!\n");
		}

		//
		// Update the maximum field size if necessary.
		//
		tmp_label_size = strlen(user_ptr->label);
		if (tmp_label_size > *max_label_size) {
			*max_label_size = tmp_label_size;
		}

		//
		// Get the cols of the field, which is the maximum field size. We are
		// not interested in the rest of the data, so we use a dummy parameter.
		//
		if (field_info(*field_ptr, &tmp_rows, &tmp_cols, &dummy, &dummy, &dummy, &dummy) != E_OK) {
			print_exit("form_get_field_sizes() Unable to get rows of field with label: %s\n", user_ptr->label);
		}

		//
		// Update the maximum field size if necessary.
		//
		if (tmp_cols > *max_col_size) {
			*max_col_size = tmp_cols;
		}

		//
		// Add the rows of the current field to the total number of rows.
		//
		*num_rows += tmp_rows;

		//
		// Increase the number of fields.
		//
		(*num_fields)++;
	}

	print_debug("form_get_field_sizes() Max label size: %d max field size: %d, num fields: %d num rows: %d \n", *max_label_size, *max_col_size, *num_fields, *num_rows);
}

/******************************************************************************
 * The function computes the various sizes of the popup and stores the result
 * in the s_popup_sizes struct. The popup consists of a form with input fields
 * and buttons that are implemented as a horizontal menu.
 * The popup is surrounded by a box and optionally by empty rows and cols (box
 * offset).
 *
 *  | Padding          | Padding
 *  v                  v
 * +--------------------+
 * |                    | <- Padding
 * | Label _______      | <- Input field
 * |                    |
 * | Label ____________ | <- Multi line input field
 * |       ____________ |
 * |                    |
 * | Label [ ]          | <- Checkbox
 * |                    |
 * |    Button Button   | <- Button menu
 * |                    | <- Padding
 * +--------------------+
 *****************************************************************************/

void popup_get_sizes(const s_popup *popup, s_popup_sizes *popup_sizes, const int padding) {
	int max_label_size, max_col_size, num_fields, num_rows;

	//
	// Get various sizes from the form fields.
	//
	form_get_field_sizes(popup->form, &num_fields, &max_label_size, &num_rows, &max_col_size);

	//
	// The form rows is the number of row of the fields. The fields are
	// separated by an empty row, which have to be added.
	//
	popup_sizes->form_rows = num_rows + num_fields - 1;
	popup_sizes->form_cols = max_col_size;

	//
	// The button menu has one row.
	//
	popup_sizes->menu_rows = 1;
	popup_sizes->menu_cols = menus_get_size(popup->menu);

	//
	// The total win has the form and menu rows separate by an empty row.
	//
	popup_sizes->win_rows = (2 * (BOX + padding)) + popup_sizes->form_rows + 1 + popup_sizes->menu_rows;
	popup_sizes->win_cols = (2 * (BOX + padding)) + max_label_size + max_col_size;

	//
	// This is a task for the future.
	//
	if (popup_sizes->win_cols < popup_sizes->menu_cols) {
		print_exit_str("popup_get_sizes() Menu is larger than form!\n");
	}

	//
	// The form win starts after the labels.
	//
	popup_sizes->form_start_row = (BOX + padding);
	popup_sizes->form_start_label = (BOX + padding);
	popup_sizes->form_start_col = (BOX + padding) + max_label_size;

	//
	// The button menu starts one row after the form. The menu is centered.
	//
	popup_sizes->menu_start_row = popup_sizes->form_start_row + popup_sizes->form_rows + 1;
	popup_sizes->menu_start_col = (popup_sizes->win_cols - popup_sizes->menu_cols) / 2;
}

/******************************************************************************
 * The function prints the content of the window. It contains of the fields
 * with their labels, inside a window with a box.
 *****************************************************************************/

static void win_filter_print_content() {

	//
	// Add a box
	//
	if (box(popup.win, 0, 0) != OK) {
		print_exit_str("win_filter_print_content() Unable to setup win!\n");
	}

	//
	// post form and menu with their wins
	//
	popup_post(&popup);

	//
	// Get the array of fields from the form. The array is NULL terminated.
	//
	FIELD **field_ptr = form_fields(popup.form);
	if (field_ptr == NULL) {
		print_exit_str("win_filter_print_content() Unable to get form fields!\n");
	}

	//
	// Loop through the field array, which is NULL terminated.
	//
	for (int row = popup_sizes.form_start_row; *field_ptr != NULL; field_ptr++, row += 2) {

		//
		// Get the user ptr from the field, which contains the label and the
		// field type.
		//
		s_field_user_ptr *user_ptr = (s_field_user_ptr *) field_userptr(*field_ptr);
		if (user_ptr == NULL) {
			print_exit_str("win_filter_print_content() Field has no user ptr!\n");
		}

		//
		// Print the label for the field.
		//
		mvwaddstr(popup.win, row, popup_sizes.form_start_label, user_ptr->label);

		//
		// If the field is a checkbox add a: [ ]
		//
		if (user_ptr->type == FIELD_TYPE_CHECKBOX) {
			mvwaddstr(popup.win, row, popup_sizes.form_start_col, "[");
			mvwaddstr(popup.win, row, popup_sizes.form_start_col + 2, "]");
		}
	}

	//
	// Last step is to set the cursor.
	//
	popup_pos_cursor(&popup);
}

/******************************************************************************
 * The function initializes the filter window.
 *****************************************************************************/

void win_filter_init() {

	//
	// Create attributes for the input field and the rest, which are checkboxes
	// and buttons
	//
	const chtype attr_input = ncurses_attr_color(COLOR_PAIR(CP_STATUS) | A_UNDERLINE, A_REVERSE | A_UNDERLINE);
	const chtype attr_normal = ncurses_attr_color(COLOR_PAIR(CP_STATUS), A_REVERSE);

	//
	// Do the initialization of the popup, which is allocating memory.
	//
	popup_init(&popup, 3, 2);

	//
	// Create filter field
	//
	popup.fields[0] = forms_create_field(FIELD_HIGHT, FILTER_FIELD_COLS, FILTER_ROW, 0, attr_input);
	field_user_ptr_create(popup.fields[0], FIELD_TYPE_INPUT, "Filter: ", forms_process_input_field);

	//
	// Create case checkbox field
	//
	popup.fields[1] = forms_create_field(FIELD_HIGHT, CKBOX_FIELD_LEN, CASE_ROW, 1, attr_normal);
	field_user_ptr_create(popup.fields[1], FIELD_TYPE_CHECKBOX, "Case: ", forms_process_checkbox);

	//
	// Create search checkbox field
	//
	popup.fields[2] = forms_create_field(FIELD_HIGHT, CKBOX_FIELD_LEN, SEARCH_ROW, 1, attr_normal);
	field_user_ptr_create(popup.fields[2], FIELD_TYPE_CHECKBOX, "Search: ", forms_process_checkbox);

	popup.fields[3] = NULL;

	//
	// Create the for with the fields
	//
	popup.form = forms_create_form(popup.fields);

	//
	// Create the buttons, which are menu items.
	//
	const char *labels[] = { "OK", "Cancel" };
	menus_create_items(popup.items, 2, labels);

	//
	// Create the menu with the items.
	//
	popup.menu = menus_create_menu(popup.items, 2, attr_normal);

	//
	// After the form and the menu is created we can start computing the sizes
	// of the popup.
	//
	popup_get_sizes(&popup, &popup_sizes, PADDING);

	//
	// Create a centered filter window
	//
	popup.win = ncurses_win_create(popup_sizes.win_rows, popup_sizes.win_cols, 0, 0);

	ncurses_win_center(popup.win);

	ncurses_attr_back(popup.win, COLOR_PAIR(CP_STATUS), A_REVERSE);

	//
	// Necessary to enable function keys like arrows
	//
	if (keypad(popup.win, TRUE) != OK) {
		print_exit_str("win_filter_init() Unable to call keypad!");
	}

	popup.win_form = ncurses_derwin_create(popup.win, popup_sizes.form_rows, popup_sizes.form_cols, popup_sizes.form_start_row, popup_sizes.form_start_col);

	popup.win_menu = ncurses_derwin_create(popup.win, popup_sizes.menu_rows, popup_sizes.menu_cols, popup_sizes.menu_start_row, popup_sizes.menu_start_col);

	popup_set_wins(&popup);

	//
	// Initial printing of the form
	//
	win_filter_print_content();
}

/******************************************************************************
 * The function is called on resizing the terminal window.
 *****************************************************************************/

void win_filter_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HAS_MIN_SIZE(popup_sizes.win_rows, popup_sizes.win_cols)) {
		print_debug_str("win_filter_resize() Do resize the window!\n");

		//
		// All of the 3 functions have to be called to ensure that all window
		// sizes are set if necessary.
		//
		const bool do_update_win = ncurses_win_ensure_size(popup.win, popup_sizes.win_rows, popup_sizes.win_cols);
		const bool do_update_form = ncurses_win_ensure_size(popup.win_form, popup_sizes.form_rows, popup_sizes.form_cols);
		const bool do_update_menu = ncurses_win_ensure_size(popup.win_menu, popup_sizes.menu_rows, popup_sizes.menu_cols);

		print_debug("win_filter_resize() do_update_win: %s\n", bool_2_str(do_update_win));
		print_debug("win_filter_resize() do_update_form: %s\n", bool_2_str(do_update_form));
		print_debug("win_filter_resize() do_update_menu: %s\n", bool_2_str(do_update_menu));

		//
		// Both checks have to be executed. If do_update_win is true the
		// second test is skipped.
		//
		if (do_update_win || do_update_form || do_update_menu) {
			print_debug_str("win_filter_resize() Do update!\n");

			//
			// On resize do an unpost at the beginning. (Resizing the window does
			// not work well with forms.)
			//
			popup_unpost(&popup);

			//
			// Ensure the correct position of the derived window.
			//
			ncurses_derwin_move(popup.win_form, popup_sizes.form_start_row, popup_sizes.form_start_col);
			ncurses_derwin_move(popup.win_menu, popup_sizes.menu_start_row, popup_sizes.menu_start_col);

			//
			// If everything (forms, menus, subwins) is on its place print the
			// content.
			//
			win_filter_print_content();
		}

		//
		// Move the window to the center.
		//
		ncurses_win_center(popup.win);
	}
}

/******************************************************************************
 * The function updates the filter struct with the data from the form fields.
 * It sets the has_changed flag if the struct changed. It is assumed that the
 * filtering should be activated if the filter string is not empty.
 *****************************************************************************/

static void win_filter_get_filter(s_filter *to_filter, s_popup *popup, const bool is_active) {
	s_filter from_filter;

	//
	// Fill the new s_filter with the data from the form.
	//
	forms_get_input_str(popup->fields[0], from_filter.str, FILTER_STR_LEN + 1);

	from_filter.case_insensitive = !forms_checkbox_is_checked(popup->fields[1]);

	from_filter.is_search = !forms_checkbox_is_checked(popup->fields[2]);

	//
	// On CANCEL or ESC the filter is inactive although the filter string is
	// set. The filtering can be tried to be activated, but if the filter
	// string is empty, this is useless.
	//
	from_filter.is_active = is_active && wcslen(from_filter.str) > 0;

	//
	// Do the update and return the result.
	//
	to_filter->has_changed = s_filter_update(to_filter, &from_filter);

#ifdef DEBUG
	s_filter_print(to_filter);
#endif
}

/******************************************************************************
 * The menu input processing only takes care on ENTER input, which selects a
 * button that is clicked. It returns true if the function processed the input.
 * If not, the processing can be delegated to an other function.
 *****************************************************************************/

static bool process_menu_input(s_filter *filter, s_popup *popup, const int key_type, const wint_t chr) {

	if (key_type == OK && (chr == KEY_ENTER || chr == NCV_KEY_NEWLINE)) {

		switch (menus_get_index(popup->menu)) {

		case 0:
			//
			// OK button
			//
			win_filter_get_filter(filter, popup, true);
			return true;

		case 1:
			//
			// CANCEL button
			//
			win_filter_get_filter(filter, popup, false);
			return true;
		}
	}

	return false;
}

/******************************************************************************
 * The function does the customized processing of the popup form fields. On
 * ENTER the form is submitted. On CTRL-x the filter field is deleted. All
 * other input processing is delegated to the field processing function. The
 * fields can be normal input fields or checkboxes. The function returns true
 * if the popup should be close.
 *****************************************************************************/

static bool process_form_input(s_filter *filter, s_popup *popup, const int key_type, const wint_t chr) {

	if (key_type == OK) {

		switch (chr) {

		case KEY_ENTER:
		case NCV_KEY_NEWLINE:

			//
			// Submit form
			//
			win_filter_get_filter(filter, popup, true);

			return true;

		case CTRL('x'):

			//
			// Clear filter input
			//
			if (set_field_buffer(popup->fields[0], 0, "") != E_OK) {
				print_exit_str("process_form_input() Unable to reset the buffer\n");
			}

			return false;
		}
	}

	//
	// Delegate the input processing to the specific field functions (input or
	// checkbox)
	//
	FIELD *field = current_field(popup->form);

	const s_field_user_ptr *driver = (s_field_user_ptr *) field_userptr(field);
	if (driver == NULL) {
		print_exit_str("process_form_input() Unable to get field user ptr!\n");
	}

	driver->driver(popup->form, field, key_type, chr);

	return false;
}

/******************************************************************************
 * The function processes the input for the filter window / popup. The popup
 * consists of a from with several fields and a menu with buttons. The
 * functions simply delegates the processing to a form or a menu processing
 * function.
 *****************************************************************************/

bool win_filter_process_input(s_filter *filter, const int key_type, const wint_t chr) {

	//
	// First do the processing of the core popup inputs. This is moving
	// through the fields and buttons.
	//
	if (popup_process_input(&popup, key_type, chr)) {

		//
		// Moving through the popup elements means that the popup is not
		// closed.
		//
		return false;
	}

	if (popup.is_on_form) {
		return process_form_input(filter, &popup, key_type, chr);

	} else {
		return process_menu_input(filter, &popup, key_type, chr);
	}
}

/******************************************************************************
 * The function resets the popup for showing. The main task is to set the focus
 * on the first field of the form. It does not make sense to continue on the
 * 'OK' or 'CANCEL' button.
 *****************************************************************************/

void win_filter_prepair_show() {
	popup_prepair_show(&popup);
}

/******************************************************************************
 * The function does a refresh with no update if the terminal is large enough.
 *****************************************************************************/

void win_filter_refresh_no() {

	print_debug_str("win_filter_refresh_no() Refresh footer window.\n");
	ncurses_win_refresh_no(popup.win, popup_sizes.win_rows, popup_sizes.win_cols);
}

/******************************************************************************
 * The function returns the filter window (which is defined static).
 *****************************************************************************/

WINDOW *win_filter_get_win() {
	return popup.win;
}

/******************************************************************************
 * The function touches the window, so that a refresh has an effect.
 *****************************************************************************/

void win_filter_show() {

	if (touchwin(popup.win) == ERR) {
		print_exit_str("win_filter_show() Unable to touch filter window!\n");
	}
}

/******************************************************************************
 * The function frees the allocated resources.
 *****************************************************************************/

void win_filter_free() {

	print_debug_str("win_filter_free() Removing filter windows, forms and fields.\n");

	forms_user_ptr_free(popup.form);

	forms_free(popup.form);

	menus_free(popup.menu);

	ncurses_win_free(popup.win_form);

	ncurses_win_free(popup.win_menu);

	ncurses_win_free(popup.win);

	popup_free(&popup);
}


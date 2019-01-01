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
 * Define the offset for the field and the box.
 *****************************************************************************/

#define START_FIELDS 10

#define BOX_SIZE 2

#define BOX_OFFSET 1

/******************************************************************************
 * The definition of the window and the sub window sizes.
 *****************************************************************************/

//TODO: total size not only for form
// TODO: compute WIIN_ROW from WIN_SUB_ROW
#define WIN_FORM_ROWS 5

#define WIN_MENU_ROWS 1

#define WIN_FORM_COLS 32

#define WIN_ROWS WIN_FORM_ROWS + 1 + WIN_MENU_ROWS + BOX_SIZE

#define WIN_COLS START_FIELDS + 32 + BOX_SIZE

/******************************************************************************
 * Some definitions for the fields.
 *****************************************************************************/

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

/******************************************************************************
 * The necessary field / form / window variables are defined as static.
 *****************************************************************************/

static WINDOW *win_filter = NULL;

static WINDOW *win_sub_form = NULL;

static WINDOW *win_sub_menu = NULL;

//
// Form and field
//
#define NUM_FIELDS 3

//
// Buttons ok, cancel
//
#define NUM_BUTTONS 2

//
// The struc holds the main data for the popup.
//
static s_popup popup;

/******************************************************************************
 * The struct contains data specific to a field. The only one is currently the
 * driver function.
 *
 * ISO C forbids assignment between function pointer and 'void *' with
 * [-pedantic] so the struct is used as a level of indirection.
 *****************************************************************************/

typedef struct s_field_user_ptr {

	void (*driver)(FORM *form, FIELD *field, const int key_type, const wint_t chr);

} s_field_user_ptr;

//
// Structs for the two fields
//
static s_field_user_ptr filter_user_ptr = { .driver = forms_process_input_field };

static s_field_user_ptr checkbox_user_ptr = { .driver = forms_process_checkbox };

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
	ncurses_win_refresh_no(win_filter, WIN_ROWS, WIN_COLS);
}

/******************************************************************************
 * The function returns the filter window (which is defined static).
 *****************************************************************************/

WINDOW *win_filter_get_win() {
	return win_filter;
}

/******************************************************************************
 * The function touches the window, so that a refresh has an effect.
 *****************************************************************************/

void win_filter_show() {

	if (touchwin(win_filter) == ERR) {
		print_exit_str("win_filter_show() Unable to touch filter window!\n");
	}
}

/******************************************************************************
 * The function frees the allocated resources.
 *****************************************************************************/

void win_filter_free() {

	print_debug_str("win_filter_free() Removing filter windows, forms and fields.\n");

	forms_free(popup.form, popup.fields);

	menus_free(popup.menu, popup.items);

	ncurses_win_free(win_sub_form);

	ncurses_win_free(win_sub_menu);

	ncurses_win_free(win_filter);

	popup_free(&popup);
}

/******************************************************************************
 * The function prints the content of the window, which is the form with the
 * fields, the labels and the box of the window.
 *****************************************************************************/

static void win_filter_print_content() {

	//
	// Add a box
	//
	if (box(win_filter, 0, 0) != OK) {
		print_exit_str("win_filter_print_content() Unable to setup win!\n");
	}

	//
	// post form and menu with their wins
	//
	forms_set_win_and_post(popup.form, win_filter, win_sub_form);

	menus_set_win_and_post(popup.menu, win_filter, win_sub_menu);

	//
	// Print the field labels
	//
	mvwaddstr(win_filter, FILTER_ROW + BOX_OFFSET, BOX_OFFSET, "Filter:  ");

	mvwaddstr(win_filter, CASE_ROW + BOX_OFFSET, BOX_OFFSET, "Case:");
	mvwaddstr(win_filter, CASE_ROW + BOX_OFFSET, START_FIELDS + 2, "]");
	mvwaddstr(win_filter, CASE_ROW + BOX_OFFSET, START_FIELDS, "[");

	mvwaddstr(win_filter, SEARCH_ROW + BOX_OFFSET, BOX_OFFSET, "Search:");
	mvwaddstr(win_filter, SEARCH_ROW + BOX_OFFSET, START_FIELDS + 2, "]");
	mvwaddstr(win_filter, SEARCH_ROW + BOX_OFFSET, START_FIELDS, "[");

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
	popup_init(&popup, NUM_FIELDS, NUM_BUTTONS);

	//
	// Create filter field
	//
	popup.fields[0] = forms_create_field(FIELD_HIGHT, FILTER_FIELD_COLS, FILTER_ROW, 0, attr_input);
	if (set_field_userptr(popup.fields[0], &filter_user_ptr) != E_OK) {
		print_exit_str("win_filter_init() Unable to set user ptr for field!\n");
	}

	//
	// Create case checkbox field
	//
	popup.fields[1] = forms_create_field(FIELD_HIGHT, CKBOX_FIELD_LEN, CASE_ROW, 1, attr_normal);
	if (set_field_userptr(popup.fields[1], &checkbox_user_ptr) != E_OK) {
		print_exit_str("win_filter_init() Unable to set user ptr for field!\n");
	}

	//
	// Create search checkbox field
	//
	popup.fields[2] = forms_create_field(FIELD_HIGHT, CKBOX_FIELD_LEN, SEARCH_ROW, 1, attr_normal);
	if (set_field_userptr(popup.fields[2], &checkbox_user_ptr) != E_OK) {
		print_exit_str("win_filter_init() Unable to set user ptr for field!\n");
	}

	//
	// Create the for with the fields
	//
	popup.form = forms_create_form(popup.fields);

	//
	// Create the buttons, which are menu items.
	//
	const char *labels[] = { "OK", "Cancel" };
	menus_create_items(popup.items, NUM_BUTTONS, labels);

	//
	// Create the menu with the items.
	//
	popup.menu = menus_create_menu(popup.items, NUM_BUTTONS, attr_normal);

	//
	// Create a centered filter window
	//
	win_filter = ncurses_win_create(WIN_ROWS, WIN_COLS, 0, 0);

	ncurses_win_center(win_filter);

	ncurses_attr_back(win_filter, COLOR_PAIR(CP_STATUS), A_REVERSE);

	//
	// Necessary to enable function keys like arrows
	//
	if (keypad(win_filter, TRUE) != OK) {
		print_exit_str("win_filter_init() Unable to call keypad!");
	}

	//
	// Create the sub windows for the menu and the form.
	//
	const int menu_size = menus_get_size(popup.items, NUM_BUTTONS);

	win_sub_menu = ncurses_derwin_create(win_filter, WIN_MENU_ROWS, menu_size, WIN_FORM_ROWS + BOX_OFFSET + 1, (WIN_COLS - BOX_SIZE - menu_size) / 2);

	win_sub_form = ncurses_derwin_create(win_filter, WIN_FORM_ROWS, WIN_FORM_COLS, BOX_OFFSET, START_FIELDS);

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
	//TODO: min size for all
	if (WIN_HAS_MIN_SIZE(WIN_ROWS, WIN_COLS)) {
		print_debug_str("win_filter_resize() Do resize the window!\n");

		const bool do_update_win = ncurses_win_ensure_size(win_filter, WIN_ROWS, WIN_COLS);
		const bool do_update_sub = ncurses_win_ensure_size(win_sub_form, WIN_FORM_ROWS, WIN_FORM_COLS);

		// TODO: win_sub_menu size
		const int menu_size = menus_get_size(popup.items, NUM_BUTTONS);
		const bool do_update_sub_menu = ncurses_win_ensure_size(win_sub_menu, 1, menu_size);
		//
		// Both checks have to be executed. If do_update_win is true the
		// second test is skipped.
		//
		if (do_update_win || do_update_sub || do_update_sub_menu) {
			print_debug_str("win_filter_resize() Do update!\n");

			//
			// On resize do an unpost at the beginning. (Resizing the window does
			// not work well with forms.)
			//
			if (unpost_form(popup.form) != E_OK) {
				print_exit_str("win_filter_resize() Unable to unpost filter form!\n");
			}

			if (unpost_menu(popup.menu) != E_OK) {
				print_exit_str("win_filter_resize() Unable to unpost menu!\n");
			}

			//
			// Ensure the correct position of the derived window.
			//
			ncurses_derwin_move(win_sub_form, BOX_OFFSET, START_FIELDS);

			// TODO: win_sub_menu size
			ncurses_derwin_move(win_sub_menu, WIN_FORM_ROWS + BOX_OFFSET + 1, (WIN_COLS - menu_size) / 2);

			//
			// If everything (forms, menus, subwins) is on its place print the
			// content.
			//
			win_filter_print_content();
		}

		//
		// Move the window to the center.
		//
		ncurses_win_center(win_filter);
	}
}

/******************************************************************************
 * The function updates the filter struct with the data from the form fields.
 * The function returns true if the struct changed. It is assumed that the
 * filtering should be activated if the filter string is not empty.
 *****************************************************************************/
//TODO: maybe an update flag is reasonable in the filter struct.
static bool win_filter_get_filter(s_filter *to_filter, s_popup *popup, const bool is_active) {
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
	return s_filter_update(to_filter, &from_filter);
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
	// First do the processing of the core popup inputs
	//
	if (popup_process_input(&popup, key_type, chr)) {
		return false;
	}

	if (popup.is_on_form) {
		return process_form_input(filter, &popup, key_type, chr);

	} else {
		return process_menu_input(filter, &popup, key_type, chr);
	}
}

///*********************************************************************************
// * The function does the input processing of the form. This is mainly the
// * navigation through the fields of the form. If necessary it switches to
// * the menu.
// *
// * The processing of the field input is delegated to a fields processing
// * function. The field can be an input field or a checkbox, with a specific
// * processing function.
// ********************************************************************************/
//
//bool process_form_input_old(s_filter *filter, FORM *form, MENU *menu, const int key_type, const wint_t chr) {
//
//	switch (key_type) {
//
//	case KEY_CODE_YES:
//
//		switch (chr) {
//
//		case KEY_DOWN:
//			popup_req_prev_next_field(form, menu, &popup.is_on_form, NUM_FIELDS - 1);
//			return false;
//
//		case KEY_UP:
//			popup_req_prev_next_field(form, menu, &popup.is_on_form, 0);
//			return false;
//		}
//
//		break; // case KEY_CODE_YES
//
//	case OK:
//
//		switch (chr) {
//
//		case KEY_ENTER:
//		case NCV_KEY_NEWLINE:
//			win_filter_get_filter(filter, true);
//			return true;
//
//		case CTRL('x'):
//			win_filter_clear_filter();
//			return false;
//
//		case W_TAB:
//			popup_req_prev_next_field(form, menu, &popup.is_on_form, NUM_FIELDS - 1);
//			return false;
//		}
//
//		break; // case OK
//	}
//
//	//
//	// If the user does not request an other field, get the current field
//	// and delegate the input.
//	//
//	FIELD *field = current_field(form);
//
//	const s_field_user_ptr *driver = (s_field_user_ptr *) field_userptr(field);
//	if (driver == NULL) {
//		print_exit_str("process_form_input() Unable to get field user ptr!\n");
//	}
//
//	driver->driver(form, field, key_type, chr);
//
//	return false;
//}

///******************************************************************************
// * The function does the input processing of the menu. This is mainly the
// * navigation through the items of the menu. If necessary it switches to
// * the form.
// *****************************************************************************/
////TODO: enter on ok and on cancel
//bool process_menu_input_old(s_filter *filter, FORM *form, MENU *menu, const int key_type, const wint_t chr) {
//
//	switch (key_type) {
//
//	case KEY_CODE_YES:
//
//		//
//		// Process function keys
//		//
//		switch (chr) {
//
//		case KEY_LEFT:
//
//			if (menus_has_index(menu, 0)) {
//				menus_driver(menu, REQ_LAST_ITEM);
//			} else {
//				menus_driver(menu, REQ_LEFT_ITEM);
//			}
//			break;
//
//		case KEY_RIGHT:
//
//			if (menus_has_index(menu, NUM_BUTTONS - 1)) {
//				menus_driver(menu, REQ_FIRST_ITEM);
//			} else {
//				menus_driver(menu, REQ_RIGHT_ITEM);
//			}
//			break;
//
//		case KEY_UP:
//			popup.is_on_form = popup_switch(form, menu, popup.is_on_form, false);
//			break;
//
//		case KEY_DOWN:
//			popup.is_on_form = popup_switch(form, menu, popup.is_on_form, true);
//			break;
//
//		default:
//			print_debug("process_menu_input() Key code: %d key: %lc\n", key_type, chr);
//			break;
//		}
//
//		break; // case KEY_CODE_YES
//
//	case OK:
//
//		//
//		// Process function keys
//		//
//		switch (chr) {
//
//		// TODO:
//
//		//
//		// ESC char
//		//
//		//TODO: process before
//		case NCV_KEY_ESC:
//			// filter.is_active = false
//			// return DONE
//			break;
//		case KEY_ENTER:
//		case NCV_KEY_NEWLINE:
//
//			// OK
//			if (menus_has_index(menu, 0)) {
//				win_filter_get_filter(filter, true);
//				return true;
//			}
//
//			// CANCEL
//			if (menus_has_index(menu, 1)) {
//				win_filter_get_filter(filter, false);
//				return true;
//			}
//
//			break;
//
//		case W_TAB:
//
//			//
//			// Tabbing goes to the next item or the first field if the
//			// current item is the last item.
//			//
//			if (menus_has_index(menu, NUM_BUTTONS - 1)) {
//				popup.is_on_form = popup_switch(form, menu, popup.is_on_form, true);
//			} else {
//				menus_driver(menu, REQ_RIGHT_ITEM);
//			}
//
//			break;
//
//		default:
//			print_debug("process_menu_input() Key code: %d key: %lc\n", key_type, chr);
//			break;
//		}
//		break; // case OK
//
//	default:
//		print_exit("process_menu_input() Unknown key code: %d key: %lc\n", key_type, chr)
//		;
//		break;
//	}
//
//	return false;
//}

///*********************************************************************************
// * The function is called on a form field. It moves the cursor to the next /
// * previous field. If necessary it moves to the menu.
// ********************************************************************************/
//
//static void popup_req_prev_next_field(FORM *form, MENU *menu, bool *is_buttons, const int switch_on_idx) {
//	wint_t req_field;
//	bool to_first;
//
//	if (switch_on_idx == 0) {
//		req_field = REQ_PREV_FIELD;
//		to_first = false;
//
//	} else {
//		req_field = REQ_NEXT_FIELD;
//		to_first = true;
//	}
//
//	if (forms_has_index(form, switch_on_idx)) {
//		*is_buttons = popup_switch(form, menu, *is_buttons, to_first);
//		return;
//	}
//
//	//
//	// Move to the requested field and move the cursor to the end of the field.
//	//
//	forms_driver(form, KEY_CODE_YES, req_field);
//	forms_driver(form, KEY_CODE_YES, REQ_END_FIELD);
//}

///******************************************************************************
// * The function sets the cursor to the field in the form or an item in the
// * menu.
// *****************************************************************************/
//
//static void popup_pos_cursor(FORM *form, const MENU *menu, const bool is_menu) {
//
//	if (is_menu) {
//		if (pos_menu_cursor(menu) != E_OK) {
//			print_exit_str("popup_pos_cursor() Unable to set the menu cursor!\n");
//		}
//	} else {
//		if (pos_form_cursor(form) != E_OK) {
//			print_exit_str("popup_pos_cursor() Unable to set the form cursor!\n");
//		}
//	}
//}

///******************************************************************************
// * The function switches from the from to the menu or from the menu to the
// * form. The direction is defined by the flag "is_on_menu". The function
// * returns the revered flag, which is the new value. The last flag "to_first"
// * defines whether the switching should go to the first or last field.
// *****************************************************************************/
//
//static bool popup_switch(FORM *form, MENU *menu, const bool is_on_menu, const bool to_first) {
//
//	if (is_on_menu) {
//
//		//
//		// Switch off menu and cursor
//		//
//		set_menu_fore(menu, A_REVERSE);
//		curs_set(1);
//
//		//
//		// Move to the first or last field
//		//
//		if (to_first) {
//			forms_driver(form, KEY_CODE_YES, REQ_FIRST_FIELD);
//		} else {
//			forms_driver(form, KEY_CODE_YES, REQ_LAST_FIELD);
//		}
//		forms_driver(form, KEY_CODE_YES, REQ_END_FIELD);
//
//		print_debug_str("popup_switch() Switch to form.\n");
//
//	} else {
//
//		//
//		// Switch on menu and cursor
//		//
//		set_menu_fore(menu, A_NORMAL);
//		curs_set(0);
//
//		//
//		// Always move to the first item
//		//
//		menus_driver(menu, REQ_FIRST_ITEM);
//
//		print_debug_str("popup_switch() Switch to menu.\n");
//	}
//
//	return !is_on_menu;
//}

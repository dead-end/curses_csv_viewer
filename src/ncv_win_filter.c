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

#include "ncv_filter.h"
#include "ncv_win_filter.h"
#include "ncv_ncurses.h"
#include "ncv_forms.h"
#include "ncv_common.h"

#include <string.h>
#include <ncursesw/form.h>
#include <ncursesw/menu.h>

/***************************************************************************
 * Define the offset for the field and the box.
 **************************************************************************/

#define START_FIELDS 10

#define BOX_SIZE 2

#define BOX_OFFSET 1

/***************************************************************************
 * The definition of the window and the sub window sizes.
 **************************************************************************/

//TODO: total size not only for form
// TODO: compute WIIN_ROW from WIN_SUB_ROW

#define WIN_FORM_ROWS 5

#define WIN_MENU_ROWS 1

#define WIN_FORM_COLS 32

#define WIN_ROWS WIN_FORM_ROWS + 1 + WIN_MENU_ROWS + BOX_SIZE

#define WIN_COLS START_FIELDS + 32 + BOX_SIZE

/***************************************************************************
 * Some definitions for the fields.
 **************************************************************************/

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

/***************************************************************************
 * The necessary field / form / window variables are defined as static.
 **************************************************************************/

static WINDOW *win_filter = NULL;

static WINDOW *win_sub_form = NULL;

static WINDOW *win_sub_menu = NULL;

//
// Form and field
//
static FORM *filter_form = NULL;

#define NUM_FIELDS 3

static FIELD *filter_fields[NUM_FIELDS + 1];

//
// Buttons ok, clear, cancel
//
static MENU *button_menu = NULL;

#define NUM_BUTTONS 3

static ITEM *button_items[NUM_BUTTONS + 1];

static bool is_on_buttons = false;

/***************************************************************************
 * The struct contains data specific to a field. The only one is currently
 * the driver function.
 *
 * ISO C forbids assignment between function pointer and 'void *' with
 * [-pedantic] so the struct is used as a level of indirection.
 **************************************************************************/

typedef struct s_field_user_ptr {

	void (*driver)(FORM *form, FIELD *field, const int key_type, const wint_t chr);

} s_field_user_ptr;

//
// Declaration for the struct initialization
//
static void win_filter_process_filter_input(FORM *form, FIELD *field, const int key_type, const wint_t chr);

//
// Structs for the two fields
//
static s_field_user_ptr filter_user_ptr = { .driver = win_filter_process_filter_input };

static s_field_user_ptr checkbox_user_ptr = { .driver = forms_process_checkbox };

/***************************************************************************
 * The function sets the cursor to the field in the form or an item in the
 * menu.
 **************************************************************************/

static void win_filter_move_cursor(FORM *form, const MENU *menu, const bool is_menu) {

	if (is_menu) {
		if (pos_menu_cursor(menu) != E_OK) {
			print_exit_str("win_filter_move_cursor() Unable to set the menu cursor!\n");
		}
	} else {
		if (pos_form_cursor(form) != E_OK) {
			print_exit_str("win_filter_move_cursor() Unable to set the form cursor!\n");
		}
	}
}

/***************************************************************************
 * The function prints the content of the window, which is the form with
 * the fields, the labels and the box of the window.
 **************************************************************************/

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
	forms_set_win_and_post(filter_form, win_filter, win_sub_form);

	menus_set_win_and_post(button_menu, win_filter, win_sub_menu);

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

	win_filter_move_cursor(filter_form, button_menu, is_on_buttons);
}

/***************************************************************************
 * The function initializes the filter window.
 **************************************************************************/

void win_filter_init() {
	chtype attr;

	//
	// Create filter field
	//
	attr = ncurses_attr_color(COLOR_PAIR(CP_STATUS) | A_UNDERLINE, A_REVERSE | A_UNDERLINE);
	filter_fields[0] = forms_create_field(FIELD_HIGHT, FILTER_FIELD_COLS, FILTER_ROW, 0, attr);
	if (set_field_userptr(filter_fields[0], &filter_user_ptr) != E_OK) {
		print_exit_str("win_filter_init() Unable to set user ptr for field!\n");
	}

	//
	// Create case checkbox field
	//
	attr = ncurses_attr_color(COLOR_PAIR(CP_STATUS), A_REVERSE);
	filter_fields[1] = forms_create_field(FIELD_HIGHT, CKBOX_FIELD_LEN, CASE_ROW, 1, attr);
	if (set_field_userptr(filter_fields[1], &checkbox_user_ptr) != E_OK) {
		print_exit_str("win_filter_init() Unable to set user ptr for field!\n");
	}

	//
	// Create search checkbox field
	//
	attr = ncurses_attr_color(COLOR_PAIR(CP_STATUS), A_REVERSE);
	filter_fields[2] = forms_create_field(FIELD_HIGHT, CKBOX_FIELD_LEN, SEARCH_ROW, 1, attr);
	if (set_field_userptr(filter_fields[2], &checkbox_user_ptr) != E_OK) {
		print_exit_str("win_filter_init() Unable to set user ptr for field!\n");
	}

	//
	// Mark end of the array
	//
	filter_fields[3] = NULL;

	filter_form = forms_create_form(filter_fields);

	//
	// ------------------------------------------------
	//
	const char *labels[] = { "OK", "Clear", "Cancel" };
	menus_create_items(button_items, NUM_BUTTONS, labels);

	attr = ncurses_attr_color(COLOR_PAIR(CP_STATUS), A_REVERSE);
	button_menu = menus_create_menu(button_items, NUM_BUTTONS, attr);

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
	const int menu_size = menus_get_size(button_items, NUM_BUTTONS);

	win_sub_menu = ncurses_derwin_create(win_filter, WIN_MENU_ROWS, menu_size, WIN_FORM_ROWS + BOX_OFFSET + 1, (WIN_COLS - BOX_SIZE - menu_size) / 2);

	win_sub_form = ncurses_derwin_create(win_filter, WIN_FORM_ROWS, WIN_FORM_COLS, BOX_OFFSET, START_FIELDS);

	//
	// Initial printing of the form
	//
	win_filter_print_content();
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

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
		const int menu_size = menus_get_size(button_items, NUM_BUTTONS);
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
			if (unpost_form(filter_form) != E_OK) {
				print_exit_str("win_filter_resize() Unable to unpost filter form!\n");
			}

			if (unpost_menu(button_menu) != E_OK) {
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

/***************************************************************************
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_filter_refresh_no() {

	print_debug_str("win_filter_refresh_no() Refresh footer window.\n");
	ncurses_win_refresh_no(win_filter, WIN_ROWS, WIN_COLS);
}

/***************************************************************************
 * The function returns the filter window (which is defined static).
 **************************************************************************/

WINDOW *win_filter_get_win() {
	return win_filter;
}

/***************************************************************************
 * The function updates the filter struct with the data from the form
 * fields. The function returns true if the struct changed. It is assumed
 * that the filtering should be activated if the filter string is not empty.
 **************************************************************************/

bool win_filter_get_filter(s_filter *to_filter) {
	s_filter from_filter;

	//
	// Fill the new s_filter with the data from the form.
	//
	forms_get_input_str(filter_fields[0], from_filter.str, FILTER_STR_LEN + 1);

	from_filter.case_insensitive = !forms_checkbox_is_checked(filter_fields[1]);

	from_filter.is_search = !forms_checkbox_is_checked(filter_fields[2]);

	// TODO: if (is_active) => check strlen
	from_filter.is_active = wcslen(from_filter.str) > 0;

	//
	// Do the update and return the result.
	//
	return s_filter_update(to_filter, &from_filter);
}

/***************************************************************************
 * The function deletes the content of the filter field.
 **************************************************************************/

static inline void win_filter_clear_filter() {

	if (set_field_buffer(filter_fields[0], 0, "") != E_OK) {
		print_exit_str("win_filter_clear_filter() Unable to reset the buffer\n");
	}
}

/***************************************************************************
 * The function switches from the from to the menu or from the menu to the
 * form. The direction is defined by the flag "is_on_menu". The function
 * returns the revered flag, which is the new value. The last flag
 * "to_first" defines whether the switching should go to the first or last
 * field.
 **************************************************************************/

bool switch_form_menu(FORM *form, MENU *menu, const bool is_on_menu, const bool to_first) {

	if (is_on_menu) {

		//
		// Switch off menu and cursor
		//
		set_menu_fore(menu, A_REVERSE);
		curs_set(1);

		//
		// Move to the first or last field
		//
		if (to_first) {
			forms_driver(form, KEY_CODE_YES, REQ_FIRST_FIELD);
		} else {
			forms_driver(form, KEY_CODE_YES, REQ_LAST_FIELD);
		}

		print_debug_str("switch_form_menu() Switch to form.\n");

	} else {

		//
		// Switch on menu and cursor
		//
		set_menu_fore(menu, A_NORMAL);
		curs_set(0);

		//
		// Move to the first item
		//
		menus_driver(menu, REQ_FIRST_ITEM);

		print_debug_str("switch_form_menu() Switch to menu.\n");
	}

	return !is_on_menu;
}

/***************************************************************************
 * The function processes the input from the user. The error handling of
 * key_type is done by the calling function.
 **************************************************************************/

//TODO: check general function for input field (but: CTRL(x) and enter)

static void win_filter_process_filter_input(FORM *form, DEBUG_USED FIELD *field, const int key_type, const wint_t chr) {

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
			print_debug("win_filter_process_input() Found key code: %d\n", chr);
			break;
		}

		break; // case KEY_CODE_YES

	case OK:

		//
		// Process function keys
		//
		switch (chr) {

		//
		// Deletes the filter string in FILTER mode
		//
		case CTRL('x'):
			win_filter_clear_filter();

			break;

		default:

			//
			// Process char keys
			//
			forms_driver(form, OK, (wchar_t) chr);
			forms_driver(form, KEY_CODE_YES, REQ_VALIDATION);
			print_debug("win_filter_process_input() Found char: %d field content: %s\n", chr, field_buffer(field, 0));
		}

		break; // case OK

	default:
		print_exit("win_filter_process_filter_input() Unknown key code: %d key: %lc\n", key_type, chr)
		;
		break;
	}
}

/***************************************************************************
 * The function does the input processing of the menu. This is mainly the
 * navigation through the items of the menu. If necessary it switches to
 * the form.
 **************************************************************************/
//TODO: enter on ok and on cancel
static void process_menu_input(FORM *form, MENU *menu, const int key_type, const wint_t chr) {

	switch (key_type) {

	case KEY_CODE_YES:

		//
		// Process function keys
		//
		switch (chr) {

		case KEY_LEFT:

			if (menus_has_index(menu, 0)) {
				menus_driver(menu, REQ_LAST_ITEM);
			} else {
				menus_driver(menu, REQ_LEFT_ITEM);
			}
			break;

		case KEY_RIGHT:

			if (menus_has_index(menu, NUM_BUTTONS - 1)) {
				menus_driver(menu, REQ_FIRST_ITEM);
			} else {
				menus_driver(menu, REQ_RIGHT_ITEM);
			}
			break;

		case KEY_UP:
			is_on_buttons = switch_form_menu(form, menu, is_on_buttons, false);
			break;

		case KEY_DOWN:
			is_on_buttons = switch_form_menu(form, menu, is_on_buttons, true);
			break;

		default:
			print_debug("process_menu_input() Key code: %d key: %lc\n", key_type, chr);
			break;
		}

		break; // case KEY_CODE_YES

	case OK:

		//
		// Process function keys
		//
		switch (chr) {

		// TODO:

		//
		// ESC char
		//
		case NCV_KEY_ESC:
			// filter.is_active = false
			// return DONE
			break;
		case KEY_ENTER:
			// filter.is_active = true
			// return DONE

			// OK
			if (menus_has_index(menu, 0)) {
				return;
			}

			// CANCEL
			if (menus_has_index(menu, 1)) {
				return;
			}

			break;

		case W_TAB:

			//
			// Tabbing goes to the next item or the first field if the
			// current item is the last item.
			//
			if (menus_has_index(menu, NUM_BUTTONS - 1)) {
				is_on_buttons = switch_form_menu(form, menu, is_on_buttons, true);
			} else {
				menus_driver(menu, REQ_RIGHT_ITEM);
			}

			break;

		default:
			print_debug("process_menu_input() Key code: %d key: %lc\n", key_type, chr);
			break;
		}
		break; // case OK

	default:
		print_exit("process_menu_input() Unknown key code: %d key: %lc\n", key_type, chr)
		;
		break;
	}
}

/***************************************************************************
 * The function does the input processing of the form. This is mainly the
 * navigation through the fields of the form. If necessary it switches to
 * the menu.
 *
 * The processing of the field input is delegated to a fields processing
 * function. The field can be an input field or a checkbox, with a specific
 * processing function.
 **************************************************************************/

static void process_form_input(FORM *form, MENU *menu, const int key_type, const wint_t chr) {

	//
	// On key DOWN or TAB go to the next field
	//
	if ((key_type == KEY_CODE_YES && chr == KEY_DOWN) || (key_type == OK && chr == W_TAB)) {

		//
		// If the current field is the last field in the form, switch to
		// the menu
		//
		if (forms_has_index(form, NUM_FIELDS - 1)) {
			is_on_buttons = switch_form_menu(form, menu, is_on_buttons, true);
			return;
		}

		//
		// Go to the end of the next field
		//
		forms_driver(form, KEY_CODE_YES, REQ_NEXT_FIELD);
		forms_driver(form, KEY_CODE_YES, REQ_END_FIELD);
		return;
	}

	//
	// On key UP go to the previous field
	//
	if (key_type == KEY_CODE_YES && chr == KEY_UP) {

		//
		// If the current field is the first field in the form, switch to
		// the menu
		//
		if (forms_has_index(form, 0)) {
			is_on_buttons = switch_form_menu(form, menu, is_on_buttons, false);
			return;
		}

		//
		// Go to the end of the previous field
		//
		forms_driver(form, KEY_CODE_YES, REQ_PREV_FIELD);
		forms_driver(form, KEY_CODE_YES, REQ_END_FIELD);
		return;
	}

	//
	// If the user does not request an other field, get the current field
	// and delegate the input.
	//
	FIELD *field = current_field(form);

	const s_field_user_ptr * driver = (s_field_user_ptr *) field_userptr(field);
	if (driver == NULL) {
		print_exit_str("win_filter_process_input() Unable to get field user ptr!\n");
	}

	driver->driver(form, field, key_type, chr);
}

/***************************************************************************
 * The function processes the input for the filter window / popup. The popup
 * consists of a from with several fields and a menu with buttons. The
 * functions simply delegates the processing to a form or a menu processing
 * function.
 **************************************************************************/

void win_filter_process_input(const int key_type, const wint_t chr) {

	if (is_on_buttons) {
		process_menu_input(filter_form, button_menu, key_type, chr);
	} else {
		process_form_input(filter_form, button_menu, key_type, chr);
	}
}

/***************************************************************************
 * The function touches the window, so that a refresh has an effect.
 **************************************************************************/

void win_filter_show() {

	if (touchwin(win_filter) == ERR) {
		print_exit_str("win_filter_show() Unable to touch filter window!\n");
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_filter_free() {

	print_debug_str("win_filter_free() Removing filter windows, forms and fields.\n");

	forms_free(filter_form, filter_fields);

	menus_free(button_menu, button_items);

	ncurses_win_free(win_sub_form);

	ncurses_win_free(win_sub_menu);

	ncurses_win_free(win_filter);
}

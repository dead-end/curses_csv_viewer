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

#include "ncv_win_filter.h"
#include "ncv_ncurses.h"
#include "ncv_forms.h"
#include "ncv_common.h"

#include <string.h>
#include <ncursesw/form.h>

/***************************************************************************
 * The field label length is an offset for the fields. It is the longest
 * label.
 **************************************************************************/

#define FILTER_FIELD_LABEL_LEN 9

/***************************************************************************
 * The columns of the window consist of the label, the field and the box.
 * The rows consist of the field rows, empty rows between and the box.
 **************************************************************************/

#define WIN_FILTER_SIZE_COLS FILTER_FIELD_LABEL_LEN + FILTER_FIELD_COLS + 2

#define WIN_FILTER_SIZE_ROWS 2 + 1 + 2

/***************************************************************************
 * The necessary field / form / window variables are defined as static.
 **************************************************************************/

static WINDOW* win_filter = NULL;

static WINDOW *win_filter_sub = NULL;

static FIELD *fields[3];

static FORM *filter_form = NULL;

/***************************************************************************
 *
 **************************************************************************/

#define IDX_FILTER 0

#define IDX_CASE 1

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
	// post form and wins
	//
	forms_post_form(filter_form, win_filter, win_filter_sub);

	//
	// Add the filter label. The filter is the last lable, so the cursor is
	// at the right position.
	//
	mvwaddstr(win_filter, 3, 1, "Case:");
	mvwaddstr(win_filter, 3, 12, "]");
	mvwaddstr(win_filter, 3, 10, "[");

	mvwaddstr(win_filter, 1, 1, "Filter:  ");

	if (pos_form_cursor(filter_form) != E_OK) {
		print_exit_str("win_filter_print_content() shit!\n");
	}
}

/***************************************************************************
 * The function initializes the filter window.
 **************************************************************************/

void win_filter_init() {
	chtype attr;

	//
	// Create the filter window.
	//
	win_filter = ncurses_win_create(2 + 1 + 2, 10 + 32 + 2, 0, 0);

	//
	// Move the window to the center
	//
	ncurses_win_center(win_filter);

	//
	// Set the background of the filter window.
	//
	ncurses_attr_back(win_filter, COLOR_PAIR(CP_STATUS), A_REVERSE);

	// TODO: Error / necessary?
	keypad(win_filter, TRUE);

	attr = ncurses_attr_color(COLOR_PAIR(CP_FIELD), A_UNDERLINE);
	fields[IDX_FILTER] = forms_create_field(1, FILTER_FIELD_COLS, 0, 0, attr);

	attr = ncurses_attr_color(COLOR_PAIR(CP_STATUS), A_UNDERLINE);
	fields[IDX_CASE] = forms_create_field(1, 1, 2, 1, attr);
	//forms_set_checkbox(fields[IDX_CASE], false);

	fields[2] = NULL;

	filter_form = forms_create_form(fields);



	//
	// Create a sub win for the form.
	//
	win_filter_sub = derwin(win_filter, getmaxy(win_filter) - 2, getmaxx(win_filter) - 2 - 10, 1, 10);
	if (win_filter_sub == NULL) {
		print_exit("win_filter_init() Unable to create sub window with y: %d x: %d\n", getmaxy(win_filter), getmaxx(win_filter));
	}

	win_filter_print_content();

	//set_current_field(filter_form, fields[0]);
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_filter_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HAS_MIN_SIZE(WIN_FILTER_SIZE_ROWS, WIN_FILTER_SIZE_COLS)) {
		print_debug_str("win_filter_resize() Do resize the window!\n");

		if (ncurses_win_ensure_size(win_filter, WIN_FILTER_SIZE_ROWS, WIN_FILTER_SIZE_COLS)) {

			//
			// On resize do an unpost at the beginning. (Resizing the window does
			// not work well with forms.)
			//
			if (unpost_form(filter_form) != E_OK) {
				print_exit_str("win_filter_resize() Unable to set form to the window!\n");
			}

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
	ncurses_win_refresh_no(win_filter, WIN_FILTER_SIZE_ROWS, WIN_FILTER_SIZE_COLS);
}

/***************************************************************************
 * The function returns the filter window (which is defined static).
 **************************************************************************/

WINDOW *win_filter_get_win() {
	return win_filter;
}

/***************************************************************************
 * The function copies the content of the filter field to the given wchar_t
 * buffer.
 **************************************************************************/

void win_filter_get_filter(wchar_t *buffer, const int buf_size) {
	char *raw_field = field_buffer(fields[0], 0);
	size_t raw_len = strlen(raw_field);
	char str[raw_len + 1];

	//
	// Create a copy of the field content that can be modified (trimmed).
	//
	strncpy(str, raw_field, raw_len);
	str[raw_len] = '\0';

	print_debug("win_filter_get_filter() raw len: '%zu'\n", raw_len);

	//
	// The field content is filled with blanks, which has to be trimmed
	// before the copying.
	//
	const char *trimed = trim(str);

	//
	// Convert and copy.
	//
	mbs_2_wchars(trimed, buffer, buf_size);

	print_debug("win_filter_get_filter() Filter: '%ls'\n", buffer);
}

/***************************************************************************
 * The function deletes the content of the filter field.
 **************************************************************************/

void win_filter_clear_filter() {

	if (set_field_buffer(fields[0], 0, "") != E_OK) {
		print_exit_str("win_filter_clear_filter() Unable to reset the buffer\n");
	}
}

/***************************************************************************
 * The function processes the input from the user. The error handling of
 * key_type is done by the calling function.
 **************************************************************************/

static void win_filter_process_filter_input(const int key_type, const wint_t chr) {

	switch (key_type) {

	case KEY_CODE_YES:

		//
		// Process function keys
		//
		switch (chr) {

		case KEY_DC:
			forms_driver(filter_form, KEY_CODE_YES, REQ_DEL_CHAR);

			//
			// If the field content has changed, do an update.
			//
			forms_driver(filter_form, KEY_CODE_YES, REQ_VALIDATION);
			break;

		case KEY_BACKSPACE:
			forms_driver(filter_form, KEY_CODE_YES, REQ_DEL_PREV);

			//
			// If the field content has changed, do an update.
			//
			forms_driver(filter_form, KEY_CODE_YES, REQ_VALIDATION);
			break;

		case KEY_LEFT:
			forms_driver(filter_form, KEY_CODE_YES, REQ_LEFT_CHAR);
			break;

		case KEY_RIGHT:
			forms_driver(filter_form, KEY_CODE_YES, REQ_RIGHT_CHAR);
			break;

		case KEY_HOME:
			forms_driver(filter_form, KEY_CODE_YES, REQ_BEG_FIELD);
			break;

		case KEY_END:
			forms_driver(filter_form, KEY_CODE_YES, REQ_END_FIELD);
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
			forms_driver(filter_form, OK, (wchar_t) chr);
			forms_driver(filter_form, KEY_CODE_YES, REQ_VALIDATION);
			print_debug("win_filter_process_input() Found char: %d field content: %s\n", chr, field_buffer(fields[0], 0));

		}

		break; // case OK
	}
}

/***************************************************************************
 * The function does the input processing of the form. It processes the up,
 * down and the tab key, which result in a switch of the current field. If
 * an other key was input, the processing is delegated to a special input
 * processor.
 **************************************************************************/

void win_filter_process_input(const int key_type, const wint_t chr) {

	//
	// On key down or tab go to the next field
	//
	if ((key_type == KEY_CODE_YES && chr == KEY_DOWN) || (key_type == OK && chr == L'\t')) {

		forms_driver(filter_form, KEY_CODE_YES, REQ_NEXT_FIELD);
		forms_driver(filter_form, KEY_CODE_YES, REQ_END_FIELD);
		return;
	}

	//
	// On key up go to the previous field
	//
	if (key_type == KEY_CODE_YES && chr == KEY_UP) {

		forms_driver(filter_form, KEY_CODE_YES, REQ_PREV_FIELD);
		forms_driver(filter_form, KEY_CODE_YES, REQ_END_FIELD);
		return;
	}

	//
	// If the user does not request an other field, get the current field
	// and delegate the input.
	//
	FIELD *field = current_field(filter_form);

	if (field == fields[IDX_FILTER]) {
		win_filter_process_filter_input(key_type, chr);

	} else {
		forms_process_checkbox(filter_form, field, key_type, chr);
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

	forms_free(filter_form, fields);

	ncurses_win_free(win_filter_sub);

	ncurses_win_free(win_filter);
}

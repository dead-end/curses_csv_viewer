/*
 * ncv_header.c
 */
#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_filter.h"

/***************************************************************************
 * The necessary field / form / window variables are defined as static.
 **************************************************************************/

static FIELD *field[2];

static FORM *filter_form;

static WINDOW *win_filter_sub;

/***************************************************************************
 * The function initializes the filter window.
 **************************************************************************/

void filter_init(WINDOW *filter_win) {
	int result;

	//
	// Create the field and set the position after the label.
	//
	field[0] = new_field(FILTER_FIELD_ROWS, FILTER_FIELD_COLS, 0, FILTER_FIELD_LABEL_LEN, 0, 0);
	if (field[0] == NULL) {
		print_exit_str("filter_init() Unable to create filter field!\n");
	}

	field[1] = NULL;

	//
	// Switch off autoskip for the field.
	//
	if ((result = field_opts_off(field[0], O_AUTOSKIP)) != E_OK) {
		print_exit("filter_init() Unable to set option: O_AUTOSKIP result: %d\n", result);
	}

	//
	// Create the form and post it.
	//
	filter_form = new_form(field);
	if (filter_form == NULL) {
		print_exit_str("filter_init() Unable to create filter form!\n");
	}

	//
	// Create a sub win for the form.
	//
	win_filter_sub = derwin(filter_win, getmaxy(filter_win), getmaxx(filter_win), 0, 0);
	if (win_filter_sub == NULL) {
		print_exit("filter_init() Unable to create sub window with y: %d x: %d\n", getmaxy(filter_win), getmaxx(filter_win));
	}

	//
	// Set the form to the window and the sub window.
	//
	if ((result = set_form_win(filter_form, filter_win)) != E_OK) {
		print_exit("filter_init() Unable to set form to the window: %d\n", result);
	}

	if ((result = set_form_sub(filter_form, win_filter_sub)) != E_OK) {
		print_exit("filter_init() Unable to set form to the sub window: %d\n", result);
	}

	//
	// Post the form. (E_NO_ROOM is returned if the window is too small)
	//
	if ((result = post_form(filter_form)) != E_OK) {
		print_exit("filter_init() Unable to post filter form: %d\n", result);
	}

	//
	// Add the filter label
	//
	mvwaddstr(filter_win, 0, 0, FILTER_FIELD_LABEL);

	//
	// Refresh the wins
	//
	refresh();
	wrefresh(filter_win);

}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void filter_free() {

	if (unpost_form(filter_form) != E_OK) {
		print_exit_str("filter_free() Unable to unpost form!\n");
	}

	if (free_form(filter_form) != E_OK) {
		print_exit_str("filter_free() Unable to free form!\n");
	}

	if (free_field(field[0]) != E_OK) {
		print_exit_str("filter_free() Unable to free field!\n");
	}

	if (delwin(win_filter_sub) != E_OK) {
		print_exit_str("filter_free() Unable to free sub window!\n");
	}
}

/***************************************************************************
 *
 **************************************************************************/

void filter_loop() {
	wint_t chr;
	int key_type;

	//form_driver(filter_form, REQ_FIRST_FIELD);

	wmove(win_filter, 0, FILTER_FIELD_LABEL_LEN);
	curs_set(1);
	wrefresh(win_filter);

	//
	// Loop through to get user requests
	//
	while (true) {
		key_type = get_wch(&chr);

		switch (key_type) {
		case KEY_CODE_YES:
			switch (chr) {

			case KEY_DC:
				form_driver_w(filter_form, KEY_CODE_YES, REQ_DEL_CHAR);
				wrefresh(win_filter);
				break;

			case KEY_BACKSPACE:
				form_driver_w(filter_form, KEY_CODE_YES, REQ_DEL_PREV);
				wrefresh(win_filter);
				break;

			case KEY_LEFT:
				form_driver_w(filter_form, KEY_CODE_YES, REQ_LEFT_CHAR);
				wrefresh(win_filter);
				break;

			case KEY_RIGHT:
				form_driver_w(filter_form, KEY_CODE_YES, REQ_RIGHT_CHAR);
				wrefresh(win_filter);
				break;

			case KEY_HOME:
				form_driver_w(filter_form, KEY_CODE_YES, REQ_BEG_FIELD);
				wrefresh(win_filter);
				break;

			case KEY_END:
				form_driver_w(filter_form, KEY_CODE_YES, REQ_END_FIELD);
				wrefresh(win_filter);
				break;

				//TODO: where
			case KEY_ENTER:
				curs_set(0);
				return;
				break;

			default:
				break;
			}
			break;
		case OK:
			switch (chr) {

			//TODO: where
			case KEY_ENTER:
				curs_set(0);
				return;
				break;

				// ESC
			case 27:
				curs_set(0);
				return;
				break;

			default:
				form_driver_w(filter_form, OK, (wchar_t) chr);
				wrefresh(win_filter);
				break;
			}
			break;
		}
	}
}

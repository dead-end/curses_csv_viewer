/*
 * ncv_win_filter.c
 */

#include "ncv_win_filter.h"
#include "ncv_ncurses.h"

/***************************************************************************
 * The necessary field / form / window variables are defined as static.
 **************************************************************************/

static FIELD *field[2];

static FORM *filter_form;

static WINDOW *win_filter_sub;

/***************************************************************************
 * The function initializes the filter window.
 **************************************************************************/

void win_filter_init(WINDOW *filter_win) {
	int result;

	//
	// Create the field and set the position after the label.
	//
	field[0] = new_field(FILTER_FIELD_ROWS, FILTER_FIELD_COLS, 0, FILTER_FIELD_LABEL_LEN, 0, 0);
	if (field[0] == NULL) {
		print_exit_str("win_filter_init() Unable to create filter field!\n");
	}

	field[1] = NULL;

	//
	// Switch off autoskip for the field.
	//
	if ((result = field_opts_off(field[0], O_AUTOSKIP)) != E_OK) {
		print_exit("win_filter_init() Unable to set option: O_AUTOSKIP result: %d\n", result);
	}

	if ((result = set_field_back(field[0], COLOR_PAIR(CP_FIELD))) != E_OK) {
		print_exit("win_filter_init() Unable to set field background result: %d\n", result);
	}

	//
	// Create the filter form.
	//
	filter_form = new_form(field);
	if (filter_form == NULL) {
		print_exit_str("win_filter_init() Unable to create filter form!\n");
	}

	//
	// Create a sub win for the form.
	//
	win_filter_sub = derwin(filter_win, getmaxy(filter_win), getmaxx(filter_win), 0, 0);
	if (win_filter_sub == NULL) {
		print_exit("win_filter_init() Unable to create sub window with y: %d x: %d\n", getmaxy(filter_win), getmaxx(filter_win));
	}

	//
	// Set the form to the window and the sub window.
	//
	if ((result = set_form_win(filter_form, filter_win)) != E_OK) {
		print_exit("win_filter_init() Unable to set form to the window: %d\n", result);
	}

	if ((result = set_form_sub(filter_form, win_filter_sub)) != E_OK) {
		print_exit("win_filter_init() Unable to set form to the sub window: %d\n", result);
	}

	//
	// Post the form. (E_NO_ROOM is returned if the window is too small)
	//
	if ((result = post_form(filter_form)) != E_OK) {
		print_exit("win_filter_init() Unable to post filter form: %d\n", result);
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
 * The function is called on resizing the terminal window. The filter window
 * has a constant size. So it has to be ensured that the terminal window is
 * large enough.
 *
 * If the terminal window is smaller the window is resized by ncurses and
 * has to be given the correct size.
 **************************************************************************/

void win_filter_resize() {
	print_debug_str("win_filter_resize() Start resize.");

	//
	// Ensure that the window is large enough for the filter window.
	//
	if (getmaxx(stdscr) - WIN_FILTER_SIZE > 0) {

		//
		// The filter window has a constant size. If the stdscr is too small
		// ncurses resizes the window.
		//
		if (getmaxy(win_filter) != WIN_FILTER_SIZE) {
			ncurses_win_resize(win_filter, 1, WIN_FILTER_SIZE);
		}

		//
		// Move the filter window to the new position.
		//
		ncurses_win_move(win_filter, 0, getmaxx(stdscr) - WIN_FILTER_SIZE);
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_filter_free() {

	if (unpost_form(filter_form) != E_OK) {
		print_exit_str("win_filter_free() Unable to unpost form!\n");
	}

	if (free_form(filter_form) != E_OK) {
		print_exit_str("win_filter_free() Unable to free form!\n");
	}

	if (free_field(field[0]) != E_OK) {
		print_exit_str("win_filter_free() Unable to free field!\n");
	}

	if (delwin(win_filter_sub) != E_OK) {
		print_exit_str("win_filter_free() Unable to free sub window!\n");
	}
}

/***************************************************************************
 *
 **************************************************************************/

void win_filter_loop() {
	wint_t chr;
	int key_type;

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

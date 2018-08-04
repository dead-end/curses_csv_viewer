/*
 * ncv_win_filter.c
 */

#include "ncv_win_filter.h"
#include "ncv_ncurses.h"

//
// The filter window has a fixed size, so the stdscr should have at least
// that size.
//
#define WIN_FILTER_MIN_SIZE (getmaxx(stdscr) - WIN_FILTER_SIZE > 0)

/***************************************************************************
 * The necessary field / form / window variables are defined as static.
 **************************************************************************/

static WINDOW* win_filter = NULL;

static WINDOW *win_filter_sub = NULL;

static FIELD *field[2];

static FORM *filter_form = NULL;

/***************************************************************************
 * The function sets the associated windows of the form and posts the form.
 * It is called with a reset flag. This does an unpost as a first step.
 **************************************************************************/

static void set_form_win_and_post(const bool reset) {
	int result;

	//
	// On reset do an unpost at the beginning.
	//
	if (reset && (result = unpost_form(filter_form)) != E_OK) {
		print_exit("set_form_win_and_post() Unable to set form to the window! (result: %d)\n", result);
	}

	//
	// Set the form to the window and the sub window.
	//
	if ((result = set_form_win(filter_form, win_filter)) != E_OK) {
		print_exit("set_form_win_and_post() Unable to set form to the window! (result: %d)\n", result);
	}

	if ((result = set_form_sub(filter_form, win_filter_sub)) != E_OK) {
		print_exit("set_form_win_and_post() Unable to set form to the sub window! (result: %d)\n", result);
	}

	//
	// Post the form. (E_NO_ROOM is returned if the window is too small)
	//
	if ((result = post_form(filter_form)) != E_OK) {
		print_exit("set_form_win_and_post() Unable to post filter form! (result: %d)\n", result);
	}
}

/***************************************************************************
 * The function initializes the filter window.
 **************************************************************************/

void win_filter_init() {
	int result;

	//
	// Create the filter window. The window has one row and a fixed width.
	//
	win_filter = ncurses_win_create(1, WIN_FILTER_SIZE, 0, getmaxx(stdscr) - WIN_FILTER_SIZE);

	//
	// Set the background of the filter window.
	//
	ncurses_attr_back(win_filter, COLOR_PAIR(CP_STATUS), A_REVERSE);

	keypad(win_filter, TRUE);

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

	//
	// Switch off O_BLANK which deletes the content of the field if the first
	// char is changed.
	//
	if ((result = field_opts_off(field[0], O_BLANK)) != E_OK) {
		print_exit("win_filter_init() Unable to set option: O_AUTOSKIP result: %d\n", result);
	}

	//
	// Set the background of the field.
	//
	if ((result = set_field_back(field[0], ncurses_attr_color(COLOR_PAIR(CP_FIELD), A_UNDERLINE))) != E_OK) {
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
	win_filter_sub = derwin(win_filter, getmaxy(win_filter), getmaxx(win_filter), 0, 0);
	if (win_filter_sub == NULL) {
		print_exit("win_filter_init() Unable to create sub window with y: %d x: %d\n", getmaxy(win_filter), getmaxx(win_filter));
	}

	set_form_win_and_post(false);

	//
	// Add the filter label
	//
	mvwaddstr(win_filter, 0, 0, FILTER_FIELD_LABEL);
}

/***************************************************************************
 * The function is called on resizing the terminal window. The filter window
 * has a constant size. So it has to be ensured that the terminal window is
 * large enough.
 *
 * If the terminal window is smaller, the window is resized by ncurses and
 * has to be given the correct size.
 **************************************************************************/

void win_filter_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_FILTER_MIN_SIZE) {
		print_debug_str("win_filter_resize() Do resize the window!\n");

		//
		// The filter window has a constant size. If the stdscr is too small
		// ncurses has resized the window.
		//
		if (getmaxx(win_filter) != WIN_FILTER_SIZE) {
			print_debug_str("win_filter_resize() Resize win_filter!\n");

			ncurses_win_resize(win_filter, 1, WIN_FILTER_SIZE);

			//
			// Reset the form.
			//
			set_form_win_and_post(true);
		}

		//
		// Add the filter label
		//
		mvwaddstr(win_filter, 0, 0, FILTER_FIELD_LABEL);

		//
		// Move the filter window to the new position.
		//
		ncurses_win_move(win_filter, 0, getmaxx(stdscr) - WIN_FILTER_SIZE);
	}
}

/***************************************************************************
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_filter_refresh_no() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_FILTER_MIN_SIZE) {
		print_debug_str("win_filter_refresh_no() Do refresh the window!\n");

		//
		// Do the refresh.
		//
		if (wnoutrefresh(win_filter) != OK) {
			print_exit_str("win_filter_refresh_no() Unable to refresh the window!\n");
		}
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_filter_free() {

	print_debug_str("win_header_free() Removing filter windows, forms and fields.\n");

	if (filter_form != NULL) {

		if (unpost_form(filter_form) != E_OK) {
			print_exit_str("win_filter_free() Unable to unpost form!\n");
		}

		if (free_form(filter_form) != E_OK) {
			print_exit_str("win_filter_free() Unable to free form!\n");
		}

		if (free_field(field[0]) != E_OK) {
			print_exit_str("win_filter_free() Unable to free field!\n");
		}
	}

	if (win_filter_sub != NULL && delwin(win_filter_sub) != E_OK) {
		print_exit_str("win_filter_free() Unable to free sub window!\n");
	}

	if (win_filter != NULL && delwin(win_filter) != OK) {
		print_exit_str("win_filter_free() Unable to delete filter window!\n");
	}
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
	char *raw_field = field_buffer(field[0], 0);
	size_t raw_len = strlen(raw_field);
	char str[raw_len + 1];

	//
	// Create a copy of the field content that can be modified (trimmed).
	//
	strncpy(str, raw_field, raw_len);
	str[raw_len] = '\0';

	//
	// The field content is filled with blanks, which has to be trimmed
	// before the copying.
	//
	char *trimed = trim(str);

	//
	// Convert and copy.
	//
	mbs_2_wchars(trim(trimed), buffer, buf_size);

	print_debug("win_filter_get_filter() Filter: '%ls'\n", buffer);
}

/***************************************************************************
 * The function deletes the content of the filter field.
 **************************************************************************/

void win_filter_clear_filter() {

	set_field_buffer(field[0], 0, "");
}

/***************************************************************************
 * The function processes the input from the user. The error handling of
 * key_type is done by the calling function.
 **************************************************************************/

void win_filter_process_input(const int key_type, const wint_t chr) {

	switch (key_type) {

	case KEY_CODE_YES:

		//
		// Process function keys
		//
		switch (chr) {

		case KEY_DC:
			form_driver_w(filter_form, KEY_CODE_YES, REQ_DEL_CHAR);

			//
			// If the field content has changed, do an update.
			//
			form_driver_w(filter_form, KEY_CODE_YES, REQ_VALIDATION);
			break;

		case KEY_BACKSPACE:
			form_driver_w(filter_form, KEY_CODE_YES, REQ_DEL_PREV);

			//
			// If the field content has changed, do an update.
			//
			form_driver_w(filter_form, KEY_CODE_YES, REQ_VALIDATION);
			break;

		case KEY_LEFT:
			form_driver_w(filter_form, KEY_CODE_YES, REQ_LEFT_CHAR);
			break;

		case KEY_RIGHT:
			form_driver_w(filter_form, KEY_CODE_YES, REQ_RIGHT_CHAR);
			break;

		case KEY_HOME:
			form_driver_w(filter_form, KEY_CODE_YES, REQ_BEG_FIELD);
			break;

		case KEY_END:
			form_driver_w(filter_form, KEY_CODE_YES, REQ_END_FIELD);
			break;

		default:
			print_debug("win_filter_process_input() Found key code: %d\n", chr);
			break;
		}

		break;

	case OK:

		//
		// Process char keys
		//
		form_driver_w(filter_form, OK, (wchar_t) chr);
		form_driver_w(filter_form, KEY_CODE_YES, REQ_VALIDATION);
		print_debug("win_filter_process_input() Found char: %d field content: %s\n", chr, field_buffer(field[0], 0));

		break;
	}
}

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

#include "ncv_forms.h"
#include "ncv_ncurses.h"
#include "ncv_common.h"
#include <string.h>

/***************************************************************************
 * Definition of the text to be displayed. From the string array, the number
 * of rows and columns can be derived. The values include a border.
 **************************************************************************/

static int win_help_size_cols;

static int win_help_size_rows;

static const char *msgs[] = {
		"CTRL-H        Shows this message.",
		"ESC           Hides dialog window.",
		"CTRL-C CTRL-Q Terminates the program.",
		"CTRL-F        Shows filter dialog.",
		"CTRL-P        Searches previous filter match.",
		"CTRL-N        Searches next filter match.",
		"CTRL-X        Deletes filter content.",
		NULL };

#define BORDER 2

/***************************************************************************
 * Definition of the help window.
 **************************************************************************/

static WINDOW* win_help = NULL;

static WINDOW* win_menu = NULL;

static MENU *menu;

/***************************************************************************
 * The function prints the application label if the help window is large
 * enough.
 **************************************************************************/

static void win_help_print_content() {

	//
	// Add a box
	//
	if (box(win_help, 0, 0) != OK) {
		print_exit_str("win_help_print_content() Unable to setup win!\n");
	}

	//
	// If the help window has enough space for the label, it is printed.
	//
	if (WIN_HAS_MIN_SIZE(win_help_size_rows, win_help_size_cols)) {

		for (int tmp = 0; msgs[tmp] != NULL; tmp++) {
			mvwaddstr(win_help, tmp + BORDER, BORDER, msgs[tmp]);
		}
	}
}

/***************************************************************************
 * The function is called to initialize the help window. If the window
 * size is OK, the label is printed.
 **************************************************************************/

void win_help_init() {
	int rows, cols;

	//
	// Compute the window sizes.
	//
	str_array_sizes(msgs, &rows, &cols);

	win_help_size_cols = cols + 2 * BORDER;
	win_help_size_rows = rows + 2 + 2 * BORDER;

	char *labels[] = { " OK ", NULL };
	menu = menus_create_menu(labels);

	const chtype attr_normal = ncurses_attr_color(COLOR_PAIR(CP_STATUS), A_REVERSE);
	menus_format_menu(menu, attr_normal, true);

	//
	// Reuse of rows, cols to determine the menu sizes.
	//
	if (scale_menu(menu, &rows, &cols) != E_OK) {
		print_exit_str("win_help_init() Unable to determine the menu width and height!\n");
	}

	//
	// Create the help window at (0,0)
	//
	win_help = ncurses_win_create(win_help_size_rows, win_help_size_cols, 0, 0);

	ncurses_attr_back(win_help, COLOR_PAIR(CP_STATUS), A_REVERSE);

	win_menu = ncurses_derwin_create(win_help, rows, cols, win_help_size_rows - BORDER - 1, center(win_help_size_cols, cols));

	menus_set_wins(menu, win_help, win_menu);

	ncurses_win_center(win_help);

	menus_unpost_post(menu, false);

	//
	// Print the label.
	//
	win_help_print_content();

	if (pos_menu_cursor(menu) != E_OK) {
		print_exit_str("popup_pos_cursor() Unable to set the menu cursor!\n");
	}

	menus_switch_on_off(menu, true);

	menus_driver(menu, REQ_FIRST_ITEM);
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_help_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HAS_MIN_SIZE(win_help_size_rows, win_help_size_cols)) {
		print_debug_str("win_help_resize() Do resize the window!\n");

		int rows, cols;
		if (scale_menu(menu, &rows, &cols) != E_OK) {
			print_exit_str("win_help_resize() Unable to determine the menu width and height!\n");
		}

		const bool do_update_win = ncurses_win_ensure_size(win_help, win_help_size_rows, win_help_size_cols);
		const bool do_update_menu = ncurses_win_ensure_size(win_menu, rows, cols);

		if (do_update_win || do_update_menu) {

			//
			// Move the menu window to its position, this requires an repost
			//
			ncurses_derwin_move(win_menu, win_help_size_rows - BORDER - 1, center(win_help_size_cols, cols));

			menus_unpost_post(menu, true);

			//
			// now print the content
			//
			win_help_print_content();
		}

		//
		// Move the window to the center.
		//
		ncurses_win_center(win_help);
	}
}

/***************************************************************************
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_help_refresh_no() {

	print_debug_str("win_help_refresh_no() Refresh help window.\n");
	ncurses_win_refresh_no(win_help, win_help_size_rows, win_help_size_cols);
}

/***************************************************************************
 * The function touches the window, so that a refresh has an effect.
 **************************************************************************/

void win_help_show() {

	if (touchwin(win_help) == ERR) {
		print_exit_str("win_help_show() Unable to touch help window!\n");
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_help_free() {

	print_debug_str("win_help_free() Freeing help window data.\n");

	//
	// Free the menu and the associated items (including the items array).
	//
	menus_free(menu);

	ncurses_win_free(win_menu);

	ncurses_win_free(win_help);
}

/***************************************************************************
 * The function processes the input of the help window. The only input that
 * is relevant is the OK button.
 **************************************************************************/

bool win_help_process_input(const int key_type, const wint_t chr) {
	return key_type == OK && (chr == KEY_ENTER || chr == NCV_KEY_NEWLINE);
}

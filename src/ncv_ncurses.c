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

#include "ncv_ncurses.h"
#include "ncv_common.h"

/******************************************************************************
 * The color mode configuration. On true colors are used if the terminal
 * supports colors. On false monochrome mode is used.
 *****************************************************************************/

static bool use_colors;

static SCREEN *screen = NULL;

/******************************************************************************
 * The function ensures that a new window with a given size and offset fits in
 * its parent window.
 *****************************************************************************/

static void check_subwin_size(WINDOW *win, const int rows, const int cols, const int begin_y, const int begin_x) {

	log_debug("Rows: %d cols: % d begin y: %d x: %d", rows, cols, begin_y, begin_x);

	//
	// Ensure that the minimum size of a window is requested.
	//
	if (rows < 1 || cols < 1) {
		log_exit("Invalid dimension (rows: %d cols: % d)! Ensure that the parent window is large enough.", rows, cols);
	}

	//
	// Ensure that the window fits in its parent window.
	//
	if (rows + begin_y > getmaxy(win) || cols + begin_x > getmaxx(win)) {
		log_exit_str("Window does not fit in its parent window!");
	}
}

/******************************************************************************
 * The function creates a window. It is ensured that the window has a valid
 * size and fits in its parent (which is: stdscr).
 *****************************************************************************/

WINDOW* ncurses_win_create(const int rows, const int cols, const int begin_y, const int begin_x) {
	WINDOW *win;

	check_subwin_size(stdscr, rows, cols, begin_y, begin_x);

	//
	// Create the window.
	//
	if ((win = newwin(rows, cols, begin_y, begin_x)) == NULL) {
		log_exit_str("Unable to create window!");
	}

	return win;
}

/******************************************************************************
 * The function creates a derived window. It is ensured that the window has a
 * valid size and fits in its parent.
 *****************************************************************************/

WINDOW* ncurses_derwin_create(WINDOW *win, const int rows, const int cols, const int begin_y, const int begin_x) {
	WINDOW *win_sub;

	check_subwin_size(win, rows, cols, begin_y, begin_x);

	//
	// Create the sub win
	//
	if ((win_sub = derwin(win, rows, cols, begin_y, begin_x)) == NULL) {
		log_exit_str("Unable to create derived window!");
	}

	return win_sub;
}

/******************************************************************************
 * The function moves a given window to the new position, if its position has
 * changed.
 *****************************************************************************/

void ncurses_win_move(WINDOW *win, const int to_y, const int to_x) {
	int from_y, from_x;

	//
	// Ensure that the target is valid.
	//
	if (to_y < 0 || to_x < 0) {
		log_exit("Win position is not valid - y: %d x: %d", to_y, to_x);
	}

	//
	// Check whether the position changed.
	//
	getbegyx(win, from_y, from_x);

	if (from_y == to_y && from_x == to_x) {
		log_debug("Win position has not changed - y: %d x: %d", from_y, from_x);
		return;
	}

	log_debug("Moving win from y: %d x: %d to y: %d x: %d", from_y, from_x, to_y, to_x);

	//
	// Do the actual moving.
	//
	if (mvwin(win, to_y, to_x) != OK) {
		log_debug("Stdscr max y: %d x: %d", getmaxy(stdscr), getmaxx(stdscr));

		log_debug("Win max y: %d x: %d", getmaxy(win), getmaxx(win));
		log_exit_str("Unable to move window");
	}
}

/******************************************************************************
 * The function moves a given derived window to the new position. The position
 * is relative to the parent window.
 *
 * To check whether the window has an other position, the absolut position of
 * the parent and the derived window has to be computed. With this values the
 * relative position could be computed.
 *****************************************************************************/

void ncurses_derwin_move(WINDOW *win, const int to_y, const int to_x) {

	//
	// Ensure that the target is valid.
	//
	if (to_y < 0 || to_x < 0) {
		log_exit("Win position is not valid - y: %d x: %d", to_y, to_x);
	}

	//
	// Do the actual moving.
	//
	if (mvderwin(win, to_y, to_x) != OK) {
		log_debug("Win max y: %d x: %d", getmaxy(win), getmaxx(win));
		log_exit_str("Unable to move window");
	}
}

/******************************************************************************
 * The function moves a window to the center. This works only if the window is
 * smaller than the terminal. If the window is too large, the function does
 * nothing.
 *****************************************************************************/

void ncurses_win_center(WINDOW *win) {

	//
	// Get the center positions
	//
	const int to_x = (getmaxx(stdscr) - getmaxx(win)) / 2;
	const int to_y = (getmaxy(stdscr) - getmaxy(win)) / 2;

	//
	// Ensure that the window is not too large
	//
	if (to_y < 0 || to_x < 0) {
		log_debug("Win position is not valid - y: %d x: %d", to_y, to_x);
		return;
	}

	log_debug("x stdscr: %d win: %d pos: %d", getmaxx(stdscr), getmaxx(win), to_x);

	log_debug("y stdscr: %d win: %d pos: %d", getmaxy(stdscr), getmaxy(win), to_y);

	ncurses_win_move(win, to_y, to_x);
}

/******************************************************************************
 * The function resizes a window. On success it returns true. It returns false
 * if the size does not changed.
 *
 * The function has to be called only if the size of the window is not trivial.
 * The header and the footer have one row with the full length. This size is
 * updated automatically.
 *****************************************************************************/

bool ncurses_win_resize(WINDOW *win, const int to_y, const int to_x) {
	int from_y, from_x;
	int result;

	//
	// Ensure that the target is valid.
	//
	if (to_y < 1 || to_x < 1) {
		log_exit("Win size is not valid (y: %d x: %d)", to_y, to_x);
	}

	//
	// Check whether the position changed.
	//
	getmaxyx(win, from_y, from_x);

	log_debug("Win size from y: %d x: %d to y: %d x: %d", from_y, from_x, to_y, to_x);

	if (from_y == to_y && from_x == to_x) {
		log_debug("Win size has not changed: y: %d x: %d", to_y, to_x);
		return false;
	}

	//
	// Do the actual resizing.
	//
	if ((result = wresize(win, to_y, to_x)) != OK) {
		log_exit("Unable to resize window (result: %d)", result);
	}

	return true;
}

/******************************************************************************
 * The function is used for windows with constant sizes. If the stdscr is too
 * small ncurses has resized the window. The function checks the desired size
 * and resizes the window if necessary. It returns true if the window has been
 * resized.
 *****************************************************************************/

bool ncurses_win_ensure_size(WINDOW *win, const int y, const int x) {

	if (getmaxy(win) != y || getmaxx(win) != x) {

		log_debug("Resize win to y: %d x: %d!", y, x);
		ncurses_win_resize(win, y, x);

		return true;
	}

	return false;
}

/******************************************************************************
 * The function does a refresh with no update if the terminal is large enough.
 *****************************************************************************/

void ncurses_win_refresh_no(WINDOW *win, const int min_rows, const int min_cols) {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_HAS_MIN_SIZE(min_rows, min_cols)) {
		log_debug_str("Do refresh the window!");

		//
		// Do the refresh.
		//
		if (wnoutrefresh(win) != OK) {
			log_exit_str("Unable to refresh the window!");
		}
	}
}

/******************************************************************************
 * The function frees a window if the window is not null.
 *****************************************************************************/

void ncurses_win_free(WINDOW *win) {

	if (win != NULL && delwin(win) != OK) {
		log_exit_str("Unable to delete window!");
	}
}

/******************************************************************************
 * The function is called with two attributes one for color mode and an
 * alternative attribute if monochrome mode is configured or the terminal does
 * not support colors.
 *****************************************************************************/

chtype ncurses_attr_color(const chtype color, const chtype alt) {

	if (use_colors && has_colors()) {
		return color;
	} else {
		return alt;
	}
}

/******************************************************************************
 * The function sets the background of a window depending on whether the
 * terminal has colors or not.
 *****************************************************************************/

void ncurses_attr_back(WINDOW *win, const chtype color, const chtype alt) {

	if (wbkgd(win, ncurses_attr_color(color, alt)) != OK) {
		log_exit_str("Unable to set background for window!");
	}
}

/******************************************************************************
 * The initializes colors if the terminal supports colors.
 *****************************************************************************/

static void ncurses_init_colors() {

	if (!has_colors()) {
		log_debug_str("Terminal has no colors!");
		return;
	}

	if (start_color() != OK) {
		log_exit_str("Unable to init colors!");
	}

	//
	// table
	//
	if (init_pair(CP_TABLE, COLOR_WHITE, COLOR_BLUE) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	if (init_pair(CP_TABLE_HL, COLOR_RED, COLOR_BLUE) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	//
	// table header
	//
	if (init_pair(CP_TABLE_HEADER, COLOR_YELLOW, COLOR_BLUE) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	if (init_pair(CP_TABLE_HEADER_HL, COLOR_RED, COLOR_BLUE) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	//
	// cursor
	//
	if (init_pair(CP_CURSOR, COLOR_WHITE, COLOR_CYAN) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	if (init_pair(CP_CURSOR_HL, COLOR_RED, COLOR_CYAN) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	//
	// cursor header
	//
	if (init_pair(CP_HEADER_CURSOR, COLOR_YELLOW, COLOR_CYAN) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	if (init_pair(CP_HEADER_CURSOR_HL, COLOR_RED, COLOR_CYAN) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	//
	// rest
	//
	if (init_pair(CP_STATUS, COLOR_BLACK, COLOR_WHITE) != OK) {
		log_exit_str("Unable to init color pair!");
	}

	if (init_pair(CP_MSG, COLOR_WHITE, COLOR_RED) != OK) {
		log_exit_str("Unable to init color pair!");
	}
}

/******************************************************************************
 * The function initializes the ncurses screen. If the csv file is read from
 * stdin, a new screen has to be created.
 *****************************************************************************/

void ncurses_initscr(const bool use_initscr) {

	if (use_initscr) {

		//
		// Use the standard ncurses initsrc.
		//
		if (initscr() == NULL) {
			log_exit_str("Unable to init screen (initscr())!");
		}

	} else {

		//
		// Leave stdin untouched and create a terminal input for ncurses.
		//
		FILE *term_in = fopen("/dev/tty", "r");
		if (term_in == NULL) {
			log_exit_str("Unable to open tty!");
		}

		//
		// Create a screen (which has to be freed).
		//
		screen = newterm(NULL, stdout, term_in);
		if (screen == NULL) {
			log_exit_str("Unable create new terminal screen!");
		}

		set_term(screen);
	}
}

/******************************************************************************
 * The function initializes the ncurses.
 *****************************************************************************/

void ncurses_init(const bool monochrom, const bool use_initscr) {

	use_colors = !monochrom;

	//
	// Initialize ncurses with initscr or a new terminal screen. If we read the
	// csv file from stdin, the latter is required.
	//
	ncurses_initscr(use_initscr);

	//
	// Allow KEY_RESIZE to be read on SIGWINCH
	//
	if (keypad(stdscr, TRUE) != OK) {
		log_exit_str("Unable to call keypad!");
	}

	//
	// Switch off cursor by default
	//
	curs_set(0);

	//
	// Switch off echoing until the filter chars are inputed.
	//
	noecho();

	//
	// Setting raw mode is similar to cbreak, but <crtl>-c and <ctrl>-q key
	// have to be processed. Especially <ctrl>-q can be used to quit the
	// program.
	//
	raw();

	//
	// Set the esc delay to 0. By default the value is greater than 0.
	//
	set_escdelay(0);

	log_debug("Win - y: %d x: %d", getmaxy(stdscr), getmaxx(stdscr));

	//
	// Initialize the colors
	//
	ncurses_init_colors();
}

/******************************************************************************
 * The function frees the ncurses resources.
 *****************************************************************************/

void ncurses_free() {

	//
	// Reset the terminal.
	//
	if (!isendwin()) {
		log_debug_str("Calling: endwin()");
		endwin();
	}

	//
	// Delete screen, if one is present after calling endwin (see man page)
	//
	if (screen != NULL) {
		log_debug_str("Delete screen.");
		delscreen(screen);
	}
}

/******************************************************************************
 * The function prints a string to the first row of a window. The alignment can
 * be given, as well as window attributes. This can be used for highlighting.
 *****************************************************************************/

int nc_cond_addstr_attr(WINDOW *win, const wchar_t *str, const int max, const enum e_align align, const chtype attr_normal, const chtype attr_highlight) {

	const size_t len = wcslen(str);

	const int start = get_align_start(max, len, align);

	log_debug("String: '%ls' len: %zd max: %d", str, len, max);

	//
	// If the window is too small, there is nothing to do.
	//
	if (start < 0) {
		return 0;
	}

	//
	// Switch the attribute if necessary.
	//
	if (attr_normal != attr_highlight) {
		wattrset(win, attr_highlight);
	}

	mvwaddwstr(win, 0, start, str);

	//
	// Switch back the attribute if necessary.
	//
	if (attr_normal != attr_highlight) {
		wattrset(win, attr_normal);
	}

	return len;
}

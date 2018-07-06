/*
 * demo_subwin.c
 *
 */
#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include <string.h>

WINDOW* win_header;
WINDOW* win_main;
WINDOW* win_footer;

#define STR_SIZE 20
static char string[STR_SIZE];

/**
 * The function prints some infos to all of the windows to see
 * if they are updated properly,
 */
static void print_str(const int x, const char* value, const int win_x) {

	if (x + (int) strlen(value) <= win_x) {
		mvwaddstr(win_header, 0, x, value);
		mvwaddstr(win_main, 0, x, value);
		mvwaddstr(win_footer, 0, x, value);
	}
}

/**
 * The function refreshes all windows that are visible. If
 * the terminal is too small, some windows disappear.
 */
static void refresh_all(const int win_y, const int win_x) {

	if (win_x > 0) {

		if (wnoutrefresh(stdscr) != OK) {
			fprintf(stderr, "Unable to refresh the stdscr!\n");
			return;
		}

		if (win_y >= 1 && wnoutrefresh(win_header) != OK) {
			fprintf(stderr, "Unable to refresh the header win!\n");
			return;
		}

		if (win_y >= 2 && wnoutrefresh(win_main) != OK) {
			fprintf(stderr, "Unable to refresh the main win!\n");
			return;
		}

		if (win_y >= 3 && wnoutrefresh(win_footer) != OK) {
			fprintf(stderr, "Unable to refresh the footer win!\n");
			return;
		}

		if (doupdate() != OK) {
			fprintf(stderr, "Unable to update all wins!\n");
			return;
		}
	}
}

/**
 * The function moves the window to the new position, if it
 * has changed.
 */
static void win_move(WINDOW *win, const int to_y, const int to_x) {
	int from_y, from_x;

	//
	// Check whether the position changed.
	//
	getbegyx(win, from_y, from_x);
	if (from_y == to_y && from_x == to_x) {
		fprintf(stderr, "Win position has not changed!\n");
		return;
	}

	fprintf(stderr, "from y: %d x: %d to y: %d x: %d\n", from_y, from_x, to_y, to_x);

	//
	// Do the actual moving.
	//
	if (mvwin(win, to_y, to_x) != OK) {
		fprintf(stderr, "Unable to move window!\n");
		exit(1);
	}
}

/**
 * The function resizes all windows. The footer window has to be moved.
 */
static void do_resize_windows(const int win_y, const int win_x) {

	if (win_x == 0 || win_y == 0) {
		return;
	}

	//
	// The terminal has a min height of 1, so the header is always shown
	// and always at the same position.
	//
	if (wresize(win_header, 1, win_x) != OK) {
		fprintf(stderr, "Unable to resize header window!\n");
		exit(1);
	}

	//
	// Ensure that the main window is not pushed outside the
	// window. If so, move it back.
	//
	if (win_y >= 2) {

		int y = (win_y == 2 ? 1 : win_y - 2);

		//
		// If win_y == 2 then the footer disappeared.
		//
		if (wresize(win_main, y, win_x) != OK) {
			fprintf(stderr, "Unable to resize main window!\n");
			exit(1);
		}

		win_move(win_main, 1, 0);
	}

	//
	// The footer is only visible if the terminal has at least 3 lines.
	//
	if (win_y >= 3) {

		if (wresize(win_footer, 1, win_x) != OK) {
			fprintf(stderr, "Unable to resize footer window!\n");
			exit(1);
		}

		win_move(win_footer, win_y - 1, 0);
	}
}

/**
 *
 */
int main() {
	int win_y, win_x;
	int key_input;

	initscr();

	start_color();
	init_pair(1, COLOR_BLACK, COLOR_WHITE);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);

	getmaxyx(stdscr, win_y, win_x);

	win_header = newwin(1, win_x, 0, 0);
	win_main = newwin(win_y - 2, win_x, 1, 0);
	win_footer = newwin(1, win_x, win_y - 1, 0);

	wbkgd(win_header, COLOR_PAIR(1));
	wbkgd(win_main, COLOR_PAIR(2));
	wbkgd(win_footer, COLOR_PAIR(1));

	curs_set(0);
	keypad(stdscr, TRUE);

	mvwaddstr(win_header, 0, 0, "header");
	mvwaddstr(win_main, 0, 0, "main");
	mvwaddstr(win_footer, 0, 0, "footer");

	snprintf(string, STR_SIZE, "x: %03d y: %03d", win_x, win_y);
	print_str(8, string, win_x);

	refresh_all(win_y, win_x);

	while (true) {

		move(0, 0);
		key_input = getch();

		if (key_input == ERR) {
			continue;

		} else if (key_input == 'q' || key_input == 'Q') {
			break;

		} else if (key_input == KEY_RESIZE) {
			getmaxyx(stdscr, win_y, win_x);
			do_resize_windows(win_y, win_x);
			snprintf(string, STR_SIZE, "x: %03d y: %03d", win_x, win_y);
			print_str(8, string, win_x);
			refresh_all(win_y, win_x);

		} else {
			snprintf(string, STR_SIZE, "key: %d", key_input);
			print_str(23, string, win_x);
			refresh_all(win_y, win_x);
		}
	}

	endwin();
	exit(0);
}

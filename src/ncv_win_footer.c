/*
 * ncv_win_footer.c
 */

#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_table.h"

//
// Size of the string buffer.
//
#define FOOTER_WIN_MAX 1024

//
// The footer is only visible if the terminal has at least 3 rows. One row
// for the header and one row for the table and one row for the footer.
//
#define WIN_FOOTER_MIN_SIZE (getmaxy(stdscr) >= 3)

//
// Definition of the footer window.
//
static WINDOW* win_footer = NULL;

/***************************************************************************
 * The function is called to initialize the footer window.
 **************************************************************************/

void win_footer_init() {

	//
	// Create the footer window in the last row.
	//
	win_footer = ncurses_win_create(1, getmaxx(stdscr), getmaxy(stdscr) - 1, 0);

	//
	// Set the footer window background.
	//
	ncurses_attr_back(win_footer, COLOR_PAIR(CP_STATUS), A_REVERSE);
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void win_footer_resize() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_FOOTER_MIN_SIZE) {
		print_debug_str("win_footer_resize() Do resize the window!\n");

		//
		// Resize the window.
		//
		ncurses_win_resize(win_footer, 1, getmaxx(stdscr));

		//
		// Move the footer to the bottom of the terminal. This is necessary
		// even if the size of the window does not change.
		//
		ncurses_win_move(win_footer, getmaxy(stdscr) - 1, 0);
	}
}

/***************************************************************************
 * The function does a refresh with no update if the terminal is large
 * enough.
 **************************************************************************/

void win_footer_refresh_no() {

	//
	// Ensure the minimum size of the window.
	//
	if (WIN_FOOTER_MIN_SIZE) {
		print_debug_str("win_footer_refresh_no() Do refresh the window!\n");

		//
		// Do the refresh.
		//
		if (wnoutrefresh(win_footer) != OK) {
			print_exit_str("win_footer_refresh_no() Unable to refresh the window!\n");
		}
	}
}

/***************************************************************************
 * The function frees the allocated resources.
 **************************************************************************/

void win_footer_free() {

	print_debug_str("win_footer_free() Removing footer window.\n");

	if (win_footer != NULL && delwin(win_footer) != OK) {
		print_exit_str("win_footer_free() Unable to delete footer window!\n");
	}
}

/***************************************************************************
 * The function prints the footer line, which consists of the current row /
 * column index of the field cursor and the filename. If there is not
 * enough space, the filename is shorten or completely left out.
 **************************************************************************/

void win_footer_content_print(const char *filename, const s_table *table, const s_field *cursor) {
	char buf[FOOTER_WIN_MAX];
	int max_width, strlen_row_col, strlen_filename;

	//
	// Erase window to ensure that no garbage is left behind.
	//
	werase(win_footer);

	max_width = getmaxx(win_footer);

	//
	// Try to print row / column infos
	//
	snprintf(buf, FOOTER_WIN_MAX, " Row: %d/%d Col: %d/%d ", cursor->row + 1, table->no_rows, cursor->col + 1, table->no_columns);
	strlen_row_col = (int) strlen(buf);

	//
	// If the terminal is too small, nothing is printed.
	//
	if (strlen_row_col > max_width) {
		return;
	}

	mvwaddstr(win_footer, 0, max_width - strlen_row_col, buf);

	//
	// Try to print complete filename. It is not checked whether the filename
	// it self is larger than the BUFFER_SIZE.
	//
	snprintf(buf, FOOTER_WIN_MAX, " File: %s ", filename);
	strlen_filename = (int) strlen(buf);

	if (strlen_row_col + strlen_filename <= max_width) {
		mvwaddstr(win_footer, 0, 0, buf);
		return;
	}

	//
	//  Get short filename
	//
	char *short_name = rindex(filename, '/');
	if (short_name == NULL) {
		return;
	}
	short_name++;

	//
	// Try to print the short filename
	//
	snprintf(buf, FOOTER_WIN_MAX, " File: %s ", short_name);
	strlen_filename = (int) strlen(buf);

	if (strlen_row_col + strlen_filename <= max_width) {
		mvwaddstr(win_footer, 0, 0, buf);
	}
}

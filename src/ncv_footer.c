/*
 * ncv_footer.c
 *
 */

#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_table.h"

//
// Size of the string buffer.
//
#define BUFFER_SIZE 1024

/***************************************************************************
 * The function prints the footer line, which consists of the current row /
 * column index of the field cursor and the filename. If there is not
 * enough space, the filename is shorten or completely left out.
 **************************************************************************/

void footer_content_print(const char *filename, const s_table *table, const s_field *cursor) {
	char buf[BUFFER_SIZE];
	int max_width, strlen_row_col, strlen_filename;

	//
	// Erase window to ensure that no garbage is left behind.
	//
	werase(win_footer);

	max_width = getmaxx(win_footer);

	//
	// Try to print row / column infos
	//
	snprintf(buf, BUFFER_SIZE, " Row: %d/%d Col: %d/%d ", cursor->row + 1, table->no_rows, cursor->col + 1, table->no_columns);
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
	snprintf(buf, BUFFER_SIZE, " File: %s ", filename);
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
	snprintf(buf, BUFFER_SIZE, " File: %s ", short_name);
	strlen_filename = (int) strlen(buf);

	if (strlen_row_col + strlen_filename <= max_width) {
		mvwaddstr(win_footer, 0, 0, buf);
	}
}

/***************************************************************************
 * The function is called on resizing the terminal window.
 **************************************************************************/

void footer_resize() {
	//
	// The footer is only visible if the terminal has at least 3 rows. One
	// row for the header and one row for the table and one row for the
	// footer.
	//
	if (getmaxy(stdscr) >= 3) {

		if (wresize(win_footer, 1, getmaxx(stdscr)) != OK) {
			print_exit("ncurses_resize_wins() Unable to resize footer window with y: 1 x: %d\n", getmaxx(stdscr));
		}

		//
		// Move the footer to the bottom of the terminal.
		//
		ncurses_win_move(win_footer, getmaxy(stdscr) - 1, 0);
	}
}

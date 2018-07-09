/*
 * ncv_footer.c
 *
 */

#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_table.h"

/***************************************************************************
 *
 **************************************************************************/

#define BUFFER_SIZE 256

/***************************************************************************
 *
 **************************************************************************/

void win_footer_content_print(const char *filename, const s_table *table, const s_field *cursor) {
	char buf[BUFFER_SIZE];
	int max_width, width;

	//
	// Erase window to ensure that no garbage is present.
	//
	werase(win_footer);

	max_width = getmaxx(win_footer);

	//
	// Try to print row / column infos
	//
	snprintf(buf, BUFFER_SIZE, " Row: %d/%d Col: %d/%d ", cursor->row + 1, table->no_rows, cursor->col + 1, table->no_columns);
	width = (int) strlen(buf);

	if (width > max_width) {
		return;
	}

	mvwaddstr(win_footer, 0, max_width - width, buf);

	//
	// Try to print complete filename
	//
	snprintf(buf, BUFFER_SIZE, " File: %s ", filename);
	int width_file = (int) strlen(buf);

	if (width + width_file <= max_width) {
		mvwaddstr(win_footer, 0, 0, buf);

	} else {
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
		int width_file = (int) strlen(buf);

		if (width + width_file <= max_width) {
			mvwaddstr(win_footer, 0, 0, buf);
		}
	}
}

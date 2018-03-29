/*
 * file: ncv_curses.c
 */

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_curses.h"

/***************************************************************************
 * The function updates the view state with a new start column and a
 * direction. It should be called on a key event with an left or right
 * arrow.
 **************************************************************************/
void s_view_state_update(s_view_state *view_state, const s_table *table, const int col_start, const int direction, const int win_x) {

	//
	// copy the new reference column with its alignment and compute the rest
	//
	view_state->col_start = col_start;
	view_state->direction = direction;

	int sum;
	int precursor = 1;
	int index = view_state->col_start;

	while (true) {
		sum = precursor + table->sizes[index] + 1;

		//
		// if the summed width exceeds the window width we are finished
		//
		if (sum >= win_x) {

			//
			// we found the last column and update the remaining size of the
			// end column
			//
			view_state->col_end = index;
			view_state->rest = win_x - precursor - 1;
			break;
		}

		precursor = sum;
		index += direction;
	}

	s_view_state_debug(view_state);
}

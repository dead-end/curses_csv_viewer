/*
 * file: ncv_curses.h
 */

#ifndef INC_NCV_CURSES_H_
#define INC_NCV_CURSES_H_

//
// If the csv file is large, the table shows only a part of the file. The
// view state struct contains data, that define the visible part.
//
typedef struct s_view_state {

	//
	// The start column it the reference how the view is constructed. The
	// column is fully visible and aligned to the right or left border,
	// depending on the value of direction.
	//
	int col_start;

	//
	// This is the column at the opposite site of the window. Depending on
	// the size of the window, only a part of the column is visible.
	//
	int col_end;

	//
	// The rest member shows the size available for the end column. If rest
	// is the max column size, the end column is fully visible.
	//
	int rest;

	//
	// The direction has the value DIR_FORWARD or DIR_BACKWARD, depending
	// on the side the start column is aligned. If the start column is
	// aligned left the direction is DIR_FORWARD.
	//
	int direction;

	//
	// The row of the csv structure, which is visible at the fist table row.
	//
	int row;

} s_view_state;

#define DIR_FORWARD 1
#define DIR_BACKWARD -1

//
// The view state related function
//
void s_view_state_update(s_view_state *view_state, const s_table *table, const int col_start, const int direction, const int win_x);

#define s_view_state_debug(v) print_debug("s_view_state_debug() col_start: %d direction: %d col_end: %d rest: %d\n", \
		                                  v->col_start, v->direction, v->col_end, v->rest)

#endif /* INC_NCV_CURSES_H_ */

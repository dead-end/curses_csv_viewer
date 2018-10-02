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

#include "ncv_table_part.h"
#include "ncv_common.h"

#include <ncursesw/ncurses.h>

/***************************************************************************
 * Each field can have between 1 and 4 visible corner chars. Each corner can
 * have 4 shapes. The shapes are:
 *
 * - The field corner can be a corner of the table (example: ┌)
 * - The field corner can be top / bottom tee element (example: ┬)
 * - The field corner can be left / right tee element (example: ├)
 * - The field corner can be plus (example: ┼)
 **************************************************************************/

typedef struct s_corner {

	//
	// The 4 shapes of the field corner.
	//
	chtype corner;
	chtype tb_tee;
	chtype lr_tee;
	chtype plus;

	//
	// The parameters hold the indices of the border. It is used to check whether
	// the current field is a corner field. In this case the shape is 'corner'.
	//
	int table_corner_row;
	int table_corner_col;

	//
	// If the row / column of the corner is at the field end, we have to add
	// the field width to get the corner position.
	//
	bool field_start_row;
	bool field_start_col;

} s_corner;

//
// The 4 corners of a field with their position.
//
static s_corner UL_CORNER;
static s_corner UR_CORNER;
static s_corner LR_CORNER;
static s_corner LL_CORNER;

/***************************************************************************
 * The macro initializes a s_corner struct.
 **************************************************************************/

#define s_corner_init(c,CO,TB,LR,PL,ROW,COL,ROW_START,COL_START) \
		c.corner = CO; \
		c.tb_tee = TB; \
		c.lr_tee = LR; \
		c.plus = PL; \
		c.table_corner_row = ROW; \
		c.table_corner_col = COL; \
		c.field_start_row = ROW_START; \
		c.field_start_col = COL_START

/***************************************************************************
 * The function initializes all s_corner structs. It uses the no_rows and
 * no_columns of the table, so it has to be updated when every the table
 * changes.
 **************************************************************************/

void s_corner_inits(const int no_rows, const int no_columns) {

	s_corner_init(UL_CORNER, ACS_ULCORNER, ACS_TTEE, ACS_LTEE, ACS_PLUS, 0, 0, true, true);
	s_corner_init(UR_CORNER, ACS_URCORNER, ACS_TTEE, ACS_RTEE, ACS_PLUS, 0, no_columns - 1, true, false);
	s_corner_init(LL_CORNER, ACS_LLCORNER, ACS_BTEE, ACS_LTEE, ACS_PLUS, no_rows - 1, 0, false, true);
	s_corner_init(LR_CORNER, ACS_LRCORNER, ACS_BTEE, ACS_RTEE, ACS_PLUS, no_rows - 1, no_columns - 1, false, false);
}

/***************************************************************************
 * The function prints a corner char of a field.
 *
 * The index struct contains the row and column of the field in the table.
 *
 * The win_field struct contains the coordinates of the upper left corner of
 * the field in the window.
 *
 * The win_field_end struct contains the coordinates of the lower right
 * corner of the field in the window.
 *
 * The corner struct contains the shapes of the corner. It determines which
 * one of the 4 corners are printed (e.g. UL_CORNER)
 **************************************************************************/

static void print_corner(WINDOW *win, const s_field *idx, const s_field *win_field, const s_field *win_field_end, const s_corner *corner) {
	chtype ch;
	int row, col;

	//
	// The row and column of the field determines the shape of the field
	// corner.
	//
	if (idx->row == corner->table_corner_row) {
		if (idx->col == corner->table_corner_col) {
			ch = corner->corner;
		} else {
			ch = corner->tb_tee;
		}
	} else {
		if (idx->col == corner->table_corner_col) {
			ch = corner->lr_tee;
		} else {
			ch = corner->plus;
		}
	}

	//
	// If the row / col of the corner is at the field end, we have to add
	// the field width to get the corner position.
	//
	if (corner->field_start_row) {
		row = win_field->row;
	} else {
		row = win_field_end->row;
	}

	if (corner->field_start_col) {
		col = win_field->col;
	} else {
		col = win_field_end->col;
	}

	//
	// Print the corner char.
	//
	mvwaddch(win, row, col, ch);
}

/***************************************************************************
 * The function prints the corners of the field. This can be 1, 2 or 4.
 **************************************************************************/

void print_corners(WINDOW *win, const s_field *idx, const s_field *win_field, const s_field *win_field_end, const bool row_untruncated, const bool col_untruncated, const s_corner corners[4]) {

	//
	// This corner is always printed.
	//
	print_corner(win, idx, win_field, win_field_end, &corners[0]);

	//
	// If the row is not truncated an additional corner is printed.
	//
	if (row_untruncated) {
		print_corner(win, idx, win_field, win_field_end, &corners[1]);
	}

	//
	// If the column is not truncated an additional corner is printed.
	//
	if (col_untruncated) {
		print_corner(win, idx, win_field, win_field_end, &corners[2]);
	}

	//
	// If the row and column is not truncated an additional corner is printed.
	//
	if (row_untruncated && col_untruncated) {
		print_corner(win, idx, win_field, win_field_end, &corners[3]);
	}
}

/***************************************************************************
 * The function checks whether a table part is truncated and the index is
 * the index of the untruncated end.
 **************************************************************************/

static bool is_not_truncated_and_border(const s_table_part *table_part, const int idx) {

	//
	// If the table part is truncated (index >= 0) we are ready.
	//
	if (table_part->truncated != -1) {
		return false;
	}

	//
	// If the direction is DIR_FORWARD, the first corner is always visible.
	// If the table part is untruncated and the last corner is also visible.
	//
	if (table_part->direction == DIR_FORWARD) {
		return table_part->last == idx;

	} else {
		return table_part->first == idx;
	}
}

/***************************************************************************
 * The function is called to print a corner char. One of the four corners
 * is always visible. Which one depends on the table_part directions. It is
 * the first element of the s_corner corners[4] array. Two are visible if
 * the row or the col is not truncated and four are visible if both are not
 * truncated.
 **************************************************************************/

void s_corner_print(WINDOW *win, const s_field *idx, const s_field *win_field, const s_field *win_field_end, const s_table_part *row_table_part, const s_table_part *col_table_part) {

	const bool row_untruncated = is_not_truncated_and_border(row_table_part, idx->row);
	const bool col_untruncated = is_not_truncated_and_border(col_table_part, idx->col);

	//
	// print the field corners
	//
	if (row_table_part->direction == DIR_FORWARD) {

		if (col_table_part->direction == DIR_FORWARD) {

			//
			// corners - row: FORWARD / column: FORWARD
			//
			const s_corner corners[4] = { UL_CORNER, LL_CORNER, UR_CORNER, LR_CORNER };
			print_corners(win, idx, win_field, win_field_end, row_untruncated, col_untruncated, corners);

		} else {

			//
			// corners - row: FORWARD / column: BACKWARD
			//
			const s_corner corners[4] = { UR_CORNER, LR_CORNER, UL_CORNER, LL_CORNER };
			print_corners(win, idx, win_field, win_field_end, row_untruncated, col_untruncated, corners);
		}

	} else {

		if (col_table_part->direction == DIR_FORWARD) {

			//
			// corners - row: BACKWARD / column: FORWARD
			//
			const s_corner corners[4] = { LL_CORNER, UL_CORNER, LR_CORNER, UR_CORNER };
			print_corners(win, idx, win_field, win_field_end, row_untruncated, col_untruncated, corners);

		} else {

			//
			// corners - row: BACKWARD / column: BACKWARD
			//
			const s_corner corners[4] = { LR_CORNER, UR_CORNER, LL_CORNER, UL_CORNER };
			print_corners(win, idx, win_field, win_field_end, row_untruncated, col_untruncated, corners);
		}
	}
}

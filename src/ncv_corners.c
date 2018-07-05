/*
 * ncv_corners.c
 *
 */

#include "ncv_corners.h"

/***************************************************************************
 * The function and the macro are used to setup the corner definitions.
 **************************************************************************/

#define s_corner_init(c,CO,TB,LR,PL,ROW,COL) \
		c.corner = CO; \
		c.tb_tee = TB; \
		c.lr_tee = LR; \
		c.plus = PL; \
		c.table_corner_row = ROW; \
		c.table_corner_col = COL

void s_corner_inits(const s_table *table) {

	s_corner_init(UL_CORNER, ACS_ULCORNER, ACS_TTEE, ACS_LTEE, ACS_PLUS, 0, 0);
	s_corner_init(UR_CORNER, ACS_URCORNER, ACS_TTEE, ACS_RTEE, ACS_PLUS, 0, table->no_columns - 1);
	s_corner_init(LL_CORNER, ACS_LLCORNER, ACS_BTEE, ACS_LTEE, ACS_PLUS, table->no_rows - 1, 0);
	s_corner_init(LR_CORNER, ACS_LRCORNER, ACS_BTEE, ACS_RTEE, ACS_PLUS, table->no_rows - 1, table->no_columns - 1);
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
	// The row and column of the corner also determines if we have to add the
	// field width / height to determine the position of the corner.
	//
	if (0 == corner->table_corner_row) {
		row = win_field->row;
	} else {
		row = win_field_end->row;
	}

	if (0 == corner->table_corner_col) {
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
 * The function prints the corners of the field. This is at least 1 and at
 * most 4.
 **************************************************************************/

// TODO: rename the parameters
void print_corners(WINDOW *win, const s_field *idx, const s_field *win_field, const s_field *win_field_end, const bool row_cond, const bool col_cond, const s_corner *yy, const s_corner *ny, const s_corner *yn, const s_corner *nn) {

	//
	// This corner is always printed.
	//
	print_corner(win, idx, win_field, win_field_end, yy);

	//
	// if the row is not truncated an additional border is printed.
	//
	if (row_cond) {
		print_corner(win, idx, win_field, win_field_end, ny);
	}

	//
	// if the column is not truncated an additional border is printed.
	//
	if (col_cond) {
		print_corner(win, idx, win_field, win_field_end, yn);
	}

	//
	// if the row and column is not truncated an additional border is printed.
	//
	if (row_cond && col_cond) {
		print_corner(win, idx, win_field, win_field_end, nn);
	}
}

/*
 * ncv_corners.h
 */

#ifndef INC_NCV_CORNERS_H_
#define INC_NCV_CORNERS_H_

#include <ncursesw/ncurses.h>
#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_field.h"

/***************************************************************************
 * Each field can have up to 4 corner chars. Each corner can have 4 shapes.
 * The shapes are:
 *
 * - The field corner can be a corner of the table (example: ┌)
 * - The field corner can be top / bottom tee element (example: ┬)
 * - The field corner can be left / right tee element (example: ├)
 * - The field corner can be plus (example: ┼)
 *
 * Additionally it has a row and a column. This is the row and column if the
 * field corner is a corner of the table.
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
	// The row and column of the field, where the corner member of the struct
	// is applied. This is the row and column of the corresponding table
	// corner.
	//
	int table_corner_row;
	int table_corner_col;
} s_corner;

//
// The 4 corners of a field with their position.
//
s_corner UL_CORNER;
s_corner UR_CORNER;
s_corner LR_CORNER;
s_corner LL_CORNER;

/***************************************************************************
 * Function definitions
 **************************************************************************/

void s_corner_inits(const s_table *table);

void print_corners(WINDOW *win, const s_field *idx, const s_field *win_field, const s_field *win_field_end, const bool row_untruncated, const bool col_untruncated, const s_corner *yy, const s_corner *ny, const s_corner *yn, const s_corner *nn);

#endif /* INC_NCV_CORNERS_H_ */

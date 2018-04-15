/*
 * file: ncv_curses.h
 */

#ifndef INC_NCV_CURSES_H_
#define INC_NCV_CURSES_H_

//
// constances that affect for loop directions
//
#define DIR_FORWARD 1
#define DIR_BACKWARD -1

/***************************************************************************
 * The table has two s_table_part structures, one for the row and one for
 * the column. The structure defines which part of the table is visible.
 ***************************************************************************/

typedef struct s_table_part {

	//
	// the index of the first row / column that is visible
	//
	int start;

	//
	// the index of the last row / column that is visible
	//
	int end;

	//
	// the index of the row / column that is truncated. The value is equal to
	// start or end
	//
	int truncated;

	//
	// the size of the truncated row / column
	//
	int size;

} s_table_part;

/***************************************************************************
 * A field has two s_field_part structure, one for the row and one for the
 * column. The structure defines which part of the field is visible. If the
 * field is not truncated, start is 0  and size if the row height or the
 * column width.
 ***************************************************************************/

typedef struct s_field_part {

	//
	// row:    the first line that is visible
	// column: the first char that is visible
	//
	int start;

	//
	// row:    the number of lines that are visible
	// column: the number of chars that are visible
	//
	int size;

} s_field_part;

/***************************************************************************
 *
 **************************************************************************/

typedef struct s_ref_point {

	int row;

	int col;

	int aligned;

} s_ref_point;

/***************************************************************************
 * Function definitions
 **************************************************************************/

//
// functions that are used throughout the program
//

void curses_init();

void curses_finish();

void curses_loop(const s_table *table);

//
// functions that are exported for tests
//

void s_field_part_update(const s_table_part *table_part, const int size, const int index, s_field_part *field_part);

void s_table_part_update(s_table_part *table_part, const int *sizes, const int index_start, const int index_max, const int direction, const int win_size);

wchar_t *field_truncated_line(wchar_t *str_ptr, wchar_t *buffer, s_field_part *col_field_part);


#endif /* INC_NCV_CURSES_H_ */

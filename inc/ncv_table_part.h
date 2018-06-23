/*
 * ncv_table_part.h
 */

#ifndef INC_NCV_TABLE_PART_H_
#define INC_NCV_TABLE_PART_H_

//
// Direction definitions that allow to be used in increments of loops
//
#define DIR_FORWARD 1

#define DIR_BACKWARD -1

/***************************************************************************
 * The table has two s_table_part structures, one for the row and one for
 * the column. The structure defines which part of the table is visible and
 * which fields are truncated.
 ***************************************************************************/

typedef struct s_table_part {

	//
	// The index of the first row / column that is visible.
	//
	int first;

	//
	// The index of the last row / column that is visible.
	//
	int last;

	//
	// The index of the row / column that is truncated. The value is equal to
	// start or end or -1 if none of them is truncated.
	//
	int truncated;

	//
	// The size of the truncated row / column. If truncated is -1 size is 0.
	//
	int size;

	//
	// One of the values first or last is given and the other value is
	// computed with a direction. If first is given, direction is forward
	// and last is computed. If last is give, the direction is backward
	// and first is computed.
	//
	int direction;

} s_table_part;

/***************************************************************************
 * Macro definitions
 **************************************************************************/

//
// The macro is called with a table part and an index and it checks whether
// the given index is before the table part first value.
//
#define is_index_before_first(p,i) (i < (p)->first || (i == (p)->first && (p)->first == (p)->truncated))

//
// The macro is called with a table part and an index and it checks whether
// the given index is before the table part last value.
//
#define is_index_after_last(p,i) (i > (p)->last || (i == (p)->last && (p)->last == (p)->truncated))

//
// The macro is called with a table part and returns the first or the last
// table part index depending on the direction.
//
#define s_table_part_start(p) ((p)->direction == DIR_FORWARD ? (p)->first : (p)->last)

//
// The macro ensures that a table part is not truncated and the current index
// is "first".
//
#define is_not_truncated_and_first(p, i) ((p)->truncated == -1 && (i) == (p)->first)

//
// The macro ensures that a table part is not truncated and the current index
// is "last".
//
#define is_not_truncated_and_last(p, i) ((p)->truncated == -1 && (i) == (p)->last)

/***************************************************************************
 * Function definitions
 **************************************************************************/

void s_table_part_update(s_table_part *table_part, const int *sizes, const int index_start, const int index_max, const int direction, const int win_size);

bool adjust_dir_on_resize(s_table_part *table_part, const int table_end);

#endif /* INC_NCV_TABLE_PART_H_ */

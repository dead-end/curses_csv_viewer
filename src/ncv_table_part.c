/*
 * ncv_table_part.c
 */

#include "ncv_common.h"
#include "ncv_table_part.h"

/***************************************************************************
 * The method updates the row /column table part for the table.
 *
 * The sizes parameter is an array with the row heights or the column
 * widths.
 *
 * The index_start can be the first or the last index depending on the
 * direction.
 *
 * The num parameter defines the number of rows or columns.
 *
 * The direction can be DIR_FORWARD or DIR_BACKWARD.
 *
 * The win_size parameter is the size of the window.
 **************************************************************************/

void s_table_part_update(s_table_part *table_part, const int *sizes, const int index_start, const int num, const int direction, const int win_size) {

	int precursor = 0;
	int sum;

	table_part->first = index_start;
	table_part->truncated = -1;
	table_part->size = 0;
	table_part->direction = direction;

	for (table_part->last = index_start; 0 <= table_part->last && table_part->last < num; table_part->last += direction) {

		//
		// Sum up widths / heights with their borders.
		//
		sum = precursor + sizes[table_part->last] + 1;

		//
		// If sum of width / heights with a border is the win_size - 1 it
		// fits in the window with an additional border.
		//
		// Example:
		// (border)(field)(border)(field)(additional-border)
		//
		if (sum == win_size - 1) {
			break;

			//
			// If the sum of width / heights is larger than or equal to the
			// window size, the part has to be truncated.
			//
		} else if (sum >= win_size) {
			table_part->truncated = table_part->last;
			table_part->size = win_size - precursor - 1;
			break;
		}

		precursor = sum;
	}

	//
	// If the end member is not inside the boundaries, the window is larger
	// than necessary.
	//
	if (table_part->last < 0) {
		table_part->last = 0;

	} else if (table_part->last >= num) {
		table_part->last = num - 1;
	}

	//
	// If the direction is backwards swap the first and the last.
	//
	if (direction == DIR_BACKWARD) {
		const int tmp = table_part->first;
		table_part->first = table_part->last;
		table_part->last = tmp;
	}

	print_debug("s_table_part_update() first: %d last: %d truncated: %d size: %d dir: %d\n", table_part->first, table_part->last, table_part->truncated, table_part->size, table_part->direction);
}

/***************************************************************************
 * The function reverses the direction of a table part in certain
 * situations.
 **************************************************************************/

bool adjust_dir_on_resize(s_table_part *table_part, const int table_end) {

	//
	// If a row / column is truncated everything is ok
	//
	if (table_part->truncated != -1) {
		return false;
	}

	//
	// On enlarging of the window the fist table row / column has completely appeared.
	//
	if (table_part->first == 0 && table_part->last != table_end && table_part->direction == DIR_BACKWARD) {
		table_part->direction = DIR_FORWARD;
		return true;
	}

	//
	// On enlarging of the window the last table row / column has completely appeared.
	//
	if (table_part->first != 0 && table_part->last == table_end && table_part->direction == DIR_FORWARD) {
		table_part->direction = DIR_BACKWARD;
		return true;
	}

	return false;
}

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

#ifndef INC_NCV_TABLE_PART_H_
#define INC_NCV_TABLE_PART_H_

//
// Defintion of directions (can be used to increment in loops)
//
#define DIR_FORWARD 1

#define DIR_BACKWARD -1

/***************************************************************************
 * The table has two s_table_part structures, one for the row and one for
 * the column. The structure defines which fields of the table are visible
 * and which fields are truncated.
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

void s_table_part_update(s_table_part *table_part, const int *sizes, const int index_start, const int index_max, const int direction, const int win_size);

bool adjust_dir_on_resize(s_table_part *table_part, const int table_end);

#endif

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

#include "ut_utils.h"
#include "ncv_table_part.h"

#include <locale.h>

/******************************************************************************
 * The function checks whether a s_table_part structure has the expected
 * values.
 *****************************************************************************/

static void check_table_part(const s_table_part *table_part, const int first, const int last, const int truncated, const int size) {

	ut_check_int(table_part->first, first, "s_table_part: first");

	ut_check_int(table_part->last, last, "s_table_part: last");

	ut_check_int(table_part->truncated, truncated, "s_table_part: truncated");

	ut_check_int(table_part->size, size, "s_table_part: size");
}

/******************************************************************************
 * The function checks the s_table_part_update function, which computes the
 * visible fields of the table for a row or column.
 *****************************************************************************/

static void test_table_part_update() {
	s_table_part table_part;

	print_debug_str("test_table_part_update() Start\n");

	//
	// 1234567890 <- win size == 10
	// |****|*|***|
	//          ^
	//
	print_debug_str("test_table_part_update() test 1\n");
	int sizes_1[3] = { 4, 1, 3 };
	s_table_part_update(&table_part, sizes_1, 0, 3, DIR_FORWARD, 10);
	check_table_part(&table_part, 0, 2, 2, 2);

	//
	// 1234567890 <- win size == 10
	// |****|***|*|
	//          ^
	//
	print_debug_str("test_table_part_update() test 2\n");
	int sizes_2[3] = { 4, 3, 1 };
	s_table_part_update(&table_part, sizes_2, 0, 3, DIR_FORWARD, 10);
	check_table_part(&table_part, 0, 1, -1, 0);

	//
	// 1234567890 <- win size == 10
	// |****|**|***|
	//          ^
	//
	print_debug_str("test_table_part_update() test 3\n");
	int sizes_3[3] = { 4, 2, 3 };
	s_table_part_update(&table_part, sizes_3, 0, 3, DIR_FORWARD, 10);
	check_table_part(&table_part, 0, 2, 2, 1);

	//
	// 12345678901234567890 <- win size == 20
	// |****|*|***|
	//            ^
	//
	print_debug_str("test_table_part_update() test 4\n");
	int sizes_4[3] = { 4, 1, 3 };
	s_table_part_update(&table_part, sizes_4, 0, 3, DIR_FORWARD, 20);
	check_table_part(&table_part, 0, 2, -1, 0);

	//
	//   1234567890 <- win size == 10
	// |***|*|****|
	//   ^
	//
	print_debug_str("test_table_part_update() test 5\n");
	int sizes_5[3] = { 3, 1, 4 };
	s_table_part_update(&table_part, sizes_5, 2, 3, DIR_BACKWARD, 10);
	check_table_part(&table_part, 0, 2, 0, 2);

	//
	//   1234567890 <- win size == 10
	// |*|***|****|
	//   ^
	//
	print_debug_str("test_table_part_update() test 6\n");
	int sizes_6[3] = { 1, 3, 4 };
	s_table_part_update(&table_part, sizes_6, 2, 3, DIR_BACKWARD, 10);
	check_table_part(&table_part, 1, 2, -1, 0);

	//
	//    1234567890 <- win size == 10
	// |***|**|****|
	//    ^
	//
	print_debug_str("test_table_part_update() test 7\n");
	int sizes_7[3] = { 3, 2, 4 };
	s_table_part_update(&table_part, sizes_7, 2, 3, DIR_BACKWARD, 10);
	check_table_part(&table_part, 0, 2, 0, 1);

	//
	// 12345678901234567890 <- win size == 20
	//         |***|*|****|
	//         ^
	//
	print_debug_str("test_table_part_update() test 8\n");
	int sizes_8[3] = { 3, 1, 4 };
	s_table_part_update(&table_part, sizes_8, 2, 3, DIR_BACKWARD, 20);

	print_debug_str("test_table_part_update() End\n");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	print_debug_str("ut_table_part.c - Start tests\n");

	setlocale(LC_ALL, "");

	test_table_part_update();

	print_debug_str("ut_table_part.c - End tests\n");

	return EXIT_SUCCESS;
}

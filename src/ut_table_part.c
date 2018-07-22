/*
 * file: ut_curses.c
 */

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_table_part.h"
#include "ncv_field.h"
#include "ncv_parser.h"

/***************************************************************************
 * The function checks whether a s_table_part struct has the expected
 * values.
 **************************************************************************/

static void check_table_part(const s_table_part *table_part, const int first, const int last, const int truncated, const int size, const char *msg) {

	print_debug("check_table_part() %s\n", msg);

	ut_check_int(table_part->first, first, "s_table_part: first");

	ut_check_int(table_part->last, last, "s_table_part: last");

	ut_check_int(table_part->truncated, truncated, "s_table_part: truncated");

	ut_check_int(table_part->size, size, "s_table_part: size");
}

/***************************************************************************
 * The function checks the s_table_part_update function, which computes the
 * visible part of the table for a row or column.
 **************************************************************************/

static void test_table_part_update() {
	s_table_part table_part;

	print_debug_str("test_table_part_update() Start\n");

	//
	// 1234567890 <- win size
	// |****|*|***|
	//          ^
	//
	int sizes_1[3] = { 4, 1, 3 };
	s_table_part_update(&table_part, sizes_1, 0, 3, DIR_FORWARD, 10);
	check_table_part(&table_part, 0, 2, 2, 2, "test 1");

	//
	// 1234567890 <- win size
	// |****|***|*|
	//          ^
	//
	int sizes_2[3] = { 4, 3, 1 };
	s_table_part_update(&table_part, sizes_2, 0, 3, DIR_FORWARD, 10);
	check_table_part(&table_part, 0, 1, -1, 0, "test 2");

	//
	// 1234567890 <- win size
	// |****|**|***|
	//          ^
	//
	int sizes_3[3] = { 4, 2, 3 };
	s_table_part_update(&table_part, sizes_3, 0, 3, DIR_FORWARD, 10);
	check_table_part(&table_part, 0, 2, 2, 1, "test 3");

	//
	// 12345678901234567890 <- win size
	// |****|*|***|
	//            ^
	//
	int sizes_4[3] = { 4, 1, 3 };
	s_table_part_update(&table_part, sizes_4, 0, 3, DIR_FORWARD, 20);
	check_table_part(&table_part, 0, 2, -1, 0, "test 4");

	//
	//   1234567890 <- win size
	// |***|*|****|
	//   ^
	//
	int sizes_5[3] = { 3, 1, 4 };
	s_table_part_update(&table_part, sizes_5, 2, 3, DIR_BACKWARD, 10);
	check_table_part(&table_part, 0, 2, 0, 2, "test 5");

	//
	//   1234567890 <- win size
	// |*|***|****|
	//   ^
	//
	int sizes_6[3] = { 1, 3, 4 };
	s_table_part_update(&table_part, sizes_6, 2, 3, DIR_BACKWARD, 10);
	check_table_part(&table_part, 1, 2, -1, 0, "test 6");

	//
	//    1234567890 <- win size
	// |***|**|****|
	//    ^
	//
	int sizes_7[3] = { 3, 2, 4 };
	s_table_part_update(&table_part, sizes_7, 2, 3, DIR_BACKWARD, 10);
	check_table_part(&table_part, 0, 2, 0, 1, "test 7");

	//
	// 12345678901234567890 <- win size
	//         |***|*|****|
	//         ^
	//
	int sizes_8[3] = { 3, 1, 4 };
	s_table_part_update(&table_part, sizes_8, 2, 3, DIR_BACKWARD, 20);
	check_table_part(&table_part, 0, 2, -1, 0, "test 8");

	print_debug_str("test_table_part_update() End\n");
}

/***************************************************************************
 * The main function simply starts the test.
 **************************************************************************/

int main() {

	print_debug_str("ut_table_part.c - Start tests\n");

	setlocale(LC_ALL, "");

	test_table_part_update();

	print_debug_str("ut_table_part.c - End tests\n");

	return EXIT_SUCCESS;
}

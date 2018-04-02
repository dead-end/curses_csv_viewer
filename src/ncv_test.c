/*
 * file: ncv_test.c
 */

#include <locale.h>

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_parser.h"
#include "ncv_curses.h"

/***************************************************************************
 * The function checks whether an int parameter has the expected value or
 * not.
 **************************************************************************/

static void check_int(const int current, const int expected, const char *msg) {

	if (current != expected) {
		print_exit("check_int() %s current: %d expected: %d\n", msg, current, expected);
	}

	print_debug("check_int() %s current: %d OK\n", msg, current);
}

/***************************************************************************
 * The function compares two strings.
 **************************************************************************/

static void compare(wchar_t *str1, wchar_t *str2) {
	if (wcscmp(str1, str2) != 0) {
		print_exit("compare() Strings differ: %ls and: %ls\n", str1, str2);
	}
}

/***************************************************************************
 * The function reads and parses a csv file. All fields are compared with
 * the expected values.
 **************************************************************************/

static void test1() {
	s_table table;

	print_debug_str("test1() Start\n");

	parser_process_file("res/test1.csv", W_DELIM, &table);

	//
	// check all fields "by hand"
	//
	compare(table.fields[0][0], L"f00");
	compare(table.fields[0][1], L"f01");
	compare(table.fields[0][2], L"f02");

	compare(table.fields[1][0], L"f10");
	compare(table.fields[1][1], L"f11->\n\"d111\"");
	compare(table.fields[1][2], L"f12");

	compare(table.fields[2][0], L"f20");
	compare(table.fields[2][1], L"f21");
	compare(table.fields[2][2], L"f22");

	compare(table.fields[3][0], L"f30->,d30");
	compare(table.fields[3][1], L"f31");
	compare(table.fields[3][2], L"f32->\nd32");

	compare(table.fields[4][0], L"");
	compare(table.fields[4][1], L"end");
	compare(table.fields[4][2], L"");

	//
	// check the meta data
	//
	check_int(table.width[0], 9, "col width: 0");
	check_int(table.width[1], 6, "col width: 1");
	check_int(table.width[2], 5, "col width: 2");

	check_int(table.height[0], 1, "row_height: 0");
	check_int(table.height[1], 2, "row_height: 1");
	check_int(table.height[2], 1, "row_height: 2");
	check_int(table.height[3], 2, "row_height: 3");
	check_int(table.height[4], 1, "row_height: 4");

	s_table_free(&table);

	print_debug_str("test1() End\n");
}

/***************************************************************************
 *
 **************************************************************************/

static void check_start_end(const s_start_end *start_end, const int start, const int end, const int truncated, const int size, const char *msg) {

	print_debug("check_start_end() %s\n", msg);

	check_int(start_end->start, start, "s_start_end: start");

	check_int(start_end->end, end, "s_start_end: end");

	check_int(start_end->truncated, truncated, "s_start_end: truncated");

	check_int(start_end->size, size, "s_start_end: size");
}

static void test2() {
	s_start_end start_end;

	print_debug_str("test2() Start\n");

	//
	// 1234567890 <- win size
	// |****|*|***|
	//          ^
	//
	int sizes_1[3] = { 4, 1, 3 };
	s_start_end_update(&start_end, sizes_1, 0, 3, DIR_FORWARD, 10);
	check_start_end(&start_end, 0, 2, 2, 1, "test 1");

	//
	// 1234567890 <- win size
	// |****|***|*|
	//          ^
	//
	int sizes_2[3] = { 4, 3, 1 };
	s_start_end_update(&start_end, sizes_2, 0, 3, DIR_FORWARD, 10);
	check_start_end(&start_end, 0, 1, -1, 0, "test 2");

	//
	// 1234567890 <- win size
	// |****|**|***|
	//          ^
	//
	int sizes_3[3] = { 4, 2, 3 };
	s_start_end_update(&start_end, sizes_3, 0, 3, DIR_FORWARD, 10);
	check_start_end(&start_end, 0, 2, 2, 0, "test 3");

	//
	// 12345678901234567890 <- win size
	// |****|*|***|
	//            ^
	//
	int sizes_4[3] = { 4, 1, 3 };
	s_start_end_update(&start_end, sizes_4, 0, 3, DIR_FORWARD, 20);
	check_start_end(&start_end, 0, 2, -1, 0, "test 4");

	//
	//   1234567890 <- win size
	// |***|*|****|
	//   ^
	//
	int sizes_5[3] = { 3, 1, 4 };
	s_start_end_update(&start_end, sizes_5, 2, 3, DIR_BACKWARD, 10);
	check_start_end(&start_end, 0, 2, 0, 1, "test 5");

	//
	//   1234567890 <- win size
	// |*|***|****|
	//   ^
	//
	int sizes_6[3] = { 1, 3, 4 };
	s_start_end_update(&start_end, sizes_6, 2, 3, DIR_BACKWARD, 10);
	check_start_end(&start_end, 1, 2, -1, 0, "test 6");

	//
	//    1234567890 <- win size
	// |***|**|****|
	//    ^
	//
	int sizes_7[3] = { 3, 2, 4 };
	s_start_end_update(&start_end, sizes_7, 2, 3, DIR_BACKWARD, 10);
	check_start_end(&start_end, 0, 2, 0, 0, "test 7");

	//
	// 12345678901234567890 <- win size
	//         |***|*|****|
	//         ^
	//
	int sizes_8[3] = { 3, 1, 4 };
	s_start_end_update(&start_end, sizes_8, 2, 3, DIR_BACKWARD, 20);
	check_start_end(&start_end, 0, 2, -1, 0, "test 8");


	print_debug_str("test2() End\n");
}

/***************************************************************************
 * The main functon simply starts the test.
 **************************************************************************/

int main(const int argc, char * const argv[]) {

	print_debug_str("main() Start\n");

	setlocale(LC_ALL, "");

	test1();

	test2();

	print_debug_str("main() End\n");

	return EXIT_SUCCESS;
}

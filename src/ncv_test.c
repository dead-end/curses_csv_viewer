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

static void check_str(wchar_t *str1, wchar_t *str2) {

	if (wcscmp(str1, str2) != 0) {
		print_exit("check_str() Strings differ: '%ls' and: '%ls'\n", str1, str2);
	}

	print_debug("check_str() Strings OK: '%ls' and: '%ls'\n", str1, str2);
}

/***************************************************************************
 * The function ensures that a given string is null.
 **************************************************************************/

static void check_null(wchar_t *str) {

	if (str != NULL) {
		print_exit_str("check_null() Pointer is not null!\n");
	}

	print_debug("check_null() Pointer is expected null\n");
}

/***************************************************************************
 * The function reads and parses a csv file. All fields are compared with
 * the expected values.
 **************************************************************************/

static void test_parser() {
	s_table table;

	print_debug_str("test_parser() Start\n");

	parser_process_file("res/test1.csv", W_DELIM, &table);

	//
	// check all fields "by hand"
	//
	check_str(table.fields[0][0], L"f00");
	check_str(table.fields[0][1], L"f01");
	check_str(table.fields[0][2], L"f02");

	check_str(table.fields[1][0], L"f10");
	check_str(table.fields[1][1], L"f11->\n\"d111\"");
	check_str(table.fields[1][2], L"f12");

	check_str(table.fields[2][0], L"f20");
	check_str(table.fields[2][1], L"f21");
	check_str(table.fields[2][2], L"f22");

	check_str(table.fields[3][0], L"f30->,d30");
	check_str(table.fields[3][1], L"f31");
	check_str(table.fields[3][2], L"f32->\nd32");

	check_str(table.fields[4][0], L"");
	check_str(table.fields[4][1], L"end");
	check_str(table.fields[4][2], L"");

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

	print_debug_str("test_parser() End\n");
}

/***************************************************************************
 *
 **************************************************************************/

static void check_table_part(const s_table_part *table_part, const int start, const int end, const int truncated, const int size, const char *msg) {

	print_debug("check_table_part() %s\n", msg);

	check_int(table_part->start, start, "s_table_part: start");

	check_int(table_part->end, end, "s_table_part: end");

	check_int(table_part->truncated, truncated, "s_table_part: truncated");

	check_int(table_part->size, size, "s_table_part: size");
}

/***************************************************************************
 *
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
	check_table_part(&table_part, 0, 2, 2, 1, "test 1");

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
	check_table_part(&table_part, 0, 2, 2, 0, "test 3");

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
	check_table_part(&table_part, 0, 2, 0, 1, "test 5");

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
	check_table_part(&table_part, 0, 2, 0, 0, "test 7");

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
 *
 **************************************************************************/

static void test_field_part() {

	s_table_part table_part;
	s_field_part field_part;

	print_debug_str("test_table_part() Start\n");

	//
	// truncated right
	//
	table_part.start = 0;
	table_part.end = 2;
	table_part.truncated = 2;
	table_part.size = 2;

	s_field_part_update(&table_part, 0, 4, &field_part);
	check_int(field_part.start, 0, "start");
	check_int(field_part.size, 4, "size");

	s_field_part_update(&table_part, 2, 4, &field_part);
	check_int(field_part.start, 0, "start");
	check_int(field_part.size, 2, "size");

	//
	// truncated left
	//
	table_part.start = 0;
	table_part.end = 2;
	table_part.truncated = 0;
	table_part.size = 2;

	s_field_part_update(&table_part, 0, 4, &field_part);
	check_int(field_part.start, 2, "start");
	check_int(field_part.size, 2, "size");

	s_field_part_update(&table_part, 2, 4, &field_part);
	check_int(field_part.start, 0, "start");
	check_int(field_part.size, 4, "size");

	print_debug_str("test_table_part() End\n");
}

/***************************************************************************
 *
 **************************************************************************/

static void test_field_truncated_line() {

	print_debug_str("field_truncated_line() Start\n");

	s_field_part col_field_part;
	wchar_t buffer[5 + 1];
	wchar_t *ptr;

	//
	// str lines (height 6 width 5)
	//
	wchar_t *str1 = L"\n1\n12\n123\n1234";
	ptr = str1;

	//
	// truncated right
	//
	col_field_part.start = 0;
	col_field_part.size = 2;

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"");

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"1");

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"12");

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"12");

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"12");

	check_null(ptr);

	//
	// truncated left
	//
	col_field_part.start = 2;
	col_field_part.size = 2;

	ptr = str1;

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"");

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"");

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"");

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"3");

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"34");

	check_null(ptr);

	//
	// test an empty field
	//
	wchar_t *str2 = L"";
	ptr = str2;

	col_field_part.start = 0;
	col_field_part.size = 2;

	ptr = field_truncated_line(ptr, buffer, &col_field_part);
	check_str(buffer, L"");

	check_null(ptr);

	//
	// calling the function with NULL returns NULL
	//
	ptr = field_truncated_line(NULL, buffer, &col_field_part);
	check_null(ptr);

	print_debug_str("field_truncated_line() Start\n");
}

/***************************************************************************
 * The main functon simply starts the test.
 **************************************************************************/

int main(const int argc, char * const argv[]) {

	print_debug_str("main() Start\n");

	setlocale(LC_ALL, "");

	test_parser();

	test_table_part_update();

	test_field_part();

	test_field_truncated_line();

	print_debug_str("main() End\n");

	return EXIT_SUCCESS;
}

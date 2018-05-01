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

static void ut_check_int(const int current, const int expected, const char *msg) {

	if (current != expected) {
		print_exit("ut_check_int() %s current: %d expected: %d\n", msg, current, expected);
	}

	print_debug("ut_check_int() OK - %s current: %d \n", msg, current);
}

/***************************************************************************
 * The function checks whether an size_t parameter has the expected value or
 * not.
 **************************************************************************/

static void ut_check_size(const size_t current, const size_t expected, const char *msg) {

	if (current != expected) {
		print_exit("ut_check_size() %s current: %zu expected: %zu\n", msg, current, expected);
	}

	print_debug("ut_check_size() OK - %s size: %zu \n", msg, current);
}

/***************************************************************************
 * The function compares two wchar strings.
 **************************************************************************/

static void ut_check_wchar_str(const wchar_t *str1, const wchar_t *str2) {

	if (wcscmp(str1, str2) != 0) {
		print_exit("ut_check_wchar_str() Strings differ: '%ls' and: '%ls'\n", str1, str2);
	}

	print_debug("ut_check_wchar_str() OK - String are equal: '%ls'\n", str1);
}

/***************************************************************************
 * The function ensures that a given wchar string is null.
 **************************************************************************/

static void ut_check_wchar_null(const wchar_t *str) {

	if (str != NULL) {
		print_exit("ut_check_wchar_null() Pointer is not null: '%ls'\n", str);
	}

	print_debug_str("ut_check_wchar_null() OK\n");
}

/***************************************************************************
 * The function checks whether a s_table_part struct has the expected
 * values.
 **************************************************************************/

static void check_table_part(const s_table_part *table_part, const int start, const int end, const int truncated, const int size, const char *msg) {

	print_debug("check_table_part() %s\n", msg);

	ut_check_int(table_part->start, start, "s_table_part: start");

	ut_check_int(table_part->end, end, "s_table_part: end");

	ut_check_int(table_part->truncated, truncated, "s_table_part: truncated");

	ut_check_int(table_part->size, size, "s_table_part: size");
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
	ut_check_wchar_str(table.fields[0][0], L"f00");
	ut_check_wchar_str(table.fields[0][1], L"f01");
	ut_check_wchar_str(table.fields[0][2], L"f02");

	ut_check_wchar_str(table.fields[1][0], L"f10");
	ut_check_wchar_str(table.fields[1][1], L"f11->\n\"d111\"");
	ut_check_wchar_str(table.fields[1][2], L"f12");

	ut_check_wchar_str(table.fields[2][0], L"f20");
	ut_check_wchar_str(table.fields[2][1], L"f21");
	ut_check_wchar_str(table.fields[2][2], L"f22");

	ut_check_wchar_str(table.fields[3][0], L"f30->,d30");
	ut_check_wchar_str(table.fields[3][1], L"f31");
	ut_check_wchar_str(table.fields[3][2], L"f32->\nd32");

	ut_check_wchar_str(table.fields[4][0], L"");
	ut_check_wchar_str(table.fields[4][1], L"end");
	ut_check_wchar_str(table.fields[4][2], L"");

	//
	// check the meta data
	//
	ut_check_int(table.width[0], 9, "col width: 0");
	ut_check_int(table.width[1], 6, "col width: 1");
	ut_check_int(table.width[2], 5, "col width: 2");

	ut_check_int(table.height[0], 1, "row_height: 0");
	ut_check_int(table.height[1], 2, "row_height: 1");
	ut_check_int(table.height[2], 1, "row_height: 2");
	ut_check_int(table.height[3], 2, "row_height: 3");
	ut_check_int(table.height[4], 1, "row_height: 4");

	s_table_free(&table);

	print_debug_str("test_parser() End\n");
}

/***************************************************************************
 * The function reads and parses a csv file that contains: ",\n,"
 **************************************************************************/

static void test_parser_empty() {
	s_table table;

	print_debug_str("test_parser_empty() Start\n");

	parser_process_file("res/empty.csv", W_DELIM, &table);

	//
	// check all fields "by hand"
	//
	ut_check_wchar_str(table.fields[0][0], L"");
	ut_check_wchar_str(table.fields[0][1], L"");

	ut_check_wchar_str(table.fields[1][0], L"");
	ut_check_wchar_str(table.fields[1][1], L"");

	//
	// check the meta data
	//
	ut_check_int(table.width[0], 1, "col empty: 0");
	ut_check_int(table.width[1], 1, "col empty: 1");

	ut_check_int(table.height[0], 1, "row empty: 0");
	ut_check_int(table.height[1], 1, "row empty: 1");

	s_table_free(&table);

	print_debug_str("test_parser_empty() End\n");
}

/***************************************************************************
 * The function checks the dimensions of a field. An empty field has width 0
 * and height 1. The minimum width of a column is 1 to ensure that the
 * cursor field can be displayed.
 **************************************************************************/

static void test_table_field_dimension() {
	int row_size;
	int col_size;

	print_debug_str("s_table_field_dimension_new() Start\n");

	//
	// Test: Empty field
	//
	s_table_field_dimension(L"", &col_size, &row_size);
	ut_check_size(col_size, 0, "col: Empty field");
	ut_check_size(row_size, 1, "row: Empty field");

	//
	// Test: Two empty lines
	//
	s_table_field_dimension(L"\n", &col_size, &row_size);
	ut_check_size(col_size, 0, "col: Two empty lines");
	ut_check_size(row_size, 2, "row: Two empty lines");

	//
	// Test: Simple string
	//
	s_table_field_dimension(L"привет", &col_size, &row_size);
	ut_check_size(col_size, wcslen(L"привет"), "col: Simple string");
	ut_check_size(row_size, 1, "row: Simple string");

	//
	// Test: Multi lines
	//
	s_table_field_dimension(L"привет\nпривет привет\nпривет", &col_size, &row_size);
	ut_check_size(col_size, wcslen(L"привет привет"), "col: Multi lines");
	ut_check_size(row_size, 3, "row: Multi lines");

	print_debug_str("s_table_field_dimension() End\n");
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
 * The function checks the s_field_part_update function, which computes the
 * visible part of a truncated field.
 **************************************************************************/

static void test_field_part_update() {

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
	ut_check_int(field_part.start, 0, "start");
	ut_check_int(field_part.size, 4, "size");

	s_field_part_update(&table_part, 2, 4, &field_part);
	ut_check_int(field_part.start, 0, "start");
	ut_check_int(field_part.size, 2, "size");

	//
	// truncated left
	//
	table_part.start = 0;
	table_part.end = 2;
	table_part.truncated = 0;
	table_part.size = 2;

	s_field_part_update(&table_part, 0, 4, &field_part);
	ut_check_int(field_part.start, 2, "start");
	ut_check_int(field_part.size, 2, "size");

	s_field_part_update(&table_part, 2, 4, &field_part);
	ut_check_int(field_part.start, 0, "start");
	ut_check_int(field_part.size, 4, "size");

	print_debug_str("test_table_part() End\n");
}

/***************************************************************************
 * The function checks the get_field_line function for a truncated field. It
 * is repeatedly called and returns a truncated and padded line of a fixed
 *  width.
 **************************************************************************/

#define FIELD_SIZE 2

static void test_get_field_line() {

	print_debug_str("test_get_field_line() Start\n");

	s_field_part col_field_part;
	wchar_t buffer[FIELD_SIZE + 1];
	wchar_t *ptr;

	//
	// The string with the field content (height 6 width 5)
	//
	wchar_t *str1 = L"\nз\nза\nзая\nзаяц";


	ptr = str1;

	//
	// Field is truncated right.
	//
	col_field_part.start = 0;
	col_field_part.size = FIELD_SIZE;
	buffer[col_field_part.size] = W_STR_TERM;

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"  ");

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"з ");

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"за");

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"за");

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"за");

	ut_check_wchar_null(ptr);

	//
	// Field is truncated left.
	//
	col_field_part.start = 2;
	col_field_part.size = FIELD_SIZE;

	ptr = str1;

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"  ");

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"  ");

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"  ");

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"я ");

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"яц");

	ut_check_wchar_null(ptr);

	//
	// Test an empty field
	//
	wchar_t *str2 = L"";
	ptr = str2;

	col_field_part.start = 0;
	col_field_part.size = FIELD_SIZE;

	ptr = get_field_line(ptr, buffer, &col_field_part);
	ut_check_wchar_str(buffer, L"  ");

	ut_check_wchar_null(ptr);

	print_debug_str("test_get_field_line() End\n");
}

/***************************************************************************
 * The main function simply starts the test.
 **************************************************************************/

int main() {

	print_debug_str("main() Start\n");

	setlocale(LC_ALL, "");

	test_parser();

	test_parser_empty();

	test_table_field_dimension();

	test_table_part_update();

	test_field_part_update();

	test_get_field_line();

	print_debug_str("main() End\n");

	return EXIT_SUCCESS;
}

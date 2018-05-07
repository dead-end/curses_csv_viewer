/*
 * file: ut_curses.c
 */

#include <locale.h>

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_parser.h"
#include "ncv_curses.h"

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
	table_part.first = 0;
	table_part.last = 2;
	table_part.truncated = 2;
	table_part.size = 2;

	s_field_part_update(&table_part, 0, 4, &field_part);
	ut_check_int(field_part.start, 0, "test 1 - start");
	ut_check_int(field_part.size, 4, "test 2 - size");

	s_field_part_update(&table_part, 2, 4, &field_part);
	ut_check_int(field_part.start, 0, "test 3 - start");
	ut_check_int(field_part.size, 2, "test 4 - size");

	//
	// truncated left
	//
	table_part.first = 0;
	table_part.last = 2;
	table_part.truncated = 0;
	table_part.size = 2;

	s_field_part_update(&table_part, 0, 4, &field_part);
	ut_check_int(field_part.start, 2, "test 5 - start");
	ut_check_int(field_part.size, 2, "test 6 - size");

	s_field_part_update(&table_part, 2, 4, &field_part);
	ut_check_int(field_part.start, 0, "test 7 - start");
	ut_check_int(field_part.size, 4, "test 8 - size");

	print_debug_str("test_table_part() End\n");
}

/***************************************************************************
 * The function checks the get_field_line function for a truncated field. It
 * is repeatedly called and returns a truncated and padded line of a fixed
 * width.
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

	print_debug_str("ut_curses.c - Start tests\n");

	setlocale(LC_ALL, "");

	test_table_part_update();

	test_field_part_update();

	test_get_field_line();

	print_debug_str("ut_curses.c - End tests\n");

	return EXIT_SUCCESS;
}

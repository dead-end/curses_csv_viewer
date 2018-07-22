/*
 * file: ut_parser.c
 */

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_parser.h"

/***************************************************************************
 * The function reads and parses a csv file. All fields are compared with
 * the expected values.
 **************************************************************************/

static void test_parser() {
	s_table table;

	print_debug_str("test_parser() Start\n");

	parser_process_filename("res/test1.csv", W_DELIM, &table);

	//
	// Check all fields "by hand"
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
	// Check the meta data
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

	parser_process_filename("res/empty.csv", W_DELIM, &table);

	//
	// Check all fields "by hand"
	//
	ut_check_wchar_str(table.fields[0][0], L"");
	ut_check_wchar_str(table.fields[0][1], L"");

	ut_check_wchar_str(table.fields[1][0], L"");
	ut_check_wchar_str(table.fields[1][1], L"");

	//
	// Check the meta data
	//
	ut_check_int(table.width[0], 1, "col empty: 0");
	ut_check_int(table.width[1], 1, "col empty: 1");

	ut_check_int(table.height[0], 1, "row empty: 0");
	ut_check_int(table.height[1], 1, "row empty: 1");

	s_table_free(&table);

	print_debug_str("test_parser_empty() End\n");
}

/***************************************************************************
 * The main function simply starts the test.
 **************************************************************************/

int main() {

	print_debug_str("ut_parser.c - Start tests\n");

	setlocale(LC_ALL, "");

	test_parser();

	test_parser_empty();

	print_debug_str("ut_parser.c - End tests\n");

	return EXIT_SUCCESS;
}

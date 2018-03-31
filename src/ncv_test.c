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
 * The function ensures that the view state has the expected values. The
 * view state shows which part of the csv file is visible.
 **************************************************************************/

static void checkViewState(const s_view_state *view_state, const int col_start, const int col_end, const int col_truncated, const int truncated_len) {

	check_int(view_state->col_start, col_start, "col_start");

	check_int(view_state->col_end, col_end, "col_end");

	check_int(view_state->col_truncated, col_truncated, "col_truncated");

	check_int(view_state->truncated_len, truncated_len, "truncated_len");
}

/***************************************************************************
 * The function reads test csv files and checks view states.
 **************************************************************************/

#define WIN_X_10 10

static void test2() {
	s_table table;
	s_view_state view_state;

	print_debug_str("test2() Start\n");

	//
	// DIR_FORWARD
	//
	parser_process_file("res/test2_case1.csv", W_DELIM, &table);
	s_view_state_update(&view_state, &table, 0, DIR_FORWARD, WIN_X_10);
	checkViewState(&view_state, 0, 2, 2, 1);
	s_table_free(&table);

	parser_process_file("res/test2_case2.csv", W_DELIM, &table);
	s_view_state_update(&view_state, &table, 0, DIR_FORWARD, WIN_X_10);
	checkViewState(&view_state, 0, 1, 1, 3);
	s_table_free(&table);

	parser_process_file("res/test2_case3.csv", W_DELIM, &table);
	s_view_state_update(&view_state, &table, 0, DIR_FORWARD, WIN_X_10);
	checkViewState(&view_state, 0, 2, 2, 0);
	s_table_free(&table);

	//
	// DIR_BACKWARD
	//
	parser_process_file("res/test2_case1.csv", W_DELIM, &table);
	s_view_state_update(&view_state, &table, 2, DIR_BACKWARD, WIN_X_10);
	checkViewState(&view_state, 0, 2, 0, 2);
	s_table_free(&table);

	parser_process_file("res/test2_case2.csv", W_DELIM, &table);
	s_view_state_update(&view_state, &table, 1, DIR_BACKWARD, WIN_X_10);
	checkViewState(&view_state, 0, 1, 0, 4);
	s_table_free(&table);

	parser_process_file("res/test2_case3.csv", W_DELIM, &table);
	s_view_state_update(&view_state, &table, 2, DIR_BACKWARD, WIN_X_10);
	checkViewState(&view_state, 0, 2, 0, 1);
	s_table_free(&table);

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

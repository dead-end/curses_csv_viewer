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

#include "ncv_table_part.h"
#include "ncv_parser.h"
#include "ut_utils.h"

#include <locale.h>
#include <stdarg.h>

#define STRICT_COL_TRUE true

#define DO_TRIM_FALSE false

/******************************************************************************
 * The macro checks the dimension of a field, that is the width and hight.
 *****************************************************************************/

#define ut_check_dim(_col_cur,_col_exp,_row_cur,_row_exp,_msg) { \
		ut_check_size(_col_cur, _col_exp, "col - " _msg); \
		ut_check_size(_row_cur, _row_exp, "row - " _msg); \
}

/******************************************************************************
 * The test checks the s_table_field_dimension function, which computes the
 * width and height of a field. An empty field has width 0 and height 1. The
 * minimum width of a column is 1 to ensure that the cursor field can be
 * displayed.
 *****************************************************************************/

static void test_table_field_dimension() {
	int row_size;
	int col_size;

	print_debug_str("test_table_field_dimension() Start\n");

	//
	// Test: Empty field
	//
	s_table_field_dimension(L"", &col_size, &row_size);
	ut_check_dim(col_size, 0, row_size, 1, "empty field");

	//
	// Test: Two empty lines
	//
	s_table_field_dimension(L"\n", &col_size, &row_size);
	ut_check_dim(col_size, 0, row_size, 2, "two empty lines");

	//
	// Test: Simple string
	//
	s_table_field_dimension(L"привет", &col_size, &row_size);
	ut_check_dim(col_size, wcslen(L"привет"), row_size, 1, "simple string");

	//
	// Test: Multi lines
	//
	s_table_field_dimension(L"привет\nпривет привет\nпривет", &col_size, &row_size);
	ut_check_dim(col_size, wcslen(L"привет привет"), row_size, 3, "multi lines");

	print_debug_str("test_table_field_dimension() End\n");
}

/******************************************************************************
 * The function tests the correct cursor position.
 *****************************************************************************/

static void check_cursor(const s_cursor *cursor, const int row, const int col, const char *msg) {

	ut_check_int(cursor->row, row, msg);
	ut_check_int(cursor->col, col, msg);
}

/******************************************************************************
 * The function checks the results of filtering the table, which is the number
 * of matches, the number of filtered rows and the active flag. If no match
 * was found, the filtering will be deactivated.
 *****************************************************************************/

static void check_filter_result(const s_table *table, const bool is_active, const int count, const int rows, const char *msg) {

	ut_check_bool(table->filter.is_active, is_active);
	ut_check_int(table->filter.count, count, msg);
	ut_check_int(table->no_rows, rows, msg);
}

/******************************************************************************
 * The struct is used to provide an array of fields matching a search / filter.
 * With this array the s_table_prev_next() function is checked. The function
 * set the row and col of the field with the next / previous match.
 *****************************************************************************/

typedef struct s_row_col {

	//
	// Row of the field with the next / previous match.
	//
	int row;

	//
	// Column of the field with the next / previous match.
	//
	int col;

} s_row_col;

/******************************************************************************
 * The function tests the prev / next function. We start at the current match
 * and go forward / backward until we are again at the start position.
 *
 * A calling example:
 *
 * check_prev_next(t, c, m, num_matches,
 *     (s_row_col[] ) { { row-1, col-1 }, ..., { row-n, col-n } });
 *****************************************************************************/

static void check_prev_next(const s_table *table, s_cursor *cursor, const char *msg, const int num_matches, const s_row_col row_col[num_matches]) {

	//
	// Check the current match position
	//
	print_debug("check_prev_next() start match idx: %d - %s\n", 0, msg);
	ut_check_int(cursor->row, row_col[0].row, "row - start match");
	ut_check_int(cursor->col, row_col[0].col, "col - start match");

	//
	// Check forward
	//
	for (int i = 0; i < num_matches; i++) {

		//
		// The index of the next match
		//
		int idx = (i + 1) % num_matches;

		s_table_prev_next(table, cursor, E_DIR_FORWARD);

		print_debug("check_prev_next() forward match idx: %d - %s\n", idx, msg);
		ut_check_int(cursor->row, row_col[idx].row, "row - forward match");
		ut_check_int(cursor->col, row_col[idx].col, "col - forward match");
	}

	//
	// Check backward (from the end to the start)
	//
	for (int idx = num_matches - 1; idx >= 0; idx--) {

		s_table_prev_next(table, cursor, E_DIR_BACKWARD);

		print_debug("check_prev_next() backward match idx: %d - %s\n", idx, msg);
		ut_check_int(cursor->row, row_col[idx].row, "row - backward match");
		ut_check_int(cursor->col, row_col[idx].col, "col - backward match");
	}
}

/******************************************************************************
 * The marco is a wrapper around the s_table_update_filter_sort() function call
 * that ensures that the result returned is or is not null.
 *****************************************************************************/

#define check_table_update_filter_sort(t,c,fc,sc,n) ut_check_wcs_null(s_table_update_filter_sort(t, c, fc, sc), n)

/******************************************************************************
 * The function tests the searching and filtering of the table as well as the
 * moving of the cursor to the next / prev match.
 *
 * The prev / next tests search forward / backward until they reach the initial
 * position.
 *****************************************************************************/

#define MATCH_1 1

#define MATCH_2 2

#define MATCH_3 3

static void test_table_search_filter() {
	s_cursor cursor, cursor_save;

	s_table table;
	s_table_set_defaults(table);

	print_debug_str("test_table_search_filter() Start\n");

	const wchar_t *data =

	L"Number" DL "Header-2" DL "Header-3" NL
	"1" DL "abcd" DL "ABCD" NL
	"2" DL "efgh" DL "XfgX" NL
	"3" DL "ixxl" DL "ijXX" NL
	"4" DL "mnop" DL "XXOP" NL;

	FILE *tmp = ut_create_tmp_file(data);

	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, DO_TRIM_FALSE, STRICT_COL_TRUE);

	parser_process_file(tmp, &cfg_parser, &table);

	//
	// SEARCHING, INSENSITIVE WITH 2 MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"bc", SF_IS_INSENSITIVE, SF_IS_SEARCHING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, MATCH_2, 5, "search insensitive");

	//
	// matches: 2 points: (1,1) (1,2)
	//
	check_prev_next(&table, &cursor, "search insensitive", MATCH_2, (const s_row_col[] ) { { 1, 1 }, { 1, 2 } });

	//
	// SEARCHING, SENSITIVE WITH 1 MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"bc", SF_IS_SENSITIVE, SF_IS_SEARCHING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, MATCH_1, 5, "search sensitive");

	//
	// matches: 1 points: (1,1)
	//
	check_prev_next(&table, &cursor, "search insensitive", MATCH_1, (const s_row_col[] ) { { 1, 1 } });

	//
	// SEARCHING WITH NO MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"hallo", SF_IS_SENSITIVE, SF_IS_SEARCHING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NOT_NULL);
	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "search no matches - filter");
	check_cursor(&cursor, 1, 1, "search no matches - cursor");

	//
	// FILTERING, INSENSITIVE WITH 3 MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_INSENSITIVE, SF_IS_FILTERING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, MATCH_3, 3, "filter insensitive - filter");

	//
	// matches: 3 points: (1,1) (1,2) (2,2)
	//
	check_prev_next(&table, &cursor, "filter insensitive", MATCH_3, (const s_row_col[] ) { { 1, 1 }, { 1, 2 }, { 2, 2 } });

	//
	// FILTERING, SENSITIVE WITH 1 MATCH
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_SENSITIVE, SF_IS_FILTERING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, MATCH_1, 2, "filter sensitive - filter");

	//
	// matches: 1 points: (1,1)
	//
	check_prev_next(&table, &cursor, "filter sensitive", MATCH_1, (const s_row_col[] ) { { 1, 1 } });

	//
	// FILTERING WITH NO MATCHES (cursor is unchanged)
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"eF", SF_IS_SENSITIVE, SF_IS_FILTERING);
	s_cursor_pos(&cursor_save, cursor.row, cursor.col);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NOT_NULL);
	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "filter no matches - filter");
	check_cursor(&cursor, cursor_save.row, cursor_save.col, "filter no matches - cursor");

	//
	// RESET AFTER FILTERING, SENSITIVE WITH 1 MATCH
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"XO", SF_IS_SENSITIVE, SF_IS_FILTERING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, 1, 2, "reset after filtering - filter");
	check_cursor(&cursor, 1, 2, "reset after filtering - cursor");

	//
	// Do the reset (cursor is unchanged)
	//
	s_filter_set_inactive(&table.filter);
	s_cursor_pos(&cursor_save, cursor.row, cursor.col);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "no filtering - filter");
	check_cursor(&cursor, cursor_save.row, cursor_save.col, "no filtering - cursor");

	//
	// Calling prev / next has no effect if filtering is inactive. This should
	// be validated in the following tests.
	//
	s_cursor_pos(&cursor, 1, 2);
	s_table_prev_next(&table, &cursor, E_DIR_FORWARD);
	check_cursor(&cursor, 1, 2, "no filtering - forward");

	s_cursor_pos(&cursor, 2, 1);
	s_table_prev_next(&table, &cursor, E_DIR_BACKWARD);
	check_cursor(&cursor, 2, 1, "no filtering - backward");

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_table_search_filter() End\n");
}

// TODO: split test case => separate test for prev / next.

/******************************************************************************
 * The function checks the combination of sorting and filtering.
 *****************************************************************************/

static void test_filter_and_sort() {
	s_table table;
	s_cursor cursor;
	s_table_set_defaults(table);

	print_debug_str("test_filter_and_sort() Start\n");

	const wchar_t *data =

	L"0" DL "DD" DL "-z-" NL
	L"1" DL "CC" DL "---" NL
	L"2" DL "BB" DL "--z" NL
	L"3" DL "AA" DL "" NL
	L"4" DL "EE" DL "z--" NL;

	FILE *tmp = ut_create_tmp_file(data);

	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, DO_TRIM_FALSE, STRICT_COL_TRUE);

	parser_process_file(tmp, &cfg_parser, &table);

	table.show_header = false;

	//
	// Set a filter for "z"
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"z", SF_IS_SENSITIVE, SF_IS_FILTERING);
	s_table_update_filter_sort(&table, &cursor, true, false);

	check_table_column(&table, 1, 3, (const wchar_t*[] ) { L"DD", L"BB", L"EE" });

	//
	// Sort the already filtered data
	//
	s_sort_update(&table.sort, 1, E_DIR_FORWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	check_table_column(&table, 1, 3, (const wchar_t*[] ) { L"BB", L"DD", L"EE" });

	//
	// Change the sorted column and direction
	//
	s_sort_update(&table.sort, 0, E_DIR_BACKWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	check_table_column(&table, 1, 3, (const wchar_t*[] ) { L"EE", L"BB", L"DD" });

	//
	// Change the sorted column and direction
	//
	s_filter_set_inactive(&table.filter);
	s_table_update_filter_sort(&table, &cursor, true, false);

	check_table_column(&table, 1, 5, (const wchar_t*[] ) { L"EE", L"AA", L"BB", L"CC", L"DD" });

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_filter_and_sort() End\n");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	print_debug_str("ut_table.c - Start tests\n");

	setlocale(LC_ALL, "");

	test_table_field_dimension();

	test_table_search_filter();

	test_filter_and_sort();

	print_debug_str("ut_table.c - End tests\n");

	return EXIT_SUCCESS;
}

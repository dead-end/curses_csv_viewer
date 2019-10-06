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

	log_debug_str("Start");

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

	log_debug_str("End");
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
 * and goes forward / backward until we are again at the start position.
 *
 * A calling example:
 *
 * check_prev_next(t, c, m, num_matches,
 *     (s_row_col[] ) { { row-1, col-1 }, ..., { row-n, col-n } });
 *****************************************************************************/

static void check_prev_next(const s_table *table, s_cursor *cursor, const char *msg, const int num_matches, const s_row_col row_col[num_matches]) {

	//
	// Ensure that there is at least one match. (Without this test msg is
	// unused without DEBUG set.
	//
	if (num_matches < 1) {
		log_exit("Number of matches: %d - %s!", num_matches, msg);
	}

	//
	// Check the current match position
	//
	log_debug("Start match idx: %d - %s", 0, msg);
	ut_check_int(cursor->row, row_col[0].row, "row - start match");
	ut_check_int(cursor->col, row_col[0].col, "col - start match");

	//
	// Check forward
	//
	for (int i = 0; i < num_matches; i++) {

		//
		// The index of the next match
		//
		const int idx = (i + 1) % num_matches;

		s_table_prev_next(table, cursor, E_DIR_FORWARD);

		log_debug("Forward match idx: %d - %s", idx, msg);
		ut_check_int(cursor->row, row_col[idx].row, "row - forward match");
		ut_check_int(cursor->col, row_col[idx].col, "col - forward match");
	}

	//
	// Check backward (from the end to the start)
	//
	for (int idx = num_matches - 1; idx >= 0; idx--) {

		s_table_prev_next(table, cursor, E_DIR_BACKWARD);

		log_debug("Backward match idx: %d - %s", idx, msg);
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
 * The function tests the searching and filtering of the table.
 *****************************************************************************/

static void test_table_search_filter() {
	s_cursor cursor, cursor_save;

	s_table table;
	s_table_set_defaults(table);

	log_debug_str("Start");

	const wchar_t *data =

	L"Number" DL "Header-2" DL "Header-3" NL
	"1" DL "azza" DL "AZZA" NL
	"2" DL "bbbb" DL "BBBB" NL
	"3" DL "cxxc" DL "CCXX" NL
	"4" DL "dddd" DL "XXDD" NL;

	const s_cfg_parser cfg_parser = { .filename = NULL, .delim = W_DELIM, .do_trim = false, .strict = true };

	FILE *tmp = ut_create_tmp_file(data);
	parser_process_file(tmp, &cfg_parser, &table);

	//
	// SEARCHING, INSENSITIVE WITH 2 MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"zz", SF_IS_INSENSITIVE, SF_IS_SEARCHING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, 2, 5, "search insensitive - result");

	//
	// SEARCHING, SENSITIVE WITH 1 MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"zz", SF_IS_SENSITIVE, SF_IS_SEARCHING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, 1, 5, "search sensitive - result");

	//
	// SEARCHING WITH NO MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"hallo", SF_IS_SENSITIVE, SF_IS_SEARCHING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NOT_NULL);
	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "search sensitive - no matches - result");
	check_cursor(&cursor, 1, 1, "search sensitive - no matches - cursor");

	//
	// FILTERING, INSENSITIVE WITH 3 MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_INSENSITIVE, SF_IS_FILTERING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, 3, 3, "filter insensitive - result");

	//
	// FILTERING, SENSITIVE WITH 1 MATCH
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_SENSITIVE, SF_IS_FILTERING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, 1, 2, "filter sensitive - result");

	//
	// FILTERING WITH NO MATCHES (cursor is unchanged)
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"hallo", SF_IS_SENSITIVE, SF_IS_FILTERING);
	s_cursor_pos(&cursor_save, cursor.row, cursor.col);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NOT_NULL);
	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "filter sensitive - no matches - result");
	check_cursor(&cursor, cursor_save.row, cursor_save.col, "filter sensitive - no matches - cursor");

	//
	// RESET AFTER FILTERING, SENSITIVE WITH 1 MATCH
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"DD", SF_IS_SENSITIVE, SF_IS_FILTERING);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, 1, 2, "reset after filtering - result");
	check_cursor(&cursor, 1, 2, "reset after filtering - cursor");

	//
	// Do the reset (cursor is unchanged)
	//
	s_filter_set_inactive(&table.filter);
	s_cursor_pos(&cursor_save, cursor.row, cursor.col);
	check_table_update_filter_sort(&table, &cursor, true, false, UT_IS_NULL);
	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "no filtering - result");
	check_cursor(&cursor, cursor_save.row, cursor_save.col, "no filtering - cursor");

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	log_debug_str("End");
}

/******************************************************************************
 * The function tests prev / next function on searching and filtering of the
 * table. The prev / next tests search forward / backward until they reach the
 * initial position.
 *****************************************************************************/

static void test_table_prev_next() {
	s_cursor cursor;

	s_table table;
	s_table_set_defaults(table);

	log_debug_str("Start");

	const wchar_t *data =

	L"Number" DL "Header-2" DL "Header-3" NL
	"1" DL "aaaa" DL "AAAA" NL
	"2" DL "bxxb" DL "BBXX" NL
	"3" DL "cccc" DL "CCCC" NL
	"4" DL "dddd" DL "XXDD" NL;

	const s_cfg_parser cfg_parser = { .filename = NULL, .delim = W_DELIM, .do_trim = false, .strict = true };

	FILE *tmp = ut_create_tmp_file(data);
	parser_process_file(tmp, &cfg_parser, &table);

	//
	// SEARCHING, INSENSITIVE
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_INSENSITIVE, SF_IS_SEARCHING);
	s_table_update_filter_sort(&table, &cursor, true, false);
	check_prev_next(&table, &cursor, "search insensitive", 3, (const s_row_col[] ) { { 2, 1 }, { 2, 2 }, { 4, 2 } });

	//
	// SEARCHING, SENSITIVE
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_SENSITIVE, SF_IS_SEARCHING);
	s_table_update_filter_sort(&table, &cursor, true, false);
	check_prev_next(&table, &cursor, "search insensitive", 1, (const s_row_col[] ) { { 2, 1 } });

	//
	// FILTERING, INSENSITIVE
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_INSENSITIVE, SF_IS_FILTERING);
	s_table_update_filter_sort(&table, &cursor, true, false);
	check_prev_next(&table, &cursor, "filter insensitive", 3, (const s_row_col[] ) { { 1, 1 }, { 1, 2 }, { 2, 2 } });

	//
	// FILTERING, SENSITIVE
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_SENSITIVE, SF_IS_FILTERING);
	s_table_update_filter_sort(&table, &cursor, true, false);
	check_prev_next(&table, &cursor, "filter insensitive", 1, (const s_row_col[] ) { { 1, 1 } });

	//
	// FILTERING INACTIVE
	//
	s_filter_set_inactive(&table.filter);
	s_table_update_filter_sort(&table, &cursor, true, false);

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

	log_debug_str("End");
}

/******************************************************************************
 * The function checks the combination of sorting and filtering.
 *****************************************************************************/

#define UNCHANGED false

static void test_filter_and_sort() {
	s_table table;
	s_cursor cursor;
	s_table_set_defaults(table);

	log_debug_str("Start");

	const wchar_t *data =

	L"0" DL "DD" DL "-z-" NL
	L"1" DL "CC" DL "---" NL
	L"2" DL "BB" DL "--z" NL
	L"3" DL "AA" DL "" NL
	L"4" DL "EE" DL "z--" NL;

	const s_cfg_parser cfg_parser = { .filename = NULL, .delim = W_DELIM, .do_trim = false, .strict = true };

	FILE *tmp = ut_create_tmp_file(data);
	parser_process_file(tmp, &cfg_parser, &table);

	table.show_header = false;

	//
	// Set a filter for "z"
	//
	s_table_update_filter_sort(&table, &cursor, s_filter_set(&table.filter, SF_IS_ACTIVE, L"z", SF_IS_SENSITIVE, SF_IS_FILTERING), UNCHANGED);
	ut_check_table_column(&table, 1, 3, (const wchar_t*[] ) { L"DD", L"BB", L"EE" });

	//
	// Sort the already filtered data
	//
	s_table_update_filter_sort(&table, &cursor, UNCHANGED, s_sort_update(&table.sort, 1, E_DIR_FORWARD));
	ut_check_table_column(&table, 1, 3, (const wchar_t*[] ) { L"BB", L"DD", L"EE" });

	//
	// Change the sorted column and direction
	//
	s_table_update_filter_sort(&table, &cursor, UNCHANGED, s_sort_update(&table.sort, 0, E_DIR_BACKWARD));
	ut_check_table_column(&table, 1, 3, (const wchar_t*[] ) { L"EE", L"BB", L"DD" });

	//
	// Change the sorted column and direction
	//
	s_table_update_filter_sort(&table, &cursor, s_filter_set_inactive(&table.filter), UNCHANGED);
	ut_check_table_column(&table, 1, 5, (const wchar_t*[] ) { L"EE", L"AA", L"BB", L"CC", L"DD" });

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	log_debug_str("End");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	log_debug_str("Start");

	setlocale(LC_ALL, "C");

	test_table_field_dimension();

	test_table_search_filter();

	test_table_prev_next();

	test_filter_and_sort();

	log_debug_str("End");

	return EXIT_SUCCESS;
}

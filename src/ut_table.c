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
#include <math.h>
#include <stdarg.h>

#define STRICT_COL_TRUE true

#define DO_TRIM_FALSE false

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

	print_debug_str("test_table_field_dimension() End\n");
}

/******************************************************************************
 * The function tests the get_ratio() function. It computes the ratio between
 * the digits and the string length of a given string.
 *****************************************************************************/

static void test_table_get_ratio() {
	double result;

	print_debug_str("test_table_get_ratio() Start\n");

	result = get_ratio(L"123456");
	ut_check_double(result, 1.0, "get_ratio - 123456");

	result = get_ratio(L"123aaa");
	ut_check_double(result, 0.5, "get_ratio - 123aaa");

	result = get_ratio(L"11aabb");
	ut_check_double(result, 1.0 / 3.0, "get_ratio - 123aaa");

	result = get_ratio(L"aaabbb");
	ut_check_double(result, 0.0, "get_ratio - aaabbb");

	result = get_ratio(L"");
	ut_check_double(result, 0.0, "get_ratio - ''");

	print_debug_str("test_table_get_ratio() End\n");
}

/******************************************************************************
 * The function tests the characteristics (string length and ratio of digits in
 * the string) for each of the columns of a csv file. Each function call
 * returns 1 if the characteristic indicates a header for that column.
 *****************************************************************************/

#define HAS_HEADER 1

#define HAS_NO_HEADER 0

static void test_table_has_header() {
	int result;

	s_table table;
	s_table_set_defaults(table);

	print_debug_str("test_table_has_header() Start\n");

	const wchar_t *data =

	L"Number" DL "Date" DL "Price" DL "Mix" DL "Mix111" NL
	"1" DL "01.01.218" DL "1 Euro" DL "aa" DL "aa11" NL
	"2" DL "01.01.218" DL "1.20 Euro" DL "aabb" DL "aabb1122" NL
	"4" DL "01.01.218" DL "10 Euro" DL "cc" DL "cc11" NL
	"8" DL "01.01.218" DL "10.20 Euro" DL "ccdd" DL "ccdd1122" NL
	"16" DL "01.01.218" DL "100 Euro" DL "ee" DL "ee11" NL
	"32" DL "01.01.218" DL "100.20 Euro" DL "eeff" DL "eeff1122" NL;

	FILE *tmp = ut_create_tmp_file(data);

	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, DO_TRIM_FALSE, STRICT_COL_TRUE);

	parser_process_file(tmp, &cfg_parser, &table);
	//
	// column: 0
	// len   => has header
	// ratio => has header
	//
	result = check_column_characteristic(&table, table.no_rows, 0, get_str_len);
	ut_check_int(result, HAS_HEADER, "has_header - len: 0");

	result = check_column_characteristic(&table, table.no_rows, 0, get_ratio);
	ut_check_int(result, HAS_HEADER, "has_header - ratio: 0");

	//
	// column: 1
	// len   => has header
	// ratio => has header
	//
	result = check_column_characteristic(&table, table.no_rows, 1, get_str_len);
	ut_check_int(result, HAS_HEADER, "has_header - len: 1");

	result = check_column_characteristic(&table, table.no_rows, 1, get_ratio);
	ut_check_int(result, HAS_HEADER, "has_header - ratio: 1");

	//
	// column: 2
	// len   => has header
	// ratio => has header
	//
	result = check_column_characteristic(&table, table.no_rows, 2, get_str_len);
	ut_check_int(result, HAS_NO_HEADER, "has_header - len: 2");

	result = check_column_characteristic(&table, table.no_rows, 2, get_ratio);
	ut_check_int(result, HAS_HEADER, "has_header - ratio: 2");

	//
	// column: 3 only chars and header lenths is mean
	// len   => has no header
	// ratio => has no header
	//
	result = check_column_characteristic(&table, table.no_rows, 3, get_str_len);
	ut_check_int(result, HAS_NO_HEADER, "has_header - len: 3");

	result = check_column_characteristic(&table, table.no_rows, 3, get_ratio);
	ut_check_int(result, HAS_NO_HEADER, "has_header - ratio: 3");

	//
	// column: 4 mixed chars and int but header is mean
	// len   => has no header
	// ratio => has no header
	//
	result = check_column_characteristic(&table, table.no_rows, 4, get_str_len);
	ut_check_int(result, HAS_NO_HEADER, "has_header - len: 4");

	result = check_column_characteristic(&table, table.no_rows, 4, get_ratio);
	ut_check_int(result, HAS_NO_HEADER, "has_header - ratio: 4");

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_table_has_header() End\n");
}

/******************************************************************************
 * The function tests the computation of the mean and the standard deviation
 * for some of the columns.
 *****************************************************************************/

static void test_table_mean_std_dev() {
	double result;

	double d_tmp;

	s_table table;
	s_table_set_defaults(table);

	print_debug_str("test_table_mean_std_dev() Start\n");

	const wchar_t *data =

	L"Number" DL "Date" DL "Price" DL "Mix" DL "Mix111" NL
	"1" DL "01.01.218" DL "1 Euro" DL "aa" DL "aa11" NL
	"2" DL "01.01.218" DL "1.20 Euro" DL "aabb" DL "aabb1122" NL
	"4" DL "01.01.218" DL "10 Euro" DL "cc" DL "cc11" NL
	"8" DL "01.01.218" DL "10.20 Euro" DL "ccdd" DL "ccdd1122" NL
	"16" DL "01.01.218" DL "100 Euro" DL "ee" DL "ee11" NL
	"32" DL "01.01.218" DL "100.20 Euro" DL "eeff" DL "eeff1122" NL;

	FILE *tmp = ut_create_tmp_file(data);

	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, DO_TRIM_FALSE, STRICT_COL_TRUE);

	parser_process_file(tmp, &cfg_parser, &table);

	//
	// column: 0 mean
	//
	d_tmp = (4.0 + 2 * 2) / 6;

	result = get_table_mean(&table, table.no_rows, 0, get_str_len);
	ut_check_double(result, d_tmp, "mean len: 0");

	result = get_table_mean(&table, table.no_rows, 0, get_ratio);
	ut_check_double(result, 1.0, "mean ratio: 0");

	//
	// column: 0 std deveation
	//
	result = get_table_std_dev(&table, table.no_rows, 0, get_str_len, d_tmp);
	ut_check_double(result, sqrt((4 * (1 - d_tmp) * (1 - d_tmp) + 2 * (2 - d_tmp) * (2 - d_tmp)) / 6), "std dev str: 0");

	result = get_table_std_dev(&table, table.no_rows, 0, get_ratio, 1.0);
	ut_check_double(result, 0.0, "std dev ratio: 0");

	print_debug_str("test_table_mean_std_dev() End\n");

	//
	// column: 3 mean
	//
	result = get_table_mean(&table, table.no_rows, 3, get_str_len);
	ut_check_double(result, 3.0, "mean len: 3");

	result = get_table_mean(&table, table.no_rows, 3, get_ratio);
	ut_check_double(result, 0.0, "mean ratio: 3");

	//
	// column: 3 std deveation
	//
	result = get_table_std_dev(&table, table.no_rows, 3, get_str_len, 3.0);
	ut_check_double(result, 1.0, "std dev str: 3");

	result = get_table_std_dev(&table, table.no_rows, 3, get_ratio, 0.0);
	ut_check_double(result, 0.0, "std dev ratio: 3");

	//
	// column: 4 mean
	//
	result = get_table_mean(&table, table.no_rows, 4, get_str_len);
	ut_check_double(result, 6.0, "mean len: 4");

	result = get_table_mean(&table, table.no_rows, 4, get_ratio);
	ut_check_double(result, 0.5, "mean ratio: 4");

	//
	// column: 4 std deveation
	//
	result = get_table_std_dev(&table, table.no_rows, 4, get_str_len, 6.0);
	ut_check_double(result, 2.0, "std dev str: 4");

	result = get_table_std_dev(&table, table.no_rows, 4, get_ratio, 0.5);
	ut_check_double(result, 0.0, "std dev ratio: 4");

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_table_mean_std_dev() End\n");
}

/******************************************************************************
 * The function tests the correct cursor position.
 *****************************************************************************/

static void check_cursor(const s_cursor *cursor, const int row, const int col, const char *msg) {

	ut_check_int(cursor->row, row, msg);
	ut_check_int(cursor->col, col, msg);
}

/******************************************************************************
 * The function checks the results of filtering the table.
 *****************************************************************************/

static void check_filter_result(const s_table *table, const bool is_active, const int count, const int rows, const char *msg) {

	ut_check_bool(table->filter.is_active, is_active);
	ut_check_int(table->filter.count, count, msg);
	ut_check_int(table->no_rows, rows, msg);
}

/******************************************************************************
 * The function tests the prev / next function. We start at the current match
 * and go forward / backward until we are again at the start position.
 *
 * A calling example:
 *
 * check_prev_next(t, c, m, num_matches, row-1, col-1, ..., row-n, col-n);
 *****************************************************************************/

static void check_prev_next(const s_table *table, s_cursor *cursor, const char *msg, const int num_matches, ...) {
	char buf[MAX_LINE];

	int row[num_matches], col[num_matches];

	//
	// Copy the points to the row and column arrays.
	//
	va_list valist;

	va_start(valist, num_matches);

	for (int i = 0; i < num_matches; i++) {
		row[i] = va_arg(valist, int);
		col[i] = va_arg(valist, int);
	}

	va_end(valist);

	//
	// Check the current match position
	//
	snprintf(buf, MAX_LINE, "%s start (%d, %d)\n", msg, row[0], col[0]);
	check_cursor(cursor, row[0], col[0], buf);

	//
	// Check forward
	//
	for (int i = 0; i < num_matches; i++) {

		//
		// The index of the next match
		//
		int idx = (i + 1) % num_matches;

		s_table_prev_next(table, cursor, DIR_FORWARD);

		snprintf(buf, MAX_LINE, "%s forward match: %d (%d, %d)\n", msg, idx, row[idx], col[idx]);

		check_cursor(cursor, row[idx], col[idx], buf);
	}

	//
	// Check backward (from the end to the start)
	//
	for (int idx = num_matches - 1; idx >= 0; idx--) {

		s_table_prev_next(table, cursor, DIR_BACKWARD);

		snprintf(buf, MAX_LINE, "%s backward match: %d (%d, %d)\n", msg, idx, row[idx], col[idx]);

		check_cursor(cursor, row[idx], col[idx], buf);
	}
}

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
	s_cursor cursor;
	wchar_t *result;

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
	result = s_table_update_filter(&table, &cursor);

	ut_check_wcs_null(result, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, MATCH_2, 5, "search insensitive");

	//
	// matches: 2 points: (1,1) (1,2)
	//
	check_prev_next(&table, &cursor, "search insensitive", MATCH_2, 1, 1, 1, 2);

	//
	// SEARCHING, SENSITIVE WITH 1 MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"bc", SF_IS_SENSITIVE, SF_IS_SEARCHING);
	result = s_table_update_filter(&table, &cursor);

	ut_check_wcs_null(result, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, MATCH_1, 5, "search sensitive");

	//
	// matches: 1 points: (1,1)
	//
	check_prev_next(&table, &cursor, "search insensitive", MATCH_1, 1, 1);

	//
	// SEARCHING WITH NO MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"hallo", SF_IS_SENSITIVE, SF_IS_SEARCHING);
	result = s_table_update_filter(&table, &cursor);

	ut_check_wcs_null(result, UT_IS_NOT_NULL);
	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "search no matches");
	check_cursor(&cursor, 1, 1, "search no matches - cursor 1");

	//
	// FILTERING, INSENSITIVE WITH 3 MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_INSENSITIVE, SF_IS_FILTERING);
	result = s_table_update_filter(&table, &cursor);

	ut_check_wcs_null(result, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, MATCH_3, 3, "filter insensitive");

	//
	// matches: 3 points: (1,1) (1,2) (2,2)
	//
	check_prev_next(&table, &cursor, "filter insensitive", MATCH_3, 1, 1, 1, 2, 2, 2);

	//
	// FILTERING, SENSITIVE WITH 1 MATCH
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"xx", SF_IS_SENSITIVE, SF_IS_FILTERING);
	result = s_table_update_filter(&table, &cursor);

	ut_check_wcs_null(result, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, MATCH_1, 2, "filter sensitive");

	//
	// matches: 1 points: (1,1)
	//
	check_prev_next(&table, &cursor, "filter sensitive", MATCH_1, 1, 1);

	//
	// FILTERING WITH NO MATCHES
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"eF", SF_IS_SENSITIVE, SF_IS_FILTERING);
	result = s_table_update_filter(&table, &cursor);

	ut_check_wcs_null(result, UT_IS_NOT_NULL);
	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "filter no matches");
	check_cursor(&cursor, 0, 0, "filter no matches - cursor 1");

	//
	// RESET AFTER FILTERING, SENSITIVE WITH 1 MATCH
	//
	s_filter_set(&table.filter, SF_IS_ACTIVE, L"XO", SF_IS_SENSITIVE, SF_IS_FILTERING);
	result = s_table_update_filter(&table, &cursor);

	ut_check_wcs_null(result, UT_IS_NULL);
	check_filter_result(&table, SF_IS_ACTIVE, 1, 2, "reset - 1");
	check_cursor(&cursor, 1, 2, "reset cursor - 1");

	//
	// Do the reset
	//
	s_filter_set_inactive(&table.filter);
	s_table_reset_filter(&table, &cursor);

	check_filter_result(&table, SF_IS_INACTIVE, 0, 5, "reset - 2");
	check_cursor(&cursor, 0, 0, "reset cursor - 2");

	//
	// Calling prev / next has no effect if filtering is inactive
	//
	s_cursor_pos(&cursor, 1, 2);

	s_table_prev_next(&table, &cursor, DIR_FORWARD);
	check_cursor(&cursor, 1, 2, "reset - cursor 4");

	s_cursor_pos(&cursor, 2, 1);

	s_table_prev_next(&table, &cursor, DIR_BACKWARD);
	check_cursor(&cursor, 2, 1, "reset - cursor 5");

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_table_search_filter() End\n");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	print_debug_str("ut_table.c - Start tests\n");

	setlocale(LC_ALL, "");

	test_table_field_dimension();

	test_table_get_ratio();

	test_table_has_header();

	test_table_mean_std_dev();

	test_table_search_filter();

	print_debug_str("ut_table.c - End tests\n");

	return EXIT_SUCCESS;
}

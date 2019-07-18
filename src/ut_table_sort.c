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

#include "ncv_parser.h"
#include "ut_utils.h"

#include <locale.h>

#define STRICT_COL_TRUE true

#define DO_TRIM_FALSE false

/******************************************************************************
 * The function checks the sorting of the table by a given column with a given
 * direction,
 *****************************************************************************/

static void test_sort_wcs() {
	s_table table;
	s_cursor cursor;
	s_table_set_defaults(table);

	print_debug_str("test_sort_wcs() Start\n");

	const wchar_t data[] =

	L"bb" DL "BB" NL
	L"cc" DL "CC" NL
	L"dd" DL "DD" NL
	L"aa" DL "AA" NL;

	FILE *tmp = ut_create_tmp_file(data);

	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, DO_TRIM_FALSE, STRICT_COL_TRUE);

	parser_process_file(tmp, &cfg_parser, &table);

	table.show_header = false;
	s_filter_set_inactive(&table.filter);

	//
	// Forward with column 0
	//
	s_sort_update(&table.sort, 0, E_DIR_FORWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 0, 4, (const wchar_t*[] ) { L"aa", L"bb", L"cc", L"dd" });

	//
	// Backward with column 1
	//
	s_sort_update(&table.sort, 1, E_DIR_BACKWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 1, 4, (const wchar_t*[] ) { L"DD", L"CC", L"BB", L"AA" });

	//
	// If we switch on the header showing, a reset is necessary.
	//
	table.show_header = true;
	s_table_reset_rows(&table);

	//
	// Forward (with header)
	//
	s_sort_update(&table.sort, 0, E_DIR_FORWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 0, 4, (const wchar_t*[] ) { L"bb", L"aa", L"cc", L"dd" });

	//
	// Forward again => reset
	//
	s_sort_update(&table.sort, 0, E_DIR_FORWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 0, 4, (const wchar_t*[] ) { L"bb", L"cc", L"dd", L"aa" });

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_sort_wcs() End\n");
}

/******************************************************************************
 * The function checks the sorting of the table by a given column with a given
 * direction,
 *****************************************************************************/

static void test_sort_num() {
	s_table table;
	s_cursor cursor;
	s_table_set_defaults(table);

	print_debug_str("test_sort_num() Start\n");

	const wchar_t data[] =

	L"0" DL " 11.11 Euro" NL
	L"1" DL "  2.22 Euro" NL
	L"2" DL "           " NL
	L"3" DL "333.33 Euro" NL
	L"4" DL "  0.01 Euro" NL;

	FILE *tmp = ut_create_tmp_file(data);

	s_cfg_parser cfg_parser;
	s_cfg_parser_set(&cfg_parser, NULL, W_DELIM, DO_TRIM_FALSE, STRICT_COL_TRUE);

	parser_process_file(tmp, &cfg_parser, &table);

	table.show_header = false;
	s_filter_set_inactive(&table.filter);

	//
	// Forward with column 1
	//
	s_sort_update(&table.sort, 1, E_DIR_FORWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 0, 5, (const wchar_t*[] ) { L"4", L"1", L"0", L"3", L"2" });

	//
	// Backward with column 1
	//
	s_sort_update(&table.sort, 1, E_DIR_BACKWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 0, 5, (const wchar_t*[] ) { L"2", L"3", L"0", L"1", L"4" });

	//
	// Backward again with column 1 => reset
	//
	s_sort_update(&table.sort, 1, E_DIR_BACKWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 0, 5, (const wchar_t*[] ) { L"0", L"1", L"2", L"3", L"4" });

	//
	// If we switch on the header showing, a reset is necessary.
	//
	table.show_header = true;
	s_table_reset_rows(&table);

	//
	// Forward with column 1 with header
	//
	s_sort_update(&table.sort, 1, E_DIR_FORWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 0, 5, (const wchar_t*[] ) { L"0", L"4", L"1", L"3", L"2" });

	//
	// Backward with column 1 with header
	//
	s_sort_update(&table.sort, 1, E_DIR_BACKWARD);
	s_table_update_filter_sort(&table, &cursor, false, true);

	ut_check_table_column(&table, 0, 5, (const wchar_t*[] ) { L"0", L"2", L"3", L"1", L"4" });

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_sort_num() End\n");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	print_debug_str("ut_table_sort.c - Start tests\n");

	//
	// Use the default locale ('C' or 'POSIX') to ensure that the double
	// conversion succeeds. If the unit tests relay on the clients locale they
	// could fail.
	//
	// https://www.gnu.org/software/libc/manual/html_node/Choosing-Locale.html
	//
	setlocale(LC_ALL, "C");

	test_sort_wcs();

	test_sort_num();

	print_debug_str("ut_table_sort.c - End tests\n");

	return EXIT_SUCCESS;
}

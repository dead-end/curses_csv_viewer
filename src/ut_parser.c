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

#include "ut_utils.h"
#include "ncv_parser.h"

#include <locale.h>

/******************************************************************************
 * The function reads and parses a csv file. All fields are compared with the
 * expected values. The unit test should test escaping.
 *****************************************************************************/

#define __
#define ___

static void test_parser() {
	s_table table;

	const wchar_t *data =

	L"f00" __ __ __ DL EC"f01->\nf01"EC __ __ __ DL "f02" NL
	EC"f10"EC __ __ DL EC"f11->\r\"\"d111\"\""EC DL EC"f12"EC NL
	"f20" __ __ ___ DL "f21" __ __ __ __ ___ ___ DL "f22" NL
	EC"f30->,d30"EC DL EC"f31"EC __ __ __ __ ___ DL EC"f32->\r\nd32"EC NL
	__ __ __ __ ___ DL "end" __ __ __ __ ___ ___ DL;

	log_debug_str("Start");

	const s_cfg_parser cfg_parser = { .filename = NULL, .delim = W_DELIM, .do_trim = false, .strict = true };

	FILE *tmp = ut_create_tmp_file(data);
	parser_process_file(tmp, &cfg_parser, &table);

	ut_check_table_row(&table, 0, 3, (const wchar_t*[] ) { L"f00", L"f01->\nf01", L"f02" });
	ut_check_table_row(&table, 1, 3, (const wchar_t*[] ) { L"f10", L"f11->\n\"d111\"", L"f12" });
	ut_check_table_row(&table, 2, 3, (const wchar_t*[] ) { L"f20", L"f21", L"f22" });
	ut_check_table_row(&table, 3, 3, (const wchar_t*[] ) { L"f30->,d30", L"f31", L"f32->\nd32" });
	ut_check_table_row(&table, 4, 3, (const wchar_t*[] ) { L"", L"end", L"" });

	//
	// Check the widths of the columns and the heights of the rows.
	//
	ut_check_int_array(table.width, (int[] ) { 9, 6, 5 }, 3, "column widths");
	ut_check_int_array(table.height, (int[] ) { 2, 2, 1, 2, 1 }, 5, "row heights");

	//
	// cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	log_debug_str("End");
}

/******************************************************************************
 * The function checks an csv file with one column and 4 rows. The fields are:
 * a0 a1 a2 a3. The differences are the line endings.
 *****************************************************************************/

static void helper_line_endings(const wchar_t *data) {
	s_table table;

	const s_cfg_parser cfg_parser = { .filename = NULL, .delim = W_DELIM, .do_trim = false, .strict = true };

	FILE *tmp = ut_create_tmp_file(data);
	parser_process_file(tmp, &cfg_parser, &table);

	ut_check_table_column(&table, 0, 4, (const wchar_t*[] ) { L"a0", L"a1", L"a2", L"a3" });

	//
	// Check the widths of the columns and the heights of the rows.
	//
	ut_check_int_array(table.width, (int[] ) { 2 }, 1, "column widths");
	ut_check_int_array(table.height, (int[] ) { 1, 1, 1, 1 }, 4, "row heights");

	//
	// cleanup
	//
	s_table_free(&table);

	fclose(tmp);
}

/******************************************************************************
 * The function checks the different line endings of a csv file. The line
 * endings can occure at the end of a record or at the end of the file.
 *
 * unix:    \n
 * windows: \r\n
 * mac:     \r
 *****************************************************************************/

static void test_line_endings() {

	log_debug_str("Start");

	helper_line_endings(L"a0" NL "a1" CR "a2" CR NL "a3");
	helper_line_endings(L"a0" NL "a1" CR "a2" CR NL "a3" CR);
	helper_line_endings(L"a0" NL "a1" CR "a2" CR NL "a3" NL);
	helper_line_endings(L"a0" NL "a1" CR "a2" CR NL "a3" CR NL);

	log_debug_str("End");
}

/******************************************************************************
 * The function reads and parses a csv file that contains: ",\n,"
 *****************************************************************************/

static void test_parser_empty() {
	s_table table;

	log_debug_str("Start");

	const s_cfg_parser cfg_parser = { .filename = NULL, .delim = W_DELIM, .do_trim = false, .strict = true };

	FILE *tmp = ut_create_tmp_file(L",\n,");
	parser_process_file(tmp, &cfg_parser, &table);

	ut_check_table_column(&table, 0, 2, (const wchar_t*[] ) { L"", L"" });
	ut_check_table_column(&table, 1, 2, (const wchar_t*[] ) { L"", L"" });

	//
	// Check the widths of the columns and the heights of the rows.
	//
	ut_check_int_array(table.width, (int[] ) { 1, 1 }, 2, "column widths");
	ut_check_int_array(table.height, (int[] ) { 1, }, 2, "row heights");

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	log_debug_str("End");
}

/******************************************************************************
 * The function parses a corrupt csv file with non strict mode. It adds missing
 * fields and removes empty rows and columns at the end of the data.
 *****************************************************************************/

static void test_add_remove() {
	s_table table;
	s_table_set_defaults(table);

	log_debug_str("Start");

	const wchar_t *data =

	L"" DL " " DL NL
	L"" DL "1" NL
	L"" DL "2" DL "2" DL " " DL"" DL"" DL"" DL "" NL
	L"" DL "3" DL "3" DL "3" NL
	L"" DL " " DL " " DL " " DL" " DL NL
	L"" DL " " DL NL;

	const s_cfg_parser cfg_parser = { .filename = NULL, .delim = W_DELIM, .do_trim = true, .strict = false };

	FILE *tmp = ut_create_tmp_file(data);
	parser_process_file(tmp, &cfg_parser, &table);

	ut_check_int(table.no_columns, 4, "no cols");
	ut_check_int(table.no_rows, 4, "no rows");

	ut_check_table_row(&table, 0, 4, (const wchar_t*[] ) { L"", L"", L"", L"" });
	ut_check_table_row(&table, 1, 4, (const wchar_t*[] ) { L"", L"1", L"", L"" });
	ut_check_table_row(&table, 2, 4, (const wchar_t*[] ) { L"", L"2", L"2", L"" });
	ut_check_table_row(&table, 3, 4, (const wchar_t*[] ) { L"", L"3", L"3", L"3" });

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

	test_parser();

	test_line_endings();

	test_parser_empty();

	test_add_remove();

	log_debug_str("End");

	return EXIT_SUCCESS;
}

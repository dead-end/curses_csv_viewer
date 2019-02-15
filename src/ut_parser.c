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
 * expected values.
 *****************************************************************************/

static void test_parser() {
	s_table table;

	const wchar_t *data = L"f00,\"f01->\nf01\",f02\n"
			"\"f10\",\"f11->\r\"\"d111\"\"\",\"f12\"\n"
			"f20,f21,f22\n"
			"\"f30->,d30\",\"f31\",\"f32->\r\nd32\"\n"
			",end,";

	print_debug_str("test_parser2() Start\n");

	FILE *tmp = ut_create_tmp_file(data);

	parser_process_file(tmp, W_DELIM, &table);

	//
	// Check all fields "by hand"
	//
	ut_check_wchar_str(table.fields[0][0], L"f00");
	ut_check_wchar_str(table.fields[0][1], L"f01->\nf01");
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

	ut_check_int(table.height[0], 2, "row_height: 0");
	ut_check_int(table.height[1], 2, "row_height: 1");
	ut_check_int(table.height[2], 1, "row_height: 2");
	ut_check_int(table.height[3], 2, "row_height: 3");
	ut_check_int(table.height[4], 1, "row_height: 4");

	//
	// cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_parser() End\n");
}

/******************************************************************************
 * The function checks an csv file with one column and 4 rows. The fields are:
 * a0 a1 a2 a3. The differences are the line endings.
 *****************************************************************************/

static void helper_line_endings(const wchar_t *data) {
	s_table table;

	FILE *tmp = ut_create_tmp_file(data);

	parser_process_file(tmp, W_DELIM, &table);

	//
	// Check all fields "by hand"
	//
	ut_check_wchar_str(table.fields[0][0], L"a0");
	ut_check_wchar_str(table.fields[1][0], L"a1");
	ut_check_wchar_str(table.fields[2][0], L"a2");
	ut_check_wchar_str(table.fields[3][0], L"a3");

	//
	// Check the meta data
	//
	ut_check_int(table.width[0], 2, "col width: 0");

	ut_check_int(table.height[0], 1, "row_height: 0");
	ut_check_int(table.height[1], 1, "row_height: 1");
	ut_check_int(table.height[2], 1, "row_height: 2");
	ut_check_int(table.height[3], 1, "row_height: 3");

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

	print_debug_str("test_line_endings() Start\n");

	helper_line_endings(L"a0\na1\ra2\r\na3");
	helper_line_endings(L"a0\na1\ra2\r\na3\r");
	helper_line_endings(L"a0\na1\ra2\r\na3\n");
	helper_line_endings(L"a0\na1\ra2\r\na3\r\n");

	print_debug_str("test_line_endings() End\n");
}

/******************************************************************************
 * The function reads and parses a csv file that contains: ",\n,"
 *****************************************************************************/

static void test_parser_empty() {
	s_table table;

	print_debug_str("test_parser_empty() Start\n");

	FILE *tmp = ut_create_tmp_file(L",\n,");

	parser_process_file(tmp, W_DELIM, &table);

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

	//
	// Cleanup
	//
	s_table_free(&table);

	fclose(tmp);

	print_debug_str("test_parser_empty() End\n");
}

/******************************************************************************
 * The main function simply starts the test.
 *****************************************************************************/

int main() {

	print_debug_str("ut_parser.c - Start tests\n");

	setlocale(LC_ALL, "");

	test_parser();

	test_line_endings();

	test_parser_empty();

	print_debug_str("ut_parser.c - End tests\n");

	return EXIT_SUCCESS;
}

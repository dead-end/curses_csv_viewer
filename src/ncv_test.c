/*
 * file: ncv_test.c
 *
 */

#include <locale.h>

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_parser.h"

/***************************************************************************
 *
 **************************************************************************/

static void compare(wchar_t *str1, wchar_t *str2) {
	if (wcscmp(str1, str2) != 0) {
		print_exit("Strings differ: %ls and: %ls\n", str1, str2);
	}
}

/***************************************************************************
 *
 **************************************************************************/

static void test1() {
	s_table table;

	print_debug_str("test1() Start\n");

	parser_process_file("res/test1.csv", W_DELIM, &table);

	compare(table.fields[0][0], L"f00");
	compare(table.fields[0][1], L"f01");
	compare(table.fields[0][2], L"f02");

	compare(table.fields[1][0], L"f10");
	compare(table.fields[1][1], L"f11->\n\"d11\"");
	compare(table.fields[1][2], L"f12");

	compare(table.fields[2][0], L"f20");
	compare(table.fields[2][1], L"f21");
	compare(table.fields[2][2], L"f22");

	compare(table.fields[3][0], L"f30->,d30");
	compare(table.fields[3][1], L"f31");
	compare(table.fields[3][2], L"f32->\nd32");

	compare(table.fields[4][0], L"");
	compare(table.fields[4][1], L"");
	compare(table.fields[4][2], L"end");

	s_table_free(&table);

	print_debug_str("test1() End\n");
}

/***************************************************************************
 *
 **************************************************************************/

int main(const int argc, char * const argv[]) {

	print_debug_str("main() Start\n");

	setlocale(LC_ALL, "");

	test1();

	print_debug_str("main() End\n");

	return EXIT_SUCCESS;
}

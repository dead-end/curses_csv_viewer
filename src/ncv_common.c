/*
 * file: ncv_common.c
 */

#include "ncv_common.h"

/***************************************************************************
 * The function allocates memory and terminates the program in case of an
 * error.
 **************************************************************************/

void *xmalloc(const size_t size) {

	void *ptr = malloc(size);

	if (ptr == NULL) {
		print_exit("Unable to allocate: %zu bytes of memory!\n", size);
	}

	return ptr;
}

/***************************************************************************
 * The function is used for unit tests. It checks whether an int parameter
 * has the expected value or not.
 **************************************************************************/

void ut_check_int(const int current, const int expected, const char *msg) {

	if (current != expected) {
		print_exit("ut_check_int() %s current: %d expected: %d\n", msg, current, expected);
	}

	print_debug("ut_check_int() OK - %s current: %d \n", msg, current);
}

/***************************************************************************
 * The function is used for unit tests. It checks whether an size_t
 * parameter has the expected value or not.
 **************************************************************************/

void ut_check_size(const size_t current, const size_t expected, const char *msg) {

	if (current != expected) {
		print_exit("ut_check_size() %s current: %zu expected: %zu\n", msg, current, expected);
	}

	print_debug("ut_check_size() OK - %s size: %zu \n", msg, current);
}

/***************************************************************************
 * The function is used for unit tests. It compares two wchar strings.
 **************************************************************************/

void ut_check_wchar_str(const wchar_t *str1, const wchar_t *str2) {

	if (wcscmp(str1, str2) != 0) {
		print_exit("ut_check_wchar_str() Strings differ: '%ls' and: '%ls'\n", str1, str2);
	}

	print_debug("ut_check_wchar_str() OK - String are equal: '%ls'\n", str1);
}

/***************************************************************************
 * The function is used for unit tests. It ensures that a given wchar string
 * is null.
 **************************************************************************/

void ut_check_wchar_null(const wchar_t *str) {

	if (str != NULL) {
		print_exit("ut_check_wchar_null() Pointer is not null: '%ls'\n", str);
	}

	print_debug_str("ut_check_wchar_null() OK\n");
}

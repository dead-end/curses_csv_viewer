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
 * The function creates a temp file and copies the input from stdin to that
 * file. It uses read instead of fread, because the latter does not work
 * with fgetws() and rewind().
 **************************************************************************/

FILE *stdin_2_tmp() {
	ssize_t bytes_read, bytes_write;
	char buf[BUF_SIZE];
	FILE *tmp;

	//
	// Create a temp file
	//
	if ((tmp = tmpfile()) == NULL) {
		print_exit("stdin_2_tmp() Unable to create tmp file: %s\n", strerror(errno));
	}

	int in = fileno(stdin);
	int out = fileno(tmp);

	//
	// Copy the data until we encounter the end of input or an error.
	//
	while ((bytes_read = read(in, buf, BUF_SIZE)) > 0) {

		bytes_write = write(out, buf, bytes_read);

		//
		// Check for write errors.
		//
		if (bytes_write != bytes_read) {
			if (bytes_write == -1) {
				print_exit("stdin_2_tmp() Unable to write to temp file: %s\n", strerror (errno));
			} else {
				print_exit_str("stdin_2_tmp() Could not write the whole buffer!\n");
			}
		}
	}

	//
	// Check for read errors.
	//
	if (bytes_read == -1) {
		print_exit("stdin_2_tmp() Could not read from stdin: %s\n", strerror (errno));
	}

	//
	// Rewind the file.
	//
	if (fseek(tmp, 0L, SEEK_SET) == -1) {
		print_exit("stdin_2_tmp() Unable to rewind file due to: %s\n", strerror(errno));
	}

	return tmp;
}

/***************************************************************************
 * The function converts a multi byte char string to a wide char string. The
 * result is stored in a newly allocated wide char string and has to be
 * freed by the calling function.
 **************************************************************************/

wchar_t *convert_mbs_2_wchars(const char *mbs) {
	size_t mbslen;
	wchar_t *result;

	//
	// Calculate the wchar_t string length.
	//
	mbslen = mbstowcs(NULL, mbs, 0);
	if (mbslen == (size_t) -1) {
		print_exit_str("convert_mbs_2_wchars() Unable to calculate wchar_t string size!");
	}

	//
	// Allocate memory for the wchar_t string.
	//
	result = calloc(mbslen + 1, sizeof(wchar_t));
	if (result == NULL) {
		print_exit_str("convert_mbs_2_wchars() Unable allocate memory for the wchar_t string!");
	}

	//
	// Convert the multi byte string
	//
	if (mbstowcs(result, mbs, mbslen + 1) == (size_t) -1) {
		print_exit_str("convert_mbs_2_wchars() Unable to convert the multi byte string!");
	}

	return result;
}

/***************************************************************************
 * The function converts a multi byte char string to a wide char string.
 **************************************************************************/

size_t mbs_2_wchars(const char *mbs, wchar_t *buffer, const int buf_size) {

	size_t result = mbstowcs(buffer, mbs, buf_size);

	if (result == (size_t) -1) {
		print_exit_str("mbchar_2_wchar() Encountered an invalid multibyte sequence!\n");
	}

	if (result >= (size_t) buf_size) {
		print_exit("mbchar_2_wchar() Buffer too small (required: %zu provided: %zu)\n", result, (size_t ) buf_size)
	;
	}

	return result;
}

/***************************************************************************
 * The function removes leading and tailing spaces. The process changes the
 * argument string.
 **************************************************************************/

char *trim(char *str) {
	char *ptr;

	//
	// skip leading white spaces
	//
	for (ptr = str; isspace(*ptr); ptr++)
		;

	//
	// skip tailing white spaces by overwriting them with '\0'
	//
	size_t len = strlen(ptr);
	for (int i = len - 1; i >= 0 && isspace(ptr[i]); i--) {
		ptr[i] = '\0';
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

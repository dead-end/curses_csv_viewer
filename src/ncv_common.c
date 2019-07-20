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

#include "ncv_common.h"

#include <ctype.h>
#include <wchar.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wctype.h>

/******************************************************************************
 * The function allocates memory and terminates the program in case of an
 * error.
 *****************************************************************************/

void* xmalloc(const size_t size) {

	void *ptr = malloc(size);

	if (ptr == NULL) {
		log_exit("Unable to allocate: %zu bytes of memory!", size);
	}

	return ptr;
}

/******************************************************************************
 * The function creates a temp file and copies the input from stdin to that
 * file. It uses read instead of fread, because the latter does not work with
 * rewind() and fgetws().
 *****************************************************************************/

FILE* stdin_2_tmp() {
	ssize_t bytes_read, bytes_write;
	char buf[BUF_SIZE];
	FILE *tmp;

	//
	// Create a temp file
	//
	if ((tmp = tmpfile()) == NULL) {
		log_exit("Unable to create tmp file: %s", strerror(errno));
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
				log_exit("Unable to write to temp file: %s", strerror (errno));
			} else {
				log_exit_str("Could not write the whole buffer!");
			}
		}
	}

	//
	// Check for read errors.
	//
	if (bytes_read == -1) {
		log_exit("Could not read from stdin: %s", strerror (errno));
	}

	//
	// Rewind the file.
	//
	if (fseek(tmp, 0L, SEEK_SET) == -1) {
		log_exit("Unable to rewind file due to: %s", strerror(errno));
	}

	return tmp;
}

/******************************************************************************
 * The function reads a wchar_t from a stream. It converts the different line
 * endings (windows: \r\n mac: \r) to a standard (unix: \n). It also does error
 * processing.
 *
 * Due to the bug, the file has to be closed on errors. If not, a segmentation
 * fault can happen after calling exit.
 *
 * https://sourceware.org/bugzilla/show_bug.cgi?id=20938
 *****************************************************************************/

wchar_t read_wchar(FILE *file) {
	wint_t wint;

	//
	// Check for WEOF which indicates EOF or an error
	//
	if ((wint = fgetwc(file)) == WEOF && ferror(file)) {

		if (errno == EILSEQ) {
			fclose(file);
			log_exit_str("Character encoding error!");

		} else {
			fclose(file);
			log_exit("I/O error: %s", strerror(errno));
		}
	}

	//
	// For processing the line ending means dealing with carriage returns.
	//
	if ((wchar_t) wint == W_CR) {

		//
		// Read the next char. If it is \n then we have a windows line
		// ending. If not, we have a mac.
		//
		if ((wint = fgetwc(file)) == WEOF && ferror(file)) {

			if (errno == EILSEQ) {
				fclose(file);
				log_exit_str("Character encoding error!");

			} else {
				fclose(file);
				log_exit_str("I/O error!");
			}
		}

		//
		// If the char after the carriage return is not a line feed we have
		// a mac and we have to push back the char.
		//
		if ((wchar_t) wint != W_NEW_LINE) {

			//
			//
			//
			if (wint != WEOF && ungetwc(wint, file) == WEOF) {
				fclose(file);
				log_exit_str("Unable to push back character!");
			}

			return W_NEW_LINE;
		}

		//
		// If the char after the carriage return is a line feed, we simply
		// return that char, which means that we ignore the carriage before.
		//
	}

	return (wchar_t) wint;
}

/******************************************************************************
 * The function converts a multi byte char string to a wide char string. The
 * buffer has to be large enough or the program terminates with an error.
 *****************************************************************************/
// TODO: unit tests
size_t mbs_2_wchars(const char *mbs, wchar_t *buffer, const int buf_size) {

	const size_t result = mbstowcs(buffer, mbs, buf_size);

	if (result == (size_t) -1) {
		log_exit_str("Encountered an invalid multibyte sequence!");
	}

	//
	// If (result == buf_size) the terminating L'\0' is missing, which is an
	// error.
	//
	if (result >= (size_t) buf_size) {
		log_exit("Buffer too small (required: %zu provided: %zu)", result, (size_t ) buf_size);
	}

	return result;
}

/******************************************************************************
 * The function removes leading and tailing spaces. The process changes the
 * argument string. So do not call the function with a literal string.
 *****************************************************************************/

char* trim(char *str) {
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

/******************************************************************************
 * The function removes leading and tailing spaces. The process changes the
 * argument string. So do not call the function with a literal string.
 *****************************************************************************/

wchar_t* wcstrim(wchar_t *str) {
	wchar_t *ptr;

	//
	// skip leading white spaces
	//
	for (ptr = str; iswspace(*ptr); ptr++)
		;

	//
	// skip tailing white spaces by overwriting them with '\0'
	//
	size_t len = wcslen(ptr);
	for (int i = len - 1; i >= 0 && iswspace(ptr[i]); i--) {
		ptr[i] = '\0';
	}

	return ptr;
}

/******************************************************************************
 * The function checks if a wchar_t string is empty, which means, that the
 * string has length 0 or consists only of whitespaces.
 *****************************************************************************/
// TODO: unit tests
bool wcs_is_empty(const wchar_t *str) {
	const wchar_t *ptr;

	for (ptr = str; *ptr != W_STR_TERM; ptr++) {
		if (!iswspace(*ptr)) {
			return false;
		}
	}

	return true;
}

/******************************************************************************
 * The function is a wchar_t variant of the strstr function.
 *****************************************************************************/

wchar_t* wcs_casestr(const wchar_t *str, const wchar_t *find) {
	wchar_t first_chr, str_chr;

	//
	// Split the find string in the first char and the rest
	//
	if ((first_chr = *find++) != 0) {

		first_chr = (wchar_t) towlower((wint_t) first_chr);
		const size_t len = wcslen(find);

		do {

			//
			// Go through the string until the first char matches
			//
			do {
				if ((str_chr = *str++) == 0) {
					return NULL;
				}
			} while ((wchar_t) tolower((wint_t) str_chr) != first_chr);

			//
			// If the first char matches compare the rest of the find string.
			//
		} while (wcsncasecmp(str, find, len) != 0);
		str--;
	}

	return ((wchar_t*) str);
}

/******************************************************************************
 * The function is called with a null terminated array of strings. It
 * determines the number of rows and the the number of cols, which is the
 * maximum string length.
 *****************************************************************************/

void str_array_sizes(const char *msgs[], int *rows, int *cols) {

	int len;

	for (*rows = 0, *cols = 0; msgs[*rows] != NULL; (*rows)++) {
		len = strlen(msgs[*rows]);
		if (len > *cols) {
			*cols = len;
		}
	}
}

/******************************************************************************
 * The function computes the start of an aligned string. If the string does not
 * fit in its container, the function returns -1;
 *****************************************************************************/
// TODO: unit tests
int get_align_start(const int max, const int len, const enum e_align align) {

	if (len > max) {
		return -1;
	}

	int start;

	switch (align) {

	case AT_LEFT:
		start = 0;
		break;

	case AT_RIGHT:
		start = max - len;
		break;

	case AT_CENTER:
		start = (max - len) / 2;
		break;

	default:
		log_exit("Unknown value for align: %d", align)
		;
	}

	return start;
}


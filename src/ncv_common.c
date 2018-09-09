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

/***************************************************************************
 * The function allocates memory and terminates the program in case of an
 * error.
 **************************************************************************/

void *xmalloc(const size_t size) {

	void *ptr = malloc(size);

	if (ptr == NULL) {
		print_exit("xmalloc() Unable to allocate: %zu bytes of memory!\n", size);
	}

	return ptr;
}

/***************************************************************************
 * The function creates a temp file and copies the input from stdin to that
 * file. It uses read instead of fread, because the latter does not work
 * with rewind() and fgetws().
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
 * buffer has to be large enough or the program terminates with an error.
 **************************************************************************/

size_t mbs_2_wchars(const char *mbs, wchar_t *buffer, const int buf_size) {

	const size_t result = mbstowcs(buffer, mbs, buf_size);

	if (result == (size_t) -1) {
		print_exit_str("mbs_2_wchars() Encountered an invalid multibyte sequence!\n");
	}

	//
	// If (result == buf_size) the terminating L'\0' is missing, which is an
	// error.
	//
	if (result >= (size_t) buf_size) {
		print_exit("mbs_2_wchars() Buffer too small (required: %zu provided: %zu)\n", result, (size_t ) buf_size);
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
 * The function reads a wchar_t from a stream. It converts the different
 * line endings (windows: \r\n mac: \r) to a standard (unix: \n). It also
 * does error processing.
 **************************************************************************/

wchar_t read_wchar(FILE *file) {
	wint_t wint;

	//
	// Check for WEOF which indicates EOF or an error
	//
	if ((wint = fgetwc(file)) == WEOF && ferror(file)) {

		if (errno == EILSEQ) {
			print_exit_str("read_wchar() Character encoding error!\n");

		} else {
			print_exit("read_wchar() I/O error: %s\n", strerror(errno));
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
				print_exit_str("read_wchar() Character encoding error!\n");

			} else {
				print_exit_str("read_wchar() I/O error!\n");
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
				print_exit_str("read_wchar() Unable to push back character!\n");
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

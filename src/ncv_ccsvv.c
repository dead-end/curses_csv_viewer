/*
 * file: ncv_ccsvv.c
 */

#include <locale.h>
#include <unistd.h>
#include <ncursesw/ncurses.h>

#include "ncv_common.h"
#include "ncv_table.h"
#include "ncv_parser.h"
#include "ncv_curses.h"

/***************************************************************************
 * The function writes the program usage. It is called with an error flag.
 * Depending on the flag the stream (stdout / stderr) is selected. The
 * function contains an optional message (not NULL) that will be written.
 **************************************************************************/

static void print_usage(const bool has_error, const char* msg) {
	FILE *stream;
	int status;

	//
	// choose stdout / stderr depending on the error flag
	//
	if (has_error) {
		status = EXIT_FAILURE;
		stream = stderr;
	} else {
		status = EXIT_SUCCESS;
		stream = stdout;
	}

	//
	// if the function call contains a message it is written
	//
	if (msg != NULL) {
		if (has_error) {
			fprintf(stream, "ERROR - ");
		}
		fprintf(stream, "%s\n", msg);
	}

	//
	// print the usage information
	//
	fprintf(stream, "ccsvv [] -f FILE\n");
	fprintf(stream, "\t -f FILE The csv filename \n");

	exit(status);
}

/***************************************************************************
 * The main function parses the command line options and starts the csv file
 * processing.
 **************************************************************************/

int main(const int argc, char * const argv[]) {
	int c;
	char *filename = NULL;

	s_table table;

	print_debug_str("main() Start\n");

	//
	// parse the command line options
	//
	while ((c = getopt(argc, argv, "f:")) != -1) {
		switch (c) {

		case 'f':
			filename = optarg;
			print_debug("Found filename: %s\n", filename);
			break;

		default:
			print_usage(true, NULL);
		}
	}

	//
	// ensure that the required command line options are defined.
	//
	if (filename == NULL) {
		print_usage(true, "No csv filename defined!");
	}

	//
	// import the locale from the environment to allow proper wchar_t's
	//
	setlocale(LC_ALL, "");

	//
	// start processing the csv file
	//
	parser_process_file(filename, W_DELIM, &table);

#ifdef DEBUG
	s_table_dump(&table);
#endif

	curses_init();

	curses_loop(&table);

	curses_finish();

	//
	// free the allocated memory
	//
	s_table_free(&table);

	print_debug_str("main() End\n");
	return EXIT_SUCCESS;
}

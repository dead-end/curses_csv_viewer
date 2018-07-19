/*
 * file: ncv_ccsvv.c
 */

#include <locale.h>
#include <unistd.h>

#include "ncv_common.h"
#include "ncv_ncurses.h"
#include "ncv_table.h"
#include "ncv_parser.h"
#include "ncv_curses.h"

#include "ncv_win_header.h"
#include "ncv_win_filter.h"
#include "ncv_win_table.h"
#include "ncv_win_footer.h"

//
// The table struct is defined static to be able to use it in the function
// exit_callback().
//
static s_table table;

/***************************************************************************
 * A cleanup function for the ncurses stuff. The important call, is the call
 * of endwin() which resets the terminal. So the freeing function before
 * should not fail.
 **************************************************************************/

static void exit_callback() {

	//
	// Free table data
	//
	s_table_free(&table);

	//
	// Free window resources.
	//
	win_header_free();

	win_filter_free();

	win_table_free();

	win_footer_free();

	//
	// Finish ncurses
	//
	endwin();
}

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
	fprintf(stream, "\t -f <FILE>      The name of the csv file.\n");
	fprintf(stream, "\t -d <DELIMITER> An alternative delimiter character.\n");
	fprintf(stream, "\t -m             Do not use colors (monochrome)\n");

	exit(status);
}

/***************************************************************************
 * The main function parses the command line options and starts the csv file
 * processing.
 **************************************************************************/

int main(const int argc, char * const argv[]) {
	int c;
	char *filename = NULL;
	wchar_t delimiter = W_DELIM;
	bool monochrom = false;

	//
	// import the locale from the environment to allow proper wchar_t's
	//
	setlocale(LC_ALL, "");

	print_debug_str("main() Start\n");

	//
	// parse the command line options
	//
	while ((c = getopt(argc, argv, "mf:d:")) != -1) {
		switch (c) {

		case 'm':
			monochrom = true;
			print_debug_str("Use monochrom.\n");
			break;

		case 'f':
			filename = optarg;
			print_debug("Found filename: %s\n", filename);
			break;

		case 'd':

			//
			// ensure that the delimiter consists of one character
			//
			if (strlen(optarg) != 1) {
				print_usage(true, "Only a one character delimiter is allowed!");
			}

			//
			// Convert the char to a wide char
			//
			if (mbtowc(NULL, NULL, 0) < 0) {
				print_usage(true, "Unable to initialize the conversion to wide chars!");
			}

			if (mbtowc(&delimiter, optarg, MB_CUR_MAX) < 0) {
				print_usage(true, "Unable to convert the delimiter to a wide char!");
			}

			print_debug("Delimiter: %lc\n", delimiter);
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
	// start processing the csv file
	//
	//TODO: call with FILE * (maybe stdin)
	parser_process_file(filename, delimiter, &table);

#ifdef DEBUG
	s_table_dump(&table);
#endif

	ncurses_init(monochrom);

	//
	// Register exit callback.
	//
	if (on_exit(exit_callback, NULL) != 0) {
		print_exit_str("Unable to register exit function!\n");
	}

	//
	// Initialize the four windows of the application.
	//
	win_header_init();

	win_filter_init();

	win_table_init();

	win_footer_init();

	curses_loop(&table, filename);

	print_debug_str("main() End\n");

	return EXIT_SUCCESS;
}

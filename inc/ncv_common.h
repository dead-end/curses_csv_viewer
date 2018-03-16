/*
 * file: ncv_common.h
 */

#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

//
// common includes
//
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

//
// definition of the print_debug macro.
//
#ifdef DEBUG
#define DEBUG_OUT stdout
#define print_debug(fmt, ...) fprintf(DEBUG_OUT, "DEBUG - " fmt, ##__VA_ARGS__)
#define print_debug_str(fmt)  fprintf(DEBUG_OUT, "DEBUG - " fmt)
#else
#define print_debug(fmt, ...)
#define print_debug_str(fmt)
#endif

//
// definition of the print_error macro
//
#define print_exit(fmt, ...) fprintf(stderr, "FATAL - " fmt, ##__VA_ARGS__); exit(EXIT_FAILURE)
#define print_exit_str(fmt)  fprintf(stderr, "FATAL - " fmt); exit(EXIT_FAILURE)

#define MAX_LINE 1024

//
// some wchar characters
//
#define W_NEW_LINE L'\n'
#define W_STR_TERM L'\0'
#define W_QUOTE    L'\"'
#define W_CR       L'\r'
#define W_DELIM    L','

//
// a boolean type that can be undefined
//
enum bool_defined {
	BOOL_TRUE, BOOL_FLASE, BOOL_UNDEF
};

//
// unrelated utility functions
//
void *xmalloc(const size_t size);

#endif /* INCLUDE_COMMON_H_ */

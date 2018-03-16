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

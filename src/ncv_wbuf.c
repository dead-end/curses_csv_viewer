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

#include "ncv_wbuf.h"
#include "ncv_common.h"

#include <stdlib.h>

/******************************************************************************
 * The function is called to create a new s_wblock for a s_wbuf.
 *****************************************************************************/

static s_wblock *s_wblock_create(const int idx, const int size) {

	print_debug("s_wblock_create() Block: %d size: %d\n", idx, size);

	//
	// Allocate the memory for the struct and the buffer of the struct.
	//
	s_wblock *wblock = xmalloc(sizeof(s_wblock));

	wblock->buf = xmalloc(sizeof(wchar_t) * size);

	//
	// Initialize the struct members
	//
	wblock->idx = idx;

	wblock->size = size;

	wblock->next = NULL;

	return wblock;
}

/******************************************************************************
 * The function frees a s_wblock from a s_wbuf and returns the pointer to the
 * next member of the linked list.
 *****************************************************************************/

static s_wblock *s_wblock_free(s_wblock *wblock) {

	print_debug("s_wblock_free() Block: %d size: %d\n", wblock->idx, wblock->size);

	s_wblock *next = wblock->next;

	free(wblock->buf);

	free(wblock);

	return next;
}

/******************************************************************************
 * The function creates a s_wbuf struct with a given block size.
 *****************************************************************************/

s_wbuf *s_wbuf_create(const int block_size) {

	print_debug("s_wbuf_create() Creating s_wbuf with block size: %d\n", block_size);

	//
	// Allocate the memory
	//
	s_wbuf * wbuf = xmalloc(sizeof(s_wbuf));

	//
	// Initialize the struct members. A newly created s_wbuf has no s_wblocks.
	//
	wbuf->root = NULL;

	wbuf->block_size = block_size;

	s_wbuf_pos_init(&wbuf->end_pos);

	return wbuf;
}

/******************************************************************************
 * The function frees the s_wbuf struct with its associated s_wblocks.
 *****************************************************************************/

void s_wbuf_free(s_wbuf *wbuf) {

	print_debug_str("s_wbuf_free() Freeing s_wbuf\n");

	//
	// Free the s_wblocks of the s_wbuf struct.
	//
	for (s_wblock *start = wbuf->root; start != NULL; start = s_wblock_free(start))
		;

	//
	// Free the buffer struct
	//
	free(wbuf);
}

/******************************************************************************
 * The function adds a wchar_t to the s_wbuf. There are 3 cases. First, the
 * buffer has no blocks, so an initial block has to be created. Second, the
 * current block is full. In this case a new block has to be created. The last
 * case is that the current block has at least space for one wchar_t.
 *****************************************************************************/

void s_wbuf_add(s_wbuf *wbuf, const wchar_t wchar) {
	print_debug_str("s_wbuf_add() \n");

	if (wbuf->root == NULL) {

		//
		// On the first call of the function, the s_wbuf has no blocks.
		//
		print_debug_str("s_wbuf_add() s_wbuf is empty\n");

		//
		// Create the first s_wblock and set the end position the first wchar_t
		// in the block buffer.
		//
		wbuf->root = s_wblock_create(0, wbuf->block_size);

		s_wbuf_pos_set(&wbuf->end_pos, wbuf->root, 0);

	} else if (s_wbuf_pos_is_end_of_block(&wbuf->end_pos)) {

		//
		// Create a new s_wblock and move the position to the beginning of the
		// newly created block. On every call, we double the size of the block.
		//
		print_debug("s_wbuf_add() Reached end of block: %d\n", wbuf->end_pos.block->idx);

		//
		// Create a new s_wblock and set the position to the first wchar_t for
		// the new block. The size of the new block is doubled.
		//
		wbuf->end_pos.block->next = s_wblock_create(wbuf->end_pos.block->idx + 1, 2 * wbuf->end_pos.block->size);

		s_wbuf_pos_next_block(&wbuf->end_pos);

	} else {

		//
		// The current block has unused space, so we can use it.
		//
		wbuf->end_pos.idx++;
	}

	print_debug("s_wbuf_add() Block: %d size: %d index: %d wchar: %lc\n", wbuf->end_pos.block->idx, wbuf->end_pos.block->size, wbuf->end_pos.idx, wchar);

	//
	// Set the wchar_t to the new end position.
	//
	s_wbuf_pos_set_wchr(&wbuf->end_pos, wchar);
}

/******************************************************************************
 * The function gets the next wchar_t from the s_wbuf and stores the value in
 * the parameter wchr. It updates the position and if the end of the buffer is
 * reached it returns false.
 *****************************************************************************/

bool s_wbuf_next(s_wbuf *wbuf, s_wbuf_pos *cur_pos, wchar_t *wchr) {

	if (!s_wbuf_pos_is_set(cur_pos)) {

		//
		// At the first function call the current position is not initialized.
		//
		print_debug_str("s_wbuf_add() Initialize s_wbuf\n");

		s_wbuf_pos_set(cur_pos, wbuf->root, 0);

	} else if (s_wbuf_pos_equal(cur_pos, &wbuf->end_pos)) {

		//
		// The current position is the last valid wchar_t in the buffer, so we
		// do not switch to the next position.
		//
		print_debug_str("s_wbuf_add() Reached end of s_wbuf\n");

		return false;

	} else if (s_wbuf_pos_is_end_of_block(cur_pos)) {

		//
		// The last if condition checks for the end of the buffer, so we are
		// save to switch to the next block.
		//
		print_debug_str("s_wbuf_add() Switch to next s_wblock\n");

		s_wbuf_pos_next_block(cur_pos);

	} else {
		cur_pos->idx++;
	}

	print_debug("s_wbuf_add() Current block: %d size: %d index: %d\n", cur_pos->block->idx, cur_pos->block->size, cur_pos->idx);
	print_debug("s_wbuf_add() End     block: %d size: %d index: %d\n", wbuf->end_pos.block->idx, wbuf->end_pos.block->size, wbuf->end_pos.idx);

	//
	// Set the current wchar_t to the parameter location.
	//
	*wchr = s_wbuf_pos_get_wchr(cur_pos);

	//
	// Return true to indicate, that we are not at the end of the buffer.
	//
	return true;
}

/******************************************************************************
 * The function adds the content of a file to a s_wbuf.
 *****************************************************************************/

void s_wbuf_copy_file(FILE *file, s_wbuf *wbuf) {
	wchar_t wchr;

	while (true) {
		wchr = read_wchar(file);

		if (feof(file)) {
			return;
		}

		s_wbuf_add(wbuf, wchr);
	}
}

/******************************************************************************
 * The function adds a string to a s_wbuf. This is used for unit tests.
 *****************************************************************************/

void s_wbuf_add_str(s_wbuf *wbuf, wchar_t *str) {

	for (wchar_t *ptr = str; *ptr != L'\0'; ptr++) {
		s_wbuf_add(wbuf, *ptr);
	}
}


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

#ifndef INC_NCV_WBUF_H_
#define INC_NCV_WBUF_H_

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/******************************************************************************
 * The struct represents a block of wchar_t memory. The blocks are connected as
 * a linked list. Each block has an index for debugging and a size. Each time a
 * new block is allocated the size is doubled.
 *****************************************************************************/

typedef struct s_wblock {

	//
	// The index of the block in the linked list.
	//
	int idx;

	//
	// The size of the block, which will be doubled for each newly allocated
	// block.
	//
	int size;

	//
	// The wchar_t buffer with the given size.
	//
	wchar_t *buf;

	//
	// The next block of null if this is the last block.
	//
	struct s_wblock *next;

} s_wblock;

/******************************************************************************
 * The struct represents a position in the s_wbuf buffer. This is the current
 * block and the index in the buffer of the block.
 *****************************************************************************/

typedef struct s_wbuf_pos {

	//
	// The current block
	//
	s_wblock *block;

	//
	// The index of the wchar_t buffer in the block.
	//
	int idx;

} s_wbuf_pos;

/******************************************************************************
 * The s_wbuf struct represents the buffer. It has a link to the linked list of
 * s_wblocks and an initial block size. Additionally it has a s_wbuf_pos
 * position, which is the position of the last wchar_t in the buffer.
 *
 * If the s_wbuf is created, it has no s_wblock and the s_wbuf_pos is invalid.
 * The first block is added as the first wchar_t is added to the buffer and
 * this wchar_t is the first end position.
 *****************************************************************************/

typedef struct s_wbuf {

	//
	// The initial size of the s_wblocks.
	//
	int block_size;

	//
	// The link to the linked list of s_wblocks
	//
	s_wblock *root;

	//
	// The position of the last wchar_t in the buffer.
	//
	s_wbuf_pos end_pos;

} s_wbuf;

/******************************************************************************
 * The s_wbuf_pos struct has a lot of macros, which are added for readability.
 *****************************************************************************/

//
// Initially the s_wbuf has no blocks, so the current block is NULL and the
// index negative.
//
#define s_wbuf_pos_init(p) (p)->block = NULL; (p)->idx = -1

//
// The macro check whether the position is initial or not.
//
#define s_wbuf_pos_is_set(p) ((p)->block != NULL)

//
// The macro sets the current position.
//
#define s_wbuf_pos_set(p, b, i) (p)->block = b; (p)->idx = i

//
// The macro check whether the current position is the last wchar_t in the
// current block buffer or not.
//
#define s_wbuf_pos_is_end_of_block(p) (p)->idx == (p)->block->size - 1

//
// The macro switches to the next s_wblock.
//
#define s_wbuf_pos_next_block(p) (p)->block = (p)->block->next; (p)->idx = 0

//
// The marco sets the wchar_t of the current position.
//
#define s_wbuf_pos_set_wchr(p ,w) (p)->block->buf[(p)->idx] = w

//
// The marco returns the wchar_t of the current position.
//
#define s_wbuf_pos_get_wchr(p) ((p)->block->buf[(p)->idx])

//
// The macro checks if two positions are equal.
//
#define s_wbuf_pos_equal(p1, p2) ((p1)->block == (p2)->block && (p1)->idx == (p2)->idx)

/******************************************************************************
 * The functions for the s_wbuf struct.
 *****************************************************************************/

#define WBUF_BLOCK_SIZE 4096

s_wbuf *s_wbuf_create(const int size);

void s_wbuf_copy_file(FILE *file, s_wbuf *wbuf);

void s_wbuf_add(s_wbuf *wbuf, const wchar_t wchar);

bool s_wbuf_next(s_wbuf *wbuf, s_wbuf_pos *cur_pos, wchar_t *wchr);

void s_wbuf_free(s_wbuf *wbuf);

void s_wbuf_add_str(s_wbuf *wbuf, wchar_t *str);

#endif /* INC_NCV_WBUF_H_ */

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
#ifndef INC_NCV_POPUP_H_
#define INC_NCV_POPUP_H_

#include <stdbool.h>

#include <ncursesw/form.h>
#include <ncursesw/menu.h>

/******************************************************************************
 * The struct for a popup. It contains of a form with fields and a menu, which
 * represents buttons.
 *****************************************************************************/

typedef struct s_popup {

	FORM *form;

	FIELD **fields;

	int num_fields;

	MENU *menu;

	ITEM **items;

	int num_items;

	bool is_on_form;

} s_popup;

/******************************************************************************
 * Function declarations.
 *****************************************************************************/

void popup_init(s_popup *popup, const int num_fields, const int num_items);

void popup_free(s_popup *popup);

void popup_prepair_show(s_popup *popup);

void popup_pos_cursor(s_popup *popup);

bool popup_process_input(s_popup *popup, const int key_type, const wint_t chr);

#endif

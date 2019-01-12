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

#ifndef INC_NCV_FORMS_H_
#define INC_NCV_FORMS_H_

#include <ncursesw/form.h>
#include <ncursesw/menu.h>

/******************************************************************************
 * The enumeration of the different field types.
 *****************************************************************************/

enum FIELD_TYPE {

	//
	// Input field.
	//
	FIELD_TYPE_INPUT,

	//
	// Checkbox.
	//
	FIELD_TYPE_CHECKBOX
};

void forms_driver(FORM *form, const int key_type, const wint_t chr);

void menus_driver(MENU *menu, const int chr);

bool forms_has_index(const FORM *form, const int idx);

bool menus_has_index(const MENU *menu, const int idx);

int menus_get_index(const MENU *menu);

FIELD *forms_create_field(const int rows, const int cols, const int start_row, const int start_col, const chtype attr);

void menus_create_items(ITEM **items, const int num, const char **labels);

FORM *forms_create_form(FIELD **fields);

MENU *menus_create_menu(ITEM **items, const int num_items, const chtype attr);

void forms_set_win_and_post(FORM *form, WINDOW *win, WINDOW *win_sub);

void menus_set_win_and_post(MENU *menu, WINDOW *win, WINDOW *win_sub);

void forms_free(FORM *form);

void menus_free(MENU *menu);

void forms_get_input_str(FIELD *field, wchar_t *buffer, const int buffer_size);

bool forms_checkbox_is_checked(FIELD *field);

bool forms_get_checkbox_value(FIELD *field, bool *checked);

void forms_process_checkbox(FORM *form, FIELD *field, const int key_type, const wint_t chr);

void forms_process_input_field(FORM *form, FIELD *field, const int key_type, const wint_t chr);

int menus_get_size(const MENU *menu);

#endif

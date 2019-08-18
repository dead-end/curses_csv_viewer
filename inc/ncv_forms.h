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

#include <form.h>
#include <menu.h>

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

int forms_get_index(const FORM *form);

int menus_get_index(const MENU *menu);

#define forms_is_first(f) (forms_get_index(f) == 0)

bool forms_is_last(const FORM *form);

#define menus_is_first(m) (menus_get_index(m) == 0)

bool menus_is_last(const MENU *menu);

FIELD** forms_create_fields(const int num_fields);

FIELD* forms_create_field(const int rows, const int cols, const int start_row, const int start_col, const chtype attr);

FORM* forms_create_form(FIELD **fields);

MENU* menus_create_menu(const char **labels);

void menus_format_menu(MENU *menu, const chtype attr, const bool horizontal);

void forms_set_wins(FORM *form, WINDOW *win, WINDOW *win_form);

void menus_set_wins(MENU *menu, WINDOW *win, WINDOW *win_menu);

void forms_user_ptr_free(const FORM *form);

void forms_free(FORM *form);

void menus_free(MENU *menu);

void forms_get_input_str(FIELD *field, wchar_t *buffer, const int buffer_size);

bool forms_checkbox_is_checked(const FIELD *field);

bool forms_get_checkbox_value(const FIELD *field, bool *checked);

void forms_process_checkbox(FORM *form, const FIELD *field, const int key_type, const wint_t chr);

void forms_process_input_field(FORM *form, const FIELD *field, const int key_type, const wint_t chr);

void menus_switch_on_off(MENU *menu, const bool on);

void menus_unpost_post(MENU *menu, const bool unpost);

#endif

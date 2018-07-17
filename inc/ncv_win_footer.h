/*
 * ncv_win_footer.h
 *
 */

#ifndef INC_NCV_WIN_FOOTER_H_
#define INC_NCV_WIN_FOOTER_H_

//
// exported functions
//
void win_footer_init();

void win_footer_resize();

void win_footer_refresh_no();

void win_footer_free();

void win_footer_content_print(const char* filename, const s_table *table, const s_field *cursor);

#endif /* INC_NCV_FOOTER_H_ */

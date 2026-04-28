#ifndef CURSOR_H
#define CURSOR_H

#include <da_string.h>
#include <stddef.h>


typedef struct {
  size_t line;
  size_t col;
  size_t id; // index in the raw source file string
  string_view source;
} Cursor;


void dump_cursor(Cursor* c);
// get next character
char peek_next(Cursor *c);

// match current character with expected one
// and advance if match is found,
// else just return false without advancing
bool ch_match_cursor(Cursor *c, char expected);
bool str_match_cursor(Cursor *c, string_view expected);

string_view get_current_line(Cursor* cursor);

/*
* Get the full line from the current cursor position till the newline
* Will return till EOF if \n isnt found.
* Basically get_till_delim(cursor, '\n');
*/
string_view get_till_newline_or_eof(Cursor* cursor);
// Get current character from cursor;
char peek(Cursor* cursor);
Cursor new_cursor(string_view source);

// returns current character and advances
char advance_cursor(Cursor* cursor);
void advance_cursor_by(Cursor* cursor, size_t n);

bool is_cursor_valid(Cursor* cursor);
void skip_whitespace(Cursor* cursor);
/*
* Get the string starting from the current position till the delimiter
* Returns string_view(NULL, 0) if delim not found
*/
string_view get_till_delim(Cursor* cursor, char delim);

#endif

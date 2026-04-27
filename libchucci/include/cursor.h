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


// get next character
char peek_next(Cursor *c);

// match current character with expected one
// and advance if match is found,
// else just return false without advancing
bool match_cursor(Cursor *c, char expected);

string_view get_full_line(Cursor* cursor);

/*
* Get the full line from the current cursor position till the newline
* Will return till EOF if \n isnt found.
*/
string_view get_line_from_pos(Cursor* cursor);
// Get current character from cursor;
char peek(Cursor* cursor);
Cursor new_cursor(string_view source);

// returns current character and advances
char advance_cursor(Cursor* cursor);

#endif

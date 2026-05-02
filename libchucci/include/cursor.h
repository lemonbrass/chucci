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

typedef struct {
  size_t line;
  size_t col;
  size_t id; // index in the raw source file string
} CursorMark;

#define is_alpha(ch) ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
#define is_num(ch) (ch >= '0' && ch <= '9')

void dump_cursor(Cursor* c);
// get next character
char peek_next(Cursor *c);

// match current character with expected one
// and advance if match is found,
// else just return false without advancing
bool ch_match_cursor(Cursor *cursor, char expected);
bool str_match_cursor(Cursor *cursor, string_view expected);

// *_match_cursor counterparts that assert(match)
#define ch_expect_cursor(cursor, expected) assert(ch_match_cursor(cursor, expected))
#define str_expect_cursor(cursor, expected) assert(str_match_cursor(cursor, expected))

string_view get_current_line(Cursor* cursor);

CursorMark mark_cursor(Cursor* cursor);
void rewind_cursor(Cursor* cursor, CursorMark* mark);
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

string_view slice_cursor(Cursor* cursor, size_t n);

bool is_cursor_valid(Cursor* cursor);
void skip_whitespace(Cursor* cursor);
void skip_whitespace_except_newline(Cursor* cursor);
/*
* Get the string starting from the current position till the delimiter
* Returns string_view(NULL, 0) if delim not found
*/
string_view get_till_delim(Cursor* cursor, char delim);

#endif

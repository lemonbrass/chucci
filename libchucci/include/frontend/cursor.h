#ifndef CURSOR_H
#define CURSOR_H

#include <utils/string.h>
#include <stddef.h>
#include <stdbool.h>
#include "utils/file.h"

typedef struct {
  size_t line;
  size_t col;
  size_t id; // index in the raw source file string
  File source;
} Cursor;

typedef struct {
  size_t line;
  size_t col;
  size_t id;
} CursorMark;

#define is_alpha(ch) ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
#define is_num(ch) (ch >= '0' && ch <= '9')

void cursor_dump(Cursor* c);
// get next character
char cursor_peek_next(Cursor *c);

// match current character with expected one
// and advance if match is found,
// else just return false without advancing
bool cursor_match_ch(Cursor *cursor, char expected);
bool cursor_match_str(Cursor *cursor, StringView expected);

// *_match_cursor counterparts that assert(match)
#define cursor_expect_ch(cursor, expected) assert(ch_match_cursor(cursor, expected))
#define cursor_expect_str(cursor, expected) assert(str_match_cursor(cursor, expected))

StringView cursor_curr_line(Cursor* cursor);

CursorMark cursor_mark(Cursor* cursor);
void cursor_rewind(Cursor* cursor, CursorMark* mark);
/*
* Get the full line from the current cursor position till the newline
* Will return till EOF if \n isnt found.
* Basically get_till_delim(cursor, '\n');
*/
StringView cursor_get_till_next_line(Cursor* cursor);
// Get current character from cursor;
char cursor_peek(Cursor* cursor);
Cursor cursor_new(const File source);

// returns current character and advances
char cursor_advance(Cursor* cursor);
void cursor_advance_by(Cursor* cursor, size_t n);

StringView cursor_slice(Cursor* cursor, size_t start, size_t end);

bool is_cursor_valid(Cursor* cursor);
void skip_whitespace(Cursor* cursor);
void skip_whitespace_except_newline(Cursor* cursor);
/*
* Get the string starting from the current position till the delimiter
* Returns string_view(NULL, 0) if delim not found
*/
StringView get_till_delim(Cursor* cursor, char delim);

#endif

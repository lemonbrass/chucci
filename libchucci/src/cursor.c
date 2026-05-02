#include "da_string.h"
#include <assert.h>
#include <ctype.h>
#include <cursor.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>


/*
*  "avrjrjzieuhr \n hdjdur \n hdbd"
*                       ^ cursor here
*                   ______ => resultant string_view 
*/
string_view get_current_line(Cursor* cursor) {
  size_t start = cursor->id;
  while (start > 0 && cursor->source.cstr[start - 1] != '\n') {
    start--;
  }
  size_t end = cursor->id;
  while (end < cursor->source.len && cursor->source.cstr[end] != '\n') {
    end++;
  }
  return sv_slice(cursor->source, start, end-start);
}

void advance_cursor_by(Cursor* cursor, size_t n) {
  while (n-- > 0) advance_cursor(cursor);
}

void skip_whitespace(Cursor* cursor) {
  while (isspace((unsigned char)peek(cursor))) {
    advance_cursor(cursor);
  }
}

void skip_whitespace_except_newline(Cursor* cursor) {
  while (isspace((unsigned char)peek(cursor)) && peek(cursor) != '\n') {
    advance_cursor(cursor);
  }
}

string_view get_till_delim(Cursor* cursor, char delim) {
  size_t start = cursor->id;
  size_t end = cursor->id;
  while (end < cursor->source.len && cursor->source.cstr[end] != delim) {
    end++;
  }
  return sv_slice(cursor->source, start, end-start);
}

string_view get_till_newline_or_eof(Cursor* cursor) {
  string_view sv = get_till_delim(cursor, '\n');
  if (sv.len == 0 && sv.cstr == NULL) {
    return sv_slice(cursor->source, cursor->id, cursor->source.len-cursor->id);
  }
  return sv;
}

char peek(Cursor* cursor) {
  assert(is_cursor_valid(cursor));
  return cursor->source.cstr[cursor->id];
}

Cursor new_cursor(string_view source) {
  Cursor c = {0};
  c.col = 1;
  c.id = 0;
  c.line = 1;
  c.source = source;
  return c;
}

// advance the cursor and return the current character
char advance_cursor(Cursor* cursor) {
  char ch = peek(cursor);
  cursor->id++;
  cursor->col++;
  if (ch == '\n') {
    cursor->line++;
    cursor->col = 1;
  }
  return ch;
}

char peek_next(Cursor *cursor) {
  assert(cursor->source.len > cursor->id + 1);
  return cursor->source.cstr[cursor->id+1];
}

CursorMark mark_cursor(Cursor* cursor) {
  return (CursorMark){ .col=cursor->col, .id=cursor->id, .line=cursor->line };
}

void rewind_cursor(Cursor* cursor, CursorMark* mark) {
  cursor->col = mark->col;
  cursor->line = mark->line;
  cursor->id = mark->id;
}

bool str_match_cursor(Cursor *cursor, string_view expected) {
  if (expected.len > cursor->source.len - cursor->id)
    return false;
  Cursor mark = *cursor;
  for (size_t i=0; i<expected.len; i++) {
    char ch = advance_cursor(&mark);
    if (expected.cstr[i] != ch) return false;
  }
  *cursor = mark;
  return true;
}
bool ch_match_cursor(Cursor *cursor, char expected) {
  if (peek(cursor) != expected) return false;
  advance_cursor(cursor);
  return true;
}

bool is_cursor_valid(Cursor* cursor) {
  return cursor->source.len >= cursor->id;
}

void dump_cursor(Cursor* c) {
  string_view currline = get_current_line(c);
  int prefix = printf("Cursor at (line = %zu, col = %zu): ", c->line, c->col);
  printf("%.*s\n", (int)currline.len, currline.cstr);
  for (size_t i=0; i < (c->col-1+prefix); i++) printf(" ");
  printf("^\n");
}

string_view slice_cursor(Cursor* cursor, size_t n) {
  return sv_slice(cursor->source, cursor->id, n);
}

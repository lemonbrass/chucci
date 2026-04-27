#include "da_string.h"
#include <assert.h>
#include <cursor.h>
#include <stddef.h>
#include <stdio.h>


/*
*  "avrjrjzieuhr \n hdjdur \n hdbd"
*                       ^ cursor here
*                   ______ => resultant string_view 
*/
string_view get_full_line(Cursor* cursor) {
  size_t start = cursor->id;
  while (start > 0 && cursor->source.str[start-1] != '\n') {
    start--;
  }
  size_t end = cursor->id;
  while (end < cursor->source.len && cursor->source.str[end] != '\n') {
    end++;
  }
  return sv_slice(cursor->source, start, end-start);
}

string_view get_line_from_pos(Cursor* cursor) {
  size_t start = cursor->id;
  size_t end = cursor->id;
  while (end < cursor->source.len && cursor->source.str[end] != '\n') {
    end++;
  }
  return sv_slice(cursor->source, start, end-start);
}

char peek(Cursor* cursor) {
  assert(cursor->source.len > cursor->id);
  return cursor->source.str[cursor->id];
}

Cursor new_cursor(string_view source) {
  Cursor c = {0};
  c.col = 1;
  c.id = 0;
  c.line = 1;
  c.source = source;
  return c;
}

// advance the cursor and return the previous character
char advance_cursor(Cursor* cursor) {
  char ch = peek(cursor);
  cursor->id++;
  cursor->col = 1;
  if (peek(cursor) == '\n') {
    cursor->line++;
    cursor->col = 1;
  }
  return ch;
}

char peek_next(Cursor *cursor) {
  assert(cursor->source.len > cursor->id + 1);
  return cursor->source.str[cursor->id+1];
}

bool match_cursor(Cursor *cursor, char expected) {
  if (peek(cursor) != expected) return false;
  advance_cursor(cursor);
  return true;
}

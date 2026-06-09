#include <ctype.h>
#include <frontend/cursor.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <utils/string.h>

/*
 *  "avrjrjzieuhr \n hdjdur \n hdbd"
 *                       ^ cursor here
 *                   ______ => resultant StringView
 */
StringView cursor_curr_line(Cursor *cursor) {
  size_t start = cursor->id;
  while (start > 0 && cursor->source.contents.cstr[start - 1] != '\n') {
    start--;
  }
  size_t end = cursor->id;
  while (end < cursor->source.contents.len &&
         cursor->source.contents.cstr[end] != '\n') {
    end++;
  }
  return anystr_slice(cursor->source.contents, start, end);
}

void cursor_advance_by(Cursor *cursor, size_t n) {
  while (n-- > 0)
    cursor_advance(cursor);
}

void skip_whitespace(Cursor *cursor) {
  while (isspace((unsigned char)cursor_peek(cursor))) {
    cursor_advance(cursor);
  }
}

void skip_whitespace_except_newline(Cursor *cursor) {
  while (isspace((unsigned char)cursor_peek(cursor)) &&
         cursor_peek(cursor) != '\n') {
    cursor_advance(cursor);
  }
}

StringView get_till_delim(Cursor *cursor, char delim) {
  size_t start = cursor->id;
  size_t end = cursor->id;
  while (end < cursor->source.contents.len &&
         cursor->source.contents.cstr[end] != delim) {
    end++;
  }
  return anystr_slice(cursor->source.contents, start, end);
}

StringView cursor_get_till_next_line(Cursor *cursor) {
  StringView sv = get_till_delim(cursor, '\n');
  if (sv.len == 0 && sv.cstr == NULL) {
    return anystr_slice(cursor->source.contents, cursor->id,
                        cursor->source.contents.len);
  }
  return sv;
}

// Peek the current character
char cursor_peek(Cursor *cursor) {
  if (!is_cursor_valid(cursor))
    return '\0';
  return cursor->source.contents.cstr[cursor->id];
}

Cursor cursor_new(const File source) {
  Cursor c = {0};
  c.col = 1;
  c.id = 0;
  c.line = 1;
  c.source = source;
  return c;
}

// advance the cursor and return the previous character
char cursor_advance(Cursor *cursor) {
  char ch = cursor_peek(cursor);
  cursor->id++;
  cursor->col++;
  if (ch == '\n') {
    cursor->line++;
    cursor->col = 1;
  }
  return ch;
}

char cursor_peek_next(Cursor *cursor) {
  if (cursor->source.contents.len < cursor->id + 1)
    return '\0';
  return cursor->source.contents.cstr[cursor->id + 1];
}

CursorMark cursor_mark(Cursor *cursor) {
  return (CursorMark){
      .col = cursor->col, .id = cursor->id, .line = cursor->line};
}

void cursor_rewind(Cursor *cursor, CursorMark *mark) {
  cursor->col = mark->col;
  cursor->line = mark->line;
  cursor->id = mark->id;
}

bool cursor_match_str(Cursor *cursor, StringView expected) {
  if (expected.len > cursor->source.contents.len - cursor->id)
    return false;
  Cursor mark = *cursor;
  for (size_t i = 0; i < expected.len; i++) {
    char ch = cursor_advance(&mark);
    if (expected.cstr[i] != ch)
      return false;
  }
  *cursor = mark;
  return true;
}
bool cursor_match_ch(Cursor *cursor, char expected) {
  if (cursor_peek(cursor) != expected)
    return false;
  cursor_advance(cursor);
  return true;
}

bool is_cursor_valid(Cursor *cursor) {
  return cursor->source.contents.len > cursor->id;
}

void cursor_dump(Cursor *c) {
  StringView currline = cursor_curr_line(c);
  int prefix = printf("Cursor at (line = %zu, col = %zu): ", c->line, c->col);
  printf("%.*s\n", (int)currline.len, currline.cstr);
  for (size_t i = 0; i < (c->col - 1 + prefix); i++)
    printf(" ");
  printf("^\n");
}

StringView cursor_slice(Cursor *cursor, size_t start, size_t end) {
  return anystr_slice(cursor->source.contents, start, end);
}

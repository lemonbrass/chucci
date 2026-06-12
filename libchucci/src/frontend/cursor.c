#include <assert.h>
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
  idx_t start = cursor->id;
  while (start > 0 && cursor->source->contents.cstr[start - 1] != '\n') {
    start--;
  }
  idx_t end = cursor->id;
  while (end < cursor->source->contents.len &&
         cursor->source->contents.cstr[end] != '\n') {
    end++;
  }
  return anystr_slice(cursor->source->contents, start, end);
}

void cursor_advance_by(Cursor *cursor, idx_t n) {
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
  idx_t start = cursor->id;
  idx_t end = cursor->id;
  while (end < cursor->source->contents.len &&
         cursor->source->contents.cstr[end] != delim) {
    end++;
  }
  return anystr_slice(cursor->source->contents, start, end);
}

StringView cursor_get_till_next_line(Cursor *cursor) {
  StringView sv = get_till_delim(cursor, '\n');
  if (sv.len == 0 && sv.cstr == NULL) {
    return anystr_slice(cursor->source->contents, cursor->id,
                        cursor->source->contents.len);
  }
  return sv;
}

// Peek the current character
char cursor_peek(Cursor *cursor) {
  if (!is_cursor_valid(cursor))
    return '\0';
  return cursor->source->contents.cstr[cursor->id];
}

Cursor cursor_new(File *source) {
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
  if (cursor->source->contents.len < cursor->id + 1)
    return '\0';
  return cursor->source->contents.cstr[cursor->id + 1];
}

CursorMark cursor_mark(Cursor *cursor) {
  return (CursorMark){
      .col = cursor->col, .id = cursor->id, .line = cursor->line};
}

void cursor_rewind(Cursor *cursor, CursorMark mark) {
  cursor->col = mark.col;
  cursor->line = mark.line;
  cursor->id = mark.id;
}

bool cursor_match_str(Cursor *cursor, StringView expected) {
  if (expected.len > cursor->source->contents.len - cursor->id)
    return false;
  Cursor mark = *cursor;
  for (idx_t i = 0; i < expected.len; i++) {
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
  return cursor->source->contents.len > cursor->id;
}

void cursor_dump(Cursor *c) {
  StringView currline = cursor_curr_line(c);
  int prefix = printf("Cursor at (line = %d, col = %d): ", c->line, c->col);
  printf("%.*s\n", (int)currline.len, currline.cstr);
  for (idx_t i = 0; i < (c->col - 1 + prefix); i++)
    printf(" ");
  printf("^\n");
}

StringView cursor_slice(Cursor *cursor, idx_t start, idx_t end) {
  return anystr_slice(cursor->source->contents, start, end);
}

Span span_from_mark(Cursor *cursor, CursorMark start, CursorMark end) {
  assert(start.id < end.id);
  idx_t len = end.id - start.id;
  Span span = {0};
  span.start = start.id;
  span.len = len;
  span.source = cursor->source;
  return span;
}

Span span_from_cursor(Cursor *cursor, idx_t len) {
  Span span = {0};
  span.start = cursor->id;
  span.len = len;
  span.source = cursor->source;
  return span;
}

StringView span_to_sv(Span span) {
  char *cstr = &span.source->contents.cstr[span.start];
  return sv_new(cstr, span.len);
}

CursorMark span_start(Span span) {
  Cursor cursor = cursor_new(span.source);
  while (cursor.id < span.start)
    cursor_advance(&cursor);
  return cursor_mark(&cursor);
}

Cursor span_to_cursor(Span span) {
  Cursor cursor = cursor_new(span.source);
  while (cursor.id < span.start)
    cursor_advance(&cursor);
  return cursor;
}

StringView span_curr_line(Span span) {
  Cursor cursor = cursor_new(span.source);
  cursor.id = span.start;
  return cursor_curr_line(&cursor);
}

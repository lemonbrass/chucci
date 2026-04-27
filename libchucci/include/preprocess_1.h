/*

THIS IS STAGE 1 OF PREPROCESSOR (STRING BASED PREPROCESSING)
THIS WILL ONLY RESOLVE #include AND comments

*/
#ifndef PREPROCESSOR1_H
#define PREPROCESSOR1_H


#include <da_arena.h>
#include <da_string.h>
#include <cursor.h>

typedef enum {
  PP1_INVALID_INCLUDE,
} PP1_ERROR;

typedef struct {
  Cursor cursor;
  da_string builder;
  PP1_ERROR err;
  arena_t* arena;
} Preprocessor1;

Preprocessor1 new_pp1(string_view source, arena_t* arena);
string_view resolve_pp1(Preprocessor1* pp1);

#endif

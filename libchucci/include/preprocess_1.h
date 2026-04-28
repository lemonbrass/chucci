/*

THIS IS STAGE 1 OF PREPROCESSOR (STRING BASED PREPROCESSING)
THIS WILL ONLY RESOLVE #include AND comments

*/
#ifndef PREPROCESSOR1_H
#define PREPROCESSOR1_H


#include <thirdparty/kvec.h>
#include <da_arena.h>
#include <da_string.h>
#include <cursor.h>

typedef enum {
  PP1_OKAY,
  PP1_INVALID_INCLUDE,
} PP1Error;

typedef struct {
  Cursor cursor;
  da_string builder;
  kvec_t(string_view) include_dirs;
  PP1Error err;
} Preprocessor1;

Preprocessor1 new_pp1(string_view source);
string_view resolve_pp1(Preprocessor1* pp1);

#endif

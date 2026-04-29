/*

THIS IS STAGE 1 OF PREPROCESSOR (STRING BASED PREPROCESSING)
THIS WILL ONLY RESOLVE #include AND comments

*/
#ifndef PREPROCESSOR1_H
#define PREPROCESSOR1_H


#include <ctx.h>
#include <da_path.h>
#include <thirdparty/kvec.h>
#include <da_arena.h>
#include <da_string.h>
#include <cursor.h>

typedef struct {
  Cursor cursor;
  da_string builder;
  CompilerCtx* ctx;
} Preprocessor1;

Preprocessor1 new_pp1(string_view source, CompilerCtx* ctx);
string_view resolve_pp1(Preprocessor1* pp1);

#endif

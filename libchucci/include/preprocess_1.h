/*

THIS IS STAGE 1 OF PREPROCESSOR (STRING BASED PREPROCESSING)
THIS WILL ONLY RESOLVE #include AND comments

*/
#ifndef PREPROCESSOR1_H
#define PREPROCESSOR1_H


#include <compiler.h>
#include <da_path.h>
#include <thirdparty/kvec.h>
#include <da_arena.h>
#include <da_string.h>
#include <cursor.h>

typedef struct {
  Cursor cursor;
  ChucciCompiler* ctx;
} Preprocessor1;

Preprocessor1 new_pp1(ChucciCompiler* ctx);
string resolve_pp1(Preprocessor1* pp1);

#endif

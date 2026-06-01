/*

THIS IS STAGE 1 OF PREPROCESSOR (STRING BASED PREPROCESSING)
THIS WILL ONLY RESOLVE #include AND comments

*/
#ifndef PREPROCESSOR1_H
#define PREPROCESSOR1_H


#include <compiler.h>
#include <thirdparty/kvec.h>
#include <utils/da_path.h>
#include <utils/da_arena.h>
#include <utils/da_string.h>
#include <frontend/cursor.h>

typedef struct {
  Cursor cursor;
  PPCtx* ctx;
} Preprocessor1;

Preprocessor1 new_pp1(PPCtx* ctx);
string resolve_pp1(Preprocessor1* pp1);

#endif

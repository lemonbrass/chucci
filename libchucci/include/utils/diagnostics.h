/*
  diagnostics.h: The Diagnostics Engine for Chucci Compiler
  It has these components:
  Diagnostic: This is added by the compiler on some "faulty" code, and is later
  emitted.
  DiagnosticID  : A unique ID given to each diagnostic.
  SeverityLevel : How severe is the diagnostic (self-explanatory).
  DiagPhase     : The specific compiler phase where the "faulty" code was found.
  DiagnosticArg : A tagged union containing any information to be emitted along
  with diagnostic.
  I have implemented a new formatter for printing these DiagnosticArgs,
  the Diagnostic struct has a field const char * msg, and it can have
  format specifiers:

  %[n]     : to print the nth DiagnosticArg in the DiagnosticArg
  array inside the Diagnostic struct (n <= MAX_DIAGNOSTIC_ARGS)

  %[phase]     : to print phase
  %[level]     : severity level
  %[file]      : file containing error
  %[cfile]     : compiler source-code file which initiated the diagnostic
  %[cline]     : line of code where diagnostic was initiated
  %[msg]       : the diagnostic msg (set through the X-Macro)
  %[span]      : the actual span containing "faulty" code
  %[span-line] : full line containing the span.
  %[row]       : row-id in input source code (the line)
  %[col]       : col-id in input source code
  %[span-start-diff* ] : prints (span id - line start id) spaces
  %[span-width*^]      : printf span width amount of ^ (you can use any character instead of ^)
  
  Currently we use the X-Macro DIAGNOSTICS(X), for a easy way to add new
  diagnostics.
*/

#ifndef __DIAGNOSTICS_H
#define __DIAGNOSTICS_H

#include "frontend/cursor.h"
#include "utils/string.h"
#include "utils/vec.h"
#include "utils/vmem_arena.h"
#include <stdint.h>

#define MAX_DIAGNOSTICS 8
#define MAX_DIAGNOSTIC_ARGS 4

#define DEFAULT_FORMATTING "%[file] (%[row]:%[col]): %[level]: %[msg]\n"\
                           "Initiated by %[cfile]:%[cline]\n"\
                           "%[span-line]\n"\
                           "%[span-start-diff* ]%[span-width*^]\n"

// X(ERROR, message, default severity, phase)
#define DIAGNOSTICS(X)                                                         \
  X(ERR_INVALID_NUMERIC_LITERAL, "Invalid numeric literal", SL_ERROR,          \
    PHASE_LEXER)                                                               \
  X(ERR_UNTERMINATED_STRING, "Unterminated string literal", SL_FATAL,          \
    PHASE_LEXER)                                                               \
  X(ERR_UNTERMINATED_MULTILINE_COMMENT, "Unterminated multiline comment",      \
    SL_ERROR, PHASE_LEXER)                                                     \
  X(ERR_UNEXPECTED_TOKEN_AFTER_OP_PREPROCESS, "Unexpected token after #",      \
    SL_ERROR, PHASE_PREPROCESSOR)                                              \
  X(ERR_UNEXPECTED_TOKEN, "Unexpected token", SL_ERROR, PHASE_ANY)         \
  X(NOTE_EXPECTED_TOKEN, "Expected", SL_NOTE, PHASE_ANY)

#define SEVERITY_LEVEL(X)                                                      \
  X(SL_IGNORED, "ignored")                                                     \
  X(SL_NOTE, "note")                                                           \
  X(SL_WARNING, "warning")                                                     \
  X(SL_ERROR, "error")                                                         \
  X(SL_FATAL, "fatal error")

typedef enum DiagnosticID {
#define X(a, msg, sl, ph) a,
  DIAGNOSTICS(X)
#undef X
} DiagnosticID;

typedef enum DiagPhase {
  PHASE_LEXER,
  PHASE_PREPROCESSOR,
  PHASE_PARSER,
  PHASE_CODEGEN,
  PHASE_ANY
} DiagPhase;

typedef enum SeverityLevel {
#define X(a, _) a,
  SEVERITY_LEVEL(X)
#undef X
} SeverityLevel;

typedef enum DiagArgKind {
  DA_INT,
  DA_STRVIEW,
} DiagArgKind;

typedef struct DiagnosticArg {
  DiagArgKind kind;
  union {
    int num;
    StringView sv;
  };
} DiagnosticArg;

typedef struct DiagnosticBase {
  DiagnosticID id;
  DiagPhase phase;
  SeverityLevel level;
  StringView msg;
} DiagnosticBase;

typedef struct Diagnostic Diagnostic;
VEC_DEF(Diagnostic, DiagVec, diagvec, VMEM_ARENA_ALLOC_INT);

struct Diagnostic {
  DiagnosticBase *base;
  const char *emit_format; // the format in which to emit the diagnostic
  const char *cfile;
  int cline;
  // Span containing error
  Span span;
  DiagVec children;
  DiagnosticArg args[MAX_DIAGNOSTIC_ARGS];
  uint8_t arg_len;
};

// Opaque to the user, just for the diagnostics builder
// Sort of a "pointer" to the diagnostic
typedef struct DIAGID {
  // The index in the DiagVec vector in DiagnosticEngine
  uint32_t indx : 16;
  // is this a sub-diagnostic or a parent
  uint32_t is_child : 1;
  // if it is a sub-diagnostic, the indx of the parent
  uint32_t parent_indx : 15;
} DIAGID;

typedef struct DiagnosticEngine {
  DiagVec diagnostics;
  VMEMArena *arena;
  bool fatal;
} DiagnosticEngine;

extern DiagnosticBase id_to_diagnostic[];

#define diagnostic_new(engine, span, id)                                       \
  _diagnostic_new((engine), (span), (id), __FILE__, __LINE__)
#define subdiagnostic_new(parent, engine, span, id)                            \
  _subdiagnostic_new((parent), (engine), (span), (id), __FILE__, __LINE__)

#define diagarg_num(_num) (DiagnosticArg){.kind = DA_INT, .num = (_num)}
#define diagarg_sv(_sv) (DiagnosticArg){.kind = DA_STRVIEW, .sv = (_sv)}

// By default sets the msg, phase, and severity level etc based on the X-MACRO
// DIAGNOSTICS(X)
DIAGID _diagnostic_new(DiagnosticEngine *engine, Span span, DiagnosticID id,
                     const char *cfile, int cline);
DIAGID _subdiagnostic_new(DIAGID parent, DiagnosticEngine *engine, Span span,
                        DiagnosticID id, const char *cfile, int cline);

// same functions for both diagnostics and subdiagnostics
#define subdiag_add_arg diag_add_arg
#define subdiag_set_format diag_set_format
#define subdiagnostic_emit diagnostic_emit
void diag_add_arg(DiagnosticEngine *engine, DIAGID diagid, DiagnosticArg arg);
void diag_set_format(DiagnosticEngine *engine, DIAGID diagid, const char *format);
void diagnostic_emit(Diagnostic *diag);
void diagnostics_emit(DiagnosticEngine *engine);
void print_formatted(Diagnostic *diag, const char *format_str);

DiagnosticEngine *diagnostic_engine_new(VMEMArena *arena);
void diagnostic_engine_free(DiagnosticEngine *engine);
bool has_fatal_diagnostics(DiagnosticEngine *engine);
bool has_diagnostics(DiagnosticEngine *engine);

#endif

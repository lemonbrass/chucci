#ifndef __DIAGNOSTICS_H
#define __DIAGNOSTICS_H

#include "frontend/cursor.h"
#include "utils/string.h"
#include "utils/vec.h"
#include "utils/vmem_arena.h"

#define MAX_DIAGNOSTICS 8

// X(ERROR, message, default severity, phase)
#define DIAGNOSTICS(X)                                                         \
  X(ERR_INVALID_NUMERIC_LITERAL, "Invalid numeric literal", SL_ERROR,          \
    PHASE_LEXER)                                                               \
  X(ERR_UNTERMINATED_STRING, "Unterminated string literal", SL_ERROR,          \
    PHASE_LEXER)

#define SEVERITY_LEVEL(X)                                                      \
  X(SL_IGNORED, "ignored")                                                     \
  X(SL_NOTE, "note")                                                           \
  X(SL_WARNING, "warning")                                                     \
  X(SL_ERROR, "error")

typedef enum DiagnosticID {
#define X(a, msg, sl, ph) a,
  DIAGNOSTICS(X)
#undef X
} DiagnosticID;

typedef enum DiagPhase { PHASE_LEXER } DiagPhase;

typedef enum SeverityLevel {
#define X(a, _) a,
  SEVERITY_LEVEL(X)
#undef X
} SeverityLevel;

typedef struct Diagnostic Diagnostic;
VEC_DEF(Diagnostic, DiagVec, diagvec);

typedef struct DiagnosticBase {
  DiagnosticID id;
  DiagPhase phase;
  SeverityLevel level;
  StringView msg;
} DiagnosticBase;

struct Diagnostic {
  DiagnosticBase base;
  const char *cfile;
  int cline;
  Cursor cursor;
  // start and end of the respective segment
  CursorMark start;
  CursorMark end;
  DiagVec children;
};

typedef struct DiagnosticEngine {
  DiagVec diagnostics;
  VMEMArena *arena;
  bool fatal;
} DiagnosticEngine;

extern DiagnosticBase id_to_diagnostic[];

#define diagnostic_new(id, cursor, start, end)                                 \
  _diagnostic_new(id, cursor, start, end, __FILE__, __LINE__)
#define diagnostic_new_override(id, cursor, span, msg, phase, level)           \
  _diagnostic_new(id, cursor, span, msg, phase, level, __FILE__, __LINE__)

Diagnostic _diagnostic_new(DiagnosticID id, Cursor cursor, CursorMark start,
                           CursorMark end, const char *cfile, int cline);
Diagnostic _diagnostic_new_override(DiagnosticID id, Cursor cursor,
                                    CursorMark start, CursorMark end,
                                    StringView msg, DiagPhase phase,
                                    SeverityLevel level, const char *cfile,
                                    int cline);

void subdiagnostic_add(DiagnosticEngine *engine, Diagnostic *diag,
                       Diagnostic child);
void diagnostic_add(DiagnosticEngine *engine, Diagnostic diag);
void diagnostic_emit(Diagnostic *diag);
void diagnostics_emit(DiagnosticEngine *engine);
DiagnosticEngine *diagnostic_engine_new(VMEMArena *arena);
void diagnostic_engine_free(DiagnosticEngine *engine);
bool has_fatal_diagnostics(DiagnosticEngine *engine);

#endif

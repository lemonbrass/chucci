#include "utils/diagnostics.h"
#include "frontend/cursor.h"
#include "utils/chucci_alloc.h"
#include "utils/string.h"
#include "utils/vec.h"
#include "utils/vmem_arena.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

VEC_IMPL(Diagnostic, DiagVec, diagvec, VMEM_ARENA_ALLOC_INT)

DiagnosticBase id_to_diagnostic[] = {
#define X(_id, _msg, _sl, _ph)                                                 \
  (DiagnosticBase){                                                            \
      .id = _id, .msg = const_cstr_to_sv(_msg), .level = _sl, .phase = _ph},
    DIAGNOSTICS(X)
#undef X
};

void diagnostic_engine_free(DiagnosticEngine *engine) {
  vec_foreach(Diagnostic, &engine->diagnostics, _idx, diagnostic,
              { diagvec_free(&diagnostic.children, engine->arena); });
  diagvec_free(&engine->diagnostics, engine->arena);
}

DiagnosticEngine *diagnostic_engine_new(VMEMArena *arena) {
  DiagnosticEngine *engine = vmarena_calloc(arena, sizeof(DiagnosticEngine));
  engine->arena = arena;
  engine->diagnostics = diagvec_new();
  return engine;
}

Diagnostic _diagnostic_new(DiagnosticID id, Cursor cursor, CursorMark start,
                           CursorMark end, const char *cfile, int cline) {
  Diagnostic diag = {0};
  diag.cfile = cfile;
  diag.cline = cline;
  diag.cursor = cursor;
  diag.start = start;
  diag.end = end;
  diag.base = id_to_diagnostic[id];
  return diag;
}

Diagnostic _diagnostic_new_override(DiagnosticID id, Cursor cursor,
                                    CursorMark start, CursorMark end,
                                    StringView msg, DiagPhase phase,
                                    SeverityLevel level, const char *cfile,
                                    int cline) {
  Diagnostic diag = {0};
  diag.cfile = cfile;
  diag.cline = cline;
  diag.cursor = cursor;
  diag.start = start;
  diag.end = end;
  diag.base.id = id;
  diag.base.level = level;
  diag.base.msg = msg;
  diag.base.phase = phase;
  return diag;
}

void subdiagnostic_add(DiagnosticEngine *engine, Diagnostic *diag,
                       Diagnostic child) {
  diagvec_push(&diag->children, child, engine->arena);
}

// TODO: Multi-line errors
void diagnostic_emit(Diagnostic *diag) {
  anystr_print(diag->cursor.source.name);
  printf("(%zu:%zu): ", diag->cursor.line, diag->cursor.col);
#define X(sl, name)                                                            \
  if (diag->base.level == sl)                                                  \
    printf(name);
  SEVERITY_LEVEL(X)
#undef X
  printf(": ");
  anystr_println(diag->base.msg);
  printf("Initiated by %s:%d\n", diag->cfile, diag->cline);
  StringView current_line = cursor_curr_line(&diag->cursor);
  anystr_println(current_line);
  for (size_t i = 0; i < diag->start.col - 1; i++)
    putchar(' ');
  for (size_t i = diag->start.id; i <= diag->end.id; i++)
    putchar('^');
  putchar('\n');
  vec_foreach(Diagnostic, &diag->children, _idx, diagnostic, {
    printf("-> ");
    diagnostic_emit(&diagnostic);
  });
}

void diagnostics_emit(DiagnosticEngine *engine) {
  if (engine->diagnostics.len == 0)
    return;
  vec_foreach(Diagnostic, &engine->diagnostics, _idx, diagnostic,
              { diagnostic_emit(&diagnostic); });
}

void diagnostic_add(DiagnosticEngine *engine, Diagnostic diag) {
  diagvec_push(&engine->diagnostics, diag, engine->arena);
  if (diag.base.level == SL_ERROR)
    engine->fatal = true;
}

bool has_fatal_diagnostics(DiagnosticEngine *engine) {
  return engine->fatal || engine->diagnostics.len > MAX_DIAGNOSTICS;
}

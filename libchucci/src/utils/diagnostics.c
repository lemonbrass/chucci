#include "utils/diagnostics.h"
#include "frontend/cursor.h"
#include "utils/chucci_alloc.h"
#include "utils/string.h"
#include "utils/vec.h"
#include "utils/vmem_arena.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VEC_IMPL(Diagnostic, DiagVec, diagvec, VMEM_ARENA_ALLOC_INT)

#define X(_id, _msg, _sl, _ph)                                                 \
  (DiagnosticBase){                                                            \
      .id = _id, .msg = const_cstr_to_sv(_msg), .level = _sl, .phase = _ph},
DiagnosticBase id_to_diagnostic[] = {DIAGNOSTICS(X)};
#undef X

#define X(level, str) str,
const char *slevel_to_cstr[] = {SEVERITY_LEVEL(X)};
#undef X

DiagnosticEngine *diagnostic_engine_new(VMEMArena *arena) {
  DiagnosticEngine *engine = vmarena_calloc(arena, sizeof(DiagnosticEngine));
  engine->arena = arena;
  engine->diagnostics = diagvec_new();
  return engine;
}

void diagnostic_engine_free(DiagnosticEngine *engine) {
  vec_foreach(Diagnostic, &engine->diagnostics, _idx, diag,
              { diagvec_free(&diag.children, engine->arena); });
  diagvec_free(&engine->diagnostics, engine->arena);
}

Diagnostic *diag_get(DIAGID diagid, DiagnosticEngine *engine) {
  if (diagid.is_child) {
    Diagnostic *parent =
        diagvec_access_ptr(&engine->diagnostics, diagid.parent_indx);
    return diagvec_access_ptr(&parent->children, diagid.indx);
  } else {
    return diagvec_access_ptr(&engine->diagnostics, diagid.indx);
  }
}

void shorten_cfile_path(const char **cfile) {
  static size_t offset = 0;
  if (offset != 0)
    *cfile += offset;
  else {
    while (
        !sv_startswith_cstr(cstr_to_sv((char *)*cfile + offset), "libchucci"))
      offset++;
    *cfile += offset;
  }
}

DIAGID _diagnostic_new(DiagnosticEngine *engine, Span span, DiagnosticID id,
                       const char *cfile, int cline) {
  Diagnostic diag = {0};
  shorten_cfile_path(&cfile);
  diag.cfile = cfile;
  diag.cline = cline;
  diag.base = &id_to_diagnostic[id];
  diag.span = span;
  diag.emit_format = DEFAULT_FORMATTING;
  diagvec_push(&engine->diagnostics, diag, engine->arena);
  DIAGID diagid = {0};
  diagid.is_child = false;
  diagid.indx = engine->diagnostics.len - 1;
  return diagid;
}
DIAGID _subdiagnostic_new(DIAGID parent_diagid, DiagnosticEngine *engine,
                          Span span, DiagnosticID id, const char *cfile,
                          int cline) {
  Diagnostic diag = {0};
  diag.cfile = cfile;
  diag.cline = cline;
  diag.base = &id_to_diagnostic[id];
  diag.span = span;
  diag.emit_format = DEFAULT_FORMATTING;
  Diagnostic *parent = diag_get(parent_diagid, engine);
  diagvec_push(&parent->children, diag, engine->arena);
  DIAGID diagid = {0};
  diagid.is_child = true;
  diagid.indx = parent->children.len - 1;
  diagid.parent_indx = parent_diagid.indx;
  return diagid;
}

void diag_add_arg(DiagnosticEngine *engine, DIAGID diagid, DiagnosticArg arg) {
  Diagnostic *diag = diag_get(diagid, engine);
  assert(diag->arg_len < MAX_DIAGNOSTIC_ARGS);
  diag->args[diag->arg_len++] = arg;
}

void diag_set_format(DiagnosticEngine *engine, DIAGID diagid,
                     const char *format) {
  Diagnostic *diag = diag_get(diagid, engine);
  diag->emit_format = format;
}

// handle %[numerical-type*^] etc...
void print_handle_numerical(Diagnostic *diag, StringView format_code,
                            size_t num) {
  if (format_code.len > 2 && format_code.cstr[format_code.len - 2] == '*') {
    while (num--)
      putchar((unsigned char)format_code.cstr[format_code.len - 1]);
  } else {
    printf("%zu", num);
  }
}

void print_arg(DiagnosticArg *arg) {
  switch (arg->kind) {
  case DA_INT:
    printf("%d", arg->num);
    break;
  case DA_STRVIEW:
    anystr_print(arg->sv);
    break;
  default:
    assert(false);
  }
}

void print_format_code(Diagnostic *diag, StringView format_code) {
  // Numerical type
  static CursorMark start;
  if (sv_startswith_cstr(format_code, "row")) {
    if (start.id != diag->span.start)
      start = span_start(diag->span);
    print_handle_numerical(diag, format_code, start.line);
    return;
  }
  if (sv_startswith_cstr(format_code, "col")) {
    if (start.id != diag->span.start)
      start = span_start(diag->span);
    print_handle_numerical(diag, format_code, start.col);
    return;
  }
  if (sv_startswith_cstr(format_code, "span-start-diff")) {
    if (start.id != diag->span.start)
      start = span_start(diag->span);
    print_handle_numerical(diag, format_code, start.col - 1);
    return;
  }
  if (sv_startswith_cstr(format_code, "span-width")) {
    print_handle_numerical(diag, format_code, diag->span.len);
    return;
  }

  // String type
  if (anystr_eq(format_code, const_cstr_to_sv("level"))) {
    printf("%s", slevel_to_cstr[diag->base->level]);
    return;
  }
  if (anystr_eq(format_code, const_cstr_to_sv("msg"))) {
    print_formatted(diag, diag->base->msg.cstr);
    return;
  }
  if (anystr_eq(format_code, const_cstr_to_sv("span-line"))) {
    StringView curr_line = span_curr_line(diag->span);
    anystr_print(curr_line);
    return;
  }
  if (anystr_eq(format_code, const_cstr_to_sv("file"))) {
    anystr_print(diag->span.source->name);
    return;
  }
  if (anystr_eq(format_code, const_cstr_to_sv("cfile"))) {
    printf("%s", diag->cfile);
    return;
  }
  if (anystr_eq(format_code, const_cstr_to_sv("cline"))) {
    printf("%d", diag->cline);
    return;
  }
  if (format_code.len == 1 && isdigit(format_code.cstr[0])) {
    uint8_t num = format_code.cstr[0] - '0';
    print_arg(&diag->args[num]);
    return;
  }

  printf("Error: Invalid formatting in diagnostic: ");
  anystr_println(format_code);
  assert(false);
}

void print_formatted(Diagnostic *diag, const char *format_str) {
  StringView emit_format = cstr_to_sv((char *)format_str);
  size_t i = 0;
  while (i < emit_format.len) {
    if (i + 1 < emit_format.len && emit_format.cstr[i] == '\%' &&
        emit_format.cstr[i + 1] == '[') {
      i += 2;
      StringView format_code = sv_slice_till_delim(emit_format, i, ']');
      print_format_code(diag, format_code);
      i += format_code.len;
    } else {
      putchar((unsigned char)emit_format.cstr[i]);
    }
    i++;
  }
}

// This contains all that %[formatter] stuff
void diagnostic_emit(Diagnostic *diag) {
  print_formatted(diag, diag->emit_format);
  vec_foreach(Diagnostic, &diag->children, _idx, diagnostic, {
    printf("-> ");
    diagnostic_emit(&diagnostic);
  });
}

void diagnostics_emit(DiagnosticEngine *engine) {
  vec_foreach(Diagnostic, &engine->diagnostics, _idx, diagnostic,
              { diagnostic_emit(&diagnostic); });
}

bool has_fatal_diagnostics(DiagnosticEngine *engine) {
  return engine->fatal || engine->diagnostics.len > MAX_DIAGNOSTICS;
}

bool has_diagnostics(DiagnosticEngine *engine) {
  return engine->diagnostics.len > 0;
}

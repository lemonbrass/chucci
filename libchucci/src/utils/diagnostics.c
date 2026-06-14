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

void handle_numerical_operators(Diagnostic *diag, StringView format_code,
                                size_t format_code_base_len, size_t *num) {
  StringView op_sv;
  op_sv.cstr = format_code.cstr + format_code_base_len;
  op_sv.len = format_code.len - format_code_base_len;
  while (op_sv.len != 0) {
    if (op_sv.cstr[0] == '*' && op_sv.len >= 2) {
      size_t i = *num;
      while (i--)
        putchar(op_sv.cstr[1]);
      op_sv.len -= 2;
      op_sv.cstr += 2;
    }
    // post-decrement
    if (op_sv.cstr[0] == '-' && op_sv.len >= 2 && op_sv.cstr[1] == '-') {
      if (*num >= 1)
        (*num)--;
      else {
        printf("Error printing diagnostics: trying to decrement a 0 (unsigned "
               "integer overflow).\n");
        assert(false);
      }
      op_sv.len -= 2;
      op_sv.cstr += 2;
    }
  }
}

// handle %[numerical-type*^] etc...
void print_handle_numerical(Diagnostic *diag, StringView format_code,
                            size_t num, size_t format_code_base_len) {
  if (format_code_base_len < format_code.len) {
    handle_numerical_operators(diag, format_code, format_code_base_len, &num);
  } else {
    printf("%zu", num);
  }
}

void handle_string_operators(Diagnostic *diag, StringView format_code,
                             size_t format_code_base_len, StringView *sv) {
  StringView op_sv;
  op_sv.cstr = format_code.cstr + format_code_base_len;
  op_sv.len = format_code.len - format_code_base_len;
  while (op_sv.len != 0) {
    if (sv_startswith_cstr(op_sv, ".len")) {
      op_sv.cstr += sizeof(".len") - 1;
      op_sv.len -= sizeof(".len") - 1;
      size_t len = sv->len;
      handle_numerical_operators(diag, format_code, format_code.len - op_sv.len,
                                 &len);
      break;
    }
  }
}

void print_handle_string(Diagnostic *diag, StringView format_code,
                         StringView sv, size_t format_code_base_len) {
  if (format_code_base_len < format_code.len) {
    handle_string_operators(diag, format_code, format_code_base_len, &sv);
  } else {
    anystr_print(sv);
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

#define NUMERICAL_FORMAT_CODES(X, start, diag)                                 \
  X("row", start.line)                                                         \
  X("col", start.col)                                                          \
  X("span-start-diff", start.col - 1)                                          \
  X("span-width", diag->span.len)                                              \
  X("cline", diag->cline)

#define STRING_FORMAT_CODES(X, start, diag)                                    \
  X("level", cstr_to_sv((char *)slevel_to_cstr[diag->base->level]))            \
  X("span-line", span_curr_line(diag->span))                                   \
  X("file", diag->span.source->name)                                           \
  X("cfile", cstr_to_sv((char *)diag->cfile))

void print_format_code(Diagnostic *diag, StringView format_code) {
  // Numerical type
  static CursorMark start;
  if (start.id != diag->span.start)
    start = span_start(diag->span);
#define X(str, num)                                                            \
  if (sv_startswith_cstr(format_code, str)) {                                  \
    print_handle_numerical(diag, format_code, num, sizeof(str) - 1);           \
    return;                                                                    \
  }
  NUMERICAL_FORMAT_CODES(X, start, diag)
#undef X

// String type
#define X(code, sv)                                                            \
  if (sv_startswith_cstr(format_code, code)) {                                 \
    print_handle_string(diag, format_code, sv, sizeof(code) - 1);              \
    return;                                                                    \
  }
  STRING_FORMAT_CODES(X, start, diag)
#undef X

  if (sv_startswith_cstr(format_code, "msg")) {
    print_formatted(diag, diag->base->msg.cstr);
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

#include <cursor.h>
#include <da_string.h>
#include <da_path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Path new_path(string_view pathstr) {
  Path p;
  p.str = malloc(pathstr.len + 1);
  p.len = pathstr.len;
  assert(p.str != NULL);
  strncpy(p.str, pathstr.str, pathstr.len);
  p.str[pathstr.len] = '\0';
  return p;
}

string_view path_to_sv(Path* path) {
  return new_sv(path->str, path->len);
}

bool is_path_absolute(Path* path) {
  Cursor cursor = new_cursor(path_to_sv(path));
  // POSIX
  if (ch_match_cursor(&cursor, '/')) return true;

  // WINDOWS
  char curr = peek(&cursor);
  if ((curr >= 'A' && curr <= 'Z') || (curr >= 'a' && curr <= 'z')) {
    advance_cursor(&cursor);
    if (!ch_match_cursor(&cursor, ':')) return false;
    curr = peek(&cursor);
    return curr == '/' || curr == '\\';
  }
  return str_match_cursor(&cursor, sv_from_cstr("\\\\"));
}

bool is_path_relative(Path* path) {
  return !is_path_absolute(path);
}

bool path_exists(Path* path) {
  FILE* f = fopen(path->str, "rb");
  if (f) {
    fclose(f);
    return true;
  }
  else return false;
}

#ifndef _WIN32
#include <sys/stat.h>

PathType get_path_type(Path *path) {
  struct stat st;

  if (stat(path->str, &st) != 0)
    return PATH_INVALID;

  if (S_ISREG(st.st_mode))
    return PATH_FILE;

  if (S_ISDIR(st.st_mode))
    return PATH_DIR;

  return PATH_INVALID; // symlink, device, etc. (you can expand if needed)
}

string_view get_absolute_path(Path* path) {
  return sv_from_cstr(realpath(path->str, NULL));
}

#else

#include <windows.h>

PathType get_path_type(Path *path) {
  DWORD attr = GetFileAttributesA(path->str);

  if (attr == INVALID_FILE_ATTRIBUTES)
    return PATH_INVALID;

  if (attr & FILE_ATTRIBUTE_DIRECTORY)
    return PATH_DIR;

  return PATH_FILE;
}

string_view get_absolute_path(Path* path) {
  char buffer[MAX_PATH];
  DWORD len = GetFullPathNameA(path->str, MAX_PATH, buffer, NULL);
  if (len == 0) return NULL;
  return new_sv(_strdup(buffer), len);
}

#endif

string_view get_path_directory(Path* path) {
  assert(path->len != 0);
  size_t i = path->len-1;
  // if path has a traling '/' or '\' 
  if (path->str[i] == '/' || path->str[i] == '\\') {
    i--;
    assert(i>0);
  }
  while (i > 0 && path->str[i] != '/' && path->str[i] != '\\') i--;
  assert(i > 0 && (path->str[i] == '/' || path->str[i] == '\\'));
  return sv_slice(path_to_sv(path), 0, i+1);
}

string_view read_file(Path* path) {
  FILE* f = fopen(path->str, "rb");
  assert(f != NULL);

  assert(fseek(f, 0, SEEK_END) == 0);

  long size = ftell(f);
  assert(size>=0);
  rewind(f);

  char* buf = malloc(size + 1);
  assert(buf != NULL);
  size_t read = fread(buf, sizeof(char), size, f);

  fclose(f);

  if (read != size) {
    free(buf);
    assert(false);
  }

  buf[size] = '\0';
  return new_sv(buf, size);
}


void free_path(Path *path) {
  free(path->str);
  *path = (Path){0};
}

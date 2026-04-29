#include <cursor.h>
#include <da_string.h>
#include <da_path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Path new_path(string_view pathstr) {
  Path p = {0};
  p.cstr = malloc(pathstr.len + 1);
  p.len = pathstr.len;
  assert(p.cstr != NULL);
  strncpy(p.cstr, pathstr.cstr, pathstr.len);
  p.cstr[pathstr.len] = '\0';
  return p;
}

Path new_path_from_cstr(char* pathstr) {
  Path p = {0};
  p.len = strlen(pathstr);
  p.cstr = strdup(pathstr);
  return p;
}

bool path_cmp(Path* path1, Path* path2) {
  return path1->len == path2->len && strcmp(path1->cstr, path2->cstr);
}

string_view path_to_sv(Path *path) {
  return new_sv(path->cstr, path->len);
}

string path_to_str(Path* path) {
  return new_str(path->cstr, path->len);
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
  FILE* f = fopen(path->cstr, "rb");
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

  if (stat(path->cstr, &st) != 0)
    return PATH_INVALID;

  if (S_ISREG(st.st_mode))
    return PATH_FILE;

  if (S_ISDIR(st.st_mode))
    return PATH_DIR;

  return PATH_INVALID; // symlink, device, etc. (you can expand if needed)
}

string get_absolute_path(Path* path) {
  char* cstr = realpath(path->cstr, NULL);
  return new_str(cstr, strlen(cstr));
}

#else

#include <windows.h>

PathType get_path_type(Path *path) {
  DWORD attr = GetFileAttributesA(path->cstr);

  if (attr == INVALID_FILE_ATTRIBUTES)
    return PATH_INVALID;

  if (attr & FILE_ATTRIBUTE_DIRECTORY)
    return PATH_DIR;

  return PATH_FILE;
}

string_view get_absolute_path(Path* path) {
  char buffer[MAX_PATH];
  DWORD len = GetFullPathNameA(path->cstr, MAX_PATH, buffer, NULL);
  if (len == 0) return NULL;
  return new_sv(_strdup(buffer), len);
}

#endif

string_view get_path_directory(Path* path) {
  assert(path->len != 0);
  size_t i = path->len-1;
  // if path has a traling '/' or '\' 
  if (path->cstr[i] == '/' || path->cstr[i] == '\\') {
    i--;
    assert(i>0);
  }
  while (i > 0 && path->cstr[i] != '/' && path->cstr[i] != '\\') i--;
  assert(i > 0 && (path->cstr[i] == '/' || path->cstr[i] == '\\'));
  return sv_slice(path_to_sv(path), 0, i+1);
}

string read_file(Path* path) {
  FILE* f = fopen(path->cstr, "rb");
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
  return new_str(buf, size);
}


void free_path(Path *path) {
  free(path->cstr);
  *path = (Path){0};
}

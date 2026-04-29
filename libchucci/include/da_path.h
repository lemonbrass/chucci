#ifndef DA_PATH_H
#define DA_PATH_H

#include <da_arena.h>
#include <da_string.h>

typedef struct {
  char* str;
  size_t len;
} Path;

typedef enum {
  PATH_DIR,
  PATH_FILE,
  PATH_INVALID,
} PathType;

Path new_path(string_view pathstr);
Path new_path_from_cstr(char* pathstr);
string_view get_absolute_path(Path* path);
bool is_path_absolute(Path* path);
bool is_path_relative(Path* path);
bool path_exists(Path* path);
PathType get_path_type(Path* path);

bool path_cmp(Path* path1, Path* path2);

// these ALL malloc string_views
string_view get_path_directory(Path* path);
string_view read_file(Path* path);
string_view path_to_sv(Path* path);

void free_path(Path* path);
#endif

#ifndef __FILE_H
#define __FILE_H

#include "utils/smallvec.h"
#include "utils/string.h"

typedef struct File {
  String contents;
  StringView name;
} File;

SMALLVEC_DEF(File, FileVec, filevec);

void file_free(File *file);

#endif

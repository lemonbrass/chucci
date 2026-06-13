#ifndef __FILE_H
#define __FILE_H

#include "utils/smallvec.h"
#include "utils/string.h"

typedef struct File {
  StringView contents;
  StringView name;
} File;

SMALLVEC_DEF(File, FileVec, filevec, VMEM_ARENA_ALLOC_INT);

#endif

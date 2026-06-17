#include "utils/chucci_alloc.h"
#include "utils/file.h"
#include "utils/smallvec.h"

SMALLVEC_IMPL(File, FileVec, filevec, VMEM_ARENA_ALLOC_INT);

File file_new(StringView contents, StringView filename) {
  File file = {0};
  file.contents = contents;
  file.name = filename;
  return file;
}

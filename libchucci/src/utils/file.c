#include "utils/file.h"
#include "utils/chucci_alloc.h"
#include "utils/smallvec.h"
#include "utils/string.h"

SMALLVEC_IMPL(File, FileVec, filevec, VMEM_ARENA_ALLOC_INT);

void file_free(File *file) { str_free(file->contents); }

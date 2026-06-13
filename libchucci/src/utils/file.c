#include "utils/file.h"
#include "utils/chucci_alloc.h"
#include "utils/smallvec.h"

SMALLVEC_IMPL(File, FileVec, filevec, VMEM_ARENA_ALLOC_INT);

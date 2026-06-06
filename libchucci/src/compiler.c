#include "compiler.h"
#include "utils/chucci_alloc.h"
#include "utils/smallvec.h"
#include "utils/string.h"

SMALLVEC_IMPL(String, StringStack, string_stack, VMEM_ARENA_ALLOC_INT)

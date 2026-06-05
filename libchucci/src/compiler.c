#include "compiler.h"
#include "utils/chucci_alloc.h"
#include "utils/string.h"
#include "utils/vec.h"

VEC_IMPL(String, StringStack, string_stack, VMEM_ARENA_ALLOC_INT)

#ifndef __DA_ASSERT_H
#define __DA_ASSERT_H

void __handle_assert(const char *cond, const char *file, int line);

#define da_assert(cond) \
    ((cond) ? (void)0 : __handle_assert(#cond, __FILE__, __LINE__))

#endif

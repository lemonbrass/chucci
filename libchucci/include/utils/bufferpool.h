#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#define MAX_BUFFERS 4

#define bufferpool_t(T) struct { T buffers[MAX_BUFFERS]; uint8_t max_used; }
#define bp_get(bp, newfn) ( assert( (bp).max_used < MAX_BUFFERS ), (bp).buffers[(bp).max_used - 1] = newfn(), &(bp).buffers[(bp).max_used++ - 1])
#define bp_save(bp, buffer, resetfn) ( resetfn((buffer)), assert( buffer == &(bp).buffers[--(bp).max_used] ) )
#define bp_destroy(bp) (cv_destroy((bp).buffers))


#endif

#include "syshook-malloc.h"
#include <atomic>
#include <unistd.h>

// thread-local storage to deal with malloc in a reasonably graceful way
static __thread void  (*_libc_free)(void*) = NULL;
static __thread void* (*_libc_malloc)(size_t) = NULL;
static __thread void* (*_libc_realloc)(void *, size_t) = NULL;
static __thread void* (*_libc_calloc)(size_t, size_t) = NULL;
static __thread uint8_t _malloc_hook_enabled = 1;

static __thread uint64_t _malloc_count = 0;
static __thread uint64_t _malloc_sz = 0;
static __thread uint64_t _free_count = 0;

// nasty hack to work around dlsym call to calloc when pthreads are used.
// use 64-bit value for alignment.
static __thread uint64_t _calloc_buffer_init[256];
static __thread uint8_t _initial_buffer_used = 0;
static __thread uint8_t _enable_initial_buffer = 0;

const plugin::Instrumentation::MemoryInfo plugin::Instrumentation::GetMemoryCounts() {
    return MemoryInfo(_malloc_count, _free_count, _malloc_sz);
}

// make sure we don't end up in a recursive malloc call.
void* calloc (size_t num, size_t size) {
    if(!_initial_buffer_used && _enable_initial_buffer) {
        _initial_buffer_used = 1;
        return (void *)_calloc_buffer_init;
    }
    if(!_libc_calloc) {
        _enable_initial_buffer = 1;
        _libc_calloc = (void* (*)(size_t, size_t))(dlsym(RTLD_NEXT, "calloc"));
        _enable_initial_buffer = 0;
    }
    ++ _malloc_count;
    _malloc_sz += (num * size);
    return _libc_calloc(num, size);
}

void* realloc (void* ptr, size_t size) {
    if(!_libc_realloc) {
        _libc_realloc = (void* (*)(void*, size_t))(dlsym(RTLD_NEXT, "realloc"));
    }
    ++ _malloc_count;
    _malloc_sz += size;
    return _libc_realloc(ptr, size);
}

// NOTE: Uncomment this if using calls to items that require malloc from within malloc itself
// __thread uint8_t _malloc_noentry = 0;

void* malloc(size_t sz) {
    if(!_libc_malloc) {
        _libc_malloc = (void* (*)(size_t))(dlsym(RTLD_NEXT, "malloc"));
    }
//    if(_malloc_noentry) {
//        return _libc_malloc(sz);
//    }
//    _malloc_noentry = 1;
    ++ _malloc_count;
    _malloc_sz += sz;
    // printf("MALLOC_STATS: %lu / %lu / %lu\n", _malloc_count, _malloc_sz, _free_count);
//    _malloc_noentry = 0;
    return _libc_malloc(sz);
}

void free(void *p) {
    if(!_libc_free) {
        _libc_free = (void (*)(void *))(dlsym(RTLD_NEXT, "free"));
    }
    ++ _free_count;
    // Don't actually free anything that hit our initial calloc.
    if(p == _calloc_buffer_init) {
        return;
    }
    _libc_free(p);
}

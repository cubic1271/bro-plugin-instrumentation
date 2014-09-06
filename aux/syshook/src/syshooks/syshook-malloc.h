#ifndef _BRO_INSTRUMENTATION_SYSHOOK_MALLOC_H
#define _BRO_INSTRUMENTATION_SYSHOOK_MALLOC_H

#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

extern "C" {
	// hook calls to malloc to record statistics
    void free(void *p);
    void* malloc(size_t sz);
    void* calloc (size_t num, size_t size);
    void* realloc (void* ptr, size_t size);
}

namespace plugin {
	namespace Instrumentation {
		struct MemoryInfo {
			const uint64_t malloc_count;
			const uint64_t free_count;
			const uint64_t malloc_sz;

			MemoryInfo(const uint64_t mallocs, const uint64_t frees, const uint64_t sz)
			: malloc_count(mallocs), free_count(frees), malloc_sz(sz) { }
		};

		const MemoryInfo GetMemoryCounts();
	}
}

#endif

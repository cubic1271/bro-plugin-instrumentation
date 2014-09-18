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
			uint64_t malloc_count;
			uint64_t free_count;
			uint64_t malloc_sz;

			MemoryInfo()
			: malloc_count(0), free_count(0), malloc_sz(0) { }

			MemoryInfo(const uint64_t mallocs, const uint64_t frees, const uint64_t sz)
			: malloc_count(mallocs), free_count(frees), malloc_sz(sz) { }

			MemoryInfo operator +(const MemoryInfo& s2) {
				return MemoryInfo(malloc_count + s2.malloc_count,
								  free_count + s2.free_count,
								  malloc_sz + s2.malloc_sz);
			}

			MemoryInfo operator -(const MemoryInfo& s2) {
				return MemoryInfo(malloc_count - s2.malloc_count,
								  free_count - s2.free_count,
								  malloc_sz - s2.malloc_sz);
			}

			MemoryInfo operator +=(const MemoryInfo& s2) {
				this->malloc_count += s2.malloc_count;
				this->free_count += s2.free_count;
				this->malloc_sz += s2.malloc_sz;
				return *this;
			}

			MemoryInfo operator -=(const MemoryInfo& s2) {
				this->malloc_count -= s2.malloc_count;
				this->free_count -= s2.free_count;
				this->malloc_sz -= s2.malloc_sz;
				return *this;
			}
		};

		const MemoryInfo GetMemoryCounts();
	}
}

#endif

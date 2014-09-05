#ifndef _BRO_INSTRUMENTATION_SYSHOOK_IO_H
#define _BRO_INSTRUMENTATION_SYSHOOK_IO_H

#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <atomic>

extern "C" {
	FILE *fopen(const char *path, const char *params);
	size_t fwrite(const void *ptr, size_t sz, size_t count, FILE *target);
	size_t fread(void *ptr, size_t sz, size_t count, FILE *target);
	int fflush(FILE *target);
	int open(const char *path, int flags, ...);
	ssize_t read(int fildes, void *buf, size_t nbyte);
	ssize_t write(int fildes, const void *buf, size_t nbyte);
}

namespace plugin {
	namespace Instrumentation {
		struct ReadWriteInfo {
			const uint64_t fopen_count;
			const uint64_t fread_count;
			const uint64_t fwrite_count;
			const uint64_t fflush_count;
			const uint64_t fread_sz;			
			const uint64_t fwrite_sz;
			const uint64_t open_count;
			const uint64_t read_count;
			const uint64_t write_count;
			const uint64_t read_sz;
			const uint64_t write_sz;

			ReadWriteInfo(const uint64_t fo_count, const uint64_t fr_count, const uint64_t fw_count,
						  const uint64_t ff_count, const uint64_t fr_sz, const uint64_t fw_sz,
						  const uint64_t o_count, const uint64_t r_count, const uint64_t w_count,
						  const uint64_t r_sz, const uint64_t w_sz)
			: fopen_count(fo_count), fread_count(fr_count), fwrite_count(fw_count),
			  fflush_count(ff_count), fread_sz(fr_sz), fwrite_sz(fw_sz),
			  open_count(o_count), read_count(r_count), write_count(w_count),
			  read_sz(r_sz), write_sz(w_sz) { }

		};

		const ReadWriteInfo GetReadWriteCounts();
	}
}

#endif

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
			uint64_t fopen_count;
			uint64_t fread_count;
			uint64_t fwrite_count;
			uint64_t fflush_count;
			uint64_t fread_sz;			
			uint64_t fwrite_sz;
			uint64_t open_count;
			uint64_t read_count;
			uint64_t write_count;
			uint64_t read_sz;
			uint64_t write_sz;

			ReadWriteInfo()
			: fopen_count(0), fread_count(0), fwrite_count(0),
			  fflush_count(0), fread_sz(0), fwrite_sz(0),
			  open_count(0), read_count(0), write_count(0),
			  read_sz(0), write_sz(0) { }

			ReadWriteInfo(const uint64_t fo_count, const uint64_t fr_count, const uint64_t fw_count,
						  const uint64_t ff_count, const uint64_t fr_sz, const uint64_t fw_sz,
						  const uint64_t o_count, const uint64_t r_count, const uint64_t w_count,
						  const uint64_t r_sz, const uint64_t w_sz)
			: fopen_count(fo_count), fread_count(fr_count), fwrite_count(fw_count),
			  fflush_count(ff_count), fread_sz(fr_sz), fwrite_sz(fw_sz),
			  open_count(o_count), read_count(r_count), write_count(w_count),
			  read_sz(r_sz), write_sz(w_sz) { }

			ReadWriteInfo operator +(const ReadWriteInfo& s2) {
				return ReadWriteInfo(fopen_count + s2.fopen_count, fread_count + s2.fread_count, fwrite_count + s2.fwrite_count,
			  		                 fflush_count + s2.fflush_count, fread_sz + s2.fread_sz, fwrite_sz + s2.fwrite_sz,
			  		                 open_count + s2.open_count, read_count + s2.read_count, write_count + s2.write_count,
			  		                 read_sz + s2.read_sz, write_sz + s2.write_sz);
			}

			ReadWriteInfo operator -(const ReadWriteInfo& s2) {
				return ReadWriteInfo(fopen_count - s2.fopen_count, fread_count - s2.fread_count, fwrite_count - s2.fwrite_count,
			  		                 fflush_count - s2.fflush_count, fread_sz - s2.fread_sz, fwrite_sz - s2.fwrite_sz,
			  		                 open_count - s2.open_count, read_count - s2.read_count, write_count - s2.write_count,
			  		                 read_sz - s2.read_sz, write_sz - s2.write_sz);
			}

		};

		const ReadWriteInfo GetReadWriteCounts();
	}
}

#endif

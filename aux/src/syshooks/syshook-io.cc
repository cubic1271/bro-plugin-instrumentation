#include "syshook-io.h"
#include <fcntl.h>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

static __thread FILE*   (*_libc_fopen)(const char *, const char *) = NULL;
static __thread size_t  (*_libc_fwrite)(const void *, size_t, size_t, FILE*) = NULL;
static __thread size_t  (*_libc_fread)(void *, size_t, size_t, FILE*) = NULL;
static __thread int     (*_libc_fflush)(FILE*) = NULL;
static __thread int     (*_libc_open)(const char *, int, ...) = NULL;
static __thread ssize_t (*_libc_read)(int, void*, size_t) = NULL;
static __thread ssize_t (*_libc_write)(int, const void*, size_t) = NULL;

static std::atomic<uint64_t> _fopen_count(0);
static std::atomic<uint64_t> _fwrite_count(0);
static std::atomic<uint64_t> _fwrite_sz(0);
static std::atomic<uint64_t> _fread_count(0);
static std::atomic<uint64_t> _fread_sz(0);
static std::atomic<uint64_t> _fflush_count(0);
static std::atomic<uint64_t> _open_count(0);
static std::atomic<uint64_t> _read_count(0);
static std::atomic<uint64_t> _read_sz(0);
static std::atomic<uint64_t> _write_count(0);
static std::atomic<uint64_t> _write_sz(0);

namespace {
	struct StaticInit {
		StaticInit() {
			// 11.  Thou shalt not lock when instrumenting an application
			assert(_fopen_count.is_lock_free());
		}
	};

	StaticInit initializer;
}

const plugin::Instrumentation::ReadWriteInfo plugin::Instrumentation::GetReadWriteCounts() {
    return ReadWriteInfo(_fopen_count, _fread_count, _fwrite_count, 
    	                 _fflush_count, _fread_sz, _fwrite_sz, 
    	                 _open_count, _read_count, _write_count,
    	                 _read_sz, _write_sz);
}

FILE *fopen(const char *path, const char *params) {
    if(!_libc_fopen) {
        _libc_fopen = (FILE* (*)(const char *, const char *))(dlsym(RTLD_NEXT, "fopen"));
    }

    ++_fopen_count;
    return _libc_fopen(path, params);
}

size_t fwrite(const void *ptr, size_t sz, size_t count, FILE *target) {
    if(!_libc_fwrite) {
        _libc_fwrite = (size_t (*)(const void *, size_t, size_t, FILE*))(dlsym(RTLD_NEXT, "fwrite"));
    }

    ++_fwrite_count;
    const size_t res = _libc_fwrite(ptr, sz, count, target);
    _fwrite_sz += res * sz;
    return res;
}

size_t fread(void *ptr, size_t sz, size_t count, FILE *target) {
    if(!_libc_fread) {
        _libc_fread = (size_t (*)(void *, size_t, size_t, FILE*))(dlsym(RTLD_NEXT, "fread"));
    }

    ++_fread_count;
    const size_t res = _libc_fread(ptr, sz, count, target);
    _fread_sz += res * sz;
    return res;
}

int fflush(FILE *target) {
    if(!_libc_fflush) {
        _libc_fflush = (int (*)(FILE*))(dlsym(RTLD_NEXT, "fflush"));
    }

    ++_fflush_count;
    return _libc_fflush(target);
}

int open(const char *path, int flags, ...) {
	if(!_libc_open) {
		_libc_open = (int (*)(const char *, int, ...))(dlsym(RTLD_NEXT, "open"));
	}

	// handle variadic arguments in a graceful fashion
	if( (flags & O_CREAT) == O_CREAT) {
		va_list arg;
		va_start(arg, flags);
		mode_t mode = (mode_t)va_arg(arg, mode_t);
		va_end(arg);
		return _libc_open(path, flags, mode);
	}

	++_open_count;
	return _libc_open(path, flags);
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	if(!_libc_read) {
		_libc_read = (ssize_t (*)(int, void*, size_t))(dlsym(RTLD_NEXT, "read"));
	}

	++_read_count;
	const ssize_t res = _libc_read(fildes, buf, nbyte);
	_read_sz += (res >= 0) ? res : 0;
	return res;
}

ssize_t write(int fildes, const void *buf, size_t nbyte) {
	if(!_libc_write) {
		_libc_write = (ssize_t (*)(int, const void*, size_t))(dlsym(RTLD_NEXT, "write"));
	}

	++_write_count;
	const ssize_t res = _libc_write(fildes, buf, nbyte);
	_write_sz += (res >= 0) ? res : 0;
	return res;
}

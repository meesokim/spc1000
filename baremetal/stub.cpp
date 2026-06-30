#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <circle/types.h>
#include <circle/string.h>
#include <circle/util.h>

extern "C" {

void bz_internal_error(int n) {}

// Redirect standard memory allocations to Circle's heap allocator
void *malloc(size_t size)
{
    void *p = ::operator new(size + 4);
    if (!p) return nullptr;
    *(u32 *)p = size;
    return (void *)((char *)p + 4);
}

void free(void *ptr)
{
    if (!ptr) return;
    void *p = (void *)((char *)ptr - 4);
    ::operator delete(p);
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr) return malloc(size);
    void *p = (void *)((char *)ptr - 4);
    u32 old_size = *(u32 *)p;
    void *new_ptr = malloc(size);
    if (!new_ptr) return nullptr;
    memcpy(new_ptr, ptr, old_size < size ? old_size : size);
    free(ptr);
    return new_ptr;
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total = nmemb * size;
    void *p = malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    return 1;
}

int _open(const char *name, int flags, int mode) {
    return -1;
}

int _stat(const char *path, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

void _exit(int status) {
    while (1) {}
}

int _gettimeofday(struct timeval *tv, void *tz) {
    if (tv) {
        tv->tv_sec = 0;
        tv->tv_usec = 0;
    }
    return 0;
}

ssize_t _write(int file, const void *ptr, size_t len) {
    return len;
}

void *_sbrk(ptrdiff_t incr) {
    extern char _end;
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;
    heap_end += incr;

    return (void *)prev_heap_end;
}

int _close(int file) {
    return -1;
}

void _fini() {}

ssize_t _read(int file, void *ptr, size_t len) {
    return 0;
}

off_t _lseek(int file, off_t offset, int whence) {
    return -1;
}

int _getentropy(void *buffer, size_t length) {
    errno = ENOSYS;
    return -1;
}

}
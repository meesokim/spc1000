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

char *strrchr(const char *s, int c) {
    char *last = nullptr;
    do {
        if (*s == (char)c) {
            last = (char *)s;
        }
    } while (*s++);
    return last;
}

struct tm *localtime(const time_t *timep) {
    static struct tm t;
    memset(&t, 0, sizeof(t));
    return &t;
}

time_t mktime(struct tm *tm) {
    return 0;
}

time_t time(time_t *tloc) {
    if (tloc) *tloc = 0;
    return 0;
}

}
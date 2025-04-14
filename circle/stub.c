#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>

void bz_internal_error(int n) {
    
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

// 시스템 호출: _stat
int _stat(const char *path, struct stat *st) {
    // 파일 시스템이 없으므로 기본 값 설정
    st->st_mode = S_IFCHR; // 문자 장치로 설정
    return 0; // 성공
}

// 시스템 호출: _exit
void _exit(int status) {
    // 종료 루프 (운영 체제 없음)
    while (1) {
        // 무한 루프
    }
}

// 시스템 호출: _gettimeofday
int _gettimeofday(struct timeval *tv, void *tz) {
    if (tv) {
        tv->tv_sec = 0; // 현재 시간을 설정할 수 없으므로 0으로 설정
        tv->tv_usec = 0;
    }
    return 0; // 성공
}

// 시스템 호출: _write
ssize_t _write(int file, const void *ptr, size_t len) {
    // 표준 출력(stdout) 또는 표준 에러(stderr)만 허용
    return len; // 성공적으로 len 바이트 썼다고 반환
}

// 시스템 호출: _sbrk (메모리 할당)
void *_sbrk(ptrdiff_t incr) {
    extern char _end; // 링크 스크립트에서 제공되는 힙 시작점
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;
    heap_end += incr;

    return (void *)prev_heap_end; // 힙 포인터 반환
}

// 시스템 호출: _close
int _close(int file) {
    // 파일 시스템이 없으므로 항상 실패
    return -1;
}

// 시스템 호출: _fini
void _fini() {
    // 종료 처리가 필요하지 않으므로 빈 함수
}

// 시스템 호출: _read
ssize_t _read(int file, void *ptr, size_t len) {
    // 입력 기능이 없으므로 항상 실패
    return 0;
}

// 시스템 호출: _lseek
off_t _lseek(int file, off_t offset, int whence) {
    // 파일 시스템이 없으므로 항상 실패
    return -1;
}

// 시스템 호출: _getentropy
int _getentropy(void *buffer, size_t length) {
    // 보안 엔트로피를 제공할 수 없음. 실패 반환
    errno = ENOSYS; // 기능이 구현되지 않았음을 표시
    return -1;
}
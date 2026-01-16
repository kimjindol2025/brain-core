/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_loader.c
 *
 * Memory-Mapped File Loader
 *
 * 목적:
 *   - 파일을 포인터로 매핑 (Zero-Copy)
 *   - OS 페이징 시스템 활용
 *   - 거대 파일도 메모리처럼 접근
 *
 * 장점:
 *   - read() 없이 포인터로 직접 접근
 *   - OS가 자동으로 페이지 관리
 *   - 여러 프로세스가 공유 가능
 *
 * Zero Dependency: POSIX mmap만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Linux에서 ftruncate, madvise 선언을 위해 필요 */
#define _XOPEN_SOURCE 700

#include "mmap_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_file_open
 *
 * 파일을 메모리에 매핑
 *
 * 파라미터:
 *   - filepath: 파일 경로
 *   - writable: 1 = 읽기/쓰기, 0 = 읽기 전용
 *
 * 리턴:
 *   - 성공: mmap_file_t 구조체 포인터
 *   - 실패: NULL
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
mmap_file_t* mmap_file_open(const char* filepath, int writable) {
    if (!filepath) {
        fprintf(stderr, "[mmap] Error: filepath is NULL\n");
        return NULL;
    }

    /* 구조체 할당 */
    mmap_file_t* mf = (mmap_file_t*)malloc(sizeof(mmap_file_t));
    if (!mf) {
        fprintf(stderr, "[mmap] Error: malloc failed\n");
        return NULL;
    }

    memset(mf, 0, sizeof(mmap_file_t));

    /* 파일 열기 */
    int flags = writable ? O_RDWR : O_RDONLY;
    mf->fd = open(filepath, flags);
    if (mf->fd < 0) {
        fprintf(stderr, "[mmap] Error: cannot open file '%s': %s\n",
                filepath, strerror(errno));
        free(mf);
        return NULL;
    }

    /* 파일 크기 확인 */
    struct stat st;
    if (fstat(mf->fd, &st) < 0) {
        fprintf(stderr, "[mmap] Error: fstat failed: %s\n", strerror(errno));
        close(mf->fd);
        free(mf);
        return NULL;
    }

    mf->size = st.st_size;

    if (mf->size == 0) {
        fprintf(stderr, "[mmap] Error: file is empty\n");
        close(mf->fd);
        free(mf);
        return NULL;
    }

    /* mmap 실행 */
    int prot = writable ? (PROT_READ | PROT_WRITE) : PROT_READ;
    int map_flags = MAP_SHARED;  /* 변경사항을 파일에 반영 */

    mf->addr = mmap(NULL, mf->size, prot, map_flags, mf->fd, 0);
    if (mf->addr == MAP_FAILED) {
        fprintf(stderr, "[mmap] Error: mmap failed: %s\n", strerror(errno));
        close(mf->fd);
        free(mf);
        return NULL;
    }

    mf->writable = writable;

    printf("[mmap] ✓ Mapped '%s': %zu bytes @ %p\n",
           filepath, mf->size, mf->addr);

    return mf;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_file_close
 *
 * 매핑 해제 및 파일 닫기
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void mmap_file_close(mmap_file_t* mf) {
    if (!mf) return;

    /* munmap */
    if (mf->addr && mf->addr != MAP_FAILED) {
        if (munmap(mf->addr, mf->size) < 0) {
            fprintf(stderr, "[mmap] Warning: munmap failed: %s\n", strerror(errno));
        }
    }

    /* close */
    if (mf->fd >= 0) {
        close(mf->fd);
    }

    free(mf);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_file_sync
 *
 * 메모리 변경사항을 파일에 동기화 (msync)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int mmap_file_sync(mmap_file_t* mf) {
    if (!mf || !mf->addr) return -1;

    /* MS_SYNC: 즉시 동기화 (blocking) */
    if (msync(mf->addr, mf->size, MS_SYNC) < 0) {
        fprintf(stderr, "[mmap] Error: msync failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_file_advise
 *
 * OS에게 메모리 사용 패턴 힌트 제공
 *
 * advice:
 *   - MADV_SEQUENTIAL: 순차 읽기
 *   - MADV_RANDOM: 랜덤 접근
 *   - MADV_WILLNEED: 곧 사용 예정 (prefetch)
 *   - MADV_DONTNEED: 당분간 사용 안함 (evict)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int mmap_file_advise(mmap_file_t* mf, int advice) {
    if (!mf || !mf->addr) return -1;

    if (madvise(mf->addr, mf->size, advice) < 0) {
        fprintf(stderr, "[mmap] Warning: madvise failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_file_create
 *
 * 새 파일 생성 및 크기 설정 후 매핑
 *
 * 사용 시나리오:
 *   - 새로운 Brain DB 파일 생성
 *   - 미리 크기를 할당하여 확장 없이 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
mmap_file_t* mmap_file_create(const char* filepath, size_t size) {
    if (!filepath || size == 0) {
        fprintf(stderr, "[mmap] Error: invalid parameters\n");
        return NULL;
    }

    /* 파일 생성 */
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        fprintf(stderr, "[mmap] Error: cannot create file '%s': %s\n",
                filepath, strerror(errno));
        return NULL;
    }

    /* 파일 크기 설정 (ftruncate) */
    if (ftruncate(fd, size) < 0) {
        fprintf(stderr, "[mmap] Error: ftruncate failed: %s\n", strerror(errno));
        close(fd);
        return NULL;
    }

    close(fd);

    /* 이제 일반 open으로 매핑 */
    return mmap_file_open(filepath, 1);  /* writable */
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_file_resize
 *
 * 파일 크기 변경 및 재매핑
 *
 * 주의:
 *   - 기존 포인터 무효화됨
 *   - 재매핑 후 새 주소 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int mmap_file_resize(mmap_file_t* mf, size_t new_size) {
    if (!mf || !mf->writable) return -1;

    /* 기존 매핑 해제 */
    if (munmap(mf->addr, mf->size) < 0) {
        fprintf(stderr, "[mmap] Error: munmap failed: %s\n", strerror(errno));
        return -1;
    }

    /* 파일 크기 변경 */
    if (ftruncate(mf->fd, new_size) < 0) {
        fprintf(stderr, "[mmap] Error: ftruncate failed: %s\n", strerror(errno));
        return -1;
    }

    /* 재매핑 */
    mf->addr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, mf->fd, 0);
    if (mf->addr == MAP_FAILED) {
        fprintf(stderr, "[mmap] Error: mmap failed: %s\n", strerror(errno));
        return -1;
    }

    mf->size = new_size;

    printf("[mmap] ✓ Resized to %zu bytes @ %p\n", new_size, mf->addr);

    return 0;
}

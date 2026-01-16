/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_loader.h
 *
 * Memory-Mapped File Loader Header
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef MMAP_LOADER_H
#define MMAP_LOADER_H

#include <stddef.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * mmap_file_t
 *
 * 매핑된 파일 핸들
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    int      fd;         /* 파일 디스크립터 */
    void*    addr;       /* 매핑된 메모리 주소 */
    size_t   size;       /* 파일 크기 */
    int      writable;   /* 0 = 읽기 전용, 1 = 읽기/쓰기 */
} mmap_file_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 함수 선언
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 파일 열기 및 매핑 */
mmap_file_t* mmap_file_open(const char* filepath, int writable);

/* 새 파일 생성 및 매핑 */
mmap_file_t* mmap_file_create(const char* filepath, size_t size);

/* 매핑 해제 및 닫기 */
void mmap_file_close(mmap_file_t* mf);

/* 변경사항 동기화 */
int mmap_file_sync(mmap_file_t* mf);

/* OS에게 사용 패턴 힌트 */
int mmap_file_advise(mmap_file_t* mf, int advice);

/* 파일 크기 변경 및 재매핑 */
int mmap_file_resize(mmap_file_t* mf, size_t new_size);

#endif /* MMAP_LOADER_H */

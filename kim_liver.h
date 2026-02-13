/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_liver.h
 *
 * Liver (간) - Memory Pool Manager
 *
 * 생물학적 역할:
 *   - 독소 제거 (garbage collection)
 *   - 에너지 저장 (memory pooling)
 *   - 대사 조절 (memory allocation)
 *   - 혈액 정화 (memory consolidation)
 *
 * 소프트웨어 역할:
 *   - 16MB 메모리 풀 관리
 *   - malloc/free 대체
 *   - 단편화 방지 (first-fit allocator)
 *   - 수명 기반 가비지 컬렉션 (GC 데몬)
 *   - Spine IPC 연동 (메모리 압박 신호)
 *
 * Priority: ★1★ (Critical - 24/7 운영 필수)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_LIVER_H
#define KIM_LIVER_H

#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <time.h>
#include "kim_spine.h"

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Configuration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define LIVER_POOL_SIZE     (16 * 1024 * 1024)      /* 16MB 메모리 풀 */
#define LIVER_BLOCK_SIZE    4096                    /* 4KB 블록 단위 */
#define LIVER_NUM_BLOCKS    (LIVER_POOL_SIZE / LIVER_BLOCK_SIZE)  /* 4,096개 */
#define LIVER_GC_INTERVAL   30                      /* GC 데몬 주기 (초) */
#define LIVER_MAX_LIFETIME  120                     /* 블록 최대 생명 (초) */
#define LIVER_PRESSURE_THRESHOLD 80                 /* 메모리 압박 임계값 (%) */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Enumerations
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 블록 상태 (Block State) */
typedef enum {
    BLOCK_FREE       = 0,       /* 비어있음 */
    BLOCK_ALLOCATED  = 1,       /* 할당됨 */
    BLOCK_RESERVED   = 2        /* 예약됨 */
} block_state_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Data Structures
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 메모리 블록 메타데이터 */
typedef struct {
    void*         addr;           /* 풀 내 블록 주소 */
    size_t        size;           /* 블록 크기 (4KB의 배수) */
    block_state_t state;          /* 할당 상태 */
    uint64_t      allocated_at;   /* 할당 시간 (마이크로초) */
    int           ref_count;      /* 참조 카운트 */
    char          tag[32];        /* 디버그 태그 (예: "cortex-buffer") */
} memory_block_t;

/* 간 장기 (Liver Organ) */
typedef struct {
    /* 메모리 풀 (mmap'd 영역) */
    void*            pool;             /* 풀 기본 주소 */
    size_t           pool_size;        /* 16MB */
    memory_block_t*  blocks;           /* 블록 메타데이터 배열 */
    uint32_t         num_blocks;       /* 4,096개 */

    /* Free List (스택 기반) */
    uint32_t*        free_list;        /* 빈 블록 인덱스 스택 */
    uint32_t         free_head;        /* 스택 포인터 */
    uint32_t         free_count;       /* 빈 블록 수 */

    /* GC 데몬 */
    pthread_t        gc_thread;        /* GC 워커 스레드 */
    int              gc_running;       /* GC 실행 중 플래그 */
    uint64_t         last_gc_time;     /* 마지막 GC 시간 */

    /* 스레드 안전 */
    pthread_mutex_t  lock;             /* 뮤텍스 */
    pthread_cond_t   cond;             /* 조건 변수 */

    /* 통계 */
    uint64_t total_allocated;          /* 누적 할당 크기 */
    uint64_t total_freed;              /* 누적 해제 크기 */
    uint64_t peak_usage;               /* 최대 사용량 */
    uint64_t fragmentation_count;      /* 단편화 횟수 */
    uint64_t gc_cycles;                /* GC 주기 수 */
    uint64_t compaction_count;         /* 압축 횟수 */
    uint64_t pressure_signals;         /* 압박 신호 수 */

    /* Spine 통합 */
    spine_t*  spine;                   /* Spine IPC */
    int       organ_id;                /* 장기 ID */
} liver_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * API Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 생명주기 (Lifecycle) */
liver_t* liver_create(void);
void     liver_destroy(liver_t* liver);
int      liver_start_gc(liver_t* liver);           /* GC 데몬 시작 */
int      liver_stop_gc(liver_t* liver);            /* GC 데몬 중지 */

/* 메모리 할당 (Memory Allocation) */
void* liver_alloc(liver_t* liver, size_t size, const char* tag);
void  liver_free(liver_t* liver, void* ptr);
void* liver_realloc(liver_t* liver, void* ptr, size_t new_size);

/* 가비지 컬렉션 (Garbage Collection) */
void liver_gc_cycle(liver_t* liver);               /* 수동 GC */
void liver_compact(liver_t* liver);                /* 메모리 압축 */

/* 모니터링 (Monitoring) */
int  liver_get_usage_percent(const liver_t* liver);
int  liver_get_fragmentation(const liver_t* liver);
void liver_stats(const liver_t* liver);

/* Spine 연동 */
void liver_set_spine(liver_t* liver, spine_t* spine, int organ_id);

/* 디버그 */
void liver_dump_blocks(const liver_t* liver);
void liver_validate_heap(const liver_t* liver);

#endif /* KIM_LIVER_H */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_lungs.h
 *
 * Lungs (폐) - Async I/O Manager
 *
 * 생물학적 역할:
 *   - 산소 공급 (I/O bandwidth)
 *   - 이산화탄소 배출 (output buffering)
 *   - 호흡 조절 (throttling)
 *   - 환기 (비동기 처리)
 *
 * 소프트웨어 역할:
 *   - epoll 기반 이벤트 루프
 *   - 워커 스레드 풀 (4개)
 *   - 논블로킹 I/O 큐
 *   - 큐 깊이 모니터링
 *   - Spine IPC 연동 (성능 신호)
 *
 * Priority: ★2★ (High - 성능 최적화 필수)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_LUNGS_H
#define KIM_LUNGS_H

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "kim_spine.h"

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Configuration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define LUNGS_MAX_FDS          1024           /* 최대 파일 디스크립터 */
#define LUNGS_BUFFER_SIZE      8192           /* I/O 버퍼 크기 */
#define LUNGS_NUM_WORKERS      4              /* 워커 스레드 수 */
#define LUNGS_MAX_QUEUE        256            /* 최대 요청 큐 깊이 */
#define LUNGS_SPEED_THRESHOLD  200            /* 성능 임계값 (ms) */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Enumerations
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* I/O 요청 타입 */
typedef enum {
    IO_READ    = 1,        /* 읽기 */
    IO_WRITE   = 2,        /* 쓰기 */
    IO_CONNECT = 3,        /* 연결 */
    IO_ACCEPT  = 4         /* 수락 */
} io_type_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Data Structures
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* I/O 요청 */
typedef struct {
    int         fd;                        /* 파일 디스크립터 */
    io_type_t   type;                      /* I/O 타입 */
    void*       buffer;                    /* 데이터 버퍼 (8KB) */
    size_t      size;                      /* 버퍼 크기 */
    void (*callback)(int fd, void* data, ssize_t len, void* user_data);
    void*       user_data;                 /* 사용자 컨텍스트 */
    uint64_t    queued_at;                 /* 큐 진입 시간 */
    uint32_t    priority;                  /* 우선순위 */
} io_request_t;

/* 폐 장기 (Lungs Organ) */
typedef struct {
    /* I/O 요청 큐 (Ring Buffer) */
    io_request_t*   queue;                 /* 요청 배열 */
    uint32_t        capacity;              /* 큐 용량 */
    uint32_t        head;                  /* 헤드 포인터 */
    uint32_t        tail;                  /* 테일 포인터 */
    uint32_t        count;                 /* 큐 항목 수 */

    /* epoll 상태 */
    int                  epoll_fd;         /* epoll 파일 디스크립터 */
    struct epoll_event*  events;           /* epoll 이벤트 배열 */

    /* 워커 스레드 풀 */
    pthread_t*      workers;               /* 워커 스레드 배열 */
    int             num_workers;           /* 워커 수 (4개) */
    int             running;               /* 실행 중 플래그 */

    /* 동기화 */
    pthread_mutex_t lock;                  /* 뮤텍스 */
    pthread_cond_t  has_work;              /* 작업 있음 신호 */

    /* 통계 */
    uint64_t total_reads;                  /* 누적 읽기 수 */
    uint64_t total_writes;                 /* 누적 쓰기 수 */
    uint64_t bytes_in;                     /* 누적 입력 바이트 */
    uint64_t bytes_out;                    /* 누적 출력 바이트 */
    uint64_t queue_depth_max;              /* 최대 큐 깊이 */
    uint64_t avg_latency_us;               /* 평균 지연시간 (마이크로초) */

    /* Spine 통합 */
    spine_t*  spine;                       /* Spine IPC */
    int       organ_id;                    /* 장기 ID */
} lungs_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * API Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 생명주기 (Lifecycle) */
lungs_t* lungs_create(int num_workers);
void     lungs_destroy(lungs_t* lungs);
int      lungs_start(lungs_t* lungs);
int      lungs_stop(lungs_t* lungs);

/* 비동기 I/O */
int lungs_async_read(lungs_t* lungs, int fd,
                     void (*callback)(int, void*, ssize_t, void*),
                     void* user_data);
int lungs_async_write(lungs_t* lungs, int fd,
                      const void* data, size_t len,
                      void (*callback)(int, void*, ssize_t, void*),
                      void* user_data);

/* 모니터링 (Monitoring) */
int  lungs_get_queue_depth(const lungs_t* lungs);
int  lungs_get_queue_percent(const lungs_t* lungs);
void lungs_stats(const lungs_t* lungs);

/* Spine 연동 */
void lungs_set_spine(lungs_t* lungs, spine_t* spine, int organ_id);

#endif /* KIM_LUNGS_H */

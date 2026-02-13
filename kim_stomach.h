/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_stomach.h
 *
 * 위(Stomach) - Input Buffer System
 *
 * 생물학적 역할:
 *   - 음식을 일시적으로 저장 (버퍼링)
 *   - 위산으로 물리적 분쇄 (전처리)
 *   - 조금씩 십이지장으로 배출 (Flow Control)
 *
 * 소프트웨어 역할:
 *   - Ring Buffer (고정 크기 순환 버퍼)
 *   - Raw Data 임시 저장
 *   - 메모리 누수 방지
 *
 * Priority: ★시급★ (1/3)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_STOMACH_H
#define KIM_STOMACH_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Configuration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define STOMACH_CAPACITY        1024    /* 최대 1024개 데이터 저장 */
#define STOMACH_MAX_DATA_SIZE   4096    /* 개별 데이터 최대 크기 */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Data Structures
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 음식 (Raw Data Entry) */
typedef struct {
    char*    data;          /* Raw bytes */
    size_t   size;          /* Data size */
    int64_t  timestamp;     /* 들어온 시각 */
    uint32_t flags;         /* 상태 플래그 */
} food_chunk_t;

/* 위 (Stomach Buffer) */
typedef struct {
    food_chunk_t* buffer;           /* Ring buffer */
    uint32_t      capacity;         /* 최대 용량 */
    uint32_t      head;             /* 쓰기 위치 */
    uint32_t      tail;             /* 읽기 위치 */
    uint32_t      count;            /* 현재 저장된 개수 */

    /* Thread Safety */
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;      /* 데이터 있을 때 signal */
    pthread_cond_t  not_full;       /* 공간 있을 때 signal */

    /* Statistics */
    uint64_t      total_ingested;   /* 총 섭취량 */
    uint64_t      total_digested;   /* 총 소화량 */
    uint32_t      overflows;        /* 포만 횟수 */
} stomach_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 위 생성/삭제 */
stomach_t* stomach_create(uint32_t capacity);
void       stomach_destroy(stomach_t* stomach);

/* 데이터 삽입 (입으로 들어옴) */
int stomach_ingest(stomach_t* stomach, const char* data, size_t size);

/* 데이터 추출 (십이지장으로 나감) */
int stomach_extract(stomach_t* stomach, food_chunk_t* out_chunk);

/* 위산 분비 (전처리) */
int stomach_secrete_acid(stomach_t* stomach);

/* 상태 확인 */
int  stomach_is_empty(const stomach_t* stomach);
int  stomach_is_full(const stomach_t* stomach);
void stomach_stats(const stomach_t* stomach);

/* 대기 함수 (Blocking) */
int stomach_wait_not_empty(stomach_t* stomach, int timeout_ms);
int stomach_wait_not_full(stomach_t* stomach, int timeout_ms);

/* Utility */
int stomach_get_fill_percent(const stomach_t* s);

#endif /* KIM_STOMACH_H */

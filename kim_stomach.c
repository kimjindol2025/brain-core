/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_stomach.c
 *
 * Stomach (Ring Buffer) Implementation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* POSIX extensions for clock_gettime, pthread_cond_timedwait */
#define _POSIX_C_SOURCE 200809L

#include "kim_stomach.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
stomach_t* stomach_create(uint32_t capacity) {
    stomach_t* s = (stomach_t*)malloc(sizeof(stomach_t));
    if (!s) {
        fprintf(stderr, "[Stomach] Error: malloc failed\n");
        return NULL;
    }

    s->buffer = (food_chunk_t*)calloc(capacity, sizeof(food_chunk_t));
    if (!s->buffer) {
        fprintf(stderr, "[Stomach] Error: buffer allocation failed\n");
        free(s);
        return NULL;
    }

    /* 각 청크의 데이터 버퍼 할당 */
    for (uint32_t i = 0; i < capacity; i++) {
        s->buffer[i].data = (char*)malloc(STOMACH_MAX_DATA_SIZE);
        s->buffer[i].size = 0;
        s->buffer[i].timestamp = 0;
        s->buffer[i].flags = 0;
    }

    s->capacity = capacity;
    s->head = 0;
    s->tail = 0;
    s->count = 0;

    pthread_mutex_init(&s->lock, NULL);
    pthread_cond_init(&s->not_empty, NULL);
    pthread_cond_init(&s->not_full, NULL);

    s->total_ingested = 0;
    s->total_digested = 0;
    s->overflows = 0;

    printf("[Stomach] 위 생성 완료: capacity=%u, max_size=%d bytes\n",
           capacity, STOMACH_MAX_DATA_SIZE);

    return s;
}

void stomach_destroy(stomach_t* s) {
    if (!s) return;

    pthread_mutex_lock(&s->lock);

    for (uint32_t i = 0; i < s->capacity; i++) {
        if (s->buffer[i].data) {
            free(s->buffer[i].data);
        }
    }

    free(s->buffer);
    pthread_mutex_unlock(&s->lock);

    pthread_mutex_destroy(&s->lock);
    pthread_cond_destroy(&s->not_empty);
    pthread_cond_destroy(&s->not_full);

    free(s);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Status Check
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int stomach_is_empty(const stomach_t* s) {
    return (s->count == 0);
}

int stomach_is_full(const stomach_t* s) {
    return (s->count >= s->capacity);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Get Current Timestamp (microseconds)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static int64_t get_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Ingest (음식 삽입)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int stomach_ingest(stomach_t* s, const char* data, size_t size) {
    if (!s || !data || size == 0 || size > STOMACH_MAX_DATA_SIZE) {
        return -1;
    }

    pthread_mutex_lock(&s->lock);

    /* 포만 상태 체크 */
    while (stomach_is_full(s)) {
        s->overflows++;
        printf("   🤢 [Stomach] 포만! 대기 중... (count=%u)\n", s->count);
        pthread_cond_wait(&s->not_full, &s->lock);
    }

    /* Head 위치에 데이터 복사 */
    food_chunk_t* chunk = &s->buffer[s->head];
    memcpy(chunk->data, data, size);
    chunk->size = size;
    chunk->timestamp = get_timestamp_us();
    chunk->flags = 0;

    /* Ring buffer 포인터 이동 */
    s->head = (s->head + 1) % s->capacity;
    s->count++;
    s->total_ingested++;

    printf("   🍔 [Stomach] 섭취: %zu bytes (count=%u/%u)\n",
           size, s->count, s->capacity);

    /* 대기 중인 Extract 깨우기 */
    pthread_cond_signal(&s->not_empty);

    pthread_mutex_unlock(&s->lock);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Extract (음식 추출)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int stomach_extract(stomach_t* s, food_chunk_t* out_chunk) {
    if (!s || !out_chunk) return -1;

    pthread_mutex_lock(&s->lock);

    /* 빈 위 체크 */
    while (stomach_is_empty(s)) {
        pthread_cond_wait(&s->not_empty, &s->lock);
    }

    /* Tail 위치에서 데이터 복사 */
    food_chunk_t* chunk = &s->buffer[s->tail];
    out_chunk->data = (char*)malloc(chunk->size);
    memcpy(out_chunk->data, chunk->data, chunk->size);
    out_chunk->size = chunk->size;
    out_chunk->timestamp = chunk->timestamp;
    out_chunk->flags = chunk->flags;

    /* Ring buffer 포인터 이동 */
    s->tail = (s->tail + 1) % s->capacity;
    s->count--;
    s->total_digested++;

    int64_t residence_time = get_timestamp_us() - chunk->timestamp;
    printf("   💩 [Stomach] 배출: %zu bytes (체류시간: %ld us)\n",
           out_chunk->size, residence_time);

    /* 대기 중인 Ingest 깨우기 */
    pthread_cond_signal(&s->not_full);

    pthread_mutex_unlock(&s->lock);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Secrete Acid (위산 분비 - 전처리)
 *
 * 현재 위에 있는 모든 데이터에 간단한 전처리 수행
 * 예: Null 종료 문자 추가, 공백 제거 등
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int stomach_secrete_acid(stomach_t* s) {
    if (!s) return -1;

    pthread_mutex_lock(&s->lock);

    uint32_t processed = 0;
    for (uint32_t i = 0; i < s->count; i++) {
        uint32_t idx = (s->tail + i) % s->capacity;
        food_chunk_t* chunk = &s->buffer[idx];

        /* 전처리: 끝에 NULL 종료 문자 추가 */
        if (chunk->size > 0 && chunk->size < STOMACH_MAX_DATA_SIZE) {
            chunk->data[chunk->size] = '\0';
            processed++;
        }
    }

    if (processed > 0) {
        printf("   💧 [Stomach] 위산 분비: %u개 데이터 전처리 완료\n", processed);
    }

    pthread_mutex_unlock(&s->lock);
    return processed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Wait Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int stomach_wait_not_empty(stomach_t* s, int timeout_ms) {
    if (!s) return -1;

    pthread_mutex_lock(&s->lock);

    if (timeout_ms < 0) {
        /* 무한 대기 */
        while (stomach_is_empty(s)) {
            pthread_cond_wait(&s->not_empty, &s->lock);
        }
        pthread_mutex_unlock(&s->lock);
        return 0;
    } else {
        /* 타임아웃 대기 */
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;

        int result = 0;
        while (stomach_is_empty(s)) {
            int ret = pthread_cond_timedwait(&s->not_empty, &s->lock, &ts);
            if (ret != 0) {
                result = -1;  /* Timeout */
                break;
            }
        }

        pthread_mutex_unlock(&s->lock);
        return result;
    }
}

int stomach_wait_not_full(stomach_t* s, int timeout_ms) {
    if (!s) return -1;

    pthread_mutex_lock(&s->lock);

    if (timeout_ms < 0) {
        while (stomach_is_full(s)) {
            pthread_cond_wait(&s->not_full, &s->lock);
        }
        pthread_mutex_unlock(&s->lock);
        return 0;
    } else {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;

        int result = 0;
        while (stomach_is_full(s)) {
            int ret = pthread_cond_timedwait(&s->not_full, &s->lock, &ts);
            if (ret != 0) {
                result = -1;
                break;
            }
        }

        pthread_mutex_unlock(&s->lock);
        return result;
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void stomach_stats(const stomach_t* s) {
    if (!s) return;

    printf("\n[Stomach Statistics]\n");
    printf("  Capacity:        %u\n", s->capacity);
    printf("  Current Count:   %u (%.1f%% full)\n",
           s->count, (float)s->count / s->capacity * 100.0f);
    printf("  Total Ingested:  %lu\n", s->total_ingested);
    printf("  Total Digested:  %lu\n", s->total_digested);
    printf("  Overflows:       %u\n", s->overflows);
    printf("  Head:            %u\n", s->head);
    printf("  Tail:            %u\n", s->tail);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utility Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int stomach_get_fill_percent(const stomach_t* s) {
    if (!s || s->capacity == 0) return 0;
    return (int)((float)s->count / s->capacity * 100.0f);
}

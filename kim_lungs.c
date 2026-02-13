/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_lungs.c
 *
 * Lungs (폐) - Async I/O Manager Implementation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "kim_lungs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utility Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static uint64_t get_timestamp_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

static void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return;
    }
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Worker Thread (I/O Processing)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static void* lungs_worker(void* arg) {
    lungs_t* lungs = (lungs_t*)arg;
    uint64_t start_time = get_timestamp_us();

    printf("[Lungs] 워커 스레드 시작\n");

    while (lungs->running) {
        pthread_mutex_lock(&lungs->lock);

        /* 작업 대기 */
        while (lungs->count == 0 && lungs->running) {
            pthread_cond_wait(&lungs->has_work, &lungs->lock);
        }

        if (!lungs->running) {
            pthread_mutex_unlock(&lungs->lock);
            break;
        }

        /* 요청 디큐 */
        io_request_t req = lungs->queue[lungs->tail];
        lungs->tail = (lungs->tail + 1) % lungs->capacity;
        lungs->count--;

        pthread_mutex_unlock(&lungs->lock);

        /* I/O 처리 */
        ssize_t result = 0;
        if (req.type == IO_READ) {
            result = read(req.fd, req.buffer, req.size);
            if (result > 0) {
                pthread_mutex_lock(&lungs->lock);
                lungs->total_reads++;
                lungs->bytes_in += result;
                pthread_mutex_unlock(&lungs->lock);
            }
        } else if (req.type == IO_WRITE) {
            result = write(req.fd, req.buffer, req.size);
            if (result > 0) {
                pthread_mutex_lock(&lungs->lock);
                lungs->total_writes++;
                lungs->bytes_out += result;
                pthread_mutex_unlock(&lungs->lock);
            }
        }

        /* 콜백 실행 */
        if (req.callback) {
            req.callback(req.fd, req.buffer, result, req.user_data);
        }

        /* 버퍼 정리 */
        if (req.buffer) {
            free(req.buffer);
        }
    }

    uint64_t elapsed = get_timestamp_us() - start_time;
    printf("[Lungs] 워커 스레드 종료 (운영 시간: %lu us)\n", elapsed);
    return NULL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

lungs_t* lungs_create(int num_workers) {
    if (num_workers <= 0 || num_workers > 16) {
        fprintf(stderr, "[Lungs] Error: 워커 수가 범위를 벗어났음 (1~16)\n");
        return NULL;
    }

    lungs_t* lungs = (lungs_t*)malloc(sizeof(lungs_t));
    if (!lungs) {
        fprintf(stderr, "[Lungs] Error: malloc failed\n");
        return NULL;
    }

    /* 큐 할당 */
    lungs->queue = (io_request_t*)calloc(LUNGS_MAX_QUEUE, sizeof(io_request_t));
    if (!lungs->queue) {
        fprintf(stderr, "[Lungs] Error: queue allocation failed\n");
        free(lungs);
        return NULL;
    }

    lungs->capacity = LUNGS_MAX_QUEUE;
    lungs->head = 0;
    lungs->tail = 0;
    lungs->count = 0;

    /* epoll 초기화 */
    lungs->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (lungs->epoll_fd == -1) {
        fprintf(stderr, "[Lungs] Error: epoll_create1 failed\n");
        free(lungs->queue);
        free(lungs);
        return NULL;
    }

    lungs->events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * LUNGS_MAX_QUEUE);
    if (!lungs->events) {
        fprintf(stderr, "[Lungs] Error: events allocation failed\n");
        close(lungs->epoll_fd);
        free(lungs->queue);
        free(lungs);
        return NULL;
    }

    /* 워커 스레드 풀 생성 */
    lungs->num_workers = num_workers;
    lungs->workers = (pthread_t*)malloc(sizeof(pthread_t) * num_workers);
    if (!lungs->workers) {
        fprintf(stderr, "[Lungs] Error: workers allocation failed\n");
        free(lungs->events);
        close(lungs->epoll_fd);
        free(lungs->queue);
        free(lungs);
        return NULL;
    }

    /* 동기화 초기화 */
    pthread_mutex_init(&lungs->lock, NULL);
    pthread_cond_init(&lungs->has_work, NULL);

    /* 통계 초기화 */
    lungs->total_reads = 0;
    lungs->total_writes = 0;
    lungs->bytes_in = 0;
    lungs->bytes_out = 0;
    lungs->queue_depth_max = 0;
    lungs->avg_latency_us = 0;

    /* Spine 초기화 */
    lungs->spine = NULL;
    lungs->organ_id = 5;  /* 폐 ID=5 */

    lungs->running = 0;

    printf("[Lungs] 폐 생성 완료: 큐=%u, 워커=%d\n", LUNGS_MAX_QUEUE, num_workers);

    return lungs;
}

void lungs_destroy(lungs_t* lungs) {
    if (!lungs) return;

    lungs_stop(lungs);

    pthread_mutex_lock(&lungs->lock);

    if (lungs->workers) {
        free(lungs->workers);
    }

    if (lungs->events) {
        free(lungs->events);
    }

    if (lungs->epoll_fd != -1) {
        close(lungs->epoll_fd);
    }

    if (lungs->queue) {
        free(lungs->queue);
    }

    pthread_mutex_unlock(&lungs->lock);

    pthread_mutex_destroy(&lungs->lock);
    pthread_cond_destroy(&lungs->has_work);

    free(lungs);
    printf("[Lungs] 폐 정리 완료\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Start/Stop
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int lungs_start(lungs_t* lungs) {
    if (!lungs || lungs->running) return -1;

    lungs->running = 1;

    /* 워커 스레드 시작 */
    for (int i = 0; i < lungs->num_workers; i++) {
        if (pthread_create(&lungs->workers[i], NULL, lungs_worker, lungs) != 0) {
            fprintf(stderr, "[Lungs] Error: 워커 %d 생성 실패\n", i);
            lungs->running = 0;
            return -1;
        }
    }

    printf("[Lungs] %d개 워커 스레드 시작\n", lungs->num_workers);
    return 0;
}

int lungs_stop(lungs_t* lungs) {
    if (!lungs || !lungs->running) return -1;

    lungs->running = 0;

    /* 모든 워커 깨우기 */
    pthread_mutex_lock(&lungs->lock);
    pthread_cond_broadcast(&lungs->has_work);
    pthread_mutex_unlock(&lungs->lock);

    /* 모든 워커 대기 */
    for (int i = 0; i < lungs->num_workers; i++) {
        pthread_join(lungs->workers[i], NULL);
    }

    printf("[Lungs] %d개 워커 스레드 중지\n", lungs->num_workers);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Async I/O
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int lungs_async_read(lungs_t* lungs, int fd,
                     void (*callback)(int, void*, ssize_t, void*),
                     void* user_data) {
    if (!lungs || fd < 0) return -1;

    pthread_mutex_lock(&lungs->lock);

    if (lungs->count >= lungs->capacity) {
        pthread_mutex_unlock(&lungs->lock);
        fprintf(stderr, "[Lungs] Error: 큐 오버플로우\n");
        return -1;
    }

    /* 요청 생성 */
    io_request_t* req = &lungs->queue[lungs->head];
    req->fd = fd;
    req->type = IO_READ;
    req->size = LUNGS_BUFFER_SIZE;
    req->buffer = malloc(LUNGS_BUFFER_SIZE);
    if (!req->buffer) {
        pthread_mutex_unlock(&lungs->lock);
        return -1;
    }
    req->callback = callback;
    req->user_data = user_data;
    req->queued_at = get_timestamp_us();
    req->priority = 0;

    lungs->head = (lungs->head + 1) % lungs->capacity;
    lungs->count++;

    if (lungs->count > lungs->queue_depth_max) {
        lungs->queue_depth_max = lungs->count;
    }

    /* FD를 non-blocking으로 설정 */
    set_nonblocking(fd);

    pthread_cond_signal(&lungs->has_work);
    pthread_mutex_unlock(&lungs->lock);

    return 0;
}

int lungs_async_write(lungs_t* lungs, int fd,
                      const void* data, size_t len,
                      void (*callback)(int, void*, ssize_t, void*),
                      void* user_data) {
    if (!lungs || fd < 0 || !data || len == 0) return -1;

    pthread_mutex_lock(&lungs->lock);

    if (lungs->count >= lungs->capacity) {
        pthread_mutex_unlock(&lungs->lock);
        fprintf(stderr, "[Lungs] Error: 큐 오버플로우\n");
        return -1;
    }

    /* 요청 생성 */
    io_request_t* req = &lungs->queue[lungs->head];
    req->fd = fd;
    req->type = IO_WRITE;
    req->size = len;
    req->buffer = malloc(len);
    if (!req->buffer) {
        pthread_mutex_unlock(&lungs->lock);
        return -1;
    }
    memcpy(req->buffer, data, len);
    req->callback = callback;
    req->user_data = user_data;
    req->queued_at = get_timestamp_us();
    req->priority = 0;

    lungs->head = (lungs->head + 1) % lungs->capacity;
    lungs->count++;

    if (lungs->count > lungs->queue_depth_max) {
        lungs->queue_depth_max = lungs->count;
    }

    /* FD를 non-blocking으로 설정 */
    set_nonblocking(fd);

    pthread_cond_signal(&lungs->has_work);
    pthread_mutex_unlock(&lungs->lock);

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Monitoring
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int lungs_get_queue_depth(const lungs_t* lungs) {
    if (!lungs) return -1;
    return (int)lungs->count;
}

int lungs_get_queue_percent(const lungs_t* lungs) {
    if (!lungs) return 0;
    return (int)((lungs->count * 100) / lungs->capacity);
}

void lungs_stats(const lungs_t* lungs) {
    if (!lungs) return;

    pthread_mutex_lock((pthread_mutex_t*)&lungs->lock);

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║      폐 (Lungs) I/O 통계                ║\n");
    printf("╚════════════════════════════════════════╝\n");

    int queue_percent = lungs_get_queue_percent(lungs);

    printf("📊 I/O 작업:\n");
    printf("  읽기: %lu회, 쓰기: %lu회\n", lungs->total_reads, lungs->total_writes);
    printf("  입력: %lu bytes, 출력: %lu bytes\n", lungs->bytes_in, lungs->bytes_out);

    printf("\n🔄 큐 상태:\n");
    printf("  현재: %u / %u (%d%%)\n", lungs->count, lungs->capacity, queue_percent);
    printf("  최대 깊이: %lu\n", lungs->queue_depth_max);

    printf("\n⚙️  성능:\n");
    printf("  워커: %d개\n", lungs->num_workers);
    printf("  평균 지연: %lu us\n", lungs->avg_latency_us);

    printf("\n═════════════════════════════════════════\n\n");

    pthread_mutex_unlock((pthread_mutex_t*)&lungs->lock);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Spine Integration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void lungs_set_spine(lungs_t* lungs, spine_t* spine, int organ_id) {
    if (!lungs) return;

    pthread_mutex_lock(&lungs->lock);
    lungs->spine = spine;
    lungs->organ_id = organ_id;
    pthread_mutex_unlock(&lungs->lock);
}

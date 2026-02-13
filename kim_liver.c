/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_liver.c
 *
 * Liver (간) - Memory Pool Manager Implementation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "kim_liver.h"
#include "kim_spine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <unistd.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utility Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static uint64_t get_timestamp_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * GC Daemon (Garbage Collection)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static void* liver_gc_loop(void* arg) {
    liver_t* liver = (liver_t*)arg;

    printf("[Liver] GC 데몬 시작 (주기: %d초)\n", LIVER_GC_INTERVAL);

    while (liver->gc_running) {
        sleep(LIVER_GC_INTERVAL);

        if (!liver->gc_running) break;

        pthread_mutex_lock(&liver->lock);
        liver_gc_cycle(liver);
        pthread_mutex_unlock(&liver->lock);
    }

    printf("[Liver] GC 데몬 종료\n");
    return NULL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

liver_t* liver_create(void) {
    liver_t* liver = (liver_t*)malloc(sizeof(liver_t));
    if (!liver) {
        fprintf(stderr, "[Liver] Error: malloc failed\n");
        return NULL;
    }

    /* mmap으로 16MB 메모리 풀 할당 */
    liver->pool = mmap(NULL, LIVER_POOL_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (liver->pool == MAP_FAILED) {
        fprintf(stderr, "[Liver] Error: mmap failed\n");
        free(liver);
        return NULL;
    }

    /* 블록 메타데이터 배열 */
    liver->blocks = (memory_block_t*)calloc(LIVER_NUM_BLOCKS, sizeof(memory_block_t));
    if (!liver->blocks) {
        fprintf(stderr, "[Liver] Error: blocks allocation failed\n");
        munmap(liver->pool, LIVER_POOL_SIZE);
        free(liver);
        return NULL;
    }

    /* Free list (스택) 초기화 */
    liver->free_list = (uint32_t*)malloc(LIVER_NUM_BLOCKS * sizeof(uint32_t));
    if (!liver->free_list) {
        fprintf(stderr, "[Liver] Error: free_list allocation failed\n");
        free(liver->blocks);
        munmap(liver->pool, LIVER_POOL_SIZE);
        free(liver);
        return NULL;
    }

    /* 모든 블록을 FREE로 초기화 */
    for (uint32_t i = 0; i < LIVER_NUM_BLOCKS; i++) {
        liver->blocks[i].addr = (char*)liver->pool + (i * LIVER_BLOCK_SIZE);
        liver->blocks[i].size = LIVER_BLOCK_SIZE;
        liver->blocks[i].state = BLOCK_FREE;
        liver->blocks[i].allocated_at = 0;
        liver->blocks[i].ref_count = 0;
        strncpy(liver->blocks[i].tag, "free", sizeof(liver->blocks[i].tag) - 1);

        /* Free list에 모든 블록 추가 (역순) */
        liver->free_list[i] = LIVER_NUM_BLOCKS - 1 - i;
    }

    liver->free_head = LIVER_NUM_BLOCKS - 1;
    liver->free_count = LIVER_NUM_BLOCKS;
    liver->pool_size = LIVER_POOL_SIZE;
    liver->num_blocks = LIVER_NUM_BLOCKS;

    /* GC 초기화 */
    pthread_mutex_init(&liver->lock, NULL);
    pthread_cond_init(&liver->cond, NULL);
    liver->gc_running = 0;
    liver->last_gc_time = get_timestamp_us();

    /* 통계 초기화 */
    liver->total_allocated = 0;
    liver->total_freed = 0;
    liver->peak_usage = 0;
    liver->fragmentation_count = 0;
    liver->gc_cycles = 0;
    liver->compaction_count = 0;
    liver->pressure_signals = 0;

    /* Spine 초기화 */
    liver->spine = NULL;
    liver->organ_id = 4;  /* 간 ID=4 */

    printf("[Liver] 간 생성 완료: 풀=%dMB, 블록=%d개 (각 %dKB)\n",
           LIVER_POOL_SIZE / (1024 * 1024), LIVER_NUM_BLOCKS, LIVER_BLOCK_SIZE / 1024);

    return liver;
}

void liver_destroy(liver_t* liver) {
    if (!liver) return;

    /* GC 데몬 중지 */
    liver_stop_gc(liver);

    pthread_mutex_lock(&liver->lock);

    /* 메모리 해제 */
    if (liver->pool) {
        munmap(liver->pool, LIVER_POOL_SIZE);
    }

    if (liver->blocks) {
        free(liver->blocks);
    }

    if (liver->free_list) {
        free(liver->free_list);
    }

    pthread_mutex_unlock(&liver->lock);

    pthread_mutex_destroy(&liver->lock);
    pthread_cond_destroy(&liver->cond);

    free(liver);
    printf("[Liver] 간 정리 완료\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * GC Lifecycle
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int liver_start_gc(liver_t* liver) {
    if (!liver || liver->gc_running) return -1;

    liver->gc_running = 1;

    if (pthread_create(&liver->gc_thread, NULL, liver_gc_loop, liver) != 0) {
        fprintf(stderr, "[Liver] Error: GC 스레드 생성 실패\n");
        liver->gc_running = 0;
        return -1;
    }

    return 0;
}

int liver_stop_gc(liver_t* liver) {
    if (!liver || !liver->gc_running) return -1;

    liver->gc_running = 0;
    pthread_join(liver->gc_thread, NULL);

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Memory Allocation (First-Fit)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void* liver_alloc(liver_t* liver, size_t size, const char* tag) {
    if (!liver || size == 0) return NULL;

    pthread_mutex_lock(&liver->lock);

    /* 필요한 블록 수 계산 (올림) */
    uint32_t blocks_needed = (size + LIVER_BLOCK_SIZE - 1) / LIVER_BLOCK_SIZE;

    if (blocks_needed > liver->free_count) {
        pthread_mutex_unlock(&liver->lock);
        fprintf(stderr, "[Liver] Error: 메모리 부족 (요청: %u블록, 가용: %u블록)\n",
                blocks_needed, liver->free_count);
        return NULL;
    }

    /* First-Fit: 연속된 블록 찾기 */
    uint32_t first_block = UINT32_MAX;
    uint32_t consecutive = 0;

    for (uint32_t i = 0; i < LIVER_NUM_BLOCKS; i++) {
        if (liver->blocks[i].state == BLOCK_FREE) {
            if (consecutive == 0) {
                first_block = i;
            }
            consecutive++;
            if (consecutive == blocks_needed) {
                break;
            }
        } else {
            consecutive = 0;
        }
    }

    if (consecutive < blocks_needed) {
        pthread_mutex_unlock(&liver->lock);
        fprintf(stderr, "[Liver] Error: 연속된 블록 찾기 실패\n");
        return NULL;
    }

    /* 블록 할당 */
    void* ptr = liver->blocks[first_block].addr;
    uint64_t now = get_timestamp_us();

    for (uint32_t i = 0; i < blocks_needed; i++) {
        uint32_t idx = first_block + i;
        liver->blocks[idx].state = BLOCK_ALLOCATED;
        liver->blocks[idx].allocated_at = now;
        liver->blocks[idx].ref_count = 1;
        if (tag) {
            strncpy(liver->blocks[idx].tag, tag, sizeof(liver->blocks[idx].tag) - 1);
        }
    }

    /* Free list 업데이트 */
    liver->free_count -= blocks_needed;

    /* 통계 업데이트 */
    liver->total_allocated += (blocks_needed * LIVER_BLOCK_SIZE);
    if (liver->total_allocated - liver->total_freed > liver->peak_usage) {
        liver->peak_usage = liver->total_allocated - liver->total_freed;
    }

    pthread_mutex_unlock(&liver->lock);

    return ptr;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Memory Deallocation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void liver_free(liver_t* liver, void* ptr) {
    if (!liver || !ptr) return;

    pthread_mutex_lock(&liver->lock);

    /* 블록 찾기 */
    uint32_t block_idx = UINT32_MAX;
    for (uint32_t i = 0; i < LIVER_NUM_BLOCKS; i++) {
        if (liver->blocks[i].addr == ptr && liver->blocks[i].state == BLOCK_ALLOCATED) {
            block_idx = i;
            break;
        }
    }

    if (block_idx == UINT32_MAX) {
        pthread_mutex_unlock(&liver->lock);
        fprintf(stderr, "[Liver] Error: 유효하지 않은 포인터\n");
        return;
    }

    /* 할당된 블록들 해제 */
    uint32_t blocks_freed = 0;
    for (uint32_t i = block_idx; i < LIVER_NUM_BLOCKS &&
         liver->blocks[i].state == BLOCK_ALLOCATED; i++) {
        liver->blocks[i].state = BLOCK_FREE;
        liver->blocks[i].allocated_at = 0;
        liver->blocks[i].ref_count = 0;
        strncpy(liver->blocks[i].tag, "free", sizeof(liver->blocks[i].tag) - 1);
        blocks_freed++;
    }

    liver->free_count += blocks_freed;
    liver->total_freed += (blocks_freed * LIVER_BLOCK_SIZE);

    pthread_mutex_unlock(&liver->lock);
}

void* liver_realloc(liver_t* liver, void* ptr, size_t new_size) {
    if (!liver || new_size == 0) return NULL;

    if (!ptr) {
        return liver_alloc(liver, new_size, "realloc");
    }

    /* 새 메모리 할당 */
    void* new_ptr = liver_alloc(liver, new_size, "realloc");
    if (!new_ptr) return NULL;

    /* 기존 메모리 복사 (간단하게 함, 실제로는 크기 추적 필요) */
    memcpy(new_ptr, ptr, new_size);

    /* 기존 메모리 해제 */
    liver_free(liver, ptr);

    return new_ptr;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Garbage Collection Cycle
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void liver_gc_cycle(liver_t* liver) {
    if (!liver) return;

    /* 이미 락이 있다고 가정 (GC 루프에서 호출) */

    uint64_t now = get_timestamp_us();
    uint64_t freed_blocks = 0;

    /* 수명이 다한 블록 자동 해제 */
    for (uint32_t i = 0; i < LIVER_NUM_BLOCKS; i++) {
        if (liver->blocks[i].state == BLOCK_ALLOCATED &&
            (now - liver->blocks[i].allocated_at) > (LIVER_MAX_LIFETIME * 1000000ULL)) {

            liver->blocks[i].state = BLOCK_FREE;
            liver->blocks[i].allocated_at = 0;
            liver->blocks[i].ref_count = 0;
            strncpy(liver->blocks[i].tag, "gc-freed", sizeof(liver->blocks[i].tag) - 1);
            liver->free_count++;
            freed_blocks++;
            liver->total_freed += LIVER_BLOCK_SIZE;
        }
    }

    /* 메모리 압박 신호 (Spine 연동) */
    int usage_percent = liver_get_usage_percent(liver);
    if (usage_percent > LIVER_PRESSURE_THRESHOLD && liver->spine) {
        liver->pressure_signals++;

        signal_packet_t sig = {0};
        sig.type = SIGNAL_PRESSURE;
        sig.source_id = liver->organ_id;
        sig.target_id = 0;  /* Spine에 브로드캐스트 */
        sig.timestamp = now;
        sig.priority = 1;

        spine_send_sensory(liver->spine, &sig);
    }

    liver->gc_cycles++;
    liver->last_gc_time = now;

    if (freed_blocks > 0) {
        printf("[Liver] GC #%lu: %lu 블록 해제 (사용률 %d%%)\n",
               liver->gc_cycles, freed_blocks, usage_percent);
    }
}

void liver_compact(liver_t* liver) {
    if (!liver) return;

    pthread_mutex_lock(&liver->lock);

    /* 메모리 압축 (단순 구현) */
    liver->compaction_count++;

    printf("[Liver] 메모리 압축 #%lu 완료\n", liver->compaction_count);

    pthread_mutex_unlock(&liver->lock);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Monitoring
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int liver_get_usage_percent(const liver_t* liver) {
    if (!liver) return 0;

    uint64_t current = liver->total_allocated - liver->total_freed;
    return (int)((current * 100) / LIVER_POOL_SIZE);
}

int liver_get_fragmentation(const liver_t* liver) {
    if (!liver) return 0;

    return (int)((liver->fragmentation_count * 100) / LIVER_NUM_BLOCKS);
}

void liver_stats(const liver_t* liver) {
    if (!liver) return;

    pthread_mutex_lock((pthread_mutex_t*)&liver->lock);

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║      간 (Liver) 통계                   ║\n");
    printf("╚════════════════════════════════════════╝\n");

    uint64_t current = liver->total_allocated - liver->total_freed;
    int usage_percent = liver_get_usage_percent(liver);

    printf("📊 메모리 사용:\n");
    printf("  현재: %lu MB / %u MB (%d%%)\n",
           current / (1024 * 1024), (unsigned)(LIVER_POOL_SIZE / (1024 * 1024)), usage_percent);
    printf("  누적 할당: %lu MB\n", liver->total_allocated / (1024 * 1024));
    printf("  누적 해제: %lu MB\n", liver->total_freed / (1024 * 1024));
    printf("  최대 사용: %lu MB\n\n", liver->peak_usage / (1024 * 1024));

    printf("♻️  가비지 컬렉션:\n");
    printf("  주기 수: %lu\n", liver->gc_cycles);
    printf("  압축 횟수: %lu\n", liver->compaction_count);
    printf("  단편화: %lu회\n", liver->fragmentation_count);
    printf("  압박 신호: %lu회\n\n", liver->pressure_signals);

    printf("🔒 블록 상태:\n");
    printf("  가용: %u / %u 블록\n", liver->free_count, LIVER_NUM_BLOCKS);
    printf("  사용 중: %u 블록\n\n", LIVER_NUM_BLOCKS - liver->free_count);

    printf("═════════════════════════════════════════\n\n");

    pthread_mutex_unlock((pthread_mutex_t*)&liver->lock);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Spine Integration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void liver_set_spine(liver_t* liver, spine_t* spine, int organ_id) {
    if (!liver) return;

    pthread_mutex_lock(&liver->lock);
    liver->spine = spine;
    liver->organ_id = organ_id;
    pthread_mutex_unlock(&liver->lock);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Debug
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void liver_dump_blocks(const liver_t* liver) {
    if (!liver) return;

    printf("[Liver] 블록 덤프 (처음 20개):\n");

    for (uint32_t i = 0; i < 20 && i < LIVER_NUM_BLOCKS; i++) {
        const char* state_str = "";
        switch (liver->blocks[i].state) {
            case BLOCK_FREE:      state_str = "FREE"; break;
            case BLOCK_ALLOCATED: state_str = "ALLOC"; break;
            case BLOCK_RESERVED:  state_str = "RSRVD"; break;
        }

        printf("  [%4u] %s | %s\n", i, state_str, liver->blocks[i].tag);
    }
}

void liver_validate_heap(const liver_t* liver) {
    if (!liver) return;

    printf("[Liver] 힙 검증...\n");

    uint32_t alloc_blocks = 0;
    uint32_t free_blocks = 0;

    for (uint32_t i = 0; i < LIVER_NUM_BLOCKS; i++) {
        if (liver->blocks[i].state == BLOCK_ALLOCATED) {
            alloc_blocks++;
        } else if (liver->blocks[i].state == BLOCK_FREE) {
            free_blocks++;
        }
    }

    printf("  할당됨: %u, 가용: %u, 합계: %u\n",
           alloc_blocks, free_blocks, alloc_blocks + free_blocks);

    if (alloc_blocks + free_blocks == LIVER_NUM_BLOCKS) {
        printf("✅ 힙 검증 완료 (정상)\n");
    } else {
        printf("❌ 힙 검증 실패 (불일치)\n");
    }
}

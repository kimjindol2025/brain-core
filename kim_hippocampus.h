/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_hippocampus.h
 *
 * Hippocampus (해마) - Long-Term Memory System
 *
 * 생물학적 역할:
 *   - 단기 기억 → 장기 기억 전환 (Memory Consolidation)
 *   - 중요도 기반 선택적 저장
 *   - 수면 중 기억 공고화 (Dream)
 *   - 연관 기억 검색 (Associative Recall)
 *
 * 소프트웨어 역할:
 *   - mmap 기반 영구 저장소 (brain_longterm.db)
 *   - HNSW 벡터 유사도 검색 (O(log N))
 *   - Cortex 통합 (자동 저장/검색)
 *   - Circadian 통합 (DAWN 시간에 consolidation)
 *   - Spine IPC 신호 (SIGNAL_MEMORY_*)
 *
 * Priority: ★3★ (High - 학습 시스템 핵심)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_HIPPOCAMPUS_H
#define KIM_HIPPOCAMPUS_H

#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include "kim_spine.h"

/* Forward declarations */
/* cortex_t is defined elsewhere, use void* in this header to avoid conflicts */
typedef struct mmap_file_t mmap_file_t;
typedef struct brain_index_t brain_index_t;
typedef struct hnsw_index_t hnsw_index_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Configuration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define HIPPO_DB_PATH                  "./brain_longterm.db"
#define HIPPO_IMPORTANCE_THRESHOLD     0.7f       /* 70% 이상만 저장 */
#define HIPPO_MAX_MEMORIES             10000      /* 최대 10,000개 기억 */
#define HIPPO_VECTOR_DIM               128        /* 128차원 벡터 */
#define HIPPO_CONSOLIDATE_INTERVAL     3600       /* 1시간마다 consolidation */
#define HIPPO_PRUNE_DAYS               7          /* 7일 이상 미접근 정리 */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Data Structures
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Memory Entry (저장 단위 - 624 bytes) */
typedef struct {
    uint64_t  id;                         /* 기억 ID (타임스탬프) */
    uint64_t  timestamp;                  /* 저장 시각 (microseconds) */
    float     importance;                 /* 중요도 (0.0 ~ 1.0) */
    float     vector[HIPPO_VECTOR_DIM];   /* 의미 벡터 (128 float) */
    char      content[256];               /* 기억 내용 (텍스트) */
    uint32_t  access_count;               /* 접근 횟수 */
    uint64_t  last_accessed;              /* 마지막 접근 시각 */
    uint32_t  _padding;                   /* 정렬 */
} memory_entry_t;

/* Hippocampus Organ */
typedef struct {
    /* Persistent Storage */
    mmap_file_t*     brain_file;          /* mmap 파일 핸들 */
    brain_index_t*   index;               /* ID → Offset 매핑 */
    hnsw_index_t*    similarity_index;    /* HNSW 벡터 검색 인덱스 */

    /* Policy & Limits */
    float            importance_threshold; /* 저장 임계값 */
    uint32_t         max_memories;        /* 메모리 한계 */
    uint32_t         current_count;       /* 현재 저장된 수 */

    /* Integration */
    spine_t*         spine;               /* Spine IPC (신호 전송) */
    void*            cortex;              /* Cortex 참조 (검색 시) - opaque pointer */
    int              organ_id;            /* Organ ID (=6) */

    /* Dream Thread (Background Consolidation) */
    pthread_t        dream_thread;        /* Consolidation 워커 스레드 */
    int              dreaming;            /* 동작 플래그 */
    uint64_t         last_consolidation;  /* 마지막 consolidation 시각 */

    /* Thread Safety */
    pthread_mutex_t  lock;                /* 뮤텍스 */

    /* Statistics */
    uint64_t total_stored;                /* 총 저장 횟수 */
    uint64_t total_retrieved;             /* 총 검색 횟수 */
    uint64_t total_consolidated;          /* Consolidation 주기 수 */
    uint64_t total_pruned;                /* 정리된 기억 수 */
    uint64_t peak_usage;                  /* 최대 사용량 */
} hippocampus_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Core API - Lifecycle
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Create Hippocampus organ with mmap storage */
hippocampus_t* hippocampus_create(const char* db_path);

/* Destroy Hippocampus and close mmap file */
void           hippocampus_destroy(hippocampus_t* hippo);

/* Start dream thread for background consolidation */
int            hippocampus_start_dream(hippocampus_t* hippo);

/* Stop dream thread and save state */
int            hippocampus_stop_dream(hippocampus_t* hippo);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Memory Operations
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Store memory if importance >= threshold */
int hippocampus_store(hippocampus_t* hippo,
                      const char* content,
                      const float* vector,
                      float importance);

/* Retrieve top-k similar memories using HNSW */
memory_entry_t** hippocampus_retrieve(hippocampus_t* hippo,
                                       const float* query_vector,
                                       int top_k);

/* Consolidate and prune old memories (dream function) */
void hippocampus_consolidate(hippocampus_t* hippo);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Integration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Connect Hippocampus to Spine for signaling */
void hippocampus_set_spine(hippocampus_t* hippo, spine_t* spine, int organ_id);

/* Connect Hippocampus to Cortex for integrated operation */
void hippocampus_set_cortex(hippocampus_t* hippo, void* cortex);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Monitoring
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Print statistics */
void hippocampus_stats(const hippocampus_t* hippo);

/* Get usage percentage (0-100) */
int  hippocampus_get_usage_percent(const hippocampus_t* hippo);

/* Get current memory count */
uint32_t hippocampus_get_count(const hippocampus_t* hippo);

#endif /* KIM_HIPPOCAMPUS_H */

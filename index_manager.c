/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * index_manager.c
 *
 * ID → Offset 해시맵 (Open Addressing with Linear Probing)
 *
 * 목적:
 *   - 벡터 ID를 주면 파일 내 오프셋 리턴
 *   - O(1) 평균 검색 시간
 *   - mmap 영역에 직접 구현 (Zero-Copy)
 *
 * 해시 충돌 해결:
 *   - Linear Probing (선형 탐사)
 *   - 빈 슬롯 찾을 때까지 다음 버킷 확인
 *
 * 구조:
 *   brain_index_entry_t 배열 (BRAIN_INDEX_BUCKETS 크기)
 *   - vector_id = -1: 빈 슬롯
 *   - vector_id >= 0: 사용 중
 *
 * Zero Dependency: 표준 C만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "index_manager.h"
#include "brain_format.h"
#include <stdio.h>
#include <string.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 해시 함수 (FNV-1a variant)
 *
 * 간단하고 빠른 해시 함수
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static uint32_t hash_id(int64_t id) {
    /* FNV-1a 64비트 → 32비트 */
    uint64_t h = 14695981039346656037ULL;  /* FNV offset basis */
    const uint64_t prime = 1099511628211ULL;

    /* ID를 바이트로 분해하여 해시 */
    for (int i = 0; i < 8; i++) {
        h ^= (id >> (i * 8)) & 0xFF;
        h *= prime;
    }

    return (uint32_t)(h % BRAIN_INDEX_BUCKETS);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * index_init
 *
 * Index 테이블 초기화 (모든 슬롯을 -1로 설정)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void index_init(brain_index_entry_t* index_table) {
    if (!index_table) return;

    for (int i = 0; i < BRAIN_INDEX_BUCKETS; i++) {
        index_table[i].vector_id = -1;      /* 빈 슬롯 */
        index_table[i].data_offset = 0;
    }

    printf("[index] ✓ Initialized %d buckets\n", BRAIN_INDEX_BUCKETS);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * index_insert
 *
 * ID와 오프셋을 Index에 삽입
 *
 * 리턴:
 *    0: 성공
 *   -1: 테이블 가득 찼음
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int index_insert(brain_index_entry_t* index_table, int64_t id, uint64_t offset) {
    if (!index_table || id < 0) return -1;

    uint32_t bucket = hash_id(id);
    uint32_t start_bucket = bucket;

    /* Linear Probing: 빈 슬롯 찾기 */
    while (1) {
        brain_index_entry_t* entry = &index_table[bucket];

        /* 빈 슬롯 발견 */
        if (entry->vector_id == -1) {
            entry->vector_id = id;
            entry->data_offset = offset;
            return 0;
        }

        /* 이미 같은 ID 존재 (업데이트) */
        if (entry->vector_id == id) {
            entry->data_offset = offset;
            return 0;
        }

        /* 다음 버킷 */
        bucket = (bucket + 1) % BRAIN_INDEX_BUCKETS;

        /* 한 바퀴 돌았음 = 테이블 가득 참 */
        if (bucket == start_bucket) {
            fprintf(stderr, "[index] Error: hash table full\n");
            return -1;
        }
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * index_lookup
 *
 * ID로 오프셋 검색
 *
 * 리턴:
 *   - 성공: 오프셋 (>= 0)
 *   - 실패: -1 (찾지 못함)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int64_t index_lookup(const brain_index_entry_t* index_table, int64_t id) {
    if (!index_table || id < 0) return -1;

    uint32_t bucket = hash_id(id);
    uint32_t start_bucket = bucket;

    /* Linear Probing: ID 찾기 */
    while (1) {
        const brain_index_entry_t* entry = &index_table[bucket];

        /* 빈 슬롯 = 찾지 못함 */
        if (entry->vector_id == -1) {
            return -1;
        }

        /* ID 일치 */
        if (entry->vector_id == id) {
            return entry->data_offset;
        }

        /* 다음 버킷 */
        bucket = (bucket + 1) % BRAIN_INDEX_BUCKETS;

        /* 한 바퀴 돌았음 = 찾지 못함 */
        if (bucket == start_bucket) {
            return -1;
        }
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * index_delete
 *
 * ID를 Index에서 삭제
 *
 * 주의:
 *   - Linear Probing에서 중간 삭제는 복잡함
 *   - 여기서는 단순히 -1로 표시 (tombstone)
 *   - 실제로는 재해시 필요할 수 있음
 *
 * 리턴:
 *    0: 성공
 *   -1: 찾지 못함
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int index_delete(brain_index_entry_t* index_table, int64_t id) {
    if (!index_table || id < 0) return -1;

    uint32_t bucket = hash_id(id);
    uint32_t start_bucket = bucket;

    /* Linear Probing: ID 찾기 */
    while (1) {
        brain_index_entry_t* entry = &index_table[bucket];

        /* 빈 슬롯 = 찾지 못함 */
        if (entry->vector_id == -1) {
            return -1;
        }

        /* ID 일치 */
        if (entry->vector_id == id) {
            entry->vector_id = -1;  /* 삭제 표시 */
            entry->data_offset = 0;
            return 0;
        }

        /* 다음 버킷 */
        bucket = (bucket + 1) % BRAIN_INDEX_BUCKETS;

        /* 한 바퀴 돌았음 = 찾지 못함 */
        if (bucket == start_bucket) {
            return -1;
        }
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * index_stats
 *
 * Index 통계 출력 (디버깅용)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void index_stats(const brain_index_entry_t* index_table) {
    if (!index_table) return;

    int used = 0;
    int empty = 0;

    for (int i = 0; i < BRAIN_INDEX_BUCKETS; i++) {
        if (index_table[i].vector_id >= 0) {
            used++;
        } else {
            empty++;
        }
    }

    float load_factor = (float)used / BRAIN_INDEX_BUCKETS;

    printf("[index] Statistics:\n");
    printf("  Total buckets: %d\n", BRAIN_INDEX_BUCKETS);
    printf("  Used:          %d\n", used);
    printf("  Empty:         %d\n", empty);
    printf("  Load factor:   %.2f%%\n", load_factor * 100);

    if (load_factor > 0.7) {
        printf("  ⚠️  Warning: High load factor (>70%%), consider resize\n");
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * index_dump
 *
 * Index 내용 덤프 (디버깅용, 처음 N개만)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void index_dump(const brain_index_entry_t* index_table, int max_entries) {
    if (!index_table) return;

    printf("[index] Dump (first %d used entries):\n", max_entries);

    int printed = 0;
    for (int i = 0; i < BRAIN_INDEX_BUCKETS && printed < max_entries; i++) {
        if (index_table[i].vector_id >= 0) {
            printf("  [%5d] ID=%ld → offset=%lu\n",
                   i,
                   index_table[i].vector_id,
                   index_table[i].data_offset);
            printed++;
        }
    }
}

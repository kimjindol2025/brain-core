/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * hnsw.h
 *
 * Hierarchical Navigable Small World (HNSW)
 *
 * 목적:
 *   - 고차원 벡터 유사도 검색 (Top-K)
 *   - O(log N) 평균 검색 시간
 *   - 계층적 그래프 구조
 *
 * 알고리즘:
 *   - Layer 0: 모든 노드
 *   - Layer 1+: 점점 희소한 서브샘플
 *   - 위에서 아래로 탐색 (greedy best-first)
 *
 * 참고:
 *   - "Efficient and robust approximate nearest neighbor search
 *      using Hierarchical Navigable Small World graphs"
 *   - Malkov & Yashunin, 2018
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef HNSW_H
#define HNSW_H

#include <stdint.h>
#include <stddef.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Configuration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define HNSW_MAX_LAYERS         16      /* 최대 계층 수 */
#define HNSW_M                  16      /* Layer 0의 이웃 수 */
#define HNSW_M_MAX              32      /* Layer 0+ 이웃 수 */
#define HNSW_EF_CONSTRUCTION    200     /* 구축 시 탐색 범위 */
#define HNSW_EF_SEARCH          50      /* 검색 시 탐색 범위 */
#define HNSW_ML                 (1.0 / log(2.0))  /* 계층 확률 */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Data Structures
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* HNSW 노드 */
typedef struct {
    int64_t  id;                        /* Vector ID */
    float*   vector;                    /* Embedding (dim floats) */
    uint32_t layer;                     /* 이 노드의 최대 계층 */
    uint32_t neighbor_count[HNSW_MAX_LAYERS];  /* 각 계층의 이웃 수 */
    int64_t* neighbors[HNSW_MAX_LAYERS];        /* 각 계층의 이웃 배열 */
} hnsw_node_t;

/* HNSW 인덱스 */
typedef struct {
    uint32_t     dim;                   /* 벡터 차원 */
    uint32_t     count;                 /* 총 노드 수 */
    uint32_t     capacity;              /* 할당된 크기 */
    uint32_t     max_layer;             /* 현재 최대 계층 */
    int64_t      entry_point;           /* 진입점 노드 ID */
    hnsw_node_t* nodes;                 /* 노드 배열 */

    /* Search parameters */
    uint32_t     ef_construction;
    uint32_t     ef_search;
    uint32_t     M;
    uint32_t     M_max;
} hnsw_index_t;

/* 검색 결과 */
typedef struct {
    int64_t id;
    float   distance;
} hnsw_result_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 인덱스 생성/삭제 */
hnsw_index_t* hnsw_create(uint32_t dim, uint32_t initial_capacity);
void          hnsw_destroy(hnsw_index_t* index);

/* 벡터 삽입 */
int hnsw_insert(hnsw_index_t* index, int64_t id, const float* vector);

/* Top-K 검색 */
int hnsw_search(
    const hnsw_index_t* index,
    const float* query,
    uint32_t k,
    hnsw_result_t* results
);

/* 거리 계산 */
float hnsw_distance(const float* a, const float* b, uint32_t dim);

/* 통계 */
void hnsw_stats(const hnsw_index_t* index);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Priority Queue (Internal)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    int64_t id;
    float   priority;
} pq_item_t;

typedef struct {
    pq_item_t* items;
    uint32_t   size;
    uint32_t   capacity;
} priority_queue_t;

priority_queue_t* pq_create(uint32_t capacity);
void              pq_destroy(priority_queue_t* pq);
void              pq_push(priority_queue_t* pq, int64_t id, float priority);
pq_item_t         pq_pop(priority_queue_t* pq);
pq_item_t*        pq_peek(priority_queue_t* pq);
int               pq_empty(const priority_queue_t* pq);

#endif /* HNSW_H */

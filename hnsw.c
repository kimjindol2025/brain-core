/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * hnsw.c
 *
 * HNSW Implementation
 *
 * Zero Dependency: stdlib + math.h만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "hnsw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Cosine Distance (1 - Cosine Similarity)
 *
 * 거리가 작을수록 유사함
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
float hnsw_distance(const float* a, const float* b, uint32_t dim) {
    float dot = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;

    for (uint32_t i = 0; i < dim; i++) {
        dot += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }

    if (norm_a == 0.0f || norm_b == 0.0f) {
        return 1.0f;  /* Undefined → 최대 거리 */
    }

    float cosine = dot / (sqrtf(norm_a) * sqrtf(norm_b));
    return 1.0f - cosine;  /* [0, 2] 범위 */
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Priority Queue (Min-Heap)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
priority_queue_t* pq_create(uint32_t capacity) {
    priority_queue_t* pq = (priority_queue_t*)malloc(sizeof(priority_queue_t));
    pq->items = (pq_item_t*)malloc(capacity * sizeof(pq_item_t));
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void pq_destroy(priority_queue_t* pq) {
    if (pq) {
        free(pq->items);
        free(pq);
    }
}

static void pq_swap(pq_item_t* a, pq_item_t* b) {
    pq_item_t temp = *a;
    *a = *b;
    *b = temp;
}

static void pq_heapify_up(priority_queue_t* pq, uint32_t idx) {
    while (idx > 0) {
        uint32_t parent = (idx - 1) / 2;
        if (pq->items[idx].priority >= pq->items[parent].priority) break;
        pq_swap(&pq->items[idx], &pq->items[parent]);
        idx = parent;
    }
}

static void pq_heapify_down(priority_queue_t* pq, uint32_t idx) {
    while (1) {
        uint32_t left = 2 * idx + 1;
        uint32_t right = 2 * idx + 2;
        uint32_t smallest = idx;

        if (left < pq->size && pq->items[left].priority < pq->items[smallest].priority) {
            smallest = left;
        }
        if (right < pq->size && pq->items[right].priority < pq->items[smallest].priority) {
            smallest = right;
        }

        if (smallest == idx) break;

        pq_swap(&pq->items[idx], &pq->items[smallest]);
        idx = smallest;
    }
}

void pq_push(priority_queue_t* pq, int64_t id, float priority) {
    if (pq->size >= pq->capacity) {
        /* 용량 초과 → 확장 */
        pq->capacity *= 2;
        pq->items = (pq_item_t*)realloc(pq->items, pq->capacity * sizeof(pq_item_t));
    }

    pq->items[pq->size].id = id;
    pq->items[pq->size].priority = priority;
    pq_heapify_up(pq, pq->size);
    pq->size++;
}

pq_item_t pq_pop(priority_queue_t* pq) {
    pq_item_t result = pq->items[0];
    pq->size--;
    if (pq->size > 0) {
        pq->items[0] = pq->items[pq->size];
        pq_heapify_down(pq, 0);
    }
    return result;
}

pq_item_t* pq_peek(priority_queue_t* pq) {
    return (pq->size > 0) ? &pq->items[0] : NULL;
}

int pq_empty(const priority_queue_t* pq) {
    return pq->size == 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Random Layer Selection
 *
 * 확률적으로 계층 선택 (지수 분포)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static uint32_t select_layer(void) {
    double r = (double)rand() / RAND_MAX;
    uint32_t layer = (uint32_t)(-log(r) * HNSW_ML);
    if (layer >= HNSW_MAX_LAYERS) layer = HNSW_MAX_LAYERS - 1;
    return layer;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * HNSW Index
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
hnsw_index_t* hnsw_create(uint32_t dim, uint32_t initial_capacity) {
    hnsw_index_t* index = (hnsw_index_t*)malloc(sizeof(hnsw_index_t));

    index->dim = dim;
    index->count = 0;
    index->capacity = initial_capacity;
    index->max_layer = 0;
    index->entry_point = -1;
    index->nodes = (hnsw_node_t*)calloc(initial_capacity, sizeof(hnsw_node_t));

    index->ef_construction = HNSW_EF_CONSTRUCTION;
    index->ef_search = HNSW_EF_SEARCH;
    index->M = HNSW_M;
    index->M_max = HNSW_M_MAX;

    /* 노드 초기화 */
    for (uint32_t i = 0; i < initial_capacity; i++) {
        index->nodes[i].id = -1;  /* Empty */
        index->nodes[i].vector = NULL;
        index->nodes[i].layer = 0;
        for (uint32_t l = 0; l < HNSW_MAX_LAYERS; l++) {
            index->nodes[i].neighbor_count[l] = 0;
            index->nodes[i].neighbors[l] = NULL;
        }
    }

    printf("[hnsw] Created index: dim=%u, capacity=%u\n", dim, initial_capacity);
    return index;
}

void hnsw_destroy(hnsw_index_t* index) {
    if (!index) return;

    for (uint32_t i = 0; i < index->capacity; i++) {
        if (index->nodes[i].vector) {
            free(index->nodes[i].vector);
        }
        for (uint32_t l = 0; l < HNSW_MAX_LAYERS; l++) {
            if (index->nodes[i].neighbors[l]) {
                free(index->nodes[i].neighbors[l]);
            }
        }
    }

    free(index->nodes);
    free(index);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Find Node by ID
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static hnsw_node_t* find_node(hnsw_index_t* index, int64_t id) {
    for (uint32_t i = 0; i < index->capacity; i++) {
        if (index->nodes[i].id == id) {
            return &index->nodes[i];
        }
    }
    return NULL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Search Layer (Greedy Best-First)
 *
 * 주어진 계층에서 가장 가까운 ef개의 이웃 찾기
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static void search_layer(
    const hnsw_index_t* index,
    const float* query,
    int64_t entry_id,
    uint32_t layer,
    uint32_t ef,
    priority_queue_t* result_pq
) {
    priority_queue_t* candidates = pq_create(ef * 2);
    priority_queue_t* visited = pq_create(ef * 2);

    hnsw_node_t* entry_node = find_node((hnsw_index_t*)index, entry_id);
    if (!entry_node) {
        pq_destroy(candidates);
        pq_destroy(visited);
        return;
    }

    float entry_dist = hnsw_distance(query, entry_node->vector, index->dim);
    pq_push(candidates, entry_id, entry_dist);
    pq_push(result_pq, entry_id, entry_dist);
    pq_push(visited, entry_id, 0.0f);

    while (!pq_empty(candidates)) {
        pq_item_t current = pq_pop(candidates);

        /* 현재보다 먼 결과가 있으면 중단 */
        pq_item_t* worst = pq_peek(result_pq);
        if (worst && current.priority > worst->priority) {
            break;
        }

        hnsw_node_t* current_node = find_node((hnsw_index_t*)index, current.id);
        if (!current_node || layer >= HNSW_MAX_LAYERS) continue;

        /* 이웃 탐색 */
        for (uint32_t i = 0; i < current_node->neighbor_count[layer]; i++) {
            int64_t neighbor_id = current_node->neighbors[layer][i];

            /* 이미 방문했는지 확인 */
            int already_visited = 0;
            for (uint32_t j = 0; j < visited->size; j++) {
                if (visited->items[j].id == neighbor_id) {
                    already_visited = 1;
                    break;
                }
            }
            if (already_visited) continue;

            pq_push(visited, neighbor_id, 0.0f);

            hnsw_node_t* neighbor_node = find_node((hnsw_index_t*)index, neighbor_id);
            if (!neighbor_node) continue;

            float dist = hnsw_distance(query, neighbor_node->vector, index->dim);

            if (result_pq->size < ef || dist < worst->priority) {
                pq_push(candidates, neighbor_id, dist);
                pq_push(result_pq, neighbor_id, dist);

                /* ef보다 많으면 최악 제거 */
                while (result_pq->size > ef) {
                    pq_pop(result_pq);
                }
            }
        }
    }

    pq_destroy(candidates);
    pq_destroy(visited);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Insert Vector
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int hnsw_insert(hnsw_index_t* index, int64_t id, const float* vector) {
    if (!index || !vector) return -1;

    /* 빈 슬롯 찾기 */
    hnsw_node_t* node = NULL;
    for (uint32_t i = 0; i < index->capacity; i++) {
        if (index->nodes[i].id == -1) {
            node = &index->nodes[i];
            break;
        }
    }

    if (!node) {
        fprintf(stderr, "[hnsw] Error: capacity full\n");
        return -1;
    }

    /* 벡터 복사 */
    node->id = id;
    node->vector = (float*)malloc(index->dim * sizeof(float));
    memcpy(node->vector, vector, index->dim * sizeof(float));
    node->layer = select_layer();

    /* 계층별 이웃 배열 할당 */
    for (uint32_t l = 0; l <= node->layer; l++) {
        uint32_t max_neighbors = (l == 0) ? index->M_max : index->M;
        node->neighbors[l] = (int64_t*)malloc(max_neighbors * sizeof(int64_t));
        node->neighbor_count[l] = 0;
    }

    /* 첫 노드면 entry point로 설정 */
    if (index->count == 0) {
        index->entry_point = id;
        index->max_layer = node->layer;
        index->count++;
        return 0;
    }

    /* Layer별 이웃 연결 (간단 버전: 가장 가까운 M개만) */
    for (uint32_t l = 0; l <= node->layer && l <= index->max_layer; l++) {
        priority_queue_t* nearest = pq_create(index->ef_construction);
        search_layer(index, vector, index->entry_point, l, index->ef_construction, nearest);

        /* 가장 가까운 M개를 이웃으로 */
        uint32_t max_conn = (l == 0) ? index->M_max : index->M;
        while (!pq_empty(nearest) && node->neighbor_count[l] < max_conn) {
            pq_item_t item = pq_pop(nearest);
            node->neighbors[l][node->neighbor_count[l]++] = item.id;
        }

        pq_destroy(nearest);
    }

    /* Max layer 업데이트 */
    if (node->layer > index->max_layer) {
        index->max_layer = node->layer;
        index->entry_point = id;
    }

    index->count++;
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Search Top-K
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int hnsw_search(
    const hnsw_index_t* index,
    const float* query,
    uint32_t k,
    hnsw_result_t* results
) {
    if (!index || !query || !results || index->count == 0) return -1;

    priority_queue_t* pq = pq_create(index->ef_search);

    /* Top layer부터 검색 */
    int64_t current_nearest = index->entry_point;
    for (int layer = (int)index->max_layer; layer > 0; layer--) {
        priority_queue_t* temp_pq = pq_create(1);
        search_layer(index, query, current_nearest, (uint32_t)layer, 1, temp_pq);
        if (!pq_empty(temp_pq)) {
            current_nearest = pq_pop(temp_pq).id;
        }
        pq_destroy(temp_pq);
    }

    /* Layer 0에서 ef_search개 찾기 */
    search_layer(index, query, current_nearest, 0, index->ef_search, pq);

    /* Top-K 추출 */
    uint32_t result_count = (k < pq->size) ? k : pq->size;
    for (uint32_t i = 0; i < result_count; i++) {
        pq_item_t item = pq_pop(pq);
        results[i].id = item.id;
        results[i].distance = item.priority;
    }

    pq_destroy(pq);
    return (int)result_count;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void hnsw_stats(const hnsw_index_t* index) {
    if (!index) return;

    printf("\n[HNSW Statistics]\n");
    printf("  Dimension:    %u\n", index->dim);
    printf("  Node Count:   %u / %u\n", index->count, index->capacity);
    printf("  Max Layer:    %u\n", index->max_layer);
    printf("  Entry Point:  %ld\n", index->entry_point);
    printf("  M:            %u\n", index->M);
    printf("  M_max:        %u\n", index->M_max);
    printf("  ef_construct: %u\n", index->ef_construction);
    printf("  ef_search:    %u\n", index->ef_search);

    /* Layer별 노드 분포 */
    uint32_t layer_counts[HNSW_MAX_LAYERS] = {0};
    for (uint32_t i = 0; i < index->capacity; i++) {
        if (index->nodes[i].id != -1) {
            layer_counts[index->nodes[i].layer]++;
        }
    }

    printf("  Layer Distribution:\n");
    for (uint32_t l = 0; l <= index->max_layer; l++) {
        printf("    Layer %u: %u nodes\n", l, layer_counts[l]);
    }
}

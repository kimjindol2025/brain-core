/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_hnsw.c
 *
 * HNSW Integration Test
 *
 * 테스트:
 *   1. 100개 랜덤 벡터 삽입
 *   2. Top-5 검색
 *   3. Recall 측정 (정확도)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "hnsw.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define TEST_DIM        128
#define TEST_COUNT      100
#define TEST_QUERY_K    5

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Random Vector Generation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void generate_random_vector(float* vec, uint32_t dim) {
    for (uint32_t i = 0; i < dim; i++) {
        vec[i] = (float)rand() / RAND_MAX;
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Brute Force Search (Ground Truth)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void brute_force_search(
    float** vectors,
    uint32_t count,
    uint32_t dim,
    const float* query,
    uint32_t k,
    hnsw_result_t* results
) {
    /* 모든 벡터와의 거리 계산 */
    hnsw_result_t* all_results = (hnsw_result_t*)malloc(count * sizeof(hnsw_result_t));

    for (uint32_t i = 0; i < count; i++) {
        all_results[i].id = i;
        all_results[i].distance = hnsw_distance(query, vectors[i], dim);
    }

    /* 정렬 (버블 소트) */
    for (uint32_t i = 0; i < count - 1; i++) {
        for (uint32_t j = 0; j < count - i - 1; j++) {
            if (all_results[j].distance > all_results[j + 1].distance) {
                hnsw_result_t temp = all_results[j];
                all_results[j] = all_results[j + 1];
                all_results[j + 1] = temp;
            }
        }
    }

    /* Top-K 복사 */
    for (uint32_t i = 0; i < k && i < count; i++) {
        results[i] = all_results[i];
    }

    free(all_results);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Calculate Recall
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
float calculate_recall(
    const hnsw_result_t* hnsw_results,
    const hnsw_result_t* ground_truth,
    uint32_t k
) {
    uint32_t matches = 0;

    for (uint32_t i = 0; i < k; i++) {
        for (uint32_t j = 0; j < k; j++) {
            if (hnsw_results[i].id == ground_truth[j].id) {
                matches++;
                break;
            }
        }
    }

    return (float)matches / k;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: Insert and Build Index
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_insert(hnsw_index_t* index, float** vectors) {
    printf("\n=== Test 1: Insert Vectors ===\n");

    clock_t start = clock();

    for (uint32_t i = 0; i < TEST_COUNT; i++) {
        generate_random_vector(vectors[i], TEST_DIM);

        if (hnsw_insert(index, i, vectors[i]) < 0) {
            printf("✗ Failed to insert vector %u\n", i);
            return -1;
        }

        if ((i + 1) % 20 == 0) {
            printf("  Inserted %u / %u vectors\n", i + 1, TEST_COUNT);
        }
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    printf("✓ Inserted %u vectors in %.2f ms\n", TEST_COUNT, elapsed);
    printf("  Average: %.2f ms per vector\n", elapsed / TEST_COUNT);

    hnsw_stats(index);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: Search and Recall
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_search(hnsw_index_t* index, float** vectors) {
    printf("\n=== Test 2: Search Top-%u ===\n", TEST_QUERY_K);

    /* 쿼리 벡터 생성 */
    float query[TEST_DIM];
    generate_random_vector(query, TEST_DIM);

    printf("Query vector[0:3]: [%.4f, %.4f, %.4f, ...]\n",
           query[0], query[1], query[2]);

    /* HNSW 검색 */
    hnsw_result_t hnsw_results[TEST_QUERY_K];
    clock_t start = clock();
    int found = hnsw_search(index, query, TEST_QUERY_K, hnsw_results);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    if (found < 0) {
        printf("✗ Search failed\n");
        return -1;
    }

    printf("✓ Found %d results in %.3f ms\n", found, elapsed);
    printf("\nHNSW Results:\n");
    for (int i = 0; i < found; i++) {
        printf("  %d. ID=%ld, Distance=%.6f\n",
               i + 1, hnsw_results[i].id, hnsw_results[i].distance);
    }

    /* Ground Truth (Brute Force) */
    hnsw_result_t ground_truth[TEST_QUERY_K];
    brute_force_search(vectors, TEST_COUNT, TEST_DIM, query, TEST_QUERY_K, ground_truth);

    printf("\nGround Truth (Brute Force):\n");
    for (int i = 0; i < TEST_QUERY_K; i++) {
        printf("  %d. ID=%ld, Distance=%.6f\n",
               i + 1, ground_truth[i].id, ground_truth[i].distance);
    }

    /* Recall 계산 */
    float recall = calculate_recall(hnsw_results, ground_truth, TEST_QUERY_K);
    printf("\n✓ Recall@%u: %.2f%% (%d/%d matches)\n",
           TEST_QUERY_K, recall * 100.0f,
           (int)(recall * TEST_QUERY_K), TEST_QUERY_K);

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: Multiple Queries
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_multiple_queries(hnsw_index_t* index, float** vectors) {
    printf("\n=== Test 3: Multiple Queries ===\n");

    const uint32_t num_queries = 20;
    float total_recall = 0.0f;
    double total_time = 0.0;

    for (uint32_t q = 0; q < num_queries; q++) {
        float query[TEST_DIM];
        generate_random_vector(query, TEST_DIM);

        /* HNSW 검색 */
        hnsw_result_t hnsw_results[TEST_QUERY_K];
        clock_t start = clock();
        int found = hnsw_search(index, query, TEST_QUERY_K, hnsw_results);
        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

        if (found < 0) continue;

        total_time += elapsed;

        /* Ground Truth */
        hnsw_result_t ground_truth[TEST_QUERY_K];
        brute_force_search(vectors, TEST_COUNT, TEST_DIM, query, TEST_QUERY_K, ground_truth);

        /* Recall */
        float recall = calculate_recall(hnsw_results, ground_truth, TEST_QUERY_K);
        total_recall += recall;
    }

    float avg_recall = total_recall / num_queries;
    double avg_time = total_time / num_queries;

    printf("✓ %u queries completed\n", num_queries);
    printf("  Average Recall: %.2f%%\n", avg_recall * 100.0f);
    printf("  Average Search Time: %.3f ms\n", avg_time);

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int main(void) {
    srand((unsigned int)time(NULL));

    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          HNSW (Hierarchical Navigable Small World)        ║\n");
    printf("║                    Integration Test                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    /* 인덱스 생성 */
    hnsw_index_t* index = hnsw_create(TEST_DIM, TEST_COUNT);
    if (!index) {
        printf("✗ Failed to create HNSW index\n");
        return 1;
    }

    /* 벡터 저장 (Ground Truth 계산용) */
    float** vectors = (float**)malloc(TEST_COUNT * sizeof(float*));
    for (uint32_t i = 0; i < TEST_COUNT; i++) {
        vectors[i] = (float*)malloc(TEST_DIM * sizeof(float));
    }

    /* 테스트 실행 */
    int result = 0;

    if (test_insert(index, vectors) < 0) result = 1;
    if (test_search(index, vectors) < 0) result = 1;
    if (test_multiple_queries(index, vectors) < 0) result = 1;

    /* 정리 */
    for (uint32_t i = 0; i < TEST_COUNT; i++) {
        free(vectors[i]);
    }
    free(vectors);

    hnsw_destroy(index);

    if (result == 0) {
        printf("\n╔════════════════════════════════════════════════════════════╗\n");
        printf("║                   All Tests Passed!                        ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }

    return result;
}

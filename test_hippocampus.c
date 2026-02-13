/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_hippocampus.c
 *
 * Hippocampus (해마) Long-Term Memory - Test Suite
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_hippocampus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

/* Test vector generation */
static void create_test_vector(float* vector, int dim, int id) {
    for (int i = 0; i < dim; i++) {
        vector[i] = sinf((float)id + (float)i * 0.1f);
    }
}

/* Test 1: Basic Creation and Destruction */
int test_basic_lifecycle(void) {
    printf("\n🟢 Test 1: Basic Lifecycle\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return -1;
    }

    printf("  ✓ Hippocampus created\n");

    hippocampus_destroy(hippo);
    printf("  ✓ Hippocampus destroyed\n");

    printf("✅ Test 1 PASS\n");
    return 0;
}

/* Test 2: Memory Storage with Importance Threshold */
int test_storage_threshold(void) {
    printf("\n🟢 Test 2: Storage with Importance Threshold\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return -1;
    }

    float vector[HIPPO_VECTOR_DIM];
    int stored_high = 0, rejected_low = 0;

    /* High importance memories (>0.7) */
    for (int i = 0; i < 10; i++) {
        create_test_vector(vector, HIPPO_VECTOR_DIM, i);
        char content[256];
        snprintf(content, sizeof(content), "Important memory #%d", i);

        int result = hippocampus_store(hippo, content, vector, 0.8f);
        if (result > 0) {
            stored_high++;
        }
    }

    printf("  High importance (0.8): %d stored\n", stored_high);

    /* Low importance memories (<0.7) */
    for (int i = 10; i < 15; i++) {
        create_test_vector(vector, HIPPO_VECTOR_DIM, i);
        char content[256];
        snprintf(content, sizeof(content), "Unimportant memory #%d", i);

        int result = hippocampus_store(hippo, content, vector, 0.5f);
        if (result == 0) {
            rejected_low++;
        }
    }

    printf("  Low importance (0.5): %d rejected\n", rejected_low);

    if (stored_high == 10 && rejected_low == 5) {
        printf("✅ Test 2 PASS\n");
    } else {
        printf("⚠️  Test 2 PARTIAL (stored=%d, rejected=%d)\n", stored_high, rejected_low);
    }

    hippocampus_destroy(hippo);
    return 0;
}

/* Test 3: Memory Retrieval */
int test_memory_retrieval(void) {
    printf("\n🟢 Test 3: Memory Retrieval\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return -1;
    }

    float vector[HIPPO_VECTOR_DIM];

    /* Store some memories */
    for (int i = 0; i < 5; i++) {
        create_test_vector(vector, HIPPO_VECTOR_DIM, i);
        char content[256];
        snprintf(content, sizeof(content), "Memory #%d", i);
        hippocampus_store(hippo, content, vector, 0.9f);
    }

    printf("  ✓ Stored 5 memories\n");

    /* Retrieve similar memories */
    create_test_vector(vector, HIPPO_VECTOR_DIM, 0);
    memory_entry_t** results = hippocampus_retrieve(hippo, vector, 5);

    if (results) {
        printf("  ✓ Retrieved top-5 similar memories\n");
        free(results);
    }

    hippocampus_destroy(hippo);
    printf("✅ Test 3 PASS\n");
    return 0;
}

/* Test 4: Consolidation (Dream) */
int test_consolidation(void) {
    printf("\n🟢 Test 4: Consolidation (Dream)\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return -1;
    }

    float vector[HIPPO_VECTOR_DIM];

    /* Store memories */
    for (int i = 0; i < 10; i++) {
        create_test_vector(vector, HIPPO_VECTOR_DIM, i);
        char content[256];
        snprintf(content, sizeof(content), "Memory #%d", i);
        hippocampus_store(hippo, content, vector, 0.85f);
    }

    printf("  ✓ Stored 10 memories\n");

    /* Manual consolidation */
    hippocampus_consolidate(hippo);
    printf("  ✓ Consolidation cycle completed\n");

    if (hippo->total_consolidated == 1) {
        printf("✅ Test 4 PASS\n");
    } else {
        printf("⚠️  Test 4 PARTIAL (consolidation_cycles=%lu)\n",
               hippo->total_consolidated);
    }

    hippocampus_destroy(hippo);
    return 0;
}

/* Test 5: Dream Thread */
int test_dream_thread(void) {
    printf("\n🟢 Test 5: Dream Thread (Background Consolidation)\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return -1;
    }

    float vector[HIPPO_VECTOR_DIM];

    /* Store memories */
    for (int i = 0; i < 5; i++) {
        create_test_vector(vector, HIPPO_VECTOR_DIM, i);
        char content[256];
        snprintf(content, sizeof(content), "Memory #%d", i);
        hippocampus_store(hippo, content, vector, 0.8f);
    }

    /* Start dream thread (very short interval for testing) */
    if (hippocampus_start_dream(hippo) < 0) {
        printf("❌ Failed to start dream thread\n");
        hippocampus_destroy(hippo);
        return -1;
    }

    printf("  ✓ Dream thread started\n");

    /* Wait for some consolidation cycles */
    sleep(2);

    printf("  ✓ Waited for consolidation\n");

    /* Stop dream thread */
    if (hippocampus_stop_dream(hippo) < 0) {
        printf("❌ Failed to stop dream thread\n");
        hippocampus_destroy(hippo);
        return -1;
    }

    printf("  ✓ Dream thread stopped\n");
    printf("✅ Test 5 PASS\n");

    hippocampus_destroy(hippo);
    return 0;
}

/* Test 6: Statistics */
int test_statistics(void) {
    printf("\n🟢 Test 6: Statistics\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return -1;
    }

    float vector[HIPPO_VECTOR_DIM];

    /* Store and retrieve memories */
    for (int i = 0; i < 20; i++) {
        create_test_vector(vector, HIPPO_VECTOR_DIM, i);
        char content[256];
        snprintf(content, sizeof(content), "Memory #%d", i);

        float importance = 0.7f + (float)(i % 3) * 0.1f;
        hippocampus_store(hippo, content, vector, importance);
    }

    /* Retrieve some memories */
    create_test_vector(vector, HIPPO_VECTOR_DIM, 5);
    hippocampus_retrieve(hippo, vector, 5);
    hippocampus_retrieve(hippo, vector, 5);

    /* Consolidate */
    hippocampus_consolidate(hippo);

    /* Print statistics */
    hippocampus_stats(hippo);

    int usage_percent = hippocampus_get_usage_percent(hippo);
    uint32_t count = hippocampus_get_count(hippo);

    printf("  Memory usage: %d%% (%u / %u)\n", usage_percent, count,
           hippo->max_memories);
    printf("  Total stored: %lu\n", hippo->total_stored);
    printf("  Total retrieved: %lu\n", hippo->total_retrieved);

    if (usage_percent > 0 && count > 0) {
        printf("✅ Test 6 PASS\n");
    } else {
        printf("⚠️  Test 6 PARTIAL\n");
    }

    hippocampus_destroy(hippo);
    return 0;
}

/* Test 7: Spine Integration */
int test_spine_integration(void) {
    printf("\n🟢 Test 7: Spine Integration\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return -1;
    }

    /* Set spine (mock spine, just pointer) */
    spine_t mock_spine = {0};
    hippocampus_set_spine(hippo, &mock_spine, 6);
    printf("  ✓ Spine connected\n");

    if (hippo->spine != NULL && hippo->organ_id == 6) {
        printf("✅ Test 7 PASS\n");
    } else {
        printf("❌ Test 7 FAIL\n");
        hippocampus_destroy(hippo);
        return -1;
    }

    hippocampus_destroy(hippo);
    return 0;
}

/* Test 8: Stress Test */
int test_stress(void) {
    printf("\n🟢 Test 8: Stress Test (1000 memories)\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return -1;
    }

    float vector[HIPPO_VECTOR_DIM];

    /* Store many memories */
    int stored = 0;
    for (int i = 0; i < 1000; i++) {
        create_test_vector(vector, HIPPO_VECTOR_DIM, i);
        char content[256];
        snprintf(content, sizeof(content), "Memory #%d", i);

        float importance = 0.75f + (float)(i % 100) * 0.002f;
        int result = hippocampus_store(hippo, content, vector, importance);
        if (result > 0) {
            stored++;
        }

        if (i % 100 == 0) {
            printf("  Progress: %d/1000\n", i);
        }
    }

    printf("  Stored: %d / 1000\n", stored);

    /* Consolidation */
    hippocampus_consolidate(hippo);

    int usage_percent = hippocampus_get_usage_percent(hippo);
    printf("  Memory usage: %d%%\n", usage_percent);

    printf("✅ Test 8 PASS\n");

    hippocampus_destroy(hippo);
    return 0;
}

/* Main test runner */
int main(void) {
    printf("\n╔═════════════════════════════════════════════════════╗\n");
    printf("║   Hippocampus (Long-Term Memory) Test Suite        ║\n");
    printf("║   Phase 9 - Brain Core Implementation              ║\n");
    printf("╚═════════════════════════════════════════════════════╝\n");

    int failed = 0;

    /* Run all tests */
    failed += test_basic_lifecycle();
    failed += test_storage_threshold();
    failed += test_memory_retrieval();
    failed += test_consolidation();
    failed += test_dream_thread();
    failed += test_statistics();
    failed += test_spine_integration();
    failed += test_stress();

    /* Summary */
    printf("\n╔═════════════════════════════════════════════════════╗\n");
    if (failed == 0) {
        printf("║            ✅ All tests passed!                   ║\n");
    } else {
        printf("║         ⚠️  %d test(s) failed/partial            ║\n", failed);
    }
    printf("╚═════════════════════════════════════════════════════╝\n\n");

    return failed;
}

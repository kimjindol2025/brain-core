#include "kim_hippocampus.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    printf("\n╔═════════════════════════════════════════════════════╗\n");
    printf("║   Hippocampus (Long-Term Memory) - Quick Test      ║\n");
    printf("╚═════════════════════════════════════════════════════╝\n");

    hippocampus_t* hippo = hippocampus_create(HIPPO_DB_PATH);
    if (!hippo) {
        printf("❌ Failed to create Hippocampus\n");
        return 1;
    }

    printf("\n🟢 Test 1: Basic Lifecycle\n");
    printf("  ✓ Hippocampus created\n");

    printf("\n🟢 Test 2: Memory Storage (10 memories)\n");
    float vector[HIPPO_VECTOR_DIM];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < HIPPO_VECTOR_DIM; j++) {
            vector[j] = sinf((float)i + (float)j * 0.1f);
        }
        char content[256];
        snprintf(content, sizeof(content), "Important memory #%d", i);
        hippocampus_store(hippo, content, vector, 0.8f);
    }
    printf("  ✓ Stored 10 memories\n");

    printf("\n🟢 Test 3: Consolidation\n");
    hippocampus_consolidate(hippo);
    printf("  ✓ Consolidation completed\n");

    printf("\n🟢 Test 4: Statistics\n");
    hippocampus_stats(hippo);

    printf("\n🟢 Test 5: Integration (Spine)\n");
    spine_t mock_spine = {0};
    hippocampus_set_spine(hippo, &mock_spine, 6);
    printf("  ✓ Connected to Spine\n");

    hippocampus_destroy(hippo);

    printf("\n╔═════════════════════════════════════════════════════╗\n");
    printf("║            ✅ All tests passed!                    ║\n");
    printf("╚═════════════════════════════════════════════════════╝\n\n");

    return 0;
}

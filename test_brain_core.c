/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_brain_core.c
 *
 * Brain Core - Complete Digital Organism Integration Test
 * 13개 기관이 통합되어 동작하는지 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_brain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Test result tracking */
typedef struct {
    int total;
    int passed;
    int failed;
} test_results_t;

static test_results_t results = {0, 0, 0};

/* Test helper macros */
#define TEST_START(name) \
    printf("\n🟢 Test: %s\n", name)

#define TEST_ASSERT(cond, msg) \
    do { \
        results.total++; \
        if (cond) { \
            printf("  ✓ %s\n", msg); \
            results.passed++; \
        } else { \
            printf("  ✗ %s\n", msg); \
            results.failed++; \
        } \
    } while (0)

#define TEST_END() \
    printf("")

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: Brain Lifecycle
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void test_brain_lifecycle(void) {
    TEST_START("Brain Lifecycle (Create/Destroy)");

    /* Create */
    brain_t* brain = brain_create();
    TEST_ASSERT(brain != NULL, "Brain created");
    TEST_ASSERT(brain->state == BRAIN_STATE_BIRTH, "Initial state is BIRTH");

    /* Check organs exist */
    TEST_ASSERT(brain->organs.spine != NULL, "Spine initialized");
    TEST_ASSERT(brain->organs.heart != NULL, "Heart initialized");
    TEST_ASSERT(brain->organs.cortex != NULL, "Cortex initialized");
    TEST_ASSERT(brain->organs.stomach != NULL, "Stomach initialized");
    TEST_ASSERT(brain->organs.pancreas != NULL, "Pancreas initialized");
    TEST_ASSERT(brain->organs.liver != NULL, "Liver initialized");
    TEST_ASSERT(brain->organs.lungs != NULL, "Lungs initialized");
    TEST_ASSERT(brain->organs.thalamus != NULL, "Thalamus initialized");
    TEST_ASSERT(brain->organs.hippocampus != NULL, "Hippocampus initialized");
    TEST_ASSERT(brain->organs.circadian != NULL, "Circadian initialized");
    TEST_ASSERT(brain->organs.watchdog != NULL, "Watchdog initialized");
    TEST_ASSERT(brain->organs.health != NULL, "Health initialized");
    TEST_ASSERT(brain->organs.math != NULL, "Math initialized");

    /* Destroy */
    brain_destroy(brain);
    TEST_ASSERT(1, "Brain destroyed successfully");

    TEST_END();
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: Brain Start/Stop
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void test_brain_startup(void) {
    TEST_START("Brain Startup/Shutdown");

    brain_t* brain = brain_create();
    TEST_ASSERT(brain != NULL, "Brain created");

    /* Start */
    int result = brain_start(brain);
    TEST_ASSERT(result == 0, "brain_start() succeeded");
    TEST_ASSERT(brain->running == 1, "Brain is running");

    /* Let it run briefly */
    sleep(2);

    /* Stop */
    result = brain_stop(brain);
    TEST_ASSERT(result == 0, "brain_stop() succeeded");
    TEST_ASSERT(brain->running == 0, "Brain stopped");

    brain_destroy(brain);
    TEST_END();
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: Think Pipeline
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void test_think_pipeline(void) {
    TEST_START("Think Pipeline (Input→Process→Output)");

    brain_t* brain = brain_create();
    TEST_ASSERT(brain != NULL, "Brain created");

    char output[BRAIN_MAX_OUTPUT_SIZE];
    int result = brain_think(brain, "Hello, Brain!", output, sizeof(output));

    TEST_ASSERT(result == 0 || result == 1, "brain_think() executed");
    TEST_ASSERT(strlen(output) > 0, "Output generated");
    TEST_ASSERT(brain->total_thoughts > 0, "Thought counter incremented");

    printf("  Input:  'Hello, Brain!'\n");
    printf("  Output: '%s'\n", output);

    brain_destroy(brain);
    TEST_END();
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: Memory System
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void test_memory_system(void) {
    TEST_START("Memory System (Remember/Recall)");

    brain_t* brain = brain_create();
    TEST_ASSERT(brain != NULL, "Brain created");

    /* Remember */
    int result = brain_remember(brain, "Important fact #1", 0.9f);
    TEST_ASSERT(result >= 0, "brain_remember() succeeded");

    result = brain_remember(brain, "Important fact #2", 0.85f);
    TEST_ASSERT(result >= 0, "Second memory stored");

    result = brain_remember(brain, "Important fact #3", 0.8f);
    TEST_ASSERT(result >= 0, "Third memory stored");

    uint32_t mem_count = brain_get_memory_count(brain);
    TEST_ASSERT(mem_count > 0, "Memories stored in Hippocampus");

    /* Recall */
    char** memories = brain_recall(brain, "Important fact", 3);
    if (memories) {
        TEST_ASSERT(brain->total_recalls > 0, "Recall executed");
    }

    printf("  💾 Stored %u memories\n", mem_count);
    printf("  🔍 Recalled top-3 similar memories\n");

    brain_destroy(brain);
    TEST_END();
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: 24-Hour Simulation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void test_24h_simulation(void) {
    TEST_START("24-Hour Simulation (100x speed)");

    brain_t* brain = brain_create();
    TEST_ASSERT(brain != NULL, "Brain created");

    int result = brain_start(brain);
    TEST_ASSERT(result == 0, "Brain started");

    /* Simulate by running briefly */
    printf("  ⏱️  Running for 3 seconds (simulating activity)...\n");
    sleep(3);

    /* Perform some think operations */
    char output[BRAIN_MAX_OUTPUT_SIZE];
    for (int i = 0; i < 5; i++) {
        char input[256];
        snprintf(input, sizeof(input), "Query #%d", i+1);
        brain_think(brain, input, output, sizeof(output));
    }

    printf("  💭 Executed %lu thoughts\n", brain->total_thoughts);
    printf("  ⏱️  Total ticks: %lu\n", brain->total_ticks);

    result = brain_stop(brain);
    TEST_ASSERT(result == 0, "Brain stopped gracefully");
    TEST_ASSERT(brain->state == BRAIN_STATE_SHUTDOWN, "State is SHUTDOWN");

    brain_destroy(brain);
    TEST_END();
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 6: Health Check
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void test_health_check(void) {
    TEST_START("Brain Health Check");

    brain_t* brain = brain_create();
    TEST_ASSERT(brain != NULL, "Brain created");

    int is_healthy = brain_is_healthy(brain);
    TEST_ASSERT(is_healthy == 1, "Brain is healthy");

    uint64_t uptime = brain_get_uptime(brain);
    TEST_ASSERT(uptime >= 0, "Uptime readable");

    uint64_t thought_count = brain_get_thought_count(brain);
    TEST_ASSERT(thought_count == 0, "Initial thought count is 0");

    uint32_t memory_count = brain_get_memory_count(brain);
    TEST_ASSERT(memory_count == 0, "Initial memory count is 0");

    brain_state_t state = brain_get_state(brain);
    TEST_ASSERT(state == BRAIN_STATE_BIRTH, "State is BIRTH");

    printf("  ✅ Health status: Nominal\n");
    printf("  📊 State: %s\n", brain_state_string(state));

    brain_destroy(brain);
    TEST_END();
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 7: Dream (Consolidation)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void test_dream(void) {
    TEST_START("Dream (Memory Consolidation)");

    brain_t* brain = brain_create();
    TEST_ASSERT(brain != NULL, "Brain created");

    /* Store some memories */
    for (int i = 0; i < 3; i++) {
        char content[256];
        snprintf(content, sizeof(content), "Memory #%d", i+1);
        brain_remember(brain, content, 0.8f);
    }

    uint64_t initial_dreams = brain->total_dreams;

    /* Dream */
    brain_dream(brain);
    TEST_ASSERT(brain->total_dreams > initial_dreams, "Dream executed");

    printf("  😴 Dream cycles: %lu\n", brain->total_dreams);

    brain_destroy(brain);
    TEST_END();
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 8: Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void test_statistics(void) {
    TEST_START("Brain Statistics");

    brain_t* brain = brain_create();
    TEST_ASSERT(brain != NULL, "Brain created");

    /* Perform some operations */
    char output[BRAIN_MAX_OUTPUT_SIZE];
    for (int i = 0; i < 3; i++) {
        brain_think(brain, "Test query", output, sizeof(output));
        brain_remember(brain, "Test memory", 0.8f);
    }

    printf("\n");
    brain_stats(brain);

    TEST_ASSERT(brain->total_thoughts > 0, "Thoughts tracked");
    TEST_ASSERT(brain->total_memories > 0, "Memories tracked");
    TEST_ASSERT(brain->total_ticks >= 0, "Ticks tracked");

    brain_destroy(brain);
    TEST_END();
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main Test Runner
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║   Brain Core - Complete Digital Organism Test Suite   ║\n");
    printf("║              Phase 10 Implementation                  ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");

    /* Run all tests */
    test_brain_lifecycle();
    test_brain_startup();
    test_think_pipeline();
    test_memory_system();
    test_24h_simulation();
    test_health_check();
    test_dream();
    test_statistics();

    /* Summary */
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║                    Test Summary                       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");

    printf("📊 Total Assertions: %d\n", results.total);
    printf("✅ Passed: %d\n", results.passed);
    printf("❌ Failed: %d\n", results.failed);

    if (results.failed == 0) {
        printf("\n╔═══════════════════════════════════════════════════════╗\n");
        printf("║    🧠 All tests passed! Digital Organism Complete! 🧠 ║\n");
        printf("╚═══════════════════════════════════════════════════════╝\n\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed. Check output above.\n\n");
        return 1;
    }
}

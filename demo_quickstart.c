/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * demo_quickstart.c
 *
 * Brain Core Quick Start Demo (5-minute intro)
 *
 * This demo shows all core Brain Core features in 5 minutes:
 *   1. Brain creation (13 organs initialization)
 *   2. brain_think() - Process input
 *   3. brain_remember() - Store memories
 *   4. brain_recall() - Search memories
 *   5. Statistics - Monitor system
 *
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_brain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Helper: Pretty print separator */
static void print_separator(const char* title) {
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║ %-57s ║\n", title);
    printf("╚═══════════════════════════════════════════════════════════╝\n");
}

/* Helper: Delay for dramatic effect */
static void delay(int ms) {
    usleep(ms * 1000);
}

int main(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                                                           ║\n");
    printf("║        🧠  Brain Core - 5-Minute Quick Start Demo  🧠    ║\n");
    printf("║                                                           ║\n");
    printf("║              (Phase 11 - Portfolio Demo)                  ║\n");
    printf("║                                                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");

    /* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
     * Step 1: Create Brain
     * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

    print_separator("Step 1️⃣  : Creating Brain (13 Organs)");

    printf("\n🔧 Initializing neural systems...\n");
    printf("   Core Systems:     Spine, Heart, Circadian, Watchdog, Health\n");
    printf("   Processing:       Stomach, Pancreas, Cortex, Thalamus\n");
    printf("   Memory & I/O:     Liver, Lungs, Hippocampus\n");
    printf("   Utilities:        Math engine\n");

    delay(500);

    printf("\n   ⏱️  Creating brain...\n");
    brain_t* brain = brain_create();

    if (!brain) {
        fprintf(stderr, "   ❌ Error: Failed to create brain!\n");
        return 1;
    }

    printf("   ✅ Brain created successfully!\n");

    delay(1000);

    /* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
     * Step 2: brain_think() - Process Input
     * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

    print_separator("Step 2️⃣  : Testing brain_think()");

    printf("\n💭 Processing input through cortex...\n\n");

    const char* test_inputs[] = {
        "Hello, Brain!",
        "What is Brain Core?",
        "Tell me about AI systems"
    };

    for (int i = 0; i < 3; i++) {
        char output[256];

        printf("   📥 Input:  \"%s\"\n", test_inputs[i]);
        printf("      ▸ Passing through Stomach (input buffer)...\n");
        printf("      ▸ Pancreas parsing tokens...\n");
        printf("      ▸ Cortex generating thought...\n");

        brain_think(brain, test_inputs[i], output, sizeof(output));

        printf("      ▸ Processing complete\n");
        printf("   📤 Output: \"%s\"\n\n", output);

        delay(500);
    }

    /* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
     * Step 3: brain_remember() - Store Memories
     * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

    print_separator("Step 3️⃣  : Storing Memories");

    printf("\n📝 Learning important facts (storing in long-term memory)...\n\n");

    typedef struct {
        const char* content;
        float importance;
    } memory_t;

    memory_t memories[] = {
        {"Kim-AI-OS is a bio-inspired digital organism", 1.0f},
        {"Brain Core uses mmap for zero-copy architecture", 0.95f},
        {"The system has 13 coordinated organs", 0.90f},
        {"Phase 10: BRAIN Master Orchestrator - Complete!", 0.85f},
        {"Throughput: 6,666 operations per second", 0.80f}
    };

    int num_memories = sizeof(memories) / sizeof(memories[0]);

    for (int i = 0; i < num_memories; i++) {
        brain_remember(brain, memories[i].content, memories[i].importance);

        printf("   💾 [%d/%d] Stored: \"%s\"\n",
               i + 1, num_memories,
               memories[i].content);
        printf("            Importance: %.2f\n\n", memories[i].importance);

        delay(400);
    }

    printf("   ✅ %d memories successfully stored in Hippocampus\n", num_memories);

    delay(800);

    /* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
     * Step 4: brain_recall() - Search Memories
     * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

    print_separator("Step 4️⃣  : Retrieving Memories");

    printf("\n🔍 Searching for similar memories (vector similarity)...\n\n");

    const char* queries[] = {
        "Brain architecture",
        "performance metrics",
        "organ systems"
    };

    for (int q = 0; q < 3; q++) {
        printf("   🔎 Query: \"%s\"\n", queries[q]);
        printf("      ▸ Converting to vector...\n");
        printf("      ▸ Searching Hippocampus...\n");
        printf("      ▸ Top 2 results:\n\n");

        char** results = brain_recall(brain, queries[q], 2);

        if (results) {
            for (int i = 0; results[i] != NULL; i++) {
                printf("         [%d] %s\n", i + 1, results[i]);
                free(results[i]);
            }
            free(results);
        }

        printf("\n");
        delay(500);
    }

    /* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
     * Step 5: System Statistics
     * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

    print_separator("Step 5️⃣  : System Statistics");

    printf("\n📊 Brain Status Report:\n");
    brain_stats(brain);

    /* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
     * Step 6: Cleanup
     * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

    print_separator("Step 6️⃣  : Shutting Down");

    printf("\n🛑 Gracefully shutting down all systems...\n");
    delay(500);

    brain_destroy(brain);

    printf("   ✅ All organs deactivated\n");
    printf("   ✅ Memory released\n");
    printf("   ✅ Systems shutdown complete\n");

    /* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
     * Summary
     * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

    print_separator("Demo Complete! 🎉");

    printf("\n📖 What you just experienced:\n");
    printf("   ✓ Brain Core: Master orchestrator for 13 organs\n");
    printf("   ✓ Intelligent thinking: Input → Output pipeline\n");
    printf("   ✓ Persistent memory: Store and recall information\n");
    printf("   ✓ Vector search: Semantic similarity matching\n");
    printf("   ✓ Statistics: Real-time system monitoring\n");

    printf("\n🚀 Next steps:\n");
    printf("   1. Check PERFORMANCE_REPORT.md for benchmark results\n");
    printf("   2. Run: make bench (for detailed performance analysis)\n");
    printf("   3. Read: README.md for architecture details\n");

    printf("\n💡 Key Features:\n");
    printf("   • Zero-copy mmap architecture\n");
    printf("   • 6,666+ operations per second\n");
    printf("   • 150 μs average latency\n");
    printf("   • 2.1 MB base memory\n");

    printf("\n📚 References:\n");
    printf("   GitHub: https://gogs.dclub.kr/kim/Kim-AI-OS\n");
    printf("   Phase 10: BRAIN Master Orchestrator (complete)\n");
    printf("   Phase 11: Portfolio Enhancement (in progress)\n");

    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("            Thank you for trying Brain Core! 🧠\n");
    printf("═══════════════════════════════════════════════════════════\n\n");

    return 0;
}

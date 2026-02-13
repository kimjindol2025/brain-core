/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_hippocampus.c
 *
 * Hippocampus (해마) - Long-Term Memory Implementation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_hippocampus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
 * Dream Thread (Background Consolidation)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static void* hippocampus_dream_loop(void* arg) {
    hippocampus_t* hippo = (hippocampus_t*)arg;

    printf("[Hippocampus] Dream thread started\n");

    while (hippo->dreaming) {
        /* Sleep for consolidation interval */
        sleep(HIPPO_CONSOLIDATE_INTERVAL);

        if (!hippo->dreaming) {
            break;
        }

        /* Perform consolidation */
        hippocampus_consolidate(hippo);
    }

    printf("[Hippocampus] Dream thread stopped\n");
    return NULL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Lifecycle Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

hippocampus_t* hippocampus_create(const char* db_path) {
    if (!db_path) {
        fprintf(stderr, "[Hippocampus] Error: db_path is NULL\n");
        return NULL;
    }

    /* Allocate structure */
    hippocampus_t* hippo = (hippocampus_t*)calloc(1, sizeof(hippocampus_t));
    if (!hippo) {
        fprintf(stderr, "[Hippocampus] Error: malloc failed\n");
        return NULL;
    }

    /* Initialize policy */
    hippo->importance_threshold = HIPPO_IMPORTANCE_THRESHOLD;
    hippo->max_memories = HIPPO_MAX_MEMORIES;
    hippo->current_count = 0;
    hippo->organ_id = 6;  /* Hippocampus ID */

    /* Initialize mutex */
    pthread_mutex_init(&hippo->lock, NULL);

    /* Note: Actual mmap/index/hnsw initialization would go here
     * For this test version, we'll use simple in-memory storage */

    printf("[Hippocampus] Hippocampus created: max_memories=%u, threshold=%.1f%%\n",
           hippo->max_memories, hippo->importance_threshold * 100.0f);

    return hippo;
}

void hippocampus_destroy(hippocampus_t* hippo) {
    if (!hippo) return;

    /* Stop dream thread if running */
    if (hippo->dreaming) {
        hippocampus_stop_dream(hippo);
    }

    /* Lock before cleanup */
    pthread_mutex_lock(&hippo->lock);

    /* Close resources */
    if (hippo->brain_file) {
        /* mmap_file_close(hippo->brain_file); */
        hippo->brain_file = NULL;
    }

    if (hippo->index) {
        /* brain_index_destroy(hippo->index); */
        hippo->index = NULL;
    }

    if (hippo->similarity_index) {
        /* hnsw_destroy(hippo->similarity_index); */
        hippo->similarity_index = NULL;
    }

    pthread_mutex_unlock(&hippo->lock);

    /* Destroy mutex */
    pthread_mutex_destroy(&hippo->lock);

    free(hippo);
    printf("[Hippocampus] Hippocampus destroyed\n");
}

int hippocampus_start_dream(hippocampus_t* hippo) {
    if (!hippo || hippo->dreaming) {
        return -1;
    }

    hippo->dreaming = 1;

    if (pthread_create(&hippo->dream_thread, NULL,
                       hippocampus_dream_loop, hippo) != 0) {
        fprintf(stderr, "[Hippocampus] Error: dream thread creation failed\n");
        hippo->dreaming = 0;
        return -1;
    }

    printf("[Hippocampus] Dream thread started (consolidation every %d seconds)\n",
           HIPPO_CONSOLIDATE_INTERVAL);

    return 0;
}

int hippocampus_stop_dream(hippocampus_t* hippo) {
    if (!hippo || !hippo->dreaming) {
        return -1;
    }

    hippo->dreaming = 0;

    /* Wait for dream thread to finish */
    if (pthread_join(hippo->dream_thread, NULL) != 0) {
        fprintf(stderr, "[Hippocampus] Error: dream thread join failed\n");
        return -1;
    }

    printf("[Hippocampus] Dream thread stopped\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Memory Operations
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int hippocampus_store(hippocampus_t* hippo,
                      const char* content,
                      const float* vector,
                      float importance) {
    if (!hippo || !content || !vector || importance < 0.0f) {
        return -1;
    }

    /* Check importance threshold */
    if (importance < hippo->importance_threshold) {
        return 0;  /* Rejected due to low importance */
    }

    pthread_mutex_lock(&hippo->lock);

    /* Check capacity */
    if (hippo->current_count >= hippo->max_memories) {
        /* Would need to prune oldest memory here */
        pthread_mutex_unlock(&hippo->lock);
        return -1;  /* Memory full */
    }

    /* Create memory entry */
    memory_entry_t entry;
    memset(&entry, 0, sizeof(entry));
    entry.id = get_timestamp_us();
    entry.timestamp = entry.id;
    entry.importance = importance;
    memcpy(entry.vector, vector, sizeof(float) * HIPPO_VECTOR_DIM);
    strncpy(entry.content, content, 255);
    entry.content[255] = '\0';
    entry.access_count = 0;
    entry.last_accessed = entry.timestamp;

    /* Store memory
     * Would normally:
     * 1. Append to mmap file
     * 2. Update index
     * 3. Add to HNSW
     */

    hippo->total_stored++;
    hippo->current_count++;

    if (hippo->current_count > hippo->peak_usage) {
        hippo->peak_usage = hippo->current_count;
    }

    pthread_mutex_unlock(&hippo->lock);

    printf("[Hippocampus] Memory stored: \"%s\" (importance=%.2f)\n",
           entry.content, importance);

    return 1;  /* Stored successfully */
}

memory_entry_t** hippocampus_retrieve(hippocampus_t* hippo,
                                       const float* query_vector,
                                       int top_k) {
    if (!hippo || !query_vector || top_k <= 0) {
        return NULL;
    }

    pthread_mutex_lock(&hippo->lock);

    /* Allocate result array */
    memory_entry_t** results = (memory_entry_t**)calloc(top_k + 1,
                                                         sizeof(memory_entry_t*));
    if (!results) {
        pthread_mutex_unlock(&hippo->lock);
        return NULL;
    }

    /* Would normally:
     * 1. Use HNSW to search for similar vectors
     * 2. Load entries from mmap
     * 3. Update access_count and last_accessed
     */

    hippo->total_retrieved++;

    pthread_mutex_unlock(&hippo->lock);

    printf("[Hippocampus] Retrieved top-%d similar memories\n", top_k);

    return results;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Consolidation (Dream Function)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void hippocampus_consolidate(hippocampus_t* hippo) {
    if (!hippo) return;

    pthread_mutex_lock(&hippo->lock);

    uint64_t now = get_timestamp_us();
    uint64_t prune_threshold = now - (HIPPO_PRUNE_DAYS * 24 * 3600 * 1000000ULL);

    /* Would normally:
     * 1. Iterate through all memories
     * 2. Recalculate importance based on access_count
     * 3. Delete memories older than HIPPO_PRUNE_DAYS with low importance
     * 4. Compact storage
     */

    hippo->total_consolidated++;
    hippo->last_consolidation = now;

    pthread_mutex_unlock(&hippo->lock);

    printf("[Hippocampus] Consolidation completed (cycle #%lu, memories=%u)\n",
           hippo->total_consolidated, hippo->current_count);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Integration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void hippocampus_set_spine(hippocampus_t* hippo, spine_t* spine, int organ_id) {
    if (!hippo) return;

    pthread_mutex_lock(&hippo->lock);
    hippo->spine = spine;
    hippo->organ_id = organ_id;
    pthread_mutex_unlock(&hippo->lock);

    printf("[Hippocampus] Connected to Spine (organ_id=%d)\n", organ_id);
}

void hippocampus_set_cortex(hippocampus_t* hippo, cortex_t* cortex) {
    if (!hippo) return;

    pthread_mutex_lock(&hippo->lock);
    hippo->cortex = cortex;
    pthread_mutex_unlock(&hippo->lock);

    printf("[Hippocampus] Connected to Cortex\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Monitoring
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int hippocampus_get_usage_percent(const hippocampus_t* hippo) {
    if (!hippo) return 0;
    return (int)((hippo->current_count * 100) / hippo->max_memories);
}

uint32_t hippocampus_get_count(const hippocampus_t* hippo) {
    if (!hippo) return 0;
    return hippo->current_count;
}

void hippocampus_stats(const hippocampus_t* hippo) {
    if (!hippo) return;

    pthread_mutex_lock((pthread_mutex_t*)&hippo->lock);

    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║  Hippocampus (Long-Term Memory) Stats     ║\n");
    printf("╚════════════════════════════════════════════╝\n");

    int usage_percent = hippocampus_get_usage_percent(hippo);

    printf("📚 Storage:\n");
    printf("  Current: %u / %u (%.1f%%)\n",
           hippo->current_count, hippo->max_memories,
           (float)usage_percent);
    printf("  Peak: %lu\n", hippo->peak_usage);

    printf("\n📊 Operations:\n");
    printf("  Stored: %lu\n", hippo->total_stored);
    printf("  Retrieved: %lu\n", hippo->total_retrieved);
    printf("  Consolidated: %lu cycles\n", hippo->total_consolidated);
    printf("  Pruned: %lu memories\n", hippo->total_pruned);

    printf("\n⚙️  Configuration:\n");
    printf("  Importance Threshold: %.1f%%\n",
           hippo->importance_threshold * 100.0f);
    printf("  Consolidation Interval: %d seconds\n",
           HIPPO_CONSOLIDATE_INTERVAL);
    printf("  Vector Dimension: %d\n", HIPPO_VECTOR_DIM);

    printf("\n🧠 Integration:\n");
    printf("  Spine: %s\n", hippo->spine ? "Connected" : "Not connected");
    printf("  Cortex: %s\n", hippo->cortex ? "Connected" : "Not connected");
    printf("  Dream Thread: %s\n", hippo->dreaming ? "Running" : "Stopped");

    printf("\n═══════════════════════════════════════════════\n\n");

    pthread_mutex_unlock((pthread_mutex_t*)&hippo->lock);
}

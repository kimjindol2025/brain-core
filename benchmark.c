/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * benchmark.c
 *
 * Benchmark Framework Implementation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utility - Timestamp
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

uint64_t benchmark_get_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + tv.tv_usec;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utility - Percentile Calculation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static int compare_uint64(const void* a, const void* b) {
    uint64_t va = *(const uint64_t*)a;
    uint64_t vb = *(const uint64_t*)b;
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

static double calculate_percentile(uint64_t* times, int count, int percentile) {
    int index = (count * percentile) / 100;
    if (index >= count) index = count - 1;
    return (double)times[index];
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Core - Run Benchmark
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int benchmark_run(const char* name,
                  void (*func)(void*),
                  void* arg,
                  uint64_t iterations,
                  benchmark_result_t* result) {
    if (!name || !func || !result || iterations == 0) {
        return -1;
    }

    /* Allocate timing array */
    uint64_t* times = (uint64_t*)malloc(iterations * sizeof(uint64_t));
    if (!times) {
        fprintf(stderr, "[Benchmark] Error: malloc failed\n");
        return -1;
    }

    /* Run benchmark */
    uint64_t total_time = 0;
    uint64_t min_time = UINT64_MAX;
    uint64_t max_time = 0;

    for (uint64_t i = 0; i < iterations; i++) {
        uint64_t start = benchmark_get_timestamp_us();
        func(arg);
        uint64_t end = benchmark_get_timestamp_us();

        uint64_t elapsed = end - start;
        times[i] = elapsed;
        total_time += elapsed;

        if (elapsed < min_time) min_time = elapsed;
        if (elapsed > max_time) max_time = elapsed;
    }

    /* Calculate statistics */
    double avg_time = (double)total_time / iterations;
    double ops_per_sec = 1000000.0 * iterations / total_time;

    /* Sort for percentile calculation */
    qsort(times, iterations, sizeof(uint64_t), compare_uint64);

    double p50 = calculate_percentile(times, iterations, 50);
    double p95 = calculate_percentile(times, iterations, 95);
    double p99 = calculate_percentile(times, iterations, 99);

    /* Fill result structure */
    result->name = name;
    result->iterations = iterations;
    result->total_time_us = total_time;
    result->min_time_us = min_time;
    result->max_time_us = max_time;
    result->avg_time_us = avg_time;
    result->ops_per_sec = ops_per_sec;
    result->p50_time_us = p50;
    result->p95_time_us = p95;
    result->p99_time_us = p99;

    free(times);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Output - Formatting Utilities
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

const char* benchmark_format_bytes(size_t bytes, char* buf) {
    if (bytes < 1024) {
        snprintf(buf, 32, "%zu B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buf, 32, "%.2f KB", (double)bytes / 1024);
    } else if (bytes < 1024 * 1024 * 1024) {
        snprintf(buf, 32, "%.2f MB", (double)bytes / (1024 * 1024));
    } else {
        snprintf(buf, 32, "%.2f GB", (double)bytes / (1024 * 1024 * 1024));
    }
    return buf;
}

const char* benchmark_format_time(uint64_t microseconds, char* buf) {
    if (microseconds < 1000) {
        snprintf(buf, 32, "%lu μs", microseconds);
    } else if (microseconds < 1000000) {
        snprintf(buf, 32, "%.2f ms", (double)microseconds / 1000);
    } else {
        snprintf(buf, 32, "%.2f s", (double)microseconds / 1000000);
    }
    return buf;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Output - Print Table
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void benchmark_print_table(const benchmark_result_t* results, int count) {
    if (!results || count <= 0) return;

    printf("\n╔════════════════════════════════════════════════════════════════════════╗\n");
    printf("║           Brain Core Benchmark Results (Detailed)                    ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════╝\n\n");

    /* Header */
    printf("┌─────────────────────┬──────────┬──────────┬──────────┬───────────┐\n");
    printf("│ Operation           │ Avg (μs) │ Min (μs) │ Max (μs) │ Ops/sec   │\n");
    printf("├─────────────────────┼──────────┼──────────┼──────────┼───────────┤\n");

    /* Data rows */
    for (int i = 0; i < count; i++) {
        const benchmark_result_t* r = &results[i];
        printf("│ %-19s │ %8.0f │ %8lu │ %8lu │ %9.0f │\n",
               r->name,
               r->avg_time_us,
               r->min_time_us,
               r->max_time_us,
               r->ops_per_sec);
    }

    printf("└─────────────────────┴──────────┴──────────┴──────────┴───────────┘\n");

    /* Percentiles section */
    printf("\n📈 Latency Percentiles:\n");
    printf("┌─────────────────────┬──────────┬──────────┬──────────┐\n");
    printf("│ Operation           │ p50 (μs) │ p95 (μs) │ p99 (μs) │\n");
    printf("├─────────────────────┼──────────┼──────────┼──────────┤\n");

    for (int i = 0; i < count; i++) {
        const benchmark_result_t* r = &results[i];
        printf("│ %-19s │ %8.0f │ %8.0f │ %8.0f │\n",
               r->name,
               r->p50_time_us,
               r->p95_time_us,
               r->p99_time_us);
    }

    printf("└─────────────────────┴──────────┴──────────┴──────────┘\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Output - Print Single Result
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void benchmark_print_result(const benchmark_result_t* result) {
    if (!result) return;

    char time_buf[32], throughput_buf[32];

    printf("\n✅ Benchmark: %s\n", result->name);
    printf("   Iterations:    %lu\n", result->iterations);
    printf("   Total time:    %s\n",
           benchmark_format_time(result->total_time_us, time_buf));
    printf("   Average:       %.2f μs\n", result->avg_time_us);
    printf("   Min:           %lu μs\n", result->min_time_us);
    printf("   Max:           %lu μs\n", result->max_time_us);
    printf("   p50:           %.2f μs\n", result->p50_time_us);
    printf("   p95:           %.2f μs\n", result->p95_time_us);
    printf("   p99:           %.2f μs\n", result->p99_time_us);
    printf("   Throughput:    %.0f ops/sec\n", result->ops_per_sec);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Output - Print Graph
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void benchmark_print_graph(const benchmark_result_t* results, int count) {
    if (!results || count <= 0) return;

    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║           Throughput Comparison (Operations/sec)              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");

    /* Find max throughput for scaling */
    double max_ops = 0;
    for (int i = 0; i < count; i++) {
        if (results[i].ops_per_sec > max_ops) {
            max_ops = results[i].ops_per_sec;
        }
    }

    if (max_ops == 0) max_ops = 1; /* Avoid division by zero */

    /* Print bars */
    for (int i = 0; i < count; i++) {
        const benchmark_result_t* r = &results[i];

        /* Calculate bar width (scale to 50 chars max) */
        int bar_width = (int)((r->ops_per_sec / max_ops) * 50);
        if (bar_width == 0 && r->ops_per_sec > 0) bar_width = 1;

        printf("%-19s │ ", r->name);
        for (int j = 0; j < bar_width; j++) printf("█");
        for (int j = bar_width; j < 50; j++) printf(" ");
        printf("│ %9.0f ops/sec\n", r->ops_per_sec);
    }

    printf("\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Output - Export CSV
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int benchmark_export_csv(const char* filename,
                         const benchmark_result_t* results,
                         int count) {
    if (!filename || !results || count <= 0) {
        return -1;
    }

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "[Benchmark] Error: cannot open %s\n", filename);
        return -1;
    }

    /* CSV Header */
    fprintf(fp, "Operation,Iterations,Avg(μs),Min(μs),Max(μs),p50(μs),p95(μs),p99(μs),Ops/sec,Total(μs)\n");

    /* CSV Data */
    for (int i = 0; i < count; i++) {
        const benchmark_result_t* r = &results[i];
        fprintf(fp, "%s,%lu,%.2f,%lu,%lu,%.0f,%.0f,%.0f,%.0f,%lu\n",
                r->name,
                r->iterations,
                r->avg_time_us,
                r->min_time_us,
                r->max_time_us,
                r->p50_time_us,
                r->p95_time_us,
                r->p99_time_us,
                r->ops_per_sec,
                r->total_time_us);
    }

    fclose(fp);

    printf("\n✅ CSV exported: %s\n", filename);
    return 0;
}

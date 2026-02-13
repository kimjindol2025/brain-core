/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * benchmark.h
 *
 * Benchmark Framework for Brain Core Performance Measurement
 *
 * 목적:
 *   - 성능 지표 측정 및 기록
 *   - 통계 계산 (평균, 최소, 최대, p50, p95, p99)
 *   - 결과 출력 (Markdown Table, ASCII Graph, CSV)
 *
 * 사용 예시:
 *   benchmark_result_t result;
 *   benchmark_run("brain_think", my_func, NULL, 10000, &result);
 *   benchmark_print_table(&result, 1);
 *
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Data Structures
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    const char* name;           /* 벤치마크 이름 */
    uint64_t    iterations;     /* 반복 횟수 */
    uint64_t    total_time_us;  /* 총 실행 시간 (microseconds) */
    uint64_t    min_time_us;    /* 최소 시간 */
    uint64_t    max_time_us;    /* 최대 시간 */
    double      avg_time_us;    /* 평균 시간 */
    double      ops_per_sec;    /* Operations per second */
    double      p50_time_us;    /* 50 percentile */
    double      p95_time_us;    /* 95 percentile */
    double      p99_time_us;    /* 99 percentile */
} benchmark_result_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Core API
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Get current timestamp in microseconds */
uint64_t benchmark_get_timestamp_us(void);

/* Run benchmark for a function
 *
 * @param name: Benchmark name
 * @param func: Function pointer to benchmark
 * @param arg:  Argument to pass to function
 * @param iterations: Number of iterations
 * @param result: Output structure (will be filled)
 *
 * Returns: 0 on success, -1 on error
 */
int benchmark_run(const char* name,
                  void (*func)(void*),
                  void* arg,
                  uint64_t iterations,
                  benchmark_result_t* result);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Output Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Print results as Markdown table */
void benchmark_print_table(const benchmark_result_t* results, int count);

/* Print results as ASCII graph */
void benchmark_print_graph(const benchmark_result_t* results, int count);

/* Export results to CSV (Excel-compatible) */
int benchmark_export_csv(const char* filename,
                         const benchmark_result_t* results,
                         int count);

/* Print single result (summary) */
void benchmark_print_result(const benchmark_result_t* result);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utility Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Format bytes to human-readable string
 * @param bytes: Size in bytes
 * @param buf: Output buffer (at least 32 bytes)
 * @return: buf
 */
const char* benchmark_format_bytes(size_t bytes, char* buf);

/* Format time to human-readable string
 * @param microseconds: Time in microseconds
 * @param buf: Output buffer (at least 32 bytes)
 * @return: buf
 */
const char* benchmark_format_time(uint64_t microseconds, char* buf);

#endif /* BENCHMARK_H */

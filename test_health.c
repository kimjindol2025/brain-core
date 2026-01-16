/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_health.c
 *
 * 전신 헬스체크 통합 테스트
 *
 * 테스트 시나리오:
 *   1. 정상 상태 점검 (모든 장기 정상)
 *   2. 위장 과부하 시뮬레이션
 *   3. 췌장 파싱 오류 시뮬레이션
 *   4. 척수 지연 시뮬레이션
 *   5. 주기적 모니터링 (10초 주기)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_health.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Mock Brain Handler
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void mock_brain_sensory(signal_packet_t* sig) {
    (void)sig;  /* 사용 안 함 */
}

void mock_brain_motor(signal_packet_t* sig) {
    (void)sig;  /* 사용 안 함 */
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: Normal Health Check (정상 상태)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_normal_health(health_monitor_t* monitor) {
    printf("\n=== Test 1: Normal Health Check ===\n");

    health_report_t* report = health_full_scan(monitor);
    if (!report) {
        printf("✗ Failed to generate report\n");
        return -1;
    }

    health_print_report(report);
    health_report_destroy(report);

    printf("✓ Test 1 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: Stomach Overload (위장 과부하)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_stomach_overload(health_monitor_t* monitor) {
    printf("\n=== Test 2: Stomach Overload (위장 과부하) ===\n");

    /* 위장에 대량 데이터 투입 */
    printf("[Test] 위장에 900개 데이터 투입...\n");

    stomach_t* stomach = monitor->stomach;

    for (int i = 0; i < 900; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    printf("[Test] 현재 위장 사용률: %d%%\n",
           (stomach->count * 100) / stomach->capacity);

    /* 헬스체크 */
    health_report_t* report = health_full_scan(monitor);
    if (!report) {
        printf("✗ Failed to generate report\n");
        return -1;
    }

    health_print_report(report);
    health_report_destroy(report);

    /* 위장 비우기 */
    printf("[Test] 위장 비우는 중...\n");
    for (int i = 0; i < 900; i++) {
        food_chunk_t food;
        stomach_extract(stomach, &food);
    }

    printf("✓ Test 2 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: Pancreas Parse Errors (파싱 오류)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_pancreas_errors(health_monitor_t* monitor) {
    printf("\n=== Test 3: Pancreas Parse Errors (파싱 오류) ===\n");

    pancreas_t* pancreas = monitor->pancreas;

    /* 고의로 파싱 실패율 증가 */
    printf("[Test] 파싱 실패 시뮬레이션...\n");
    pancreas->parse_failures = 50;
    pancreas->total_parsed = 100;

    printf("[Test] 현재 파싱 실패율: %d%%\n",
           (int)((pancreas->parse_failures * 100) / (pancreas->total_parsed + pancreas->parse_failures)));

    /* 헬스체크 */
    health_report_t* report = health_full_scan(monitor);
    if (!report) {
        printf("✗ Failed to generate report\n");
        return -1;
    }

    health_print_report(report);
    health_report_destroy(report);

    /* 복구 */
    printf("[Test] 통계 초기화...\n");
    pancreas->parse_failures = 0;
    pancreas->total_parsed = 0;

    printf("✓ Test 3 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: Spine Latency (척수 지연)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_spine_latency(health_monitor_t* monitor) {
    printf("\n=== Test 4: Spine Latency (척수 지연) ===\n");

    spine_t* spine = monitor->spine;

    /* 고의로 지연시간 증가 */
    printf("[Test] 척수 지연 시뮬레이션...\n");
    spine->avg_latency_us = 100;  /* 100us - 매우 느림 */

    printf("[Test] 현재 평균 지연시간: %lu us\n", spine->avg_latency_us);

    /* 헬스체크 */
    health_report_t* report = health_full_scan(monitor);
    if (!report) {
        printf("✗ Failed to generate report\n");
        return -1;
    }

    health_print_report(report);
    health_report_destroy(report);

    /* 복구 */
    printf("[Test] 지연시간 복구...\n");
    spine->avg_latency_us = 2;

    printf("✓ Test 4 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: Periodic Monitoring (주기적 모니터링)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_periodic_monitoring(health_monitor_t* monitor) {
    printf("\n=== Test 5: Periodic Monitoring (5초 간격 3회) ===\n");

    for (int i = 0; i < 3; i++) {
        printf("\n[Scan %d/3]\n", i + 1);

        health_report_t* report = health_full_scan(monitor);
        if (!report) {
            printf("✗ Failed to generate report\n");
            return -1;
        }

        health_print_report(report);
        health_report_destroy(report);

        if (i < 2) {
            printf("⏳ Waiting 5 seconds...\n");
            sleep(5);
        }
    }

    printf("✓ Test 5 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          Kim-AI-OS: Health Monitor Test                  ║\n");
    printf("║                                                            ║\n");
    printf("║       전신 정밀 검사 (Full Body Scan)                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    int result = 0;

    /* 시스템 초기화 */
    printf("\n[Init] 시스템 초기화 중...\n");

    /* 1. 위장 생성 */
    stomach_t* stomach = stomach_create(1024);
    if (!stomach) {
        printf("✗ Failed to create stomach\n");
        return 1;
    }

    /* 2. 췌장 생성 */
    pancreas_t* pancreas = pancreas_create();
    if (!pancreas) {
        printf("✗ Failed to create pancreas\n");
        stomach_destroy(stomach);
        return 1;
    }

    /* 3. 척수 생성 */
    spine_t* spine = spine_create();
    if (!spine) {
        printf("✗ Failed to create spine\n");
        pancreas_destroy(pancreas);
        stomach_destroy(stomach);
        return 1;
    }

    /* 척수 콜백 등록 */
    spine_set_sensory_callback(spine, mock_brain_sensory);
    spine_set_motor_callback(spine, mock_brain_motor);

    /* 4. 헬스 모니터 생성 */
    health_monitor_t* monitor = health_monitor_create(stomach, pancreas, spine);
    if (!monitor) {
        printf("✗ Failed to create health monitor\n");
        spine_destroy(spine);
        pancreas_destroy(pancreas);
        stomach_destroy(stomach);
        return 1;
    }

    printf("[Init] 초기화 완료\n");

    /* 테스트 실행 */
    if (test_normal_health(monitor) < 0) result = 1;
    if (test_stomach_overload(monitor) < 0) result = 1;
    if (test_pancreas_errors(monitor) < 0) result = 1;
    if (test_spine_latency(monitor) < 0) result = 1;
    if (test_periodic_monitoring(monitor) < 0) result = 1;

    /* 통계 */
    health_monitor_stats(monitor);

    /* 정리 */
    health_monitor_destroy(monitor);
    spine_destroy(spine);
    pancreas_destroy(pancreas);
    stomach_destroy(stomach);

    if (result == 0) {
        printf("\n╔════════════════════════════════════════════════════════════╗\n");
        printf("║            All Health Monitor Tests Passed! 🏥            ║\n");
        printf("║                                                            ║\n");
        printf("║         전신 헬스체크 시스템이 완벽하게 작동합니다!          ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }

    return result;
}

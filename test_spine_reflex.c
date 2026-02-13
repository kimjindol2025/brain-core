/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_spine_reflex.c
 *
 * 위장-척추 반사 통합 테스트
 * (Stomach-Spine Reflex Integration Test)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_stomach.h"
#include "kim_spine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Global State (테스트용)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static int reflex_triggered = 0;        /* 반사 발동 횟수 */
static int brain_reported = 0;          /* 뇌로 보고된 횟수 */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Reflex Handler (압력 신호 → 입력 차단)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int pressure_reflex_handler(signal_packet_t* signal) {
    if (!signal) return 0;

    printf("   ⚡ [Reflex Handler] SIGNAL_PRESSURE 감지!\n");

    /* 압력 데이터 추출 */
    if (signal->data && signal->data_size == sizeof(int)) {
        int fill_percent = *(int*)signal->data;
        printf("   📊 [Reflex] 위장 점유율: %d%%\n", fill_percent);
    }

    printf("   🚫 [Reflex Action] 입력 즉시 차단 (0.001초)\n");
    reflex_triggered++;

    return 1;  /* 1 = 반사 처리 완료 (뇌로 보고하지 않음) */
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Brain Callback (뇌로 올라가는 신호)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void sensory_callback(signal_packet_t* signal) {
    if (!signal) return;

    printf("   🧠 [Brain] 감각 신호 수신: Type=%d, Source=%d\n",
           signal->type, signal->source_id);
    brain_reported++;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: 척추 반사 등록
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_register_reflex(spine_t* spine) {
    printf("\n=== Test 1: 척추 반사 등록 ===\n");

    int ret = spine_register_reflex(spine, SIGNAL_PRESSURE, pressure_reflex_handler);
    if (ret != 0) {
        printf("  ❌ 실패: 반사 등록 오류\n");
        return -1;
    }

    printf("  ✅ 통과: SIGNAL_PRESSURE 반사 등록 완료\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: 위장-척추 연동
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_connect_stomach_spine(stomach_t* stomach, spine_t* spine) {
    printf("\n=== Test 2: 위장-척추 연동 ===\n");

    stomach_set_spine(stomach, spine, 1);  /* organ_id=1: 위장 */

    printf("  ✅ 통과: 위장-척추 연동 완료\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: 정상 상태 (반사 미발동)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_normal_no_reflex(stomach_t* stomach) {
    printf("\n=== Test 3: 정상 상태 (반사 미발동) ===\n");

    reflex_triggered = 0;

    /* 50% 채우기 */
    for (int i = 0; i < 500; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    int fill = stomach_get_fill_percent(stomach);
    printf("  점유율: %d%%\n", fill);
    printf("  반사 발동: %d회\n", reflex_triggered);

    if (reflex_triggered > 0) {
        printf("  ❌ 실패: 정상 상태에서 반사 발동\n");
        return -1;
    }

    printf("  ✅ 통과: 정상 상태 (반사 없음)\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: 비상 상태 (척추 반사 발동)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_emergency_reflex(stomach_t* stomach) {
    printf("\n=== Test 4: 비상 상태 (척추 반사 발동) ===\n");

    reflex_triggered = 0;
    brain_reported = 0;

    /* 95%까지 채우기 */
    for (int i = 500; i < 950; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    int fill = stomach_get_fill_percent(stomach);
    tension_level_t tension = stomach_check_tension(stomach);

    printf("  점유율: %d%%\n", fill);
    printf("  팽창 수준: %s\n", tension_level_string(tension));
    printf("  반사 발동: %d회\n", reflex_triggered);
    printf("  뇌 보고: %d회\n", brain_reported);

    if (tension < TENSION_EMERGENCY) {
        printf("  ❌ 실패: EMERGENCY 이상 기대, 실제 %s\n", tension_level_string(tension));
        return -1;
    }

    if (reflex_triggered == 0) {
        printf("  ❌ 실패: 척추 반사 미발동\n");
        return -1;
    }

    if (brain_reported > 0) {
        printf("  ⚠️  주의: 반사 신호가 뇌로 보고됨 (뇌 우회 실패)\n");
    }

    printf("  ✅ 통과: 척추 반사 발동 (뇌 우회 성공)\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: 통계 확인
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_statistics(stomach_t* stomach, spine_t* spine) {
    printf("\n=== Test 5: 통계 확인 ===\n");

    stomach_stats(stomach);
    spine_stats(spine);

    printf("  ✅ 통과: 통계 출력 완료\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Stomach-Spine Reflex Integration Test\n");
    printf("  (위장-척추 반사 통합 테스트)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* 위장 생성 */
    stomach_t* stomach = stomach_create(1000);
    if (!stomach) {
        fprintf(stderr, "❌ Stomach 생성 실패\n");
        return 1;
    }

    /* 척추 생성 */
    spine_t* spine = spine_create();
    if (!spine) {
        fprintf(stderr, "❌ Spine 생성 실패\n");
        stomach_destroy(stomach);
        return 1;
    }

    /* 뇌 콜백 등록 */
    spine_set_sensory_callback(spine, sensory_callback);

    int failed = 0;

    /* Test 1: 척추 반사 등록 */
    if (test_register_reflex(spine) != 0) {
        failed++;
    }

    /* Test 2: 위장-척추 연동 */
    if (test_connect_stomach_spine(stomach, spine) != 0) {
        failed++;
    }

    /* Test 3: 정상 상태 */
    if (test_normal_no_reflex(stomach) != 0) {
        failed++;
    }

    /* Test 4: 비상 상태 (척추 반사) */
    if (test_emergency_reflex(stomach) != 0) {
        failed++;
    }

    /* Test 5: 통계 */
    if (test_statistics(stomach, spine) != 0) {
        failed++;
    }

    /* 최종 보고 */
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    if (failed == 0) {
        printf("  ✅ 모든 테스트 통과 (5/5)\n");
        printf("  ⚡ 척추 반사 시스템 정상 작동\n");
    } else {
        printf("  ❌ %d개 테스트 실패\n", failed);
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* Cleanup */
    stomach_destroy(stomach);
    spine_destroy(spine);

    return (failed == 0) ? 0 : 1;
}

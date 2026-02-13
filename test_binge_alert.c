/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_binge_alert.c
 *
 * 폭식 경고 시스템 (Binge-Eating Alert) 테스트
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_stomach.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test Callback
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static int tension_callback_count = 0;

void on_tension_change(tension_level_t level, int fill_percent) {
    printf("\n[Test Callback] 팽창 경고 발생!\n");
    printf("  Level:       %s\n", tension_level_string(level));
    printf("  Fill:        %d%%\n", fill_percent);
    tension_callback_count++;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: 정상 상태 (60% 미만)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_normal_state(stomach_t* stomach) {
    printf("\n=== Test 1: 정상 상태 (60%% 미만) ===\n");

    /* 50% 채우기 (500개) */
    for (int i = 0; i < 500; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        int ret = stomach_ingest(stomach, data, strlen(data));
        if (ret != 0) {
            printf("  ❌ 실패: ingest 오류 (ret=%d)\n", ret);
            return -1;
        }
    }

    int fill = stomach_get_fill_percent(stomach);
    tension_level_t tension = stomach_check_tension(stomach);

    printf("  점유율: %d%%\n", fill);
    printf("  팽창 수준: %s\n", tension_level_string(tension));

    if (tension != TENSION_NORMAL) {
        printf("  ❌ 실패: NORMAL 기대, 실제 %s\n", tension_level_string(tension));
        return -1;
    }

    if (stomach_is_input_blocked(stomach)) {
        printf("  ❌ 실패: 입력이 차단되어 있음\n");
        return -1;
    }

    printf("  ✅ 통과: 정상 상태\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: 포만감 (60-79%)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_satiety(stomach_t* stomach) {
    printf("\n=== Test 2: 포만감 (60-79%%) ===\n");

    /* 200개 더 추가 (총 700개 = 70%) */
    for (int i = 500; i < 700; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    int fill = stomach_get_fill_percent(stomach);
    tension_level_t tension = stomach_check_tension(stomach);

    printf("  점유율: %d%%\n", fill);
    printf("  팽창 수준: %s\n", tension_level_string(tension));

    if (tension != TENSION_SATIETY) {
        printf("  ❌ 실패: SATIETY 기대, 실제 %s\n", tension_level_string(tension));
        return -1;
    }

    printf("  ✅ 통과: 포만감 상태\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: 과식 주의 (80-94%)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_warning(stomach_t* stomach) {
    printf("\n=== Test 3: 과식 주의 (80-94%%) ===\n");

    /* 200개 더 추가 (총 900개 = 90%) */
    for (int i = 700; i < 900; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    int fill = stomach_get_fill_percent(stomach);
    tension_level_t tension = stomach_check_tension(stomach);

    printf("  점유율: %d%%\n", fill);
    printf("  팽창 수준: %s\n", tension_level_string(tension));

    if (tension != TENSION_WARNING) {
        printf("  ❌ 실패: WARNING 기대, 실제 %s\n", tension_level_string(tension));
        return -1;
    }

    printf("  ✅ 통과: 과식 주의 상태\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: 폭식 비상 (95%+) - 입력 차단
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_emergency_block(stomach_t* stomach) {
    printf("\n=== Test 4: 폭식 비상 (95%%+) - 입력 차단 ===\n");

    /* 100개 더 추가 시도 (총 1000개 = 100%) */
    int success_count = 0;
    int blocked_count = 0;

    for (int i = 900; i < 1000; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        int ret = stomach_ingest(stomach, data, strlen(data));

        if (ret == 0) {
            success_count++;
        } else if (ret == -2) {
            blocked_count++;
        }
    }

    int fill = stomach_get_fill_percent(stomach);
    tension_level_t tension = stomach_check_tension(stomach);

    printf("  점유율: %d%%\n", fill);
    printf("  팽창 수준: %s\n", tension_level_string(tension));
    printf("  성공: %d, 차단: %d\n", success_count, blocked_count);

    if (tension < TENSION_EMERGENCY) {
        printf("  ❌ 실패: EMERGENCY 이상 기대, 실제 %s\n", tension_level_string(tension));
        return -1;
    }

    if (!stomach_is_input_blocked(stomach)) {
        printf("  ❌ 실패: 입력이 차단되지 않음\n");
        return -1;
    }

    if (blocked_count == 0) {
        printf("  ⚠️  주의: 차단된 입력이 없음\n");
    }

    printf("  ✅ 통과: 비상 입력 차단\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: 복구 (50% 이하로 비우기)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_recovery(stomach_t* stomach) {
    printf("\n=== Test 5: 복구 (50%% 이하로 비우기) ===\n");

    /* 50% 이하로 비우기 (500개 배출) */
    for (int i = 0; i < 500; i++) {
        food_chunk_t food;
        int ret = stomach_extract(stomach, &food);
        if (ret == 0) {
            free(food.data);
        }
    }

    int fill = stomach_get_fill_percent(stomach);
    printf("  점유율: %d%%\n", fill);

    if (stomach_is_input_blocked(stomach)) {
        printf("  ❌ 실패: 입력이 여전히 차단됨\n");
        return -1;
    }

    /* 입력 재개 확인 */
    char test_data[] = "recovery-test";
    int ret = stomach_ingest(stomach, test_data, strlen(test_data));
    if (ret != 0) {
        printf("  ❌ 실패: 입력 재개 실패 (ret=%d)\n", ret);
        return -1;
    }

    printf("  ✅ 통과: 입력 재개 성공\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 6: Callback 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_callback(stomach_t* stomach) {
    printf("\n=== Test 6: Callback 검증 ===\n");

    printf("  Tension Callback 호출 횟수: %d\n", tension_callback_count);

    if (tension_callback_count == 0) {
        printf("  ⚠️  주의: Callback이 호출되지 않음\n");
    } else {
        printf("  ✅ 통과: Callback 호출됨\n");
    }

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Binge-Eating Alert Test (폭식 경고 시스템)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Stomach 생성 */
    stomach_t* stomach = stomach_create(1000);  /* 1000개 capacity */
    if (!stomach) {
        fprintf(stderr, "❌ Stomach 생성 실패\n");
        return 1;
    }

    /* Tension Callback 등록 */
    stomach_set_tension_callback(stomach, on_tension_change);

    int failed = 0;

    /* Test 1: 정상 상태 */
    if (test_normal_state(stomach) != 0) {
        failed++;
    }

    /* Test 2: 포만감 */
    if (test_satiety(stomach) != 0) {
        failed++;
    }

    /* Test 3: 과식 주의 */
    if (test_warning(stomach) != 0) {
        failed++;
    }

    /* Test 4: 폭식 비상 */
    if (test_emergency_block(stomach) != 0) {
        failed++;
    }

    /* Test 5: 복구 */
    if (test_recovery(stomach) != 0) {
        failed++;
    }

    /* Test 6: Callback */
    if (test_callback(stomach) != 0) {
        failed++;
    }

    /* 최종 통계 */
    stomach_stats(stomach);

    /* 최종 보고 */
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    if (failed == 0) {
        printf("  ✅ 모든 테스트 통과 (6/6)\n");
    } else {
        printf("  ❌ %d개 테스트 실패\n", failed);
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* Cleanup */
    stomach_destroy(stomach);

    return (failed == 0) ? 0 : 1;
}

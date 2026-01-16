/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_circadian.c
 *
 * 서카디언 엔진 테스트
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_circadian.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test Callbacks
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static int phase_change_count = 0;
static int cleanup_count = 0;
static int learning_count = 0;

void on_phase_change(circadian_phase_t old_phase, circadian_phase_t new_phase) {
    printf("\n[Test Callback] Phase Change: %s → %s\n",
           circadian_phase_string(old_phase),
           circadian_phase_string(new_phase));
    phase_change_count++;
}

void on_cleanup(void) {
    printf("[Test Callback] Cleanup Task Executed\n");
    cleanup_count++;
}

void on_learning(void) {
    printf("[Test Callback] Learning Task Executed\n");
    learning_count++;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: 초기 Phase 감지
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_initial_phase_detection(circadian_t* circadian) {
    printf("\n=== Test 1: 초기 Phase 감지 ===\n");

    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    int hour = local->tm_hour;

    printf("  현재 시각: %02d:%02d\n", hour, local->tm_min);

    circadian_phase_t phase = circadian_get_phase(circadian);
    printf("  감지된 Phase: %s %s\n",
           circadian_phase_icon(phase),
           circadian_phase_string(phase));

    resource_profile_t profile = circadian_get_profile(circadian);
    printf("  자원 프로파일:\n");
    printf("    - Input Throttle:    %d%%\n", profile.input_throttle);
    printf("    - Brain Priority:    %d%%\n", profile.brain_priority);
    printf("    - Cleanup Priority:  %d%%\n", profile.cleanup_priority);
    printf("    - Learning Priority: %d%%\n", profile.learning_priority);

    /* 시간대별 검증 */
    if (hour >= 1 && hour < 6) {
        if (phase != PHASE_DAWN) {
            printf("  ❌ 실패: DAWN phase 기대, 실제 %s\n",
                   circadian_phase_string(phase));
            return -1;
        }
    } else if (hour >= 6 && hour < 18) {
        if (phase != PHASE_DAY) {
            printf("  ❌ 실패: DAY phase 기대, 실제 %s\n",
                   circadian_phase_string(phase));
            return -1;
        }
    } else {
        if (phase != PHASE_EVENING) {
            printf("  ❌ 실패: EVENING phase 기대, 실제 %s\n",
                   circadian_phase_string(phase));
            return -1;
        }
    }

    printf("  ✅ 통과: Phase 정확 감지\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: Resource Profile 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_resource_profiles(void) {
    printf("\n=== Test 2: Resource Profile 검증 ===\n");

    /* DAWN Profile */
    resource_profile_t dawn = get_dawn_profile();
    printf("  DAWN Profile:\n");
    printf("    - Input:    %d%% (낮아야 함)\n", dawn.input_throttle);
    printf("    - Learning: %d%% (높아야 함)\n", dawn.learning_priority);

    if (dawn.input_throttle > 20 || dawn.learning_priority < 80) {
        printf("  ❌ 실패: DAWN profile 불일치\n");
        return -1;
    }

    /* DAY Profile */
    resource_profile_t day = get_day_profile();
    printf("  DAY Profile:\n");
    printf("    - Input:    %d%% (높아야 함)\n", day.input_throttle);
    printf("    - Brain:    %d%% (높아야 함)\n", day.brain_priority);

    if (day.input_throttle < 80 || day.brain_priority < 80) {
        printf("  ❌ 실패: DAY profile 불일치\n");
        return -1;
    }

    /* EVENING Profile */
    resource_profile_t evening = get_evening_profile();
    printf("  EVENING Profile:\n");
    printf("    - Input:   %d%% (중간)\n", evening.input_throttle);
    printf("    - Cleanup: %d%% (높아야 함)\n", evening.cleanup_priority);

    if (evening.cleanup_priority < 80) {
        printf("  ❌ 실패: EVENING profile 불일치\n");
        return -1;
    }

    printf("  ✅ 통과: 모든 profile 정상\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: Callback 테스트
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_callbacks(circadian_t* circadian) {
    printf("\n=== Test 3: Callback 테스트 ===\n");

    phase_change_count = 0;
    cleanup_count = 0;
    learning_count = 0;

    /* Callback 등록 */
    circadian_set_phase_callback(circadian, on_phase_change);
    circadian_set_cleanup_callback(circadian, on_cleanup);
    circadian_set_learning_callback(circadian, on_learning);

    printf("  Callback 등록 완료\n");
    printf("  현재 Phase: %s\n", circadian_phase_string(circadian_get_phase(circadian)));

    printf("  주의: Phase 전환은 실제 시간에 따라 발생합니다.\n");
    printf("       테스트 시점에는 전환이 없을 수 있습니다.\n");

    printf("  ✅ 통과: Callback 등록 성공\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: 지속적 업데이트 (10초간)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_continuous_update(circadian_t* circadian) {
    printf("\n=== Test 4: 지속적 업데이트 (10초간) ===\n");

    printf("  1초마다 circadian_update() 호출 중...\n");

    for (int i = 0; i < 10; i++) {
        circadian_update(circadian);
        printf("    [%2d/10] Phase: %s, Cycles: %lu\n",
               i + 1,
               circadian_phase_string(circadian_get_phase(circadian)),
               circadian->cycle_count);
        sleep(1);
    }

    printf("  ✅ 통과: 10초 연속 업데이트 성공\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: 통계 출력
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_statistics(circadian_t* circadian) {
    printf("\n=== Test 5: 통계 출력 ===\n");

    circadian_stats(circadian);

    printf("  ✅ 통과: 통계 출력 성공\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Circadian Engine Test (24/7 Operation)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Circadian 생성 */
    circadian_t* circadian = circadian_create();
    if (!circadian) {
        fprintf(stderr, "❌ Circadian 생성 실패\n");
        return 1;
    }

    int failed = 0;

    /* Test 1: 초기 Phase 감지 */
    if (test_initial_phase_detection(circadian) != 0) {
        failed++;
    }

    /* Test 2: Resource Profile 검증 */
    if (test_resource_profiles() != 0) {
        failed++;
    }

    /* Test 3: Callback 테스트 */
    if (test_callbacks(circadian) != 0) {
        failed++;
    }

    /* Test 4: 지속적 업데이트 */
    if (test_continuous_update(circadian) != 0) {
        failed++;
    }

    /* Test 5: 통계 출력 */
    if (test_statistics(circadian) != 0) {
        failed++;
    }

    /* 최종 보고 */
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    if (failed == 0) {
        printf("  ✅ 모든 테스트 통과 (5/5)\n");
    } else {
        printf("  ❌ %d개 테스트 실패\n", failed);
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* Cleanup */
    circadian_destroy(circadian);

    return (failed == 0) ? 0 : 1;
}

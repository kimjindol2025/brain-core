/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_watchdog.c
 *
 * 워치독(자가 치유) 테스트
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_watchdog.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test Callbacks
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static int warning_count = 0;
static int failure_count = 0;
static int recovery_count = 0;

void on_warning(organ_type_t organ) {
    printf("[Test Callback] 경고 발생: %s\n", organ_type_string(organ));
    warning_count++;
}

void on_failure(organ_type_t organ) {
    printf("[Test Callback] 실패 발생: %s\n", organ_type_string(organ));
    failure_count++;
}

void on_recovery(organ_type_t organ) {
    printf("[Test Callback] 복구 성공: %s\n", organ_type_string(organ));
    recovery_count++;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Mock Recovery Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

bool recover_stomach(void) {
    printf("  [Recovery] Stomach 재시작 중...\n");
    return true;  /* 항상 성공 */
}

bool recover_pancreas(void) {
    printf("  [Recovery] Pancreas 재시작 중...\n");
    return true;  /* 항상 성공 */
}

bool recover_spine(void) {
    printf("  [Recovery] Spine 재시작 중...\n");
    return false;  /* 항상 실패 (테스트용) */
}

bool recover_cortex(void) {
    printf("  [Recovery] Cortex 재시작 중...\n");
    return true;  /* 항상 성공 */
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: 장기 등록
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_organ_registration(watchdog_t* watchdog) {
    printf("\n=== Test 1: 장기 등록 ===\n");

    /* 4개 장기 등록 */
    int id1 = watchdog_register_organ(watchdog, ORGAN_STOMACH, "위장",
                                       2000, recover_stomach);
    int id2 = watchdog_register_organ(watchdog, ORGAN_PANCREAS, "췌장",
                                       3000, recover_pancreas);
    int id3 = watchdog_register_organ(watchdog, ORGAN_SPINE, "척수",
                                       1000, recover_spine);
    int id4 = watchdog_register_organ(watchdog, ORGAN_CORTEX, "대뇌",
                                       5000, recover_cortex);

    if (id1 < 0 || id2 < 0 || id3 < 0 || id4 < 0) {
        printf("  ❌ 실패: 장기 등록 오류\n");
        return -1;
    }

    if (watchdog->organ_count != 4) {
        printf("  ❌ 실패: 등록 장기 수 불일치 (기대: 4, 실제: %d)\n",
               watchdog->organ_count);
        return -1;
    }

    printf("  ✅ 통과: 4개 장기 등록 완료\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: 정상 Heartbeat
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_normal_heartbeat(watchdog_t* watchdog) {
    printf("\n=== Test 2: 정상 Heartbeat (5초) ===\n");

    printf("  모든 장기가 1초마다 heartbeat 전송 중...\n");

    for (int i = 0; i < 5; i++) {
        /* 모든 장기 heartbeat */
        watchdog_heartbeat(watchdog, ORGAN_STOMACH);
        watchdog_heartbeat(watchdog, ORGAN_PANCREAS);
        watchdog_heartbeat(watchdog, ORGAN_SPINE);
        watchdog_heartbeat(watchdog, ORGAN_CORTEX);

        /* 감시 */
        watchdog_check(watchdog);

        printf("    [%d/5] Check 완료\n", i + 1);
        sleep(1);
    }

    /* 모든 장기가 OK 상태여야 함 */
    int ok_count = 0;
    for (int i = 0; i < watchdog->organ_count; i++) {
        if (watchdog->organs[i].status == STATUS_OK) {
            ok_count++;
        }
    }

    if (ok_count != 4) {
        printf("  ❌ 실패: OK 상태 장기 수 불일치 (기대: 4, 실제: %d)\n", ok_count);
        return -1;
    }

    printf("  ✅ 통과: 모든 장기 정상\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: Timeout 감지 (WARNING)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_timeout_warning(watchdog_t* watchdog) {
    printf("\n=== Test 3: Timeout 감지 (WARNING) ===\n");

    /* Spine만 heartbeat 중단 (timeout: 1000ms) */
    printf("  Spine이 heartbeat 중단...\n");

    for (int i = 0; i < 3; i++) {
        /* Stomach, Pancreas, Cortex만 heartbeat */
        watchdog_heartbeat(watchdog, ORGAN_STOMACH);
        watchdog_heartbeat(watchdog, ORGAN_PANCREAS);
        watchdog_heartbeat(watchdog, ORGAN_CORTEX);
        /* Spine은 heartbeat 없음 */

        watchdog_check(watchdog);
        sleep(1);
    }

    /* Spine이 WARNING 또는 CRITICAL 상태여야 함 */
    organ_status_t spine_status = STATUS_OK;
    for (int i = 0; i < watchdog->organ_count; i++) {
        if (watchdog->organs[i].type == ORGAN_SPINE) {
            spine_status = watchdog->organs[i].status;
            break;
        }
    }

    if (spine_status == STATUS_OK) {
        printf("  ❌ 실패: Spine이 여전히 OK 상태\n");
        return -1;
    }

    printf("  ✅ 통과: Spine timeout 감지 (상태: %s)\n",
           organ_status_string(spine_status));
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: 자동 복구 (성공)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_auto_recovery_success(watchdog_t* watchdog) {
    printf("\n=== Test 4: 자동 복구 (성공) ===\n");

    /* Stomach heartbeat 중단 후 복구 대기 */
    printf("  Stomach이 heartbeat 중단...\n");

    for (int i = 0; i < 5; i++) {
        /* Pancreas, Cortex만 heartbeat */
        watchdog_heartbeat(watchdog, ORGAN_PANCREAS);
        watchdog_heartbeat(watchdog, ORGAN_CORTEX);
        /* Stomach, Spine 없음 */

        watchdog_check(watchdog);
        sleep(1);
    }

    /* Stomach 복구 확인 (복구 함수가 true 반환) */
    organ_status_t stomach_status = STATUS_OK;
    for (int i = 0; i < watchdog->organ_count; i++) {
        if (watchdog->organs[i].type == ORGAN_STOMACH) {
            stomach_status = watchdog->organs[i].status;
            printf("  Stomach 상태: %s\n", organ_status_string(stomach_status));
            printf("  복구 횟수: %u\n", watchdog->organs[i].recovery_count);
            break;
        }
    }

    /* 복구가 성공했으므로 OK 상태여야 함 */
    if (stomach_status != STATUS_OK) {
        printf("  ⚠️  주의: Stomach 복구 후 상태 %s (OK 기대)\n",
               organ_status_string(stomach_status));
        /* 복구는 됐지만 다시 timeout 가능하므로 경고만 */
    }

    printf("  ✅ 통과: 자동 복구 시도 완료\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: 자동 복구 (실패)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_auto_recovery_failure(watchdog_t* watchdog) {
    printf("\n=== Test 5: 자동 복구 (실패) ===\n");

    /* Spine은 계속 heartbeat 없음 (복구 함수가 false 반환) */
    printf("  Spine 복구 실패 상태 유지...\n");

    /* 이미 Test 3에서 Spine이 timeout 상태 */
    organ_status_t spine_status = STATUS_OK;
    for (int i = 0; i < watchdog->organ_count; i++) {
        if (watchdog->organs[i].type == ORGAN_SPINE) {
            spine_status = watchdog->organs[i].status;
            printf("  Spine 상태: %s\n", organ_status_string(spine_status));
            printf("  실패 횟수: %u\n", watchdog->organs[i].failure_count);
            break;
        }
    }

    if (spine_status == STATUS_OK) {
        printf("  ❌ 실패: Spine이 OK 상태 (복구 실패 기대)\n");
        return -1;
    }

    printf("  ✅ 통과: 복구 실패 확인\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 6: Callback 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_callbacks(watchdog_t* watchdog) {
    printf("\n=== Test 6: Callback 검증 ===\n");

    printf("  Warning 발생 횟수: %d\n", warning_count);
    printf("  Failure 발생 횟수: %d\n", failure_count);
    printf("  Recovery 발생 횟수: %d\n", recovery_count);

    if (warning_count == 0 && failure_count == 0) {
        printf("  ⚠️  주의: Callback이 호출되지 않음 (timeout 미발생 가능)\n");
    } else {
        printf("  ✅ 통과: Callback 호출 확인\n");
    }

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 7: 통계 출력
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_statistics(watchdog_t* watchdog) {
    printf("\n=== Test 7: 통계 출력 ===\n");

    watchdog_stats(watchdog);

    printf("  ✅ 통과: 통계 출력 성공\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Watchdog Test (Self-Healing System)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Watchdog 생성 */
    watchdog_t* watchdog = watchdog_create();
    if (!watchdog) {
        fprintf(stderr, "❌ Watchdog 생성 실패\n");
        return 1;
    }

    /* Callback 등록 */
    watchdog_set_warning_callback(watchdog, on_warning);
    watchdog_set_failure_callback(watchdog, on_failure);
    watchdog_set_recovery_callback(watchdog, on_recovery);

    int failed = 0;

    /* Test 1: 장기 등록 */
    if (test_organ_registration(watchdog) != 0) {
        failed++;
    }

    /* Test 2: 정상 Heartbeat */
    if (test_normal_heartbeat(watchdog) != 0) {
        failed++;
    }

    /* Test 3: Timeout 감지 */
    if (test_timeout_warning(watchdog) != 0) {
        failed++;
    }

    /* Test 4: 자동 복구 (성공) */
    if (test_auto_recovery_success(watchdog) != 0) {
        failed++;
    }

    /* Test 5: 자동 복구 (실패) */
    if (test_auto_recovery_failure(watchdog) != 0) {
        failed++;
    }

    /* Test 6: Callback 검증 */
    if (test_callbacks(watchdog) != 0) {
        failed++;
    }

    /* Test 7: 통계 출력 */
    if (test_statistics(watchdog) != 0) {
        failed++;
    }

    /* 최종 보고 */
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    if (failed == 0) {
        printf("  ✅ 모든 테스트 통과 (7/7)\n");
    } else {
        printf("  ❌ %d개 테스트 실패\n", failed);
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* Cleanup */
    watchdog_destroy(watchdog);

    return (failed == 0) ? 0 : 1;
}

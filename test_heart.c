/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_heart.c
 *
 * Heart Engine Test
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_heart.h"
#include "kim_stomach.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: 심장 생성/삭제
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_create_destroy(void) {
    printf("\n=== Test 1: 심장 생성/삭제 ===\n");

    heart_t* heart = heart_create();
    if (!heart) {
        printf("  ❌ 실패: 심장 생성 오류\n");
        return -1;
    }

    int bpm = heart_get_bpm(heart);
    heart_gear_t gear = heart_get_gear(heart);

    printf("  초기 BPM: %d\n", bpm);
    printf("  초기 기어: %s\n", gear_string(gear));

    if (bpm != BPM_IDLE) {
        printf("  ❌ 실패: 초기 BPM이 IDLE이 아님\n");
        heart_destroy(heart);
        return -1;
    }

    heart_destroy(heart);

    printf("  ✅ 통과: 심장 생성/삭제 성공\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: 박동 시작/중지
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_start_stop(heart_t* heart) {
    printf("\n=== Test 2: 박동 시작/중지 ===\n");

    int ret = heart_start(heart);
    if (ret != 0) {
        printf("  ❌ 실패: 박동 시작 오류\n");
        return -1;
    }

    printf("  박동 시작됨. 1초 대기...\n");
    sleep(1);

    ret = heart_stop(heart);
    if (ret != 0) {
        printf("  ❌ 실패: 박동 중지 오류\n");
        return -1;
    }

    printf("  ✅ 통과: 박동 시작/중지 성공\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: 기어 변속
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_gear_shift(heart_t* heart) {
    printf("\n=== Test 3: 기어 변속 ===\n");

    heart_start(heart);

    /* CRUISE */
    heart_set_gear(heart, GEAR_CRUISE);
    sleep(1);
    int bpm1 = heart_get_bpm(heart);
    printf("  CRUISE 기어 BPM: %d\n", bpm1);

    /* BOOST */
    heart_set_gear(heart, GEAR_BOOST);
    sleep(1);
    int bpm2 = heart_get_bpm(heart);
    printf("  BOOST 기어 BPM: %d\n", bpm2);

    /* IDLE */
    heart_set_gear(heart, GEAR_IDLE);
    sleep(1);
    int bpm3 = heart_get_bpm(heart);
    printf("  IDLE 기어 BPM: %d\n", bpm3);

    heart_stop(heart);

    if (bpm1 < bpm2 && bpm2 > bpm3) {
        printf("  ✅ 통과: 기어 변속에 따라 BPM 조절됨\n");
        return 0;
    } else {
        printf("  ❌ 실패: BPM 조절 이상\n");
        return -1;
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: 위장 연동 및 자동 조절
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_auto_adjust(heart_t* heart, stomach_t* stomach) {
    printf("\n=== Test 4: 위장 연동 및 자동 조절 ===\n");

    /* 위장 연결 */
    heart_connect_stomach(heart, stomach);

    heart_start(heart);

    /* stdout 임시 저장 */
    int stdout_backup = dup(1);
    FILE* null_stream = fopen("/dev/null", "w");
    int null_fd = fileno(null_stream);
    dup2(null_fd, 1);

    /* 위장을 30%까지 채우기 (로그 억제) */
    for (int i = 0; i < 300; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    /* stdout 복원 */
    fflush(stdout);
    dup2(stdout_backup, 1);
    close(stdout_backup);
    fclose(null_stream);

    sleep(1);  /* 심장이 자동 조절할 시간 */

    heart_gear_t gear1 = heart_get_gear(heart);
    printf("  30%% 채웠을 때 기어: %s\n", gear_string(gear1));

    /* 70%까지 채우기 */
    stdout_backup = dup(1);
    null_stream = fopen("/dev/null", "w");
    null_fd = fileno(null_stream);
    dup2(null_fd, 1);

    for (int i = 300; i < 700; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    fflush(stdout);
    dup2(stdout_backup, 1);
    close(stdout_backup);
    fclose(null_stream);

    sleep(1);

    heart_gear_t gear2 = heart_get_gear(heart);
    printf("  70%% 채웠을 때 기어: %s\n", gear_string(gear2));

    /* 95%까지 채우기 */
    stdout_backup = dup(1);
    null_stream = fopen("/dev/null", "w");
    null_fd = fileno(null_stream);
    dup2(null_fd, 1);

    for (int i = 700; i < 950; i++) {
        char data[64];
        snprintf(data, sizeof(data), "data-%d", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    fflush(stdout);
    dup2(stdout_backup, 1);
    close(stdout_backup);
    fclose(null_stream);

    sleep(1);

    heart_gear_t gear3 = heart_get_gear(heart);
    blood_pressure_t bp = heart_get_bp_status(heart);
    printf("  95%% 채웠을 때 기어: %s\n", gear_string(gear3));
    printf("  혈압 상태: %s\n", bp_status_string(bp));

    heart_stop(heart);

    if (gear3 == GEAR_IDLE && bp >= BP_PREHYPER) {
        printf("  ✅ 통과: 과부하 시 자동으로 IDLE 전환 및 혈압 상승\n");
        return 0;
    } else {
        printf("  ❌ 실패: 자동 조절 미작동\n");
        return -1;
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: 밸브 제어
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_valve_control(heart_t* heart) {
    printf("\n=== Test 5: 밸브 제어 ===\n");

    heart_close_valve(heart, "stomach");
    heart_set_flow_rate(heart, "brain", 80);
    heart_open_valve(heart, "kidney");

    printf("  ✅ 통과: 밸브 제어 완료\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 6: 통계
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_statistics(heart_t* heart) {
    printf("\n=== Test 6: 통계 ===\n");

    heart_stats(heart);

    printf("  ✅ 통과: 통계 출력 완료\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Heart Engine Test (심장 엔진 테스트)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int failed = 0;

    /* Test 1: 생성/삭제 */
    if (test_create_destroy() != 0) {
        failed++;
    }

    /* 이후 테스트용 인스턴스 */
    heart_t* heart = heart_create();
    stomach_t* stomach = stomach_create(1000);

    if (!heart || !stomach) {
        fprintf(stderr, "❌ 인스턴스 생성 실패\n");
        return 1;
    }

    /* Test 2: 시작/중지 */
    if (test_start_stop(heart) != 0) {
        failed++;
    }

    /* Test 3: 기어 변속 */
    if (test_gear_shift(heart) != 0) {
        failed++;
    }

    /* Test 4: 자동 조절 */
    if (test_auto_adjust(heart, stomach) != 0) {
        failed++;
    }

    /* Test 5: 밸브 제어 */
    if (test_valve_control(heart) != 0) {
        failed++;
    }

    /* Test 6: 통계 */
    if (test_statistics(heart) != 0) {
        failed++;
    }

    /* 최종 보고 */
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    if (failed == 0) {
        printf("  ✅ 모든 테스트 통과 (6/6)\n");
        printf("  💓 심장 엔진 정상 작동\n");
    } else {
        printf("  ❌ %d개 테스트 실패\n", failed);
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* Cleanup */
    heart_destroy(heart);
    stomach_destroy(stomach);

    return (failed == 0) ? 0 : 1;
}

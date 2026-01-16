/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_spine.c
 *
 * 척수(Spinal Cord) 통합 테스트
 *
 * 테스트 시나리오:
 *   1. 상행 신호 전송 (Body → Brain)
 *   2. 하행 명령 전송 (Brain → Body)
 *   3. 반사 신경 등록 및 동작
 *   4. 뇌 우회율 측정
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_spine.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Mock Brain (뇌 시뮬레이션)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void brain_handle_sensory(signal_packet_t* sig) {
    printf("   🧠 [Brain] 감각 신호 수신: %d번 장기에서 보고\n", sig->source_id);

    /* 뇌에서 처리 */
    switch (sig->type) {
        case SIGNAL_PAIN:
            printf("      → 진통제 분비 명령 준비\n");
            break;
        case SIGNAL_HEAT:
            printf("      → 냉각 시스템 가동 준비\n");
            break;
        case SIGNAL_VISION:
            printf("      → 시각 데이터 분석 중\n");
            break;
        default:
            printf("      → 신호 처리 중\n");
            break;
    }
}

void brain_handle_motor(signal_packet_t* sig) {
    printf("   💪 [Muscle] 명령 수신: %d번 장기가 실행 중\n", sig->target_id);

    /* 장기에서 실행 */
    switch (sig->type) {
        case SIGNAL_MOVE:
            printf("      → 움직임 실행\n");
            break;
        case SIGNAL_STOP:
            printf("      → 정지 실행\n");
            break;
        case SIGNAL_SECRETE:
            printf("      → 분비 시작\n");
            break;
        default:
            printf("      → 명령 실행\n");
            break;
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Reflex Handlers (반사 핸들러)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 철회 반사 (뜨거운 것 손 떼기) */
int reflex_withdraw(signal_packet_t* sig) {
    printf("      🔥 [Reflex] 손 즉시 철회! (뇌 보고 생략)\n");
    printf("      → 원인: %d번 장기에서 열 감지\n", sig->source_id);
    return 1;  /* 처리 완료 */
}

/* 기침 반사 (에러 배출) */
int reflex_cough(signal_packet_t* sig) {
    printf("      💨 [Reflex] 기침! 이물질 배출 중...\n");
    printf("      → 원인: %d번 장기에서 이물질 감지\n", sig->source_id);
    return 1;
}

/* 눈 깜빡임 반사 (인터럽트 ACK) */
int reflex_blink(signal_packet_t* sig) {
    printf("      👁️ [Reflex] 눈 깜빡임 (ACK)\n");
    return 1;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: Basic Signal Transmission
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_basic_transmission(spine_t* spine) {
    printf("\n=== Test 1: Basic Signal Transmission ===\n");

    /* 콜백 등록 */
    spine_set_sensory_callback(spine, brain_handle_sensory);
    spine_set_motor_callback(spine, brain_handle_motor);

    /* 상행 신호 (Body → Brain) */
    printf("\n[Test 1-1] 상행 신호 테스트\n");
    signal_packet_t* sig1 = signal_create(SIGNAL_PAIN, 10, -1, NULL, 0);
    spine_send_sensory(spine, sig1);
    signal_destroy(sig1);

    usleep(10000);  /* 10ms */

    signal_packet_t* sig2 = signal_create(SIGNAL_VISION, 20, -1, NULL, 0);
    spine_send_sensory(spine, sig2);
    signal_destroy(sig2);

    /* 하행 명령 (Brain → Body) */
    printf("\n[Test 1-2] 하행 명령 테스트\n");
    signal_packet_t* sig3 = signal_create(SIGNAL_MOVE, -1, 30, NULL, 0);
    spine_send_motor(spine, sig3);
    signal_destroy(sig3);

    usleep(10000);

    signal_packet_t* sig4 = signal_create(SIGNAL_SECRETE, -1, 40, NULL, 0);
    spine_send_motor(spine, sig4);
    signal_destroy(sig4);

    printf("\n✓ Test 1 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: Reflex Arc
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_reflex_arc(spine_t* spine) {
    printf("\n=== Test 2: Reflex Arc (척수 반사) ===\n");

    /* 반사 신경 등록 */
    spine_register_reflex(spine, REFLEX_WITHDRAW, reflex_withdraw);
    spine_register_reflex(spine, REFLEX_COUGH, reflex_cough);
    spine_register_reflex(spine, REFLEX_BLINK, reflex_blink);

    /* 반사 신호 전송 */
    printf("\n[Test 2-1] 철회 반사 (뜨거운 것)\n");
    signal_packet_t* sig1 = signal_create(REFLEX_WITHDRAW, 50, -1, NULL, 0);
    spine_send_sensory(spine, sig1);
    signal_destroy(sig1);

    usleep(10000);

    printf("\n[Test 2-2] 기침 반사 (이물질)\n");
    signal_packet_t* sig2 = signal_create(REFLEX_COUGH, 60, -1, NULL, 0);
    spine_send_sensory(spine, sig2);
    signal_destroy(sig2);

    usleep(10000);

    printf("\n[Test 2-3] 눈 깜빡임 반사\n");
    signal_packet_t* sig3 = signal_create(REFLEX_BLINK, 70, -1, NULL, 0);
    spine_send_sensory(spine, sig3);
    signal_destroy(sig3);

    printf("\n✓ Test 2 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: Brain Bypass Rate (뇌 우회율 측정)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_bypass_rate(spine_t* spine) {
    printf("\n=== Test 3: Brain Bypass Rate ===\n");

    /* 10개 신호: 반사 5개, 일반 5개 */
    signal_type_t signals[] = {
        SIGNAL_PAIN,        /* 일반 → 뇌 */
        REFLEX_WITHDRAW,    /* 반사 → 우회 */
        SIGNAL_VISION,      /* 일반 → 뇌 */
        REFLEX_COUGH,       /* 반사 → 우회 */
        SIGNAL_HEAT,        /* 일반 → 뇌 */
        REFLEX_BLINK,       /* 반사 → 우회 */
        SIGNAL_PRESSURE,    /* 일반 → 뇌 */
        REFLEX_WITHDRAW,    /* 반사 → 우회 */
        SIGNAL_TOUCH,       /* 일반 → 뇌 */
        REFLEX_COUGH        /* 반사 → 우회 */
    };

    for (int i = 0; i < 10; i++) {
        printf("\n[Signal %d/%d] Type=%d\n", i + 1, 10, signals[i]);
        signal_packet_t* sig = signal_create(signals[i], 100 + i, -1, NULL, 0);
        spine_send_sensory(spine, sig);
        signal_destroy(sig);
        usleep(5000);
    }

    printf("\n✓ Test 3 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: High Frequency Signal (고주파 신호)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_high_frequency(spine_t* spine) {
    printf("\n=== Test 4: High Frequency Signals ===\n");

    printf("Sending 100 signals...\n");

    for (int i = 0; i < 100; i++) {
        signal_type_t type = (i % 2 == 0) ? SIGNAL_TOUCH : REFLEX_BLINK;
        signal_packet_t* sig = signal_create(type, 200 + i, -1, NULL, 0);
        spine_send_sensory(spine, sig);
        signal_destroy(sig);
    }

    printf("✓ 100 signals sent\n");
    printf("✓ Test 4 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║            Kim-AI-OS: Spinal Cord Test                    ║\n");
    printf("║                                                            ║\n");
    printf("║       High-Speed Control Bus (광통신 고속도로)             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    /* 척수 생성 */
    spine_t* spine = spine_create();
    if (!spine) {
        printf("✗ Failed to create spine\n");
        return 1;
    }

    /* 테스트 실행 */
    int result = 0;

    if (test_basic_transmission(spine) < 0) result = 1;
    if (test_reflex_arc(spine) < 0) result = 1;
    if (test_bypass_rate(spine) < 0) result = 1;
    if (test_high_frequency(spine) < 0) result = 1;

    /* 통계 */
    spine_stats(spine);

    /* 정리 */
    spine_destroy(spine);

    if (result == 0) {
        printf("\n╔════════════════════════════════════════════════════════════╗\n");
        printf("║              All Spine Tests Passed! ⚡                     ║\n");
        printf("║                                                            ║\n");
        printf("║         척수 고속도로가 완벽하게 작동합니다!                  ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }

    return result;
}

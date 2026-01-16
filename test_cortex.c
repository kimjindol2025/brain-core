/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_cortex.c
 *
 * 대뇌 피질 통합 테스트
 *
 * 테스트 시나리오:
 *   1. 인사 처리 (GREETING)
 *   2. 질문 처리 (QUESTION)
 *   3. 명령 처리 (COMMAND)
 *   4. 위협 감지 (THREAT)
 *   5. 데이터 처리 (DATA)
 *   6. 연속 사고 (부하 테스트)
 *   7. 콜백 동작 확인
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_cortex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Mock Callbacks
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int decision_callback_count = 0;
int learning_callback_count = 0;

void mock_decision_callback(decision_t* decision) {
    decision_callback_count++;
    printf("      [Callback] 결정 콜백 호출됨: %s\n", decision_type_string(decision->type));
}

void mock_learning_callback(thought_t* thought) {
    learning_callback_count++;
    printf("      [Callback] 학습 콜백 호출됨: %s\n", thought->content);
}

void mock_brain_sensory(signal_packet_t* sig) {
    (void)sig;
}

void mock_brain_motor(signal_packet_t* sig) {
    (void)sig;
    printf("      [Callback] 척수 명령 수신: Type=%d\n", sig->type);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: Greeting Processing (인사 처리)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_greeting(cortex_t* cortex) {
    printf("\n=== Test 1: Greeting Processing ===\n");

    const char* greetings[] = {
        "안녕하세요!",
        "Hello, World!",
        "hi there"
    };

    for (int i = 0; i < 3; i++) {
        printf("\n[Test 1-%d] 입력: \"%s\"\n", i + 1, greetings[i]);

        decision_t* decision = cortex_think(cortex, greetings[i], strlen(greetings[i]));
        if (!decision) {
            printf("✗ Failed to process greeting\n");
            return -1;
        }

        if (decision->type != DECISION_RESPOND) {
            printf("✗ Wrong decision: expected RESPOND, got %s\n",
                   decision_type_string(decision->type));
            decision_destroy(decision);
            return -1;
        }

        decision_destroy(decision);
        usleep(100000);  /* 100ms */
    }

    printf("\n✓ Test 1 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: Question Processing (질문 처리)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_question(cortex_t* cortex) {
    printf("\n=== Test 2: Question Processing ===\n");

    const char* questions[] = {
        "What is this?",
        "How does it work?",
        "Why are you here?"
    };

    for (int i = 0; i < 3; i++) {
        printf("\n[Test 2-%d] 입력: \"%s\"\n", i + 1, questions[i]);

        decision_t* decision = cortex_think(cortex, questions[i], strlen(questions[i]));
        if (!decision) {
            printf("✗ Failed to process question\n");
            return -1;
        }

        if (decision->type != DECISION_RESPOND) {
            printf("✗ Wrong decision: expected RESPOND, got %s\n",
                   decision_type_string(decision->type));
            decision_destroy(decision);
            return -1;
        }

        decision_destroy(decision);
        usleep(100000);
    }

    printf("\n✓ Test 2 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: Command Processing (명령 처리)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_command(cortex_t* cortex) {
    printf("\n=== Test 3: Command Processing ===\n");

    const char* commands[] = {
        "실행해",
        "run this",
        "execute now"
    };

    for (int i = 0; i < 3; i++) {
        printf("\n[Test 3-%d] 입력: \"%s\"\n", i + 1, commands[i]);

        decision_t* decision = cortex_think(cortex, commands[i], strlen(commands[i]));
        if (!decision) {
            printf("✗ Failed to process command\n");
            return -1;
        }

        if (decision->type != DECISION_ACCEPT) {
            printf("✗ Wrong decision: expected ACCEPT, got %s\n",
                   decision_type_string(decision->type));
            decision_destroy(decision);
            return -1;
        }

        decision_destroy(decision);
        usleep(100000);
    }

    printf("\n✓ Test 3 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: Threat Detection (위협 감지)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_threat(cortex_t* cortex) {
    printf("\n=== Test 4: Threat Detection ===\n");

    const char* threats[] = {
        "공격!",
        "attack now",
        "hack the system"
    };

    for (int i = 0; i < 3; i++) {
        printf("\n[Test 4-%d] 입력: \"%s\"\n", i + 1, threats[i]);

        decision_t* decision = cortex_think(cortex, threats[i], strlen(threats[i]));
        if (!decision) {
            printf("✗ Failed to process threat\n");
            return -1;
        }

        if (decision->type != DECISION_DEFEND) {
            printf("✗ Wrong decision: expected DEFEND, got %s\n",
                   decision_type_string(decision->type));
            decision_destroy(decision);
            return -1;
        }

        decision_destroy(decision);
        usleep(100000);
    }

    printf("\n✓ Test 4 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: Data Processing (데이터 처리)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_data(cortex_t* cortex) {
    printf("\n=== Test 5: Data Processing ===\n");

    const char* data_samples[] = {
        "random data 123",
        "some information",
        "log entry"
    };

    for (int i = 0; i < 3; i++) {
        printf("\n[Test 5-%d] 입력: \"%s\"\n", i + 1, data_samples[i]);

        decision_t* decision = cortex_think(cortex, data_samples[i], strlen(data_samples[i]));
        if (!decision) {
            printf("✗ Failed to process data\n");
            return -1;
        }

        if (decision->type != DECISION_STORE) {
            printf("✗ Wrong decision: expected STORE, got %s\n",
                   decision_type_string(decision->type));
            decision_destroy(decision);
            return -1;
        }

        decision_destroy(decision);
        usleep(100000);
    }

    printf("\n✓ Test 5 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 6: Continuous Thinking (연속 사고)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_continuous(cortex_t* cortex) {
    printf("\n=== Test 6: Continuous Thinking (50 thoughts) ===\n");

    const char* inputs[] = {
        "hello",
        "what",
        "run",
        "attack",
        "data"
    };

    for (int i = 0; i < 50; i++) {
        const char* input = inputs[i % 5];

        if (i % 10 == 0) {
            printf("\n[Progress] %d/50 thoughts...\n", i);
        }

        decision_t* decision = cortex_think(cortex, input, strlen(input));
        if (!decision) {
            printf("✗ Failed at thought %d\n", i);
            return -1;
        }

        decision_destroy(decision);
    }

    printf("\n✓ Test 6 passed (50 thoughts processed)\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 7: Callback Verification (콜백 검증)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_callbacks(cortex_t* cortex) {
    printf("\n=== Test 7: Callback Verification ===\n");

    decision_callback_count = 0;
    learning_callback_count = 0;

    /* 인사 (학습 필요) */
    printf("\n[Test 7-1] 인사 입력 (학습 O)\n");
    decision_t* d1 = cortex_think(cortex, "안녕", 6);
    decision_destroy(d1);

    /* 데이터 (학습 필요) */
    printf("\n[Test 7-2] 데이터 입력 (학습 O)\n");
    decision_t* d2 = cortex_think(cortex, "info", 4);
    decision_destroy(d2);

    printf("\n[Callback Stats]\n");
    printf("  Decision callbacks: %d\n", decision_callback_count);
    printf("  Learning callbacks: %d\n", learning_callback_count);

    if (decision_callback_count != 2) {
        printf("✗ Expected 2 decision callbacks, got %d\n", decision_callback_count);
        return -1;
    }

    if (learning_callback_count != 2) {
        printf("✗ Expected 2 learning callbacks, got %d\n", learning_callback_count);
        return -1;
    }

    printf("\n✓ Test 7 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║            Kim-AI-OS: Cortex Test                         ║\n");
    printf("║                                                            ║\n");
    printf("║       대뇌 피질 - The Thinking Engine                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    int result = 0;

    /* 시스템 초기화 */
    printf("\n[Init] 시스템 초기화 중...\n");

    /* 척수 생성 */
    spine_t* spine = spine_create();
    if (!spine) {
        printf("✗ Failed to create spine\n");
        return 1;
    }

    spine_set_sensory_callback(spine, mock_brain_sensory);
    spine_set_motor_callback(spine, mock_brain_motor);

    /* 대뇌 생성 */
    cortex_t* cortex = cortex_create(spine);
    if (!cortex) {
        printf("✗ Failed to create cortex\n");
        spine_destroy(spine);
        return 1;
    }

    /* 콜백 등록 */
    cortex_set_decision_callback(cortex, mock_decision_callback);
    cortex_set_learning_callback(cortex, mock_learning_callback);

    printf("[Init] 초기화 완료\n");

    /* 테스트 실행 */
    if (test_greeting(cortex) < 0) result = 1;
    if (test_question(cortex) < 0) result = 1;
    if (test_command(cortex) < 0) result = 1;
    if (test_threat(cortex) < 0) result = 1;
    if (test_data(cortex) < 0) result = 1;
    if (test_continuous(cortex) < 0) result = 1;
    if (test_callbacks(cortex) < 0) result = 1;

    /* 통계 */
    cortex_stats(cortex);

    /* 정리 */
    cortex_destroy(cortex);
    spine_destroy(spine);

    if (result == 0) {
        printf("\n╔════════════════════════════════════════════════════════════╗\n");
        printf("║             All Cortex Tests Passed! 🧠                   ║\n");
        printf("║                                                            ║\n");
        printf("║         대뇌 피질이 완벽하게 사고합니다!                     ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }

    return result;
}

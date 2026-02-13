/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_math.c
 *
 * Arithmetic Accelerator Test
 * 산술 가속기 테스트
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _USE_MATH_DEFINES
#define _GNU_SOURCE

#include "kim_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TEST_EPSILON 1e-10

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: 기본 산술 연산 (1+1은 언제나 2다)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_basic_arithmetic(math_unit_t* unit) {
    printf("\n=== Test 1: 기본 산술 연산 ===\n");
    printf("   \"1+1은 언제나 2다\" 검증\n\n");

    int failed = 0;

    /* 1 + 1 = 2 */
    double result = math_add(unit, 1.0, 1.0);
    printf("   1 + 1 = %.15f\n", result);
    if (fabs(result - 2.0) > TEST_EPSILON) {
        printf("   ❌ 실패: 1+1이 2가 아님!\n");
        failed++;
    } else {
        printf("   ✅ 통과: 1+1 = 2 (결정론적)\n");
    }

    /* 3 * 4 = 12 */
    result = math_mul(unit, 3.0, 4.0);
    printf("\n   3 * 4 = %.15f\n", result);
    if (fabs(result - 12.0) > TEST_EPSILON) {
        printf("   ❌ 실패: 3*4가 12가 아님!\n");
        failed++;
    } else {
        printf("   ✅ 통과: 3*4 = 12 (결정론적)\n");
    }

    /* 10 - 7 = 3 */
    result = math_sub(unit, 10.0, 7.0);
    printf("\n   10 - 7 = %.15f\n", result);
    if (fabs(result - 3.0) > TEST_EPSILON) {
        printf("   ❌ 실패: 10-7이 3이 아님!\n");
        failed++;
    } else {
        printf("   ✅ 통과: 10-7 = 3 (결정론적)\n");
    }

    /* 15 / 3 = 5 */
    result = math_div(unit, 15.0, 3.0);
    printf("\n   15 / 3 = %.15f\n", result);
    if (fabs(result - 5.0) > TEST_EPSILON) {
        printf("   ❌ 실패: 15/3이 5가 아님!\n");
        failed++;
    } else {
        printf("   ✅ 통과: 15/3 = 5 (결정론적)\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: 복잡한 수식 (거듭제곱, 제곱근)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_complex_math(math_unit_t* unit) {
    printf("\n=== Test 2: 복잡한 수식 ===\n\n");

    int failed = 0;

    /* 2^10 = 1024 */
    double result = math_pow(unit, 2.0, 10.0);
    printf("   2^10 = %.15f\n", result);
    if (fabs(result - 1024.0) > TEST_EPSILON) {
        printf("   ❌ 실패: 2^10이 1024가 아님!\n");
        failed++;
    } else {
        printf("   ✅ 통과: 2^10 = 1024\n");
    }

    /* √16 = 4 */
    result = math_sqrt(unit, 16.0);
    printf("\n   √16 = %.15f\n", result);
    if (fabs(result - 4.0) > TEST_EPSILON) {
        printf("   ❌ 실패: √16이 4가 아님!\n");
        failed++;
    } else {
        printf("   ✅ 통과: √16 = 4\n");
    }

    /* √2 ≈ 1.414213562373095 */
    result = math_sqrt(unit, 2.0);
    printf("\n   √2 = %.15f\n", result);
    double expected_sqrt2 = 1.414213562373095;
    if (fabs(result - expected_sqrt2) > TEST_EPSILON) {
        printf("   ❌ 실패: √2 정밀도 오차!\n");
        failed++;
    } else {
        printf("   ✅ 통과: √2 (정밀도 15자리)\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: 삼각함수
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_trigonometry(math_unit_t* unit) {
    printf("\n=== Test 3: 삼각함수 ===\n\n");

    int failed = 0;

    /* sin(0) = 0 */
    math_task_t task1 = { .val_a = 0.0, .op = OP_SIN };
    math_execute_strict(unit, &task1);
    printf("   sin(0) = %.15f\n", task1.result);
    if (fabs(task1.result) > TEST_EPSILON) {
        printf("   ❌ 실패\n");
        failed++;
    } else {
        printf("   ✅ 통과\n");
    }

    /* cos(0) = 1 */
    math_task_t task2 = { .val_a = 0.0, .op = OP_COS };
    math_execute_strict(unit, &task2);
    printf("\n   cos(0) = %.15f\n", task2.result);
    if (fabs(task2.result - 1.0) > TEST_EPSILON) {
        printf("   ❌ 실패\n");
        failed++;
    } else {
        printf("   ✅ 통과\n");
    }

    /* sin(π/2) ≈ 1 */
    double pi_over_2 = M_PI / 2.0;
    math_task_t task3 = { .val_a = pi_over_2, .op = OP_SIN };
    math_execute_strict(unit, &task3);
    printf("\n   sin(π/2) = %.15f\n", task3.result);
    if (fabs(task3.result - 1.0) > TEST_EPSILON) {
        printf("   ❌ 실패\n");
        failed++;
    } else {
        printf("   ✅ 통과\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: 경계 조건 (0으로 나누기, 음수 제곱근)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_edge_cases(math_unit_t* unit) {
    printf("\n=== Test 4: 경계 조건 (오류 처리) ===\n\n");

    int failed = 0;

    /* 5 / 0 (오류 예상) */
    printf("   5 / 0 (오류 예상):\n");
    double result = math_div(unit, 5.0, 0.0);
    if (isnan(result)) {
        printf("   ✅ 통과: 0으로 나누기 차단됨\n");
    } else {
        printf("   ❌ 실패: 0으로 나누기가 통과됨!\n");
        failed++;
    }

    /* √(-1) (오류 예상) */
    printf("\n   √(-1) (오류 예상):\n");
    result = math_sqrt(unit, -1.0);
    if (isnan(result)) {
        printf("   ✅ 통과: 음수 제곱근 차단됨\n");
    } else {
        printf("   ❌ 실패: 음수 제곱근이 통과됨!\n");
        failed++;
    }

    /* log(-5) (오류 예상) */
    printf("\n   log(-5) (오류 예상):\n");
    math_task_t task = { .val_a = -5.0, .op = OP_LOG };
    bool success = math_execute_strict(unit, &task);
    if (!success) {
        printf("   ✅ 통과: 음수 로그 차단됨\n");
    } else {
        printf("   ❌ 실패: 음수 로그가 통과됨!\n");
        failed++;
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: 불변성 검증 (Logic Watcher)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_immutability(math_unit_t* unit) {
    printf("\n=== Test 5: 불변성 검증 (Logic Watcher) ===\n");
    printf("   \"AI가 1+1=3이라고 출력하려 할 때 차단\"\n\n");

    int failed = 0;

    /* 1 + 1 = 2 계산 */
    math_task_t task = { .val_a = 1.0, .val_b = 1.0, .op = OP_ADD };
    math_execute_strict(unit, &task);

    printf("   하드웨어 계산 결과: %.15f\n", task.result);
    printf("   불변성 플래그: %s\n", task.is_verified ? "✅" : "❌");

    /* AI가 "1+1=3"이라고 출력하려는 시나리오 */
    printf("\n   시나리오: AI가 '1+1=3'이라고 출력 시도\n");
    double ai_output = 3.0;

    bool verified = math_verify_result(&task, ai_output);
    if (!verified) {
        printf("   ✅ 통과: Logic Watcher가 AI 출력 차단\n");
        printf("   🚨 강제 정정: 1+1 = %.15f (하드웨어 값)\n", task.result);
    } else {
        printf("   ❌ 실패: AI 출력이 통과됨!\n");
        failed++;
    }

    /* 정상 케이스 */
    printf("\n   시나리오: AI가 '1+1=2'라고 정확히 출력\n");
    ai_output = 2.0;
    verified = math_verify_result(&task, ai_output);
    if (verified) {
        printf("   ✅ 통과: AI 출력이 하드웨어 값과 일치\n");
    } else {
        printf("   ❌ 실패: 정상 출력이 차단됨!\n");
        failed++;
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 6: 복잡한 수식 조합
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_complex_expressions(math_unit_t* unit) {
    printf("\n=== Test 6: 복잡한 수식 조합 ===\n\n");

    int failed = 0;

    /* (3 + 4) * 5 = 35 */
    printf("   (3 + 4) * 5:\n");
    double temp = math_add(unit, 3.0, 4.0);
    double result = math_mul(unit, temp, 5.0);
    printf("   결과: %.15f\n", result);
    if (fabs(result - 35.0) > TEST_EPSILON) {
        printf("   ❌ 실패\n");
        failed++;
    } else {
        printf("   ✅ 통과: (3+4)*5 = 35\n");
    }

    /* √(2^2 + 3^2) = √13 ≈ 3.606 */
    printf("\n   √(2² + 3²) = √13:\n");
    double a_sq = math_pow(unit, 2.0, 2.0);
    double b_sq = math_pow(unit, 3.0, 2.0);
    double sum = math_add(unit, a_sq, b_sq);
    result = math_sqrt(unit, sum);
    printf("   결과: %.15f\n", result);
    double expected = sqrt(13.0);
    if (fabs(result - expected) > TEST_EPSILON) {
        printf("   ❌ 실패\n");
        failed++;
    } else {
        printf("   ✅ 통과: √13 ≈ 3.606\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  산술 가속기 테스트 (Arithmetic Accelerator Test)\n");
    printf("  \"뇌 속의 기계적 격리실\" 검증\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* 산술 가속기 생성 */
    math_unit_t* unit = math_unit_create();
    if (!unit) {
        fprintf(stderr, "❌ 산술 가속기 생성 실패\n");
        return 1;
    }

    int total_failed = 0;

    /* 테스트 실행 */
    total_failed += test_basic_arithmetic(unit);
    total_failed += test_complex_math(unit);
    total_failed += test_trigonometry(unit);
    total_failed += test_edge_cases(unit);
    total_failed += test_immutability(unit);
    total_failed += test_complex_expressions(unit);

    /* 통계 */
    math_unit_stats(unit);

    /* 최종 결과 */
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    if (total_failed == 0) {
        printf("  ✅ 모든 테스트 통과 (6/6)\n");
        printf("  🧮 산술 가속기 정상 작동\n");
        printf("  🛡️  AI 추론 개입 차단 확인\n");
    } else {
        printf("  ❌ %d개 테스트 실패\n", total_failed);
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* 정리 */
    math_unit_destroy(unit);

    return (total_failed == 0) ? 0 : 1;
}

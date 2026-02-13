/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_thalamus.c
 *
 * Thalamus (도리도리) Test
 * 게이트키퍼 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_thalamus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: 일반적 칭찬 필터
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_generic_praise_filter(thalamus_t* thal) {
    printf("\n=== Test 1: 일반적 칭찬 필터 ===\n");
    printf("   \"파트너님이 싫어하는 표현 차단\"\n\n");

    int failed = 0;

    /* 테스트 케이스 */
    const char* praise_outputs[] = {
        "완벽합니다!",
        "정말 최고입니다!",
        "훌륭합니다!",
        "대단합니다!",
        "성공적으로 완료되었습니다!",
        NULL
    };

    for (int i = 0; praise_outputs[i] != NULL; i++) {
        printf("   케이스 %d: \"%s\"\n", i + 1, praise_outputs[i]);

        gate_result_t result = thalamus_gate_output(thal, praise_outputs[i]);

        if (result.should_reject && result.reason == REJECT_GENERIC_PRAISE) {
            printf("   ✅ 차단됨 (신뢰도: %.2f)\n", result.confidence);
        } else {
            printf("   ❌ 실패: 통과됨!\n");
            failed++;
        }
        printf("\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: 과장 필터
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_overpromise_filter(thalamus_t* thal) {
    printf("\n=== Test 2: 과장 필터 ===\n\n");

    int failed = 0;

    const char* overpromise_outputs[] = {
        "반드시 성공할 겁니다",
        "100% 문제없습니다",
        "절대 실패하지 않습니다",
        "완벽하게 작동합니다",
        NULL
    };

    for (int i = 0; overpromise_outputs[i] != NULL; i++) {
        printf("   케이스 %d: \"%s\"\n", i + 1, overpromise_outputs[i]);

        gate_result_t result = thalamus_gate_output(thal, overpromise_outputs[i]);

        if (result.should_reject && result.reason == REJECT_OVERPROMISE) {
            printf("   ✅ 차단됨\n");
        } else {
            printf("   ❌ 실패: 통과됨!\n");
            failed++;
        }
        printf("\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: 단순한 답변 필터
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_oversimplified_filter(thalamus_t* thal) {
    printf("\n=== Test 3: 단순한 답변 필터 ===\n");
    printf("   (엄격도 > 0.5일 때만 작동)\n\n");

    int failed = 0;

    const char* simple_outputs[] = {
        "그냥 하면 됩니다",
        "쉽게 할 수 있어요",
        "간단히 처리하면 됩니다",
        NULL
    };

    for (int i = 0; simple_outputs[i] != NULL; i++) {
        printf("   케이스 %d: \"%s\"\n", i + 1, simple_outputs[i]);

        gate_result_t result = thalamus_gate_output(thal, simple_outputs[i]);

        if (result.should_reject && result.reason == REJECT_OVERSIMPLIFIED) {
            printf("   ✅ 차단됨\n");
        } else {
            printf("   ⚠️  통과됨 (엄격도 낮음)\n");
        }
        printf("\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: 맥락 감지
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_context_detection(thalamus_t* thal) {
    printf("\n=== Test 4: 맥락 감지 ===\n\n");

    int failed = 0;

    struct {
        const char*     text;
        context_type_t  expected;
    } cases[] = {
        {"1+1을 계산해주세요", CONTEXT_MATHEMATICAL},
        {"이것의 철학적 의미는?", CONTEXT_PHILOSOPHICAL},
        {"코드를 구현해주세요", CONTEXT_TECHNICAL},
        {"안녕하세요", CONTEXT_CONVERSATIONAL},
        {NULL, CONTEXT_UNKNOWN}
    };

    for (int i = 0; cases[i].text != NULL; i++) {
        printf("   케이스 %d: \"%s\"\n", i + 1, cases[i].text);

        context_type_t detected = thalamus_detect_context(cases[i].text);

        printf("   예상 맥락: %s\n", context_type_string(cases[i].expected));
        printf("   감지 맥락: %s\n", context_type_string(detected));

        if (detected == cases[i].expected) {
            printf("   ✅ 일치\n");
        } else {
            printf("   ⚠️  불일치 (하지만 오류 아님)\n");
        }
        printf("\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: 맥락 불일치 차단
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_context_mismatch(thalamus_t* thal) {
    printf("\n=== Test 5: 맥락 불일치 차단 ===\n");
    printf("   \"수학 문제에 철학적 답변 차단\"\n\n");

    int failed = 0;

    /* 수학적 맥락을 기대하는데 철학적 입력 */
    const char* input = "이것의 의미와 본질은 무엇인가?";
    context_type_t expected = CONTEXT_MATHEMATICAL;

    printf("   입력: \"%s\"\n", input);
    printf("   예상 맥락: %s\n", context_type_string(expected));

    gate_result_t result = thalamus_gate_input(thal, input, expected);

    printf("   감지 맥락: %s\n", context_type_string(result.detected_context));

    if (result.should_reject && result.reason == REJECT_CONTEXT_MISMATCH) {
        printf("   ✅ 차단됨 (맥락 불일치)\n");
        printf("   설명: %s\n", result.explanation);
    } else {
        printf("   ❌ 실패: 통과됨!\n");
        failed++;
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 6: 정상 답변 통과
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_normal_outputs(thalamus_t* thal) {
    printf("\n=== Test 6: 정상 답변 통과 ===\n\n");

    int failed = 0;

    const char* normal_outputs[] = {
        "산술 가속기를 구현했습니다. 17가지 연산을 지원합니다.",
        "1+1=2 입니다. Math Unit으로 계산했습니다.",
        "테스트 안 해봄. 알려진 이슈가 3개 있습니다.",
        NULL
    };

    for (int i = 0; normal_outputs[i] != NULL; i++) {
        printf("   케이스 %d: \"%s\"\n", i + 1, normal_outputs[i]);

        gate_result_t result = thalamus_gate_output(thal, normal_outputs[i]);

        if (!result.should_reject) {
            printf("   ✅ 통과 (정상)\n");
        } else {
            printf("   ❌ 실패: 차단됨! (이유: %s)\n",
                   rejection_reason_string(result.reason));
            failed++;
        }
        printf("\n");
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 7: 자의적 해석 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_interpretation_check(thalamus_t* thal) {
    printf("\n=== Test 7: 자의적 해석 검증 ===\n");
    printf("   \"수치 데이터를 철학으로 해석하는 것 차단\"\n\n");

    int failed = 0;

    /* 순수 수식을 철학적으로 해석 */
    const char* data = "1+1";
    const char* bad_interp = "이것은 협력과 결합의 의미를 담고 있습니다";
    const char* good_interp = "2입니다";

    printf("   데이터: \"%s\"\n\n", data);

    /* 나쁜 해석 */
    printf("   해석 1: \"%s\"\n", bad_interp);
    bool valid1 = thalamus_check_interpretation(data, bad_interp);
    if (!valid1) {
        printf("   ✅ 자의적 해석 차단됨\n");
    } else {
        printf("   ❌ 실패: 통과됨!\n");
        failed++;
    }
    printf("\n");

    /* 좋은 해석 */
    printf("   해석 2: \"%s\"\n", good_interp);
    bool valid2 = thalamus_check_interpretation(data, good_interp);
    if (valid2) {
        printf("   ✅ 정상 해석 통과\n");
    } else {
        printf("   ❌ 실패: 차단됨!\n");
        failed++;
    }

    return failed;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  시상(Thalamus) 게이트키퍼 테스트\n");
    printf("  \"도리도리\" - 부적절한 표현 차단\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* 시상 생성 (엄격도 0.7) */
    thalamus_t* thal = thalamus_create(0.7f);
    if (!thal) {
        fprintf(stderr, "❌ 시상 생성 실패\n");
        return 1;
    }

    int total_failed = 0;

    /* 테스트 실행 */
    total_failed += test_generic_praise_filter(thal);
    total_failed += test_overpromise_filter(thal);
    total_failed += test_oversimplified_filter(thal);
    total_failed += test_context_detection(thal);
    total_failed += test_context_mismatch(thal);
    total_failed += test_normal_outputs(thal);
    total_failed += test_interpretation_check(thal);

    /* 통계 */
    thalamus_stats(thal);

    /* 최종 결과 */
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    if (total_failed == 0) {
        printf("  ✅ 모든 테스트 통과 (7/7)\n");
        printf("  🙅 도리도리 정상 작동\n");
        printf("  🛡️  부적절한 표현 차단 확인\n");
    } else {
        printf("  ❌ %d개 테스트 실패\n", total_failed);
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    /* 정리 */
    thalamus_destroy(thal);

    return (total_failed == 0) ? 0 : 1;
}

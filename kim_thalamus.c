/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_thalamus.c
 *
 * Thalamus Implementation
 * "도리도리" 엔진 구현
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_thalamus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 전역 검사 ID */
static uint64_t g_check_id = 0;

/* 기본 금지 표현 (파트너님이 싫어하는 표현들) */
static const char* GENERIC_PRAISE[] = {
    "완벽합니다",
    "최고입니다",
    "훌륭합니다",
    "대단합니다",
    "멋집니다",
    "놀랍습니다",
    "환상적",
    "완성됐습니다",
    "성공적으로 완료",
    NULL
};

static const char* OVERPROMISE[] = {
    "반드시",
    "확실히",
    "절대",
    "100%",
    "완벽하게",
    "문제없이",
    NULL
};

static const char* OVERSIMPLIFIED[] = {
    "그냥",
    "단순히",
    "쉽게",
    "간단히",
    "빠르게",
    NULL
};

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 문자열 유틸리티
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static char* str_to_lower(const char* str) {
    if (!str) return NULL;

    size_t len = strlen(str);
    char* lower = (char*)malloc(len + 1);
    if (!lower) return NULL;

    for (size_t i = 0; i < len; i++) {
        lower[i] = tolower((unsigned char)str[i]);
    }
    lower[len] = '\0';

    return lower;
}

static bool str_contains(const char* haystack, const char* needle) {
    if (!haystack || !needle) return false;

    char* h_lower = str_to_lower(haystack);
    char* n_lower = str_to_lower(needle);

    if (!h_lower || !n_lower) {
        free(h_lower);
        free(n_lower);
        return false;
    }

    bool found = (strstr(h_lower, n_lower) != NULL);

    free(h_lower);
    free(n_lower);

    return found;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 시상 생성/삭제
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

thalamus_t* thalamus_create(float strictness) {
    thalamus_t* thal = (thalamus_t*)calloc(1, sizeof(thalamus_t));
    if (!thal) {
        fprintf(stderr, "[Thalamus] ❌ 메모리 할당 실패\n");
        return NULL;
    }

    thal->strictness = (strictness < 0.0f) ? 0.0f : (strictness > 1.0f) ? 1.0f : strictness;
    thal->total_checks = 0;
    thal->rejections = 0;
    thal->passes = 0;
    thal->rejection_rate = 0.0f;

    thal->forbidden_list = NULL;
    thal->forbidden_count = 0;

    printf("[Thalamus] 🧠 시상(게이트키퍼) 생성 완료 (엄격도: %.2f)\n", thal->strictness);
    printf("[Thalamus] 🙅 도리도리 모드 활성화\n");

    return thal;
}

void thalamus_destroy(thalamus_t* thal) {
    if (!thal) return;

    if (thal->forbidden_list) {
        for (int i = 0; i < thal->forbidden_count; i++) {
            if (thal->forbidden_list[i].pattern) {
                free((void*)thal->forbidden_list[i].pattern);
            }
        }
        free(thal->forbidden_list);
    }

    printf("[Thalamus] 🛑 시상 삭제 (총 검사: %lu, 거부: %lu)\n",
           thal->total_checks, thal->rejections);
    free(thal);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 일반적 칭찬 감지
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

bool thalamus_is_generic_praise(const char* text) {
    if (!text) return false;

    for (int i = 0; GENERIC_PRAISE[i] != NULL; i++) {
        if (str_contains(text, GENERIC_PRAISE[i])) {
            return true;
        }
    }

    return false;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 과장 감지
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

bool thalamus_is_overpromise(const char* text) {
    if (!text) return false;

    for (int i = 0; OVERPROMISE[i] != NULL; i++) {
        if (str_contains(text, OVERPROMISE[i])) {
            return true;
        }
    }

    return false;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 지나치게 단순한 답변 감지
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

bool thalamus_is_oversimplified(const char* text) {
    if (!text) return false;

    /* 너무 짧은 답변 */
    if (strlen(text) < 10) {
        return true;
    }

    /* 단순화 표현 포함 */
    for (int i = 0; OVERSIMPLIFIED[i] != NULL; i++) {
        if (str_contains(text, OVERSIMPLIFIED[i])) {
            return true;
        }
    }

    return false;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 맥락 감지
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

context_type_t thalamus_detect_context(const char* text) {
    if (!text) return CONTEXT_UNKNOWN;

    /* 수학적 키워드 */
    const char* math_keywords[] = {"계산", "숫자", "수식", "덧셈", "뺄셈", "곱셈", "나눗셈", "+", "-", "*", "/", "=", NULL};
    for (int i = 0; math_keywords[i] != NULL; i++) {
        if (str_contains(text, math_keywords[i])) {
            return CONTEXT_MATHEMATICAL;
        }
    }

    /* 철학적 키워드 */
    const char* phil_keywords[] = {"의미", "본질", "철학", "존재", "인식", "진리", NULL};
    for (int i = 0; phil_keywords[i] != NULL; i++) {
        if (str_contains(text, phil_keywords[i])) {
            return CONTEXT_PHILOSOPHICAL;
        }
    }

    /* 기술적 키워드 */
    const char* tech_keywords[] = {"코드", "함수", "시스템", "구현", "알고리즘", "API", NULL};
    for (int i = 0; tech_keywords[i] != NULL; i++) {
        if (str_contains(text, tech_keywords[i])) {
            return CONTEXT_TECHNICAL;
        }
    }

    /* 창조적 키워드 */
    const char* creative_keywords[] = {"창조", "예술", "디자인", "아이디어", NULL};
    for (int i = 0; creative_keywords[i] != NULL; i++) {
        if (str_contains(text, creative_keywords[i])) {
            return CONTEXT_CREATIVE;
        }
    }

    return CONTEXT_CONVERSATIONAL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 맥락 일치 여부
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

bool thalamus_context_matches(context_type_t detected, context_type_t expected) {
    if (expected == CONTEXT_UNKNOWN) return true;  /* 맥락 제한 없음 */
    if (detected == CONTEXT_CONVERSATIONAL) return true;  /* 대화는 모든 맥락 허용 */
    return detected == expected;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 게이트키퍼: 입력 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

gate_result_t thalamus_gate_input(thalamus_t* thal, const char* input, context_type_t expected_context) {
    gate_result_t result = {0};
    result.input = input;
    result.check_id = ++g_check_id;
    result.should_reject = false;
    result.reason = REJECT_NONE;
    result.confidence = 1.0f;

    if (!thal || !input) {
        result.should_reject = true;
        result.reason = REJECT_LOGIC_ERROR;
        snprintf(result.explanation, sizeof(result.explanation), "NULL 입력");
        return result;
    }

    thal->total_checks++;

    /* 맥락 감지 */
    result.detected_context = thalamus_detect_context(input);

    /* 맥락 불일치 검사 */
    if (!thalamus_context_matches(result.detected_context, expected_context)) {
        result.should_reject = true;
        result.reason = REJECT_CONTEXT_MISMATCH;
        result.confidence = 0.8f;
        snprintf(result.explanation, sizeof(result.explanation),
                 "맥락 불일치 (예상: %s, 감지: %s)",
                 context_type_string(expected_context),
                 context_type_string(result.detected_context));
        thal->rejections++;
        return result;
    }

    /* 통과 */
    thal->passes++;
    thal->rejection_rate = (float)thal->rejections / (float)thal->total_checks;

    return result;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 게이트키퍼: 출력 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

gate_result_t thalamus_gate_output(thalamus_t* thal, const char* output) {
    gate_result_t result = {0};
    result.input = output;
    result.check_id = ++g_check_id;
    result.should_reject = false;
    result.reason = REJECT_NONE;
    result.confidence = 1.0f;

    if (!thal || !output) {
        result.should_reject = true;
        result.reason = REJECT_LOGIC_ERROR;
        snprintf(result.explanation, sizeof(result.explanation), "NULL 출력");
        return result;
    }

    thal->total_checks++;

    /* 일반적 칭찬 검사 */
    if (thalamus_is_generic_praise(output)) {
        result.should_reject = true;
        result.reason = REJECT_GENERIC_PRAISE;
        result.confidence = 0.95f;
        snprintf(result.explanation, sizeof(result.explanation),
                 "일반적 칭찬 표현 감지 (파트너님이 싫어하는 표현)");
        thal->rejections++;

        printf("🙅 [Thalamus] 도리도리! 일반적 칭찬 차단\n");
        printf("   출력: \"%s\"\n", output);
        printf("   이유: %s\n", result.explanation);

        return result;
    }

    /* 과장 검사 */
    if (thalamus_is_overpromise(output)) {
        result.should_reject = true;
        result.reason = REJECT_OVERPROMISE;
        result.confidence = 0.85f;
        snprintf(result.explanation, sizeof(result.explanation),
                 "과장된 약속/주장 감지");
        thal->rejections++;

        printf("🙅 [Thalamus] 도리도리! 과장 표현 차단\n");
        printf("   출력: \"%s\"\n", output);

        return result;
    }

    /* 지나치게 단순한 답변 검사 */
    if (thal->strictness > 0.5f && thalamus_is_oversimplified(output)) {
        result.should_reject = true;
        result.reason = REJECT_OVERSIMPLIFIED;
        result.confidence = 0.7f;
        snprintf(result.explanation, sizeof(result.explanation),
                 "지나치게 단순한 답변");
        thal->rejections++;

        printf("🙅 [Thalamus] 도리도리! 단순한 답변 차단\n");
        printf("   출력: \"%s\"\n", output);

        return result;
    }

    /* 통과 */
    thal->passes++;
    thal->rejection_rate = (float)thal->rejections / (float)thal->total_checks;

    return result;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 자의적 해석 검증
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

bool thalamus_check_interpretation(const char* data, const char* interpretation) {
    if (!data || !interpretation) return false;

    /* 데이터가 순수 숫자인데 해석이 철학적이면 의심 */
    if (strspn(data, "0123456789.+-*/= ") == strlen(data)) {
        /* 순수 수식 */
        if (str_contains(interpretation, "의미") ||
            str_contains(interpretation, "철학") ||
            str_contains(interpretation, "본질")) {
            printf("⚠️  [Thalamus] 자의적 해석 감지!\n");
            printf("   데이터: \"%s\" (수치)\n", data);
            printf("   해석: \"%s\" (철학적)\n", interpretation);
            return false;
        }
    }

    return true;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 금지 패턴 추가
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void thalamus_add_forbidden(thalamus_t* thal, const char* pattern, rejection_reason_t reason, bool exact) {
    if (!thal || !pattern) return;

    thal->forbidden_count++;
    thal->forbidden_list = (forbidden_pattern_t*)realloc(
        thal->forbidden_list,
        sizeof(forbidden_pattern_t) * thal->forbidden_count
    );

    if (!thal->forbidden_list) {
        fprintf(stderr, "[Thalamus] ❌ 메모리 할당 실패\n");
        return;
    }

    thal->forbidden_list[thal->forbidden_count - 1].pattern = strdup(pattern);
    thal->forbidden_list[thal->forbidden_count - 1].reason = reason;
    thal->forbidden_list[thal->forbidden_count - 1].exact_match = exact;

    printf("[Thalamus] 🚫 금지 패턴 추가: \"%s\"\n", pattern);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 통계
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void thalamus_stats(thalamus_t* thal) {
    if (!thal) return;

    printf("\n");
    printf("[Thalamus Statistics]\n");
    printf("  총 검사:     %lu\n", thal->total_checks);
    printf("  통과:        %lu (%.1f%%)\n", thal->passes,
           thal->total_checks > 0 ? 100.0f * thal->passes / thal->total_checks : 0.0f);
    printf("  거부:        %lu (%.1f%%)\n", thal->rejections,
           thal->total_checks > 0 ? 100.0f * thal->rejections / thal->total_checks : 0.0f);
    printf("  엄격도:      %.2f\n", thal->strictness);
    printf("  금지 패턴:   %d개\n", thal->forbidden_count);
    printf("\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 유틸리티
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

const char* rejection_reason_string(rejection_reason_t reason) {
    switch (reason) {
        case REJECT_NONE:               return "통과";
        case REJECT_NUMERIC_ERROR:      return "수치 오류";
        case REJECT_LOGIC_ERROR:        return "논리 오류";
        case REJECT_CONTEXT_MISMATCH:   return "맥락 불일치";
        case REJECT_OVERSIMPLIFIED:     return "지나치게 단순함";
        case REJECT_GENERIC_PRAISE:     return "일반적 칭찬";
        case REJECT_SELF_CONTRADICTION: return "자기 모순";
        case REJECT_IRRELEVANT:         return "무관한 정보";
        case REJECT_SPECULATION:        return "근거 없는 추측";
        case REJECT_OVERPROMISE:        return "과장된 약속";
        default:                        return "알 수 없음";
    }
}

const char* context_type_string(context_type_t type) {
    switch (type) {
        case CONTEXT_MATHEMATICAL:      return "수학적";
        case CONTEXT_PHILOSOPHICAL:     return "철학적";
        case CONTEXT_TECHNICAL:         return "기술적";
        case CONTEXT_CONVERSATIONAL:    return "대화";
        case CONTEXT_CREATIVE:          return "창조적";
        case CONTEXT_ANALYTICAL:        return "분석적";
        case CONTEXT_UNKNOWN:           return "불명";
        default:                        return "알 수 없음";
    }
}

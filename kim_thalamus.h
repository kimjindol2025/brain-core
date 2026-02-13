/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_thalamus.h
 *
 * Thalamus - Gatekeeper (시상 - 게이트키퍼)
 * "도리도리" - Skepticism & Filtering
 *
 * 목적: 모든 정보가 대뇌피질로 가기 전 검증 및 필터링
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_THALAMUS_H
#define KIM_THALAMUS_H

#include <stdint.h>
#include <stdbool.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 거부 사유 (Rejection Reasons)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef enum {
    REJECT_NONE = 0,            /* 통과 (Pass) */
    REJECT_NUMERIC_ERROR,       /* 수치 오류 (NaN, Infinity) */
    REJECT_LOGIC_ERROR,         /* 논리 오류 (모순) */
    REJECT_CONTEXT_MISMATCH,    /* 맥락 불일치 */
    REJECT_OVERSIMPLIFIED,      /* 지나치게 단순한 답변 */
    REJECT_GENERIC_PRAISE,      /* 일반적 칭찬 (완벽합니다 등) */
    REJECT_SELF_CONTRADICTION,  /* 자기 모순 */
    REJECT_IRRELEVANT,          /* 무관한 정보 */
    REJECT_SPECULATION,         /* 근거 없는 추측 */
    REJECT_OVERPROMISE          /* 과장된 약속 */
} rejection_reason_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 맥락 타입 (Context Types)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef enum {
    CONTEXT_MATHEMATICAL,       /* 수학적 맥락 */
    CONTEXT_PHILOSOPHICAL,      /* 철학적 맥락 */
    CONTEXT_TECHNICAL,          /* 기술적 맥락 */
    CONTEXT_CONVERSATIONAL,     /* 대화 맥락 */
    CONTEXT_CREATIVE,           /* 창조적 맥락 */
    CONTEXT_ANALYTICAL,         /* 분석적 맥락 */
    CONTEXT_UNKNOWN             /* 맥락 불명 */
} context_type_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 게이트키퍼 결과 (Gatekeeper Result)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    const char*         input;              /* 입력 데이터 */
    rejection_reason_t  reason;             /* 거부 사유 */
    bool                should_reject;      /* 거부 여부 */
    float               confidence;         /* 신뢰도 (0.0-1.0) */
    context_type_t      detected_context;   /* 감지된 맥락 */
    char                explanation[512];   /* 거부 설명 */
    uint64_t            check_id;           /* 검사 ID */
} gate_result_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 금지 표현 패턴 (Forbidden Patterns)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    const char*         pattern;            /* 패턴 문자열 */
    rejection_reason_t  reason;             /* 거부 사유 */
    bool                exact_match;        /* 정확히 일치해야 하는가 */
} forbidden_pattern_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 시상 (Thalamus)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    uint64_t            total_checks;       /* 총 검사 횟수 */
    uint64_t            rejections;         /* 거부 횟수 */
    uint64_t            passes;             /* 통과 횟수 */
    float               rejection_rate;     /* 거부율 */

    /* 금지 패턴 */
    forbidden_pattern_t* forbidden_list;
    int                  forbidden_count;

    /* 엄격도 */
    float               strictness;         /* 0.0(관대) ~ 1.0(엄격) */
} thalamus_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * API
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 시상 생성/삭제 */
thalamus_t* thalamus_create(float strictness);
void thalamus_destroy(thalamus_t* thal);

/* 게이트키퍼: 입력 검증 (도리도리) */
gate_result_t thalamus_gate_input(thalamus_t* thal, const char* input, context_type_t expected_context);

/* 출력 검증 (AI 답변 필터링) */
gate_result_t thalamus_gate_output(thalamus_t* thal, const char* output);

/* 일반적 칭찬 감지 */
bool thalamus_is_generic_praise(const char* text);

/* 과장 감지 */
bool thalamus_is_overpromise(const char* text);

/* 지나치게 단순한 답변 감지 */
bool thalamus_is_oversimplified(const char* text);

/* 맥락 감지 */
context_type_t thalamus_detect_context(const char* text);

/* 맥락 일치 여부 */
bool thalamus_context_matches(context_type_t detected, context_type_t expected);

/* 자의적 해석 검증 */
bool thalamus_check_interpretation(const char* data, const char* interpretation);

/* 금지 패턴 추가 */
void thalamus_add_forbidden(thalamus_t* thal, const char* pattern, rejection_reason_t reason, bool exact);

/* 통계 */
void thalamus_stats(thalamus_t* thal);

/* 유틸리티 */
const char* rejection_reason_string(rejection_reason_t reason);
const char* context_type_string(context_type_t type);

#endif /* KIM_THALAMUS_H */

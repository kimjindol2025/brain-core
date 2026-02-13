/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_math.h
 *
 * Arithmetic Accelerator (산술 가속기)
 * "뇌 속의 기계적 격리실"
 *
 * 목적: AI의 맥락적 추론을 원천 차단하고 순수 하드웨어 로직 구현
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_MATH_H
#define KIM_MATH_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 연산 타입 (Operation Types)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef enum {
    OP_ADD,         /* + */
    OP_SUB,         /* - */
    OP_MUL,         /* * */
    OP_DIV,         /* / */
    OP_MOD,         /* % */
    OP_POW,         /* ^ (거듭제곱) */
    OP_SQRT,        /* √ (제곱근) */
    OP_SIN,         /* sin */
    OP_COS,         /* cos */
    OP_TAN,         /* tan */
    OP_LOG,         /* log (자연로그) */
    OP_LOG10,       /* log10 */
    OP_EXP,         /* e^x */
    OP_ABS,         /* 절댓값 */
    OP_FLOOR,       /* 내림 */
    OP_CEIL,        /* 올림 */
    OP_ROUND        /* 반올림 */
} math_op_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 엄격한 수학 작업 (Strict Math Task)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    double          val_a;          /* 피연산자 A */
    double          val_b;          /* 피연산자 B (단항 연산 시 무시) */
    math_op_t       op;             /* 연산 타입 */
    double          result;         /* 계산 결과 */
    bool            is_verified;    /* 불변성 플래그 (검증됨) */
    bool            strict_numeric; /* STRICT_NUMERIC 플래그 */
    uint64_t        task_id;        /* 작업 ID (추적용) */
    char            expression[256]; /* 원본 수식 (기록용) */
} math_task_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 산술 가속기 (Arithmetic Accelerator)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    uint64_t        total_ops;      /* 총 연산 횟수 */
    uint64_t        verified_ops;   /* 검증된 연산 횟수 */
    uint64_t        failed_ops;     /* 실패한 연산 횟수 */
    double          precision;      /* 정밀도 (유효숫자) */
} math_unit_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * API
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 산술 가속기 생성/삭제 */
math_unit_t* math_unit_create(void);
void math_unit_destroy(math_unit_t* unit);

/* 엄격한 연산 (Deterministic Calculation) */
bool math_execute_strict(math_unit_t* unit, math_task_t* task);

/* 편의 함수 (Convenience Functions) */
double math_add(math_unit_t* unit, double a, double b);
double math_sub(math_unit_t* unit, double a, double b);
double math_mul(math_unit_t* unit, double a, double b);
double math_div(math_unit_t* unit, double a, double b);
double math_pow(math_unit_t* unit, double base, double exp);
double math_sqrt(math_unit_t* unit, double x);

/* 복잡한 수식 실행 (Expression Evaluation) */
bool math_eval_expression(math_unit_t* unit, const char* expr, double* result);

/* 불변성 검증 (Immutability Verification) */
bool math_verify_result(math_task_t* task, double expected);

/* 통계 */
void math_unit_stats(math_unit_t* unit);

/* 연산자 문자열 변환 */
const char* math_op_string(math_op_t op);

#endif /* KIM_MATH_H */

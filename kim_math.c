/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_math.c
 *
 * Arithmetic Accelerator Implementation
 * "뇌 속의 기계적 격리실" 구현
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* 정밀도 한계 */
#define MATH_EPSILON    1e-15

/* 전역 작업 ID */
static uint64_t g_task_id = 0;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 산술 가속기 생성/삭제
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

math_unit_t* math_unit_create(void) {
    math_unit_t* unit = (math_unit_t*)calloc(1, sizeof(math_unit_t));
    if (!unit) {
        fprintf(stderr, "[Math] ❌ 메모리 할당 실패\n");
        return NULL;
    }

    unit->precision = MATH_EPSILON;
    unit->total_ops = 0;
    unit->verified_ops = 0;
    unit->failed_ops = 0;

    printf("[Math] 🧮 산술 가속기 생성 완료 (정밀도: %.15f)\n", unit->precision);
    return unit;
}

void math_unit_destroy(math_unit_t* unit) {
    if (!unit) return;

    printf("[Math] 🛑 산술 가속기 삭제 (총 연산: %lu)\n", unit->total_ops);
    free(unit);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 데이터 무결성 검사 (Input Validation)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static bool validate_input(double val, const char* name) {
    if (isnan(val)) {
        fprintf(stderr, "[Math] ❌ CRITICAL: %s is NaN (데이터 오염)\n", name);
        return false;
    }
    if (isinf(val)) {
        fprintf(stderr, "[Math] ❌ CRITICAL: %s is Infinity (오버플로우)\n", name);
        return false;
    }
    return true;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 엄격한 연산 실행 (Deterministic Calculation)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

bool math_execute_strict(math_unit_t* unit, math_task_t* task) {
    if (!unit || !task) return false;

    /* 작업 ID 할당 */
    task->task_id = ++g_task_id;
    task->strict_numeric = true;
    task->is_verified = false;

    unit->total_ops++;

    /* 단계 1: 데이터 무결성 검사 (입력값 오염 방지) */
    if (!validate_input(task->val_a, "val_a")) {
        unit->failed_ops++;
        return false;
    }

    /* 이항 연산의 경우 val_b도 검증 */
    if (task->op <= OP_MOD || task->op == OP_POW) {
        if (!validate_input(task->val_b, "val_b")) {
            unit->failed_ops++;
            return false;
        }
    }

    /* 단계 2: 산술 논리 게이트 통과 (Arithmetic Logic Gate) */
    double final_res = 0.0;
    bool success = true;

    switch (task->op) {
        case OP_ADD:
            final_res = task->val_a + task->val_b;
            break;

        case OP_SUB:
            final_res = task->val_a - task->val_b;
            break;

        case OP_MUL:
            final_res = task->val_a * task->val_b;
            break;

        case OP_DIV:
            if (fabs(task->val_b) < unit->precision) {
                fprintf(stderr, "[Math] ❌ CRITICAL: Division by zero\n");
                success = false;
            } else {
                final_res = task->val_a / task->val_b;
            }
            break;

        case OP_MOD:
            if (fabs(task->val_b) < unit->precision) {
                fprintf(stderr, "[Math] ❌ CRITICAL: Modulo by zero\n");
                success = false;
            } else {
                final_res = fmod(task->val_a, task->val_b);
            }
            break;

        case OP_POW:
            final_res = pow(task->val_a, task->val_b);
            break;

        case OP_SQRT:
            if (task->val_a < 0) {
                fprintf(stderr, "[Math] ❌ CRITICAL: Square root of negative number\n");
                success = false;
            } else {
                final_res = sqrt(task->val_a);
            }
            break;

        case OP_SIN:
            final_res = sin(task->val_a);
            break;

        case OP_COS:
            final_res = cos(task->val_a);
            break;

        case OP_TAN:
            final_res = tan(task->val_a);
            break;

        case OP_LOG:
            if (task->val_a <= 0) {
                fprintf(stderr, "[Math] ❌ CRITICAL: Log of non-positive number\n");
                success = false;
            } else {
                final_res = log(task->val_a);
            }
            break;

        case OP_LOG10:
            if (task->val_a <= 0) {
                fprintf(stderr, "[Math] ❌ CRITICAL: Log10 of non-positive number\n");
                success = false;
            } else {
                final_res = log10(task->val_a);
            }
            break;

        case OP_EXP:
            final_res = exp(task->val_a);
            break;

        case OP_ABS:
            final_res = fabs(task->val_a);
            break;

        case OP_FLOOR:
            final_res = floor(task->val_a);
            break;

        case OP_CEIL:
            final_res = ceil(task->val_a);
            break;

        case OP_ROUND:
            final_res = round(task->val_a);
            break;

        default:
            fprintf(stderr, "[Math] ❌ CRITICAL: Unknown operation\n");
            success = false;
    }

    if (!success) {
        unit->failed_ops++;
        return false;
    }

    /* 단계 3: 결과 검증 (Result Validation) */
    if (!validate_input(final_res, "result")) {
        unit->failed_ops++;
        return false;
    }

    /* 단계 4: 불변성(Immutable Flag) 부여 */
    task->result = final_res;
    task->is_verified = true;
    unit->verified_ops++;

    printf("🤖 [Math] 정밀 연산 완료. AI 추론 개입 차단됨.\n");
    printf("   작업 ID: %lu\n", task->task_id);
    printf("   연산: %s\n", math_op_string(task->op));
    if (task->op <= OP_MOD || task->op == OP_POW) {
        printf("   입력: %.15f, %.15f\n", task->val_a, task->val_b);
    } else {
        printf("   입력: %.15f\n", task->val_a);
    }
    printf("   결과: %.15f\n", final_res);
    printf("   불변성: %s\n", task->is_verified ? "✅ 검증됨" : "❌ 미검증");

    return true;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 편의 함수 (Convenience Functions)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

double math_add(math_unit_t* unit, double a, double b) {
    math_task_t task = {
        .val_a = a,
        .val_b = b,
        .op = OP_ADD
    };
    return math_execute_strict(unit, &task) ? task.result : NAN;
}

double math_sub(math_unit_t* unit, double a, double b) {
    math_task_t task = {
        .val_a = a,
        .val_b = b,
        .op = OP_SUB
    };
    return math_execute_strict(unit, &task) ? task.result : NAN;
}

double math_mul(math_unit_t* unit, double a, double b) {
    math_task_t task = {
        .val_a = a,
        .val_b = b,
        .op = OP_MUL
    };
    return math_execute_strict(unit, &task) ? task.result : NAN;
}

double math_div(math_unit_t* unit, double a, double b) {
    math_task_t task = {
        .val_a = a,
        .val_b = b,
        .op = OP_DIV
    };
    return math_execute_strict(unit, &task) ? task.result : NAN;
}

double math_pow(math_unit_t* unit, double base, double exp) {
    math_task_t task = {
        .val_a = base,
        .val_b = exp,
        .op = OP_POW
    };
    return math_execute_strict(unit, &task) ? task.result : NAN;
}

double math_sqrt(math_unit_t* unit, double x) {
    math_task_t task = {
        .val_a = x,
        .op = OP_SQRT
    };
    return math_execute_strict(unit, &task) ? task.result : NAN;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 불변성 검증 (Immutability Verification)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

bool math_verify_result(math_task_t* task, double expected) {
    if (!task || !task->is_verified) {
        fprintf(stderr, "[Math] ⚠️  경고: 미검증 작업 검사 시도\n");
        return false;
    }

    double diff = fabs(task->result - expected);
    bool match = (diff < MATH_EPSILON);

    if (!match) {
        fprintf(stderr, "[Math] ❌ Logic Watcher: 결과 불일치 감지!\n");
        fprintf(stderr, "   예상값: %.15f\n", expected);
        fprintf(stderr, "   실제값: %.15f\n", task->result);
        fprintf(stderr, "   차이: %.15e\n", diff);
        fprintf(stderr, "   🚨 AI 출력 강제 정정 필요\n");
    }

    return match;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 통계
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void math_unit_stats(math_unit_t* unit) {
    if (!unit) return;

    printf("\n");
    printf("[Math Unit Statistics]\n");
    printf("  총 연산:     %lu\n", unit->total_ops);
    printf("  검증된 연산: %lu\n", unit->verified_ops);
    printf("  실패한 연산: %lu\n", unit->failed_ops);
    printf("  성공률:      %.2f%%\n",
           unit->total_ops > 0 ? (100.0 * unit->verified_ops / unit->total_ops) : 0.0);
    printf("  정밀도:      %.15f\n", unit->precision);
    printf("\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 연산자 문자열 변환
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

const char* math_op_string(math_op_t op) {
    switch (op) {
        case OP_ADD:    return "ADD (+)";
        case OP_SUB:    return "SUB (-)";
        case OP_MUL:    return "MUL (*)";
        case OP_DIV:    return "DIV (/)";
        case OP_MOD:    return "MOD (%)";
        case OP_POW:    return "POW (^)";
        case OP_SQRT:   return "SQRT (√)";
        case OP_SIN:    return "SIN";
        case OP_COS:    return "COS";
        case OP_TAN:    return "TAN";
        case OP_LOG:    return "LOG (ln)";
        case OP_LOG10:  return "LOG10";
        case OP_EXP:    return "EXP (e^x)";
        case OP_ABS:    return "ABS (|x|)";
        case OP_FLOOR:  return "FLOOR";
        case OP_CEIL:   return "CEIL";
        case OP_ROUND:  return "ROUND";
        default:        return "UNKNOWN";
    }
}

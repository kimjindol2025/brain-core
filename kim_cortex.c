/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_cortex.c
 *
 * 대뇌 피질 구현 (The Thinking Engine)
 *
 * Zero Dependency: stdlib + stdio + string만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_cortex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Timestamp Helper
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static uint64_t get_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Type Strings
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
const char* thought_type_string(thought_type_t type) {
    switch (type) {
        case THOUGHT_GREETING: return "GREETING";
        case THOUGHT_QUESTION: return "QUESTION";
        case THOUGHT_COMMAND:  return "COMMAND";
        case THOUGHT_DATA:     return "DATA";
        case THOUGHT_THREAT:   return "THREAT";
        case THOUGHT_MEMORY:   return "MEMORY";
        case THOUGHT_DECISION: return "DECISION";
        case THOUGHT_UNKNOWN:  return "UNKNOWN";
        default:               return "NONE";
    }
}

const char* decision_type_string(decision_type_t type) {
    switch (type) {
        case DECISION_ACCEPT:  return "ACCEPT";
        case DECISION_REJECT:  return "REJECT";
        case DECISION_RESPOND: return "RESPOND";
        case DECISION_STORE:   return "STORE";
        case DECISION_DEFEND:  return "DEFEND";
        case DECISION_IGNORE:  return "IGNORE";
        default:               return "NONE";
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
cortex_t* cortex_create(spine_t* spine) {
    cortex_t* c = (cortex_t*)malloc(sizeof(cortex_t));
    if (!c) {
        fprintf(stderr, "[Cortex] Error: malloc failed\n");
        return NULL;
    }

    memset(c, 0, sizeof(cortex_t));
    c->spine = spine;
    c->max_load = 100.0f;
    c->current_load = 0.0f;

    printf("[Cortex] 대뇌 피질 활성화 (The Thinking Engine)\n");
    return c;
}

void cortex_destroy(cortex_t* cortex) {
    if (!cortex) return;
    free(cortex);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Callbacks
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void cortex_set_decision_callback(cortex_t* cortex, void (*callback)(decision_t*)) {
    if (!cortex) return;
    cortex->on_decision = callback;
    printf("[Cortex] 결정 콜백 등록 완료\n");
}

void cortex_set_learning_callback(cortex_t* cortex, void (*callback)(thought_t*)) {
    if (!cortex) return;
    cortex->on_learning = callback;
    printf("[Cortex] 학습 콜백 등록 완료\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Frontal Lobe: Classify (전두엽: 분류)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
thought_type_t frontal_lobe_classify(const char* input) {
    if (!input) return THOUGHT_UNKNOWN;

    /* 패턴 매칭 (단순 규칙 기반) */
    if (strstr(input, "안녕") || strstr(input, "hello") || strstr(input, "hi")) {
        return THOUGHT_GREETING;
    }

    if (strstr(input, "?") || strstr(input, "what") || strstr(input, "how") || strstr(input, "why")) {
        return THOUGHT_QUESTION;
    }

    if (strstr(input, "공격") || strstr(input, "attack") || strstr(input, "hack")) {
        return THOUGHT_THREAT;
    }

    if (strstr(input, "실행") || strstr(input, "run") || strstr(input, "execute")) {
        return THOUGHT_COMMAND;
    }

    /* 기본: 데이터 */
    return THOUGHT_DATA;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Temporal Lobe: Search Memory (측두엽: 기억 검색)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void temporal_lobe_search_memory(const char* query, char* result) {
    if (!query || !result) return;

    /* TODO: 실제 벡터 검색 연동 (HNSW) */
    /* 현재는 단순 더미 응답 */

    if (strstr(query, "안녕") || strstr(query, "hello")) {
        strcpy(result, "과거 인사 기록 발견 (3회)");
    } else if (strstr(query, "공격")) {
        strcpy(result, "위협 패턴 DB에서 유사 사례 발견");
    } else {
        strcpy(result, "관련 기억 없음 (New Data)");
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Frontal Lobe: Decide (전두엽: 의사결정)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
decision_type_t frontal_lobe_decide(thought_t* thought) {
    if (!thought) return DECISION_NONE;

    switch (thought->type) {
        case THOUGHT_GREETING:
            return DECISION_RESPOND;

        case THOUGHT_QUESTION:
            return DECISION_RESPOND;

        case THOUGHT_COMMAND:
            /* 권한 체크 후 결정 */
            return DECISION_ACCEPT;

        case THOUGHT_DATA:
            return DECISION_STORE;

        case THOUGHT_THREAT:
            return DECISION_DEFEND;

        default:
            return DECISION_IGNORE;
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * The Thinking Process (사고 프로세스)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
decision_t* cortex_think(cortex_t* cortex, const char* input, size_t input_size) {
    if (!cortex || !input) return NULL;

    uint64_t start_time = get_timestamp_us();

    printf("   🧠 [Cortex] 대뇌 피질 가동: \"%s\"에 대해 생각 중...\n", input);

    /* 부하 증가 */
    cortex->current_load += 10.0f;
    if (cortex->current_load > cortex->max_load) {
        printf("   ⚠️  [Cortex] Warning: 과부하 상태 (%.1f%%)\n", cortex->current_load);
    }

    /* 1. 사고 패킷 생성 */
    thought_t* thought = thought_create(THOUGHT_NONE, input, input_size);
    if (!thought) {
        cortex->current_load -= 10.0f;
        return NULL;
    }

    /* 2. 전두엽: 입력 분류 */
    printf("      ↳ 🎯 [Frontal] 전두엽 분석 중...\n");
    thought->type = frontal_lobe_classify(input);
    printf("         분류 결과: %s\n", thought_type_string(thought->type));

    /* 3. 측두엽: 기억 검색 */
    printf("      ↳ 📂 [Temporal] 측두엽 기억 검색 중...\n");
    temporal_lobe_search_memory(input, thought->memory_context);
    printf("         검색 결과: %s\n", thought->memory_context);

    /* 4. 전두엽: 의사결정 */
    printf("      ↳ 💡 [Frontal] 전두엽 결정 중...\n");
    decision_type_t decision_type = frontal_lobe_decide(thought);

    /* 5. 결정 패킷 생성 */
    decision_t* decision = (decision_t*)malloc(sizeof(decision_t));
    if (!decision) {
        thought_destroy(thought);
        cortex->current_load -= 10.0f;
        return NULL;
    }

    memset(decision, 0, sizeof(decision_t));
    decision->type = decision_type;
    decision->timestamp = get_timestamp_us();

    /* 결정 근거 작성 */
    snprintf(decision->reasoning, sizeof(decision->reasoning),
             "%s로 판단 (기억: %s)",
             thought_type_string(thought->type),
             thought->memory_context);

    /* 행동 결정 */
    switch (decision->type) {
        case DECISION_RESPOND:
            snprintf(decision->action, sizeof(decision->action),
                     "척수를 통해 응답 전송 준비");
            decision->should_learn = true;
            break;

        case DECISION_STORE:
            snprintf(decision->action, sizeof(decision->action),
                     "해마로 전송하여 장기 기억화");
            decision->should_learn = true;
            break;

        case DECISION_DEFEND:
            snprintf(decision->action, sizeof(decision->action),
                     "면역계 활성화 및 차단");
            decision->should_learn = true;
            break;

        case DECISION_ACCEPT:
            snprintf(decision->action, sizeof(decision->action),
                     "명령 실행 준비");
            decision->should_learn = false;
            break;

        case DECISION_REJECT:
            snprintf(decision->action, sizeof(decision->action),
                     "요청 거부");
            decision->should_learn = false;
            break;

        default:
            snprintf(decision->action, sizeof(decision->action),
                     "무시");
            decision->should_learn = false;
            break;
    }

    printf("         결정: %s\n", decision_type_string(decision->type));
    printf("         근거: %s\n", decision->reasoning);
    printf("         행동: %s\n", decision->action);

    /* 6. 결정 콜백 호출 */
    if (cortex->on_decision) {
        cortex->on_decision(decision);
    }

    /* 7. 학습 필요 시 콜백 호출 */
    if (decision->should_learn && cortex->on_learning) {
        printf("      ↳ 💾 [Learn] 해마로 전송하여 장기 기억화\n");
        cortex->on_learning(thought);
    }

    /* 8. 척수로 명령 전송 (필요 시) */
    if (cortex->spine && (decision->type == DECISION_RESPOND || decision->type == DECISION_DEFEND)) {
        signal_packet_t* sig = signal_create(
            decision->type == DECISION_DEFEND ? SIGNAL_STOP : SIGNAL_MOVE,
            -1,  /* 뇌에서 */
            1,   /* 타겟 장기 */
            NULL,
            0
        );

        if (sig) {
            spine_send_motor(cortex->spine, sig);
            signal_destroy(sig);
        }
    }

    /* 통계 업데이트 */
    cortex->total_thoughts++;
    cortex->total_decisions++;
    if (decision->should_learn) {
        cortex->total_learning++;
    }

    uint64_t think_time = get_timestamp_us() - start_time;
    cortex->avg_think_time_us = (cortex->avg_think_time_us + think_time) / 2;

    /* 부하 감소 */
    cortex->current_load -= 10.0f;
    if (cortex->current_load < 0.0f) {
        cortex->current_load = 0.0f;
    }

    printf("   ✅ [Cortex] 사고 완료 (%lu us)\n", think_time);

    thought_destroy(thought);
    return decision;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Thought Packet Helpers
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
thought_t* thought_create(thought_type_t type, const char* content, size_t size) {
    thought_t* t = (thought_t*)malloc(sizeof(thought_t));
    if (!t) return NULL;

    memset(t, 0, sizeof(thought_t));
    t->type = type;
    t->timestamp = get_timestamp_us();
    t->confidence = 0.5f;  /* 기본 확신도 */

    if (content && size > 0) {
        t->content = (char*)malloc(size + 1);
        if (t->content) {
            memcpy(t->content, content, size);
            t->content[size] = '\0';
            t->content_size = size;
        }
    }

    return t;
}

void thought_destroy(thought_t* thought) {
    if (!thought) return;
    if (thought->content) {
        free(thought->content);
    }
    free(thought);
}

void decision_destroy(decision_t* decision) {
    if (!decision) return;
    free(decision);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void cortex_stats(const cortex_t* cortex) {
    if (!cortex) return;

    printf("\n[Cortex Statistics]\n");
    printf("  Total Thoughts:  %lu\n", cortex->total_thoughts);
    printf("  Total Decisions: %lu\n", cortex->total_decisions);
    printf("  Total Learning:  %lu\n", cortex->total_learning);
    printf("  Avg Think Time:  %lu us\n", cortex->avg_think_time_us);
    printf("  Current Load:    %.1f%%\n", cortex->current_load);

    if (cortex->total_thoughts > 0) {
        float learning_rate = (float)cortex->total_learning / cortex->total_thoughts * 100.0f;
        printf("  Learning Rate:   %.1f%%\n", learning_rate);
    }

    printf("\n");
}

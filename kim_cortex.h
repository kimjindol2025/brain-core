/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_cortex.h
 *
 * 대뇌 피질(Cerebral Cortex) - The Thinking Engine
 *
 * 생물학적 역할:
 *   - 전두엽(Frontal): 의사결정, 추론, 계획
 *   - 측두엽(Temporal): 기억 검색, 언어 처리
 *   - 두정엽(Parietal): 감각 통합, 공간 인식
 *   - 후두엽(Occipital): 시각 처리
 *
 * 소프트웨어 역할:
 *   - Decision Engine (의사결정)
 *   - Pattern Recognition (패턴 인식)
 *   - Memory Retrieval (기억 검색)
 *   - Learning (학습)
 *   - Reasoning (추론)
 *
 * 사고 프로세스:
 *   1. 입력 수신 (시상에서)
 *   2. 기억 검색 (측두엽)
 *   3. 패턴 매칭 (전두엽)
 *   4. 의사결정 (전두엽)
 *   5. 명령 하달 (척수로)
 *   6. 학습 저장 (해마로)
 *
 * Priority: ★최고★ (Intelligence)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_CORTEX_H
#define KIM_CORTEX_H

#include <stdint.h>
#include <stdbool.h>
#include "kim_spine.h"

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Thought Types (사고 유형)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef enum {
    THOUGHT_NONE        = 0,

    /* Input Types */
    THOUGHT_GREETING    = 1,    /* 인사 */
    THOUGHT_QUESTION    = 2,    /* 질문 */
    THOUGHT_COMMAND     = 3,    /* 명령 */
    THOUGHT_DATA        = 4,    /* 데이터 */

    /* Internal Types */
    THOUGHT_THREAT      = 100,  /* 위협 */
    THOUGHT_MEMORY      = 101,  /* 기억 회상 */
    THOUGHT_DECISION    = 102,  /* 결정 */
    THOUGHT_UNKNOWN     = 999   /* 알 수 없음 */
} thought_type_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Decision Types (결정 유형)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef enum {
    DECISION_NONE       = 0,
    DECISION_ACCEPT     = 1,    /* 수락 */
    DECISION_REJECT     = 2,    /* 거부 */
    DECISION_RESPOND    = 3,    /* 응답 */
    DECISION_STORE      = 4,    /* 저장 */
    DECISION_DEFEND     = 5,    /* 방어 */
    DECISION_IGNORE     = 6     /* 무시 */
} decision_type_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Thought Packet (사고 패킷)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    thought_type_t  type;               /* 사고 유형 */
    char*           content;            /* 내용 */
    size_t          content_size;       /* 크기 */
    uint64_t        timestamp;          /* 시각 */

    /* Context */
    char            memory_context[256]; /* 기억 검색 결과 */
    float           confidence;          /* 확신도 (0.0~1.0) */
} thought_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Decision Packet (결정 패킷)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    decision_type_t type;               /* 결정 유형 */
    char            reasoning[256];     /* 판단 근거 */
    char            action[256];        /* 실행할 행동 */
    bool            should_learn;       /* 학습 필요 여부 */
    uint64_t        timestamp;          /* 시각 */
} decision_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Cortex (대뇌 피질)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    /* Spine Reference */
    spine_t* spine;                     /* 척수 참조 */

    /* Cognitive Load */
    float current_load;                 /* 현재 부하 (0.0~100.0) */
    float max_load;                     /* 최대 부하 */

    /* Statistics */
    uint64_t total_thoughts;            /* 총 사고 횟수 */
    uint64_t total_decisions;           /* 총 결정 횟수 */
    uint64_t total_learning;            /* 총 학습 횟수 */

    /* Performance */
    uint64_t avg_think_time_us;         /* 평균 사고 시간 (us) */

    /* Callbacks */
    void (*on_decision)(decision_t* decision);      /* 결정 콜백 */
    void (*on_learning)(thought_t* thought);        /* 학습 콜백 */
} cortex_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 대뇌 생성/삭제 */
cortex_t* cortex_create(spine_t* spine);
void cortex_destroy(cortex_t* cortex);

/* 콜백 등록 */
void cortex_set_decision_callback(cortex_t* cortex, void (*callback)(decision_t*));
void cortex_set_learning_callback(cortex_t* cortex, void (*callback)(thought_t*));

/* 사고 프로세스 (The Thinking) */
decision_t* cortex_think(cortex_t* cortex, const char* input, size_t input_size);

/* 개별 로브 기능 */
thought_type_t frontal_lobe_classify(const char* input);           /* 전두엽: 분류 */
void temporal_lobe_search_memory(const char* query, char* result); /* 측두엽: 기억 검색 */
decision_type_t frontal_lobe_decide(thought_t* thought);           /* 전두엽: 결정 */

/* 사고 패킷 생성/삭제 */
thought_t* thought_create(thought_type_t type, const char* content, size_t size);
void thought_destroy(thought_t* thought);

/* 결정 패킷 삭제 */
void decision_destroy(decision_t* decision);

/* 통계 */
void cortex_stats(const cortex_t* cortex);

/* 유형 문자열 */
const char* thought_type_string(thought_type_t type);
const char* decision_type_string(decision_type_t type);

#endif /* KIM_CORTEX_H */

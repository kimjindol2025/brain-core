/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_circadian.h
 *
 * 서카디언 엔진(Circadian Engine) - 24시간 상시 가동 시스템
 *
 * 생물학적 역할:
 *   - 생체 리듬 (Circadian Rhythm)
 *   - 낮/밤 주기 관리
 *   - 활동/휴식/학습 교대 근무
 *   - 대사율 조절
 *
 * 소프트웨어 역할:
 *   - 24/7 Operation
 *   - Resource Scheduling
 *   - Background Tasks
 *   - Load Balancing
 *   - Maintenance Window
 *
 * 3교대 시스템:
 *   - 활동기(Day): 실시간 처리 우선 (70% 자원)
 *   - 정리기(Evening): 메모리 정리/GC (30% 자원)
 *   - 학습기(Dawn): 데이터 학습/최적화 (20% 자원)
 *
 * Priority: ★핵심★ (System Lifecycle)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_CIRCADIAN_H
#define KIM_CIRCADIAN_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Circadian Phase (생체 리듬 단계)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef enum {
    PHASE_DAWN,         /* 🌙 새벽 (01:00-06:00): 학습 및 최적화 */
    PHASE_DAY,          /* ☀️  낮 (06:00-18:00): 실시간 처리 */
    PHASE_EVENING       /* 🌆 저녁 (18:00-01:00): 정리 및 GC */
} circadian_phase_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Resource Profile (자원 프로파일)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    int input_throttle;         /* 입력 제한 (0-100) */
    int brain_priority;         /* 뇌 우선순위 (0-100) */
    int cleanup_priority;       /* 정리 우선순위 (0-100) */
    int learning_priority;      /* 학습 우선순위 (0-100) */
} resource_profile_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Circadian Engine (서카디언 엔진)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    /* Current State */
    circadian_phase_t current_phase;    /* 현재 단계 */
    time_t            phase_start_time; /* 단계 시작 시각 */
    uint64_t          cycle_count;      /* 사이클 횟수 */

    /* Resource Management */
    resource_profile_t current_profile; /* 현재 자원 프로파일 */

    /* Statistics */
    uint64_t dawn_cycles;               /* 새벽 사이클 수 */
    uint64_t day_cycles;                /* 낮 사이클 수 */
    uint64_t evening_cycles;            /* 저녁 사이클 수 */
    uint64_t phase_transitions;         /* 단계 전환 횟수 */

    /* Callbacks */
    void (*on_phase_change)(circadian_phase_t old_phase, circadian_phase_t new_phase);
    void (*on_cleanup)(void);           /* 정리 작업 콜백 */
    void (*on_learning)(void);          /* 학습 작업 콜백 */
} circadian_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 서카디언 엔진 생성/삭제 */
circadian_t* circadian_create(void);
void circadian_destroy(circadian_t* circadian);

/* 콜백 등록 */
void circadian_set_phase_callback(circadian_t* circadian,
                                   void (*callback)(circadian_phase_t, circadian_phase_t));
void circadian_set_cleanup_callback(circadian_t* circadian, void (*callback)(void));
void circadian_set_learning_callback(circadian_t* circadian, void (*callback)(void));

/* 24시간 주기 관리 */
void circadian_update(circadian_t* circadian);  /* 1초마다 호출 */
circadian_phase_t circadian_get_phase(const circadian_t* circadian);
resource_profile_t circadian_get_profile(const circadian_t* circadian);

/* 단계별 자원 프로파일 */
resource_profile_t get_dawn_profile(void);      /* 새벽: 학습 우선 */
resource_profile_t get_day_profile(void);       /* 낮: 처리 우선 */
resource_profile_t get_evening_profile(void);   /* 저녁: 정리 우선 */

/* 통계 */
void circadian_stats(const circadian_t* circadian);

/* 유틸리티 */
const char* circadian_phase_string(circadian_phase_t phase);
const char* circadian_phase_icon(circadian_phase_t phase);

#endif /* KIM_CIRCADIAN_H */

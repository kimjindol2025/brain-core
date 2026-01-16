/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_circadian.c
 *
 * 서카디언 엔진 구현 (24시간 상시 가동)
 *
 * Zero Dependency: stdlib + stdio + time만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_circadian.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Phase Strings
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
const char* circadian_phase_string(circadian_phase_t phase) {
    switch (phase) {
        case PHASE_DAWN:    return "DAWN";
        case PHASE_DAY:     return "DAY";
        case PHASE_EVENING: return "EVENING";
        default:            return "UNKNOWN";
    }
}

const char* circadian_phase_icon(circadian_phase_t phase) {
    switch (phase) {
        case PHASE_DAWN:    return "🌙";
        case PHASE_DAY:     return "☀️";
        case PHASE_EVENING: return "🌆";
        default:            return "⚪";
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Resource Profiles
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 새벽: 학습 우선 (입력 최소, 학습 최대) */
resource_profile_t get_dawn_profile(void) {
    resource_profile_t profile;
    profile.input_throttle = 10;        /* 입력 10%만 */
    profile.brain_priority = 30;        /* 뇌 30% */
    profile.cleanup_priority = 20;      /* 정리 20% */
    profile.learning_priority = 100;    /* 학습 100% */
    return profile;
}

/* 낮: 실시간 처리 우선 (입력 최대, 학습 최소) */
resource_profile_t get_day_profile(void) {
    resource_profile_t profile;
    profile.input_throttle = 100;       /* 입력 100% */
    profile.brain_priority = 100;       /* 뇌 100% */
    profile.cleanup_priority = 10;      /* 정리 10% */
    profile.learning_priority = 10;     /* 학습 10% */
    return profile;
}

/* 저녁: 정리 우선 (입력 중간, 정리 최대) */
resource_profile_t get_evening_profile(void) {
    resource_profile_t profile;
    profile.input_throttle = 50;        /* 입력 50% */
    profile.brain_priority = 50;        /* 뇌 50% */
    profile.cleanup_priority = 100;     /* 정리 100% */
    profile.learning_priority = 30;     /* 학습 30% */
    return profile;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Phase Detection (시간 기반)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static circadian_phase_t detect_phase_from_time(void) {
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    int hour = local->tm_hour;

    /* 시간대별 phase 결정 */
    if (hour >= 1 && hour < 6) {
        return PHASE_DAWN;      /* 01:00 - 06:00 */
    } else if (hour >= 6 && hour < 18) {
        return PHASE_DAY;       /* 06:00 - 18:00 */
    } else {
        return PHASE_EVENING;   /* 18:00 - 01:00 */
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
circadian_t* circadian_create(void) {
    circadian_t* c = (circadian_t*)malloc(sizeof(circadian_t));
    if (!c) {
        fprintf(stderr, "[Circadian] Error: malloc failed\n");
        return NULL;
    }

    memset(c, 0, sizeof(circadian_t));

    /* 초기 phase 설정 */
    c->current_phase = detect_phase_from_time();
    c->phase_start_time = time(NULL);
    c->current_profile = get_day_profile();  /* 기본: 낮 모드 */

    printf("[Circadian] 서카디언 엔진 활성화 (24/7 Operation)\n");
    printf("[Circadian] 현재 단계: %s %s\n",
           circadian_phase_icon(c->current_phase),
           circadian_phase_string(c->current_phase));

    return c;
}

void circadian_destroy(circadian_t* circadian) {
    if (!circadian) return;
    free(circadian);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Callbacks
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void circadian_set_phase_callback(circadian_t* circadian,
                                   void (*callback)(circadian_phase_t, circadian_phase_t)) {
    if (!circadian) return;
    circadian->on_phase_change = callback;
}

void circadian_set_cleanup_callback(circadian_t* circadian, void (*callback)(void)) {
    if (!circadian) return;
    circadian->on_cleanup = callback;
}

void circadian_set_learning_callback(circadian_t* circadian, void (*callback)(void)) {
    if (!circadian) return;
    circadian->on_learning = callback;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Update (1초마다 호출)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void circadian_update(circadian_t* circadian) {
    if (!circadian) return;

    /* 현재 시간 기반 phase 감지 */
    circadian_phase_t detected_phase = detect_phase_from_time();

    /* Phase 전환 감지 */
    if (detected_phase != circadian->current_phase) {
        circadian_phase_t old_phase = circadian->current_phase;
        circadian_phase_t new_phase = detected_phase;

        printf("\n[Circadian] Phase Transition: %s %s → %s %s\n",
               circadian_phase_icon(old_phase), circadian_phase_string(old_phase),
               circadian_phase_icon(new_phase), circadian_phase_string(new_phase));

        /* 자원 프로파일 변경 */
        switch (new_phase) {
            case PHASE_DAWN:
                circadian->current_profile = get_dawn_profile();
                circadian->dawn_cycles++;
                printf("   → 학습 모드 진입: 입력 제한 %d%%, 학습 우선순위 %d%%\n",
                       circadian->current_profile.input_throttle,
                       circadian->current_profile.learning_priority);

                /* 학습 콜백 호출 */
                if (circadian->on_learning) {
                    circadian->on_learning();
                }
                break;

            case PHASE_DAY:
                circadian->current_profile = get_day_profile();
                circadian->day_cycles++;
                printf("   → 활동 모드 진입: 입력 제한 %d%%, 뇌 우선순위 %d%%\n",
                       circadian->current_profile.input_throttle,
                       circadian->current_profile.brain_priority);
                break;

            case PHASE_EVENING:
                circadian->current_profile = get_evening_profile();
                circadian->evening_cycles++;
                printf("   → 정리 모드 진입: 입력 제한 %d%%, 정리 우선순위 %d%%\n",
                       circadian->current_profile.input_throttle,
                       circadian->current_profile.cleanup_priority);

                /* 정리 콜백 호출 */
                if (circadian->on_cleanup) {
                    circadian->on_cleanup();
                }
                break;
        }

        /* Phase 전환 콜백 */
        if (circadian->on_phase_change) {
            circadian->on_phase_change(old_phase, new_phase);
        }

        circadian->current_phase = new_phase;
        circadian->phase_start_time = time(NULL);
        circadian->phase_transitions++;
    }

    /* 사이클 카운트 */
    circadian->cycle_count++;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Getters
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
circadian_phase_t circadian_get_phase(const circadian_t* circadian) {
    if (!circadian) return PHASE_DAY;
    return circadian->current_phase;
}

resource_profile_t circadian_get_profile(const circadian_t* circadian) {
    if (!circadian) return get_day_profile();
    return circadian->current_profile;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void circadian_stats(const circadian_t* circadian) {
    if (!circadian) return;

    printf("\n[Circadian Statistics]\n");
    printf("  Current Phase:       %s %s\n",
           circadian_phase_icon(circadian->current_phase),
           circadian_phase_string(circadian->current_phase));
    printf("  Total Cycles:        %lu\n", circadian->cycle_count);
    printf("  Phase Transitions:   %lu\n", circadian->phase_transitions);
    printf("  Dawn Cycles:         %lu\n", circadian->dawn_cycles);
    printf("  Day Cycles:          %lu\n", circadian->day_cycles);
    printf("  Evening Cycles:      %lu\n", circadian->evening_cycles);

    time_t elapsed = time(NULL) - circadian->phase_start_time;
    printf("  Time in Phase:       %ld seconds\n", elapsed);

    printf("\n  Current Profile:\n");
    printf("    Input Throttle:    %d%%\n", circadian->current_profile.input_throttle);
    printf("    Brain Priority:    %d%%\n", circadian->current_profile.brain_priority);
    printf("    Cleanup Priority:  %d%%\n", circadian->current_profile.cleanup_priority);
    printf("    Learning Priority: %d%%\n", circadian->current_profile.learning_priority);

    printf("\n");
}

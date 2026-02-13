/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_heart.c
 *
 * Heart Engine Implementation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_heart.h"
#include "kim_stomach.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Internal Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 밸브 초기화 */
static void init_valve(valve_t* valve, const char* target) {
    valve->open = 1;
    valve->flow_rate = 50;  /* 기본 50% */
    strncpy(valve->target, target, sizeof(valve->target) - 1);
}

/* 박동 스레드 */
static void* heart_beat_loop(void* arg) {
    heart_t* h = (heart_t*)arg;

    printf("[Heart] 💓 박동 스레드 시작\n");

    while (h->running) {
        pthread_mutex_lock(&h->lock);

        /* 1. 자동 조절 */
        heart_auto_adjust(h);

        /* 2. 혈압 체크 */
        heart_check_pressure(h);

        /* 3. 박동 */
        h->total_beats++;

        /* 4. BPM 조정 (목표치로 점진적 이동) */
        if (h->current_bpm < h->target_bpm) {
            h->current_bpm++;
        } else if (h->current_bpm > h->target_bpm) {
            h->current_bpm--;
        }

        pthread_mutex_unlock(&h->lock);

        /* 5. 박동 간격만큼 대기 */
        int interval_ms = BEAT_INTERVAL(h->current_bpm);
        usleep(interval_ms * 1000);
    }

    printf("[Heart] 💓 박동 스레드 종료\n");
    return NULL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

heart_t* heart_create(void) {
    heart_t* h = (heart_t*)malloc(sizeof(heart_t));
    if (!h) {
        fprintf(stderr, "[Heart] Error: malloc failed\n");
        return NULL;
    }

    /* BPM 초기화 */
    h->current_bpm = BPM_IDLE;
    h->target_bpm = BPM_IDLE;
    h->gear = GEAR_IDLE;

    /* 혈압 초기화 */
    h->bp_status = BP_NORMAL;
    h->systolic = 50;
    h->diastolic = 30;

    /* 밸브 초기화 */
    init_valve(&h->valve_stomach, "stomach");
    init_valve(&h->valve_brain, "brain");
    init_valve(&h->valve_kidney, "kidney");

    /* 장기 연결 */
    h->stomach = NULL;
    h->cortex = NULL;

    /* 스레드 초기화 */
    pthread_mutex_init(&h->lock, NULL);
    h->running = 0;

    /* 통계 초기화 */
    h->total_beats = 0;
    h->gear_shifts = 0;
    h->bp_warnings = 0;

    printf("[Heart] 💓 심장 생성 완료 (BPM: %d)\n", h->current_bpm);
    return h;
}

void heart_destroy(heart_t* h) {
    if (!h) return;

    /* 박동 중지 */
    if (h->running) {
        heart_stop(h);
    }

    pthread_mutex_destroy(&h->lock);
    free(h);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Engine Control
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int heart_start(heart_t* h) {
    if (!h) return -1;

    pthread_mutex_lock(&h->lock);

    if (h->running) {
        pthread_mutex_unlock(&h->lock);
        printf("[Heart] ⚠️  이미 작동 중입니다\n");
        return -1;
    }

    h->running = 1;
    pthread_mutex_unlock(&h->lock);

    /* 박동 스레드 시작 */
    int ret = pthread_create(&h->beat_thread, NULL, heart_beat_loop, h);
    if (ret != 0) {
        fprintf(stderr, "[Heart] Error: pthread_create failed\n");
        h->running = 0;
        return -1;
    }

    printf("[Heart] 🚀 박동 시작 (BPM: %d)\n", h->current_bpm);
    return 0;
}

int heart_stop(heart_t* h) {
    if (!h) return -1;

    pthread_mutex_lock(&h->lock);

    if (!h->running) {
        pthread_mutex_unlock(&h->lock);
        return 0;
    }

    h->running = 0;
    pthread_mutex_unlock(&h->lock);

    /* 스레드 종료 대기 */
    pthread_join(h->beat_thread, NULL);

    printf("[Heart] 🛑 박동 중지\n");
    return 0;
}

void heart_set_bpm(heart_t* h, int bpm) {
    if (!h) return;

    /* BPM 범위 제한 */
    if (bpm < BPM_MIN) bpm = BPM_MIN;
    if (bpm > BPM_MAX) bpm = BPM_MAX;

    pthread_mutex_lock(&h->lock);
    h->target_bpm = bpm;
    pthread_mutex_unlock(&h->lock);

    printf("[Heart] 🎚️  목표 BPM 설정: %d\n", bpm);
}

void heart_set_gear(heart_t* h, heart_gear_t gear) {
    if (!h) return;

    pthread_mutex_lock(&h->lock);

    if (h->gear != gear) {
        h->gear = gear;
        h->gear_shifts++;

        /* 기어에 따라 목표 BPM 자동 설정 */
        switch (gear) {
            case GEAR_IDLE:
                h->target_bpm = BPM_IDLE;
                printf("[Heart] 🐌 기어 변속: IDLE (BPM: %d)\n", BPM_IDLE);
                break;
            case GEAR_CRUISE:
                h->target_bpm = BPM_NORMAL;
                printf("[Heart] 🚗 기어 변속: CRUISE (BPM: %d)\n", BPM_NORMAL);
                break;
            case GEAR_BOOST:
                h->target_bpm = BPM_HIGH;
                printf("[Heart] 🚀 기어 변속: BOOST (BPM: %d)\n", BPM_HIGH);
                break;
        }
    }

    pthread_mutex_unlock(&h->lock);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Organ Integration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void heart_connect_stomach(heart_t* h, struct stomach_t* stomach) {
    if (!h) return;

    pthread_mutex_lock(&h->lock);
    h->stomach = stomach;
    pthread_mutex_unlock(&h->lock);

    printf("[Heart] 🔗 위장 연결 완료\n");
}

void heart_connect_cortex(heart_t* h, struct cortex_t* cortex) {
    if (!h) return;

    pthread_mutex_lock(&h->lock);
    h->cortex = cortex;
    pthread_mutex_unlock(&h->lock);

    printf("[Heart] 🔗 뇌 연결 완료\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Adaptive Control
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void heart_auto_adjust(heart_t* h) {
    if (!h) return;

    int stomach_fill = 0;
    int brain_load = 0;  /* TODO: 뇌 구현 후 연동 */

    /* 위장 포만도 체크 */
    if (h->stomach) {
        stomach_fill = stomach_get_fill_percent(h->stomach);
    }

    /* 자동 조절 로직 (lock 없이 직접 변경) */
    heart_gear_t new_gear = h->gear;

    if (stomach_fill >= 90) {
        new_gear = GEAR_IDLE;
    } else if (stomach_fill >= 60) {
        new_gear = GEAR_CRUISE;
    } else if (stomach_fill >= 10) {
        new_gear = GEAR_BOOST;
    } else {
        new_gear = GEAR_IDLE;
    }

    /* 기어 변경 (이미 lock 잡힌 상태) */
    if (h->gear != new_gear) {
        h->gear = new_gear;
        h->gear_shifts++;

        /* 목표 BPM 설정 */
        switch (new_gear) {
            case GEAR_IDLE:
                h->target_bpm = BPM_IDLE;
                printf("   🐌 [Heart] 위장 %d%% → IDLE 전환 (BPM: %d)\n", stomach_fill, BPM_IDLE);
                break;
            case GEAR_CRUISE:
                h->target_bpm = BPM_NORMAL;
                printf("   🚗 [Heart] 위장 %d%% → CRUISE 전환 (BPM: %d)\n", stomach_fill, BPM_NORMAL);
                break;
            case GEAR_BOOST:
                h->target_bpm = BPM_HIGH;
                printf("   🚀 [Heart] 위장 %d%% → BOOST 전환 (BPM: %d)\n", stomach_fill, BPM_HIGH);
                break;
        }
    }

    (void)brain_load;  /* 미사용 경고 제거 */
}

void heart_check_pressure(heart_t* h) {
    if (!h) return;

    int stomach_fill = 0;

    /* 위장 포만도로 혈압 계산 */
    if (h->stomach) {
        stomach_fill = stomach_get_fill_percent(h->stomach);
    }

    /* 수축기 압력 = 위장 포만도 */
    h->systolic = stomach_fill;

    /* 이완기 압력 = BPM 기반 */
    h->diastolic = (h->current_bpm * 100) / BPM_MAX;

    /* 혈압 상태 결정 */
    blood_pressure_t old_bp = h->bp_status;

    if (h->systolic >= LOAD_CRITICAL || h->diastolic >= LOAD_CRITICAL) {
        h->bp_status = BP_HYPERTENSION;
    } else if (h->systolic >= LOAD_HIGH || h->diastolic >= LOAD_HIGH) {
        h->bp_status = BP_PREHYPER;
    } else if (h->systolic <= LOAD_LOW && h->diastolic <= LOAD_LOW) {
        h->bp_status = BP_HYPOTENSION;
    } else {
        h->bp_status = BP_NORMAL;
    }

    /* 혈압 상태 변화 감지 */
    if (old_bp != h->bp_status && h->bp_status >= BP_PREHYPER) {
        h->bp_warnings++;
        printf("   ⚠️  [Heart] 혈압 경고: %s (수축기: %d, 이완기: %d)\n",
               bp_status_string(h->bp_status), h->systolic, h->diastolic);
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Valve Control
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void heart_open_valve(heart_t* h, const char* target) {
    if (!h || !target) return;

    pthread_mutex_lock(&h->lock);

    if (strcmp(target, "stomach") == 0) {
        h->valve_stomach.open = 1;
        printf("   🚪 [Heart] 위장 밸브 개방\n");
    } else if (strcmp(target, "brain") == 0) {
        h->valve_brain.open = 1;
        printf("   🚪 [Heart] 뇌 밸브 개방\n");
    } else if (strcmp(target, "kidney") == 0) {
        h->valve_kidney.open = 1;
        printf("   🚪 [Heart] 콩팥 밸브 개방\n");
    }

    pthread_mutex_unlock(&h->lock);
}

void heart_close_valve(heart_t* h, const char* target) {
    if (!h || !target) return;

    pthread_mutex_lock(&h->lock);

    if (strcmp(target, "stomach") == 0) {
        h->valve_stomach.open = 0;
        printf("   🚫 [Heart] 위장 밸브 폐쇄\n");
    } else if (strcmp(target, "brain") == 0) {
        h->valve_brain.open = 0;
        printf("   🚫 [Heart] 뇌 밸브 폐쇄\n");
    } else if (strcmp(target, "kidney") == 0) {
        h->valve_kidney.open = 0;
        printf("   🚫 [Heart] 콩팥 밸브 폐쇄\n");
    }

    pthread_mutex_unlock(&h->lock);
}

void heart_set_flow_rate(heart_t* h, const char* target, int rate) {
    if (!h || !target) return;

    /* 유속 범위 제한 */
    if (rate < 0) rate = 0;
    if (rate > 100) rate = 100;

    pthread_mutex_lock(&h->lock);

    if (strcmp(target, "stomach") == 0) {
        h->valve_stomach.flow_rate = rate;
    } else if (strcmp(target, "brain") == 0) {
        h->valve_brain.flow_rate = rate;
    } else if (strcmp(target, "kidney") == 0) {
        h->valve_kidney.flow_rate = rate;
    }

    pthread_mutex_unlock(&h->lock);

    printf("   🎚️  [Heart] %s 유속 설정: %d%%\n", target, rate);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Status Check
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int heart_get_bpm(const heart_t* h) {
    return h ? h->current_bpm : 0;
}

heart_gear_t heart_get_gear(const heart_t* h) {
    return h ? h->gear : GEAR_IDLE;
}

blood_pressure_t heart_get_bp_status(const heart_t* h) {
    return h ? h->bp_status : BP_NORMAL;
}

void heart_stats(const heart_t* h) {
    if (!h) return;

    printf("\n[Heart Statistics]\n");
    printf("  BPM:             %d (목표: %d)\n", h->current_bpm, h->target_bpm);
    printf("  Gear:            %s\n", gear_string(h->gear));
    printf("  Blood Pressure:  %s (수축기: %d, 이완기: %d)\n",
           bp_status_string(h->bp_status), h->systolic, h->diastolic);
    printf("  Total Beats:     %lu\n", h->total_beats);
    printf("  Gear Shifts:     %lu\n", h->gear_shifts);
    printf("  BP Warnings:     %lu\n", h->bp_warnings);

    printf("\n[Valves]\n");
    printf("  Stomach: %s (유속: %d%%)\n",
           h->valve_stomach.open ? "열림" : "닫힘", h->valve_stomach.flow_rate);
    printf("  Brain:   %s (유속: %d%%)\n",
           h->valve_brain.open ? "열림" : "닫힘", h->valve_brain.flow_rate);
    printf("  Kidney:  %s (유속: %d%%)\n",
           h->valve_kidney.open ? "열림" : "닫힘", h->valve_kidney.flow_rate);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utilities
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

const char* gear_string(heart_gear_t gear) {
    switch (gear) {
        case GEAR_IDLE:     return "IDLE (아이들링)";
        case GEAR_CRUISE:   return "CRUISE (크루징)";
        case GEAR_BOOST:    return "BOOST (부스터)";
        default:            return "UNKNOWN";
    }
}

const char* bp_status_string(blood_pressure_t bp) {
    switch (bp) {
        case BP_HYPOTENSION:    return "저혈압";
        case BP_NORMAL:         return "정상";
        case BP_PREHYPER:       return "전 고혈압";
        case BP_HYPERTENSION:   return "고혈압";
        default:                return "UNKNOWN";
    }
}

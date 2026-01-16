/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_spine.c
 *
 * Spinal Cord (High-Speed Control Bus) Implementation
 *
 * Zero Dependency: pthread + stdlib만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_spine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Timestamp Helper
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static int64_t get_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Signal Type Name (디버깅용)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static const char* signal_type_name(signal_type_t type) {
    switch (type) {
        case SIGNAL_PAIN:       return "PAIN";
        case SIGNAL_HEAT:       return "HEAT";
        case SIGNAL_PRESSURE:   return "PRESSURE";
        case SIGNAL_TOUCH:      return "TOUCH";
        case SIGNAL_VISION:     return "VISION";
        case SIGNAL_SOUND:      return "SOUND";
        case SIGNAL_MOVE:       return "MOVE";
        case SIGNAL_STOP:       return "STOP";
        case SIGNAL_SPEED_UP:   return "SPEED_UP";
        case SIGNAL_SPEED_DOWN: return "SPEED_DOWN";
        case SIGNAL_SECRETE:    return "SECRETE";
        case SIGNAL_CONTRACT:   return "CONTRACT";
        case SIGNAL_RELAX:      return "RELAX";
        case REFLEX_WITHDRAW:   return "REFLEX_WITHDRAW";
        case REFLEX_COUGH:      return "REFLEX_COUGH";
        case REFLEX_BLINK:      return "REFLEX_BLINK";
        default:                return "UNKNOWN";
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
spine_t* spine_create(void) {
    spine_t* s = (spine_t*)malloc(sizeof(spine_t));
    if (!s) {
        fprintf(stderr, "[Spine] Error: malloc failed\n");
        return NULL;
    }

    memset(s, 0, sizeof(spine_t));
    pthread_mutex_init(&s->lock, NULL);

    printf("[Spine] 척수 생성 완료 (High-Speed Control Bus)\n");
    return s;
}

void spine_destroy(spine_t* s) {
    if (!s) return;
    pthread_mutex_destroy(&s->lock);
    free(s);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Callback Registration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void spine_set_sensory_callback(spine_t* s, void (*callback)(signal_packet_t*)) {
    if (!s) return;
    s->sensory_callback = callback;
    printf("[Spine] 상행로 콜백 등록 완료 (Sensory → Brain)\n");
}

void spine_set_motor_callback(spine_t* s, void (*callback)(signal_packet_t*)) {
    if (!s) return;
    s->motor_callback = callback;
    printf("[Spine] 하행로 콜백 등록 완료 (Brain → Motor)\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Reflex Arc Registration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int spine_register_reflex(spine_t* s, signal_type_t type, reflex_handler_t handler) {
    if (!s || type >= 256) return -1;

    pthread_mutex_lock(&s->lock);

    s->reflex_handlers[type] = handler;
    s->reflex_count++;

    printf("[Spine] 반사 신경 등록: %s (Type=%d)\n", signal_type_name(type), type);

    pthread_mutex_unlock(&s->lock);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Reflex Check (뇌 우회 처리)
 *
 * 반사 신경이 등록되어 있으면 즉시 처리하고 1 리턴
 * 없으면 0 리턴 (뇌로 올려보내야 함)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int spine_check_reflex(spine_t* s, signal_packet_t* signal) {
    if (!s || !signal) return 0;

    pthread_mutex_lock(&s->lock);

    signal_type_t type = signal->type;

    /* 반사 핸들러가 등록되어 있는지 확인 */
    if (type < 256 && s->reflex_handlers[type] != NULL) {
        printf("   ⚡ [Reflex] 척수 반사 발동! %s (뇌 우회)\n", signal_type_name(type));

        /* 반사 핸들러 실행 */
        int result = s->reflex_handlers[type](signal);

        s->total_reflexes++;
        pthread_mutex_unlock(&s->lock);

        return result;  /* 1 = 처리 완료 */
    }

    pthread_mutex_unlock(&s->lock);
    return 0;  /* 0 = 반사 없음, 뇌로 보내야 함 */
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Send Sensory Signal (상행로: Body → Brain)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int spine_send_sensory(spine_t* s, signal_packet_t* signal) {
    if (!s || !signal) return -1;

    int64_t start_time = get_timestamp_us();

    /* 1단계: 반사 신경 체크 */
    if (spine_check_reflex(s, signal)) {
        /* 반사로 처리됨 → 뇌로 안 보냄 */
        int64_t latency = get_timestamp_us() - start_time;
        printf("   🔥 [Spine] 반사 처리 완료: %ld us (뇌 보고 생략)\n", latency);
        return 0;
    }

    /* 2단계: 반사 아님 → 뇌로 보고 */
    printf("   ⬆️ [Ascending] %s: 장기 %d → 뇌로 보고\n",
           signal_type_name(signal->type), signal->source_id);

    pthread_mutex_lock(&s->lock);

    if (s->sensory_callback) {
        s->sensory_callback(signal);
    } else {
        printf("   ⚠️ [Spine] Warning: sensory_callback not registered\n");
    }

    s->total_sensory++;

    int64_t latency = get_timestamp_us() - start_time;
    s->avg_latency_us = (s->avg_latency_us + latency) / 2;

    pthread_mutex_unlock(&s->lock);

    printf("   ✅ [Spine] 상행 전송 완료: %ld us\n", latency);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Send Motor Command (하행로: Brain → Body)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int spine_send_motor(spine_t* s, signal_packet_t* signal) {
    if (!s || !signal) return -1;

    int64_t start_time = get_timestamp_us();

    printf("   ⬇️ [Descending] %s: 뇌 → 장기 %d로 명령\n",
           signal_type_name(signal->type), signal->target_id);

    pthread_mutex_lock(&s->lock);

    if (s->motor_callback) {
        s->motor_callback(signal);
    } else {
        printf("   ⚠️ [Spine] Warning: motor_callback not registered\n");
    }

    s->total_motor++;

    int64_t latency = get_timestamp_us() - start_time;
    s->avg_latency_us = (s->avg_latency_us + latency) / 2;

    pthread_mutex_unlock(&s->lock);

    printf("   ✅ [Spine] 하행 전송 완료: %ld us\n", latency);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void spine_stats(const spine_t* s) {
    if (!s) return;

    printf("\n[Spine Statistics]\n");
    printf("  Total Sensory:   %lu (Body → Brain)\n", s->total_sensory);
    printf("  Total Motor:     %lu (Brain → Body)\n", s->total_motor);
    printf("  Total Reflexes:  %lu (Bypassed Brain)\n", s->total_reflexes);
    printf("  Avg Latency:     %lu us\n", s->avg_latency_us);
    printf("  Registered Reflexes: %d\n", s->reflex_count);

    if (s->total_sensory + s->total_motor > 0) {
        float reflex_rate = (float)s->total_reflexes / (s->total_sensory + s->total_reflexes) * 100.0f;
        printf("  Reflex Rate:     %.1f%% (뇌 우회율)\n", reflex_rate);
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Signal Packet Helpers
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
signal_packet_t* signal_create(
    signal_type_t type,
    int source_id,
    int target_id,
    void* data,
    size_t data_size
) {
    signal_packet_t* sig = (signal_packet_t*)malloc(sizeof(signal_packet_t));
    if (!sig) return NULL;

    sig->type = type;
    sig->source_id = source_id;
    sig->target_id = target_id;
    sig->timestamp = get_timestamp_us();
    sig->priority = 0;

    if (data && data_size > 0) {
        sig->data = malloc(data_size);
        memcpy(sig->data, data, data_size);
        sig->data_size = data_size;
    } else {
        sig->data = NULL;
        sig->data_size = 0;
    }

    return sig;
}

void signal_destroy(signal_packet_t* sig) {
    if (!sig) return;
    if (sig->data) {
        free(sig->data);
    }
    free(sig);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_watchdog.c
 *
 * 워치독 구현 (자가 치유 시스템)
 *
 * Zero Dependency: stdlib + stdio + time만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_watchdog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utility Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static uint64_t get_timestamp_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * String Utilities
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

const char* organ_type_string(organ_type_t type) {
    switch (type) {
        case ORGAN_STOMACH:   return "Stomach";
        case ORGAN_PANCREAS:  return "Pancreas";
        case ORGAN_SPINE:     return "Spine";
        case ORGAN_CORTEX:    return "Cortex";
        case ORGAN_CIRCADIAN: return "Circadian";
        default:              return "Unknown";
    }
}

const char* organ_status_string(organ_status_t status) {
    switch (status) {
        case STATUS_OK:       return "🟢 OK";
        case STATUS_WARNING:  return "🟡 WARNING";
        case STATUS_CRITICAL: return "🔴 CRITICAL";
        case STATUS_DEAD:     return "⚫ DEAD";
        default:              return "❓ UNKNOWN";
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

watchdog_t* watchdog_create(void) {
    watchdog_t* wd = (watchdog_t*)malloc(sizeof(watchdog_t));
    if (!wd) {
        fprintf(stderr, "[Watchdog] Error: malloc failed\n");
        return NULL;
    }

    memset(wd, 0, sizeof(watchdog_t));
    wd->organ_count = 0;

    printf("[Watchdog] 자가 치유 시스템 활성화 (Fault Tolerance)\n");
    printf("[Watchdog] 감시 준비 완료\n");

    return wd;
}

void watchdog_destroy(watchdog_t* watchdog) {
    if (!watchdog) return;
    printf("[Watchdog] 감시 종료 (총 %lu회 체크, %lu회 복구)\n",
           watchdog->total_checks, watchdog->total_recoveries);
    free(watchdog);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Organ Registration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int watchdog_register_organ(watchdog_t* watchdog,
                             organ_type_t type,
                             const char* name,
                             uint64_t timeout_ms,
                             bool (*recovery_func)(void)) {
    if (!watchdog) {
        fprintf(stderr, "[Watchdog] Error: NULL watchdog\n");
        return -1;
    }

    if (watchdog->organ_count >= ORGAN_MAX) {
        fprintf(stderr, "[Watchdog] Error: 최대 장기 수 초과\n");
        return -1;
    }

    organ_monitor_t* organ = &watchdog->organs[watchdog->organ_count];

    organ->type = type;
    strncpy(organ->name, name, sizeof(organ->name) - 1);
    organ->name[sizeof(organ->name) - 1] = '\0';
    organ->status = STATUS_OK;
    organ->last_heartbeat = get_timestamp_ms();
    organ->heartbeat_timeout_ms = timeout_ms;
    organ->failure_count = 0;
    organ->recovery_count = 0;
    organ->recovery_func = recovery_func;

    watchdog->organ_count++;

    printf("[Watchdog] 장기 등록: %s (%s) - Timeout: %lu ms\n",
           organ->name,
           organ_type_string(type),
           timeout_ms);

    return watchdog->organ_count - 1;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Heartbeat
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void watchdog_heartbeat(watchdog_t* watchdog, organ_type_t organ) {
    if (!watchdog) return;

    for (int i = 0; i < watchdog->organ_count; i++) {
        if (watchdog->organs[i].type == organ) {
            watchdog->organs[i].last_heartbeat = get_timestamp_ms();

            /* 복구 확인 (CRITICAL → OK) */
            if (watchdog->organs[i].status != STATUS_OK) {
                printf("[Watchdog] ✅ 장기 복구 확인: %s\n",
                       watchdog->organs[i].name);
                watchdog->organs[i].status = STATUS_OK;
                watchdog->organs[i].failure_count = 0;

                if (watchdog->on_recovery) {
                    watchdog->on_recovery(organ);
                }
            }
            return;
        }
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Health Check (1초마다 호출)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void watchdog_check(watchdog_t* watchdog) {
    if (!watchdog) return;

    uint64_t now = get_timestamp_ms();
    watchdog->total_checks++;

    for (int i = 0; i < watchdog->organ_count; i++) {
        organ_monitor_t* organ = &watchdog->organs[i];
        uint64_t elapsed = now - organ->last_heartbeat;

        /* 타임아웃 체크 */
        if (elapsed > organ->heartbeat_timeout_ms) {
            organ->failure_count++;
            watchdog->total_failures++;

            /* 상태 전환 */
            organ_status_t old_status = organ->status;

            if (elapsed > organ->heartbeat_timeout_ms * 3) {
                organ->status = STATUS_DEAD;
            } else if (elapsed > organ->heartbeat_timeout_ms * 2) {
                organ->status = STATUS_CRITICAL;
            } else {
                organ->status = STATUS_WARNING;
            }

            /* 상태 변화 시 콜백 */
            if (old_status != organ->status) {
                if (organ->status == STATUS_WARNING) {
                    watchdog->total_warnings++;
                    printf("[Watchdog] ⚠️  경고: %s - %lu ms 무응답\n",
                           organ->name, elapsed);

                    if (watchdog->on_warning) {
                        watchdog->on_warning(organ->type);
                    }
                }
                else if (organ->status == STATUS_CRITICAL) {
                    printf("[Watchdog] 🔴 위험: %s - %lu ms 무응답 (복구 시도)\n",
                           organ->name, elapsed);

                    if (watchdog->on_failure) {
                        watchdog->on_failure(organ->type);
                    }

                    /* 자동 복구 시도 */
                    if (organ->recovery_func) {
                        printf("[Watchdog] 🔧 자동 복구 실행: %s\n", organ->name);
                        bool success = organ->recovery_func();
                        if (success) {
                            organ->recovery_count++;
                            watchdog->total_recoveries++;
                            organ->last_heartbeat = now;
                            organ->status = STATUS_OK;
                            organ->failure_count = 0;
                            printf("[Watchdog] ✅ 복구 성공: %s\n", organ->name);

                            if (watchdog->on_recovery) {
                                watchdog->on_recovery(organ->type);
                            }
                        } else {
                            printf("[Watchdog] ❌ 복구 실패: %s\n", organ->name);
                        }
                    }
                }
                else if (organ->status == STATUS_DEAD) {
                    printf("[Watchdog] ⚫ 사망: %s - %lu ms 무응답 (복구 불가)\n",
                           organ->name, elapsed);
                }
            }
        }
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Callbacks
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void watchdog_set_warning_callback(watchdog_t* watchdog,
                                    void (*callback)(organ_type_t)) {
    if (!watchdog) return;
    watchdog->on_warning = callback;
}

void watchdog_set_failure_callback(watchdog_t* watchdog,
                                    void (*callback)(organ_type_t)) {
    if (!watchdog) return;
    watchdog->on_failure = callback;
}

void watchdog_set_recovery_callback(watchdog_t* watchdog,
                                     void (*callback)(organ_type_t)) {
    if (!watchdog) return;
    watchdog->on_recovery = callback;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

void watchdog_stats(const watchdog_t* watchdog) {
    if (!watchdog) return;

    printf("\n[Watchdog Statistics]\n");
    printf("  Total Checks:      %lu\n", watchdog->total_checks);
    printf("  Total Warnings:    %lu\n", watchdog->total_warnings);
    printf("  Total Failures:    %lu\n", watchdog->total_failures);
    printf("  Total Recoveries:  %lu\n", watchdog->total_recoveries);

    if (watchdog->total_checks > 0) {
        printf("  Warning Rate:      %.1f%%\n",
               (watchdog->total_warnings * 100.0) / watchdog->total_checks);
        printf("  Failure Rate:      %.1f%%\n",
               (watchdog->total_failures * 100.0) / watchdog->total_checks);
    }

    printf("\n  Registered Organs: %d\n", watchdog->organ_count);
    printf("  ┌─────────────────┬───────────┬──────────┬──────────┬──────────┐\n");
    printf("  │ Organ           │ Status    │ Failures │ Recovery │ Timeout  │\n");
    printf("  ├─────────────────┼───────────┼──────────┼──────────┼──────────┤\n");

    for (int i = 0; i < watchdog->organ_count; i++) {
        const organ_monitor_t* organ = &watchdog->organs[i];
        printf("  │ %-15s │ %-9s │ %8u │ %8u │ %6lu ms│\n",
               organ->name,
               organ_status_string(organ->status),
               organ->failure_count,
               organ->recovery_count,
               organ->heartbeat_timeout_ms);
    }

    printf("  └─────────────────┴───────────┴──────────┴──────────┴──────────┘\n");
    printf("\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_watchdog.h
 *
 * 워치독(Watchdog) - 자가 치유 시스템
 *
 * 생물학적 역할:
 *   - 면역계(Immune System)
 *   - 자율신경계(Autonomic)
 *   - 항상성 유지(Homeostasis)
 *   - 손상 감지 및 복구
 *
 * 소프트웨어 역할:
 *   - Health Check
 *   - Auto Recovery
 *   - Fault Tolerance
 *   - Circuit Breaker
 *   - Self-Healing
 *
 * 감시 항목:
 *   - 장기별 응답 시간 (Timeout)
 *   - 메모리 누수 (Memory Leak)
 *   - CPU 과부하 (Overload)
 *   - 반복적 실패 (Repeated Failure)
 *
 * Priority: ★핵심★ (Fault Tolerance)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_WATCHDOG_H
#define KIM_WATCHDOG_H

#include <stdint.h>
#include <stdbool.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Organ Type (감시 대상 장기)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef enum {
    ORGAN_STOMACH,      /* 위장 */
    ORGAN_PANCREAS,     /* 췌장 */
    ORGAN_SPINE,        /* 척수 */
    ORGAN_CORTEX,       /* 대뇌 */
    ORGAN_CIRCADIAN,    /* 서카디언 */
    ORGAN_MAX
} organ_type_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Organ Status (장기 상태)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef enum {
    STATUS_OK,          /* 정상 */
    STATUS_WARNING,     /* 경고 */
    STATUS_CRITICAL,    /* 위험 */
    STATUS_DEAD         /* 사망 */
} organ_status_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Organ Monitor (장기 모니터)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    organ_type_t   type;                /* 장기 유형 */
    char           name[32];            /* 장기 이름 */
    organ_status_t status;              /* 현재 상태 */

    /* Health Metrics */
    uint64_t last_heartbeat;            /* 마지막 하트비트 (timestamp) */
    uint64_t heartbeat_timeout_ms;      /* 타임아웃 (ms) */
    uint32_t failure_count;             /* 연속 실패 횟수 */
    uint32_t recovery_count;            /* 복구 횟수 */

    /* Recovery Callback */
    bool (*recovery_func)(void);        /* 복구 함수 */
} organ_monitor_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Watchdog (워치독)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    /* Monitors */
    organ_monitor_t organs[ORGAN_MAX];  /* 장기 모니터 배열 */
    int             organ_count;        /* 등록된 장기 수 */

    /* Statistics */
    uint64_t total_checks;              /* 총 체크 횟수 */
    uint64_t total_warnings;            /* 총 경고 횟수 */
    uint64_t total_failures;            /* 총 실패 횟수 */
    uint64_t total_recoveries;          /* 총 복구 횟수 */

    /* Callbacks */
    void (*on_warning)(organ_type_t organ);     /* 경고 콜백 */
    void (*on_failure)(organ_type_t organ);     /* 실패 콜백 */
    void (*on_recovery)(organ_type_t organ);    /* 복구 콜백 */
} watchdog_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 워치독 생성/삭제 */
watchdog_t* watchdog_create(void);
void watchdog_destroy(watchdog_t* watchdog);

/* 장기 등록 */
int watchdog_register_organ(watchdog_t* watchdog,
                             organ_type_t type,
                             const char* name,
                             uint64_t timeout_ms,
                             bool (*recovery_func)(void));

/* 하트비트 (장기가 살아있음을 보고) */
void watchdog_heartbeat(watchdog_t* watchdog, organ_type_t organ);

/* 감시 (1초마다 호출) */
void watchdog_check(watchdog_t* watchdog);

/* 콜백 등록 */
void watchdog_set_warning_callback(watchdog_t* watchdog,
                                    void (*callback)(organ_type_t));
void watchdog_set_failure_callback(watchdog_t* watchdog,
                                    void (*callback)(organ_type_t));
void watchdog_set_recovery_callback(watchdog_t* watchdog,
                                     void (*callback)(organ_type_t));

/* 통계 */
void watchdog_stats(const watchdog_t* watchdog);

/* 유틸리티 */
const char* organ_type_string(organ_type_t type);
const char* organ_status_string(organ_status_t status);

#endif /* KIM_WATCHDOG_H */

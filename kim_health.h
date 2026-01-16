/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_health.h
 *
 * 전신 헬스체크 시스템 (Full Body Scan)
 *
 * 생물학적 역할:
 *   - 뇌(CNS)가 주관하는 전신 상태 점검
 *   - 척수(Spine)를 통해 각 장기 순회
 *   - 실시간 건강 상태 모니터링
 *   - 이상 징후 조기 발견 및 대응
 *
 * 소프트웨어 역할:
 *   - System Health Monitor
 *   - Performance Dashboard
 *   - Resource Usage Tracking
 *   - Alerting System
 *
 * 검사 항목:
 *   - 위장(Stomach): Buffer 사용률
 *   - 췌장(Pancreas): Parser 성능
 *   - 척수(Spine): 신호 지연/부하
 *
 * Priority: ★핵심★ (Monitoring)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_HEALTH_H
#define KIM_HEALTH_H

#include <stdint.h>
#include <stdbool.h>
#include "kim_stomach.h"
#include "kim_pancreas.h"
#include "kim_spine.h"

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Health Status (건강 상태 등급)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef enum {
    HEALTH_DEAD        = 0,     /* ⚫ 사망 (응답 없음) */
    HEALTH_CRITICAL    = 1,     /* 🔴 위험 (기능 장애) */
    HEALTH_WARNING     = 2,     /* 🟡 주의 (부하 높음) */
    HEALTH_HEALTHY     = 3      /* 🟢 정상 (양호) */
} health_status_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Diagnosis (진단서)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    char              organ_name[32];   /* 장기 이름 */
    health_status_t   status;           /* 건강 상태 */
    int               load_percent;     /* 부하율 (0~100) */
    char              message[64];      /* 상세 증상 */
    uint64_t          timestamp;        /* 진단 시각 */
} diagnosis_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Health Report (종합 건강 진단서)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    diagnosis_t*   diagnoses;           /* 진단 배열 */
    int            count;               /* 진단 수 */
    health_status_t overall_status;     /* 종합 상태 */
    bool           system_critical;     /* 비상 사태 여부 */
    uint64_t       timestamp;           /* 보고 시각 */
} health_report_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Health Monitor (헬스 모니터)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    /* 장기 참조 */
    stomach_t*  stomach;
    pancreas_t* pancreas;
    spine_t*    spine;

    /* 통계 */
    uint64_t total_scans;               /* 총 검사 횟수 */
    uint64_t total_warnings;            /* 경고 횟수 */
    uint64_t total_criticals;           /* 위험 횟수 */
    uint64_t last_scan_time;            /* 마지막 검사 시각 */
} health_monitor_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 헬스 모니터 생성/삭제 */
health_monitor_t* health_monitor_create(
    stomach_t*  stomach,
    pancreas_t* pancreas,
    spine_t*    spine
);
void health_monitor_destroy(health_monitor_t* monitor);

/* 개별 장기 진단 */
diagnosis_t health_check_stomach(const stomach_t* stomach);
diagnosis_t health_check_pancreas(const pancreas_t* pancreas);
diagnosis_t health_check_spine(const spine_t* spine);

/* 전신 헬스체크 (Full Body Scan) */
health_report_t* health_full_scan(health_monitor_t* monitor);

/* 리포트 출력 */
void health_print_report(const health_report_t* report);

/* 리포트 삭제 */
void health_report_destroy(health_report_t* report);

/* 통계 */
void health_monitor_stats(const health_monitor_t* monitor);

/* 상태 문자열 */
const char* health_status_string(health_status_t status);
const char* health_status_icon(health_status_t status);

#endif /* KIM_HEALTH_H */

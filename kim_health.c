/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_health.c
 *
 * 전신 헬스체크 시스템 구현
 *
 * Zero Dependency: stdlib + stdio만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_health.h"
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
 * Status Strings
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
const char* health_status_string(health_status_t status) {
    switch (status) {
        case HEALTH_HEALTHY:  return "HEALTHY";
        case HEALTH_WARNING:  return "WARNING";
        case HEALTH_CRITICAL: return "CRITICAL";
        case HEALTH_DEAD:     return "DEAD";
        default:              return "UNKNOWN";
    }
}

const char* health_status_icon(health_status_t status) {
    switch (status) {
        case HEALTH_HEALTHY:  return "🟢";
        case HEALTH_WARNING:  return "🟡";
        case HEALTH_CRITICAL: return "🔴";
        case HEALTH_DEAD:     return "⚫";
        default:              return "⚪";
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
health_monitor_t* health_monitor_create(
    stomach_t*  stomach,
    pancreas_t* pancreas,
    spine_t*    spine
) {
    health_monitor_t* m = (health_monitor_t*)malloc(sizeof(health_monitor_t));
    if (!m) {
        fprintf(stderr, "[Health Monitor] Error: malloc failed\n");
        return NULL;
    }

    m->stomach = stomach;
    m->pancreas = pancreas;
    m->spine = spine;

    m->total_scans = 0;
    m->total_warnings = 0;
    m->total_criticals = 0;
    m->last_scan_time = 0;

    printf("[Health Monitor] 생성 완료 (전신 헬스체크 시스템)\n");
    return m;
}

void health_monitor_destroy(health_monitor_t* monitor) {
    if (!monitor) return;
    free(monitor);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Stomach Diagnosis (위장 진단)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
diagnosis_t health_check_stomach(const stomach_t* stomach) {
    diagnosis_t d;
    memset(&d, 0, sizeof(diagnosis_t));

    strcpy(d.organ_name, "Stomach (Buffer)");
    d.timestamp = get_timestamp_us();

    if (!stomach) {
        d.status = HEALTH_DEAD;
        strcpy(d.message, "Not Initialized");
        d.load_percent = 0;
        return d;
    }

    /* 버퍼 사용률 계산 */
    d.load_percent = (stomach->count * 100) / stomach->capacity;

    /* 진단 */
    if (d.load_percent >= 90) {
        d.status = HEALTH_CRITICAL;
        strcpy(d.message, "Buffer Overflow Imminent!");
    } else if (d.load_percent >= 60) {
        d.status = HEALTH_WARNING;
        strcpy(d.message, "Indigestion (High Load)");
    } else {
        d.status = HEALTH_HEALTHY;
        strcpy(d.message, "Ready (Normal)");
    }

    return d;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Pancreas Diagnosis (췌장 진단)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
diagnosis_t health_check_pancreas(const pancreas_t* pancreas) {
    diagnosis_t d;
    memset(&d, 0, sizeof(diagnosis_t));

    strcpy(d.organ_name, "Pancreas (Parser)");
    d.timestamp = get_timestamp_us();

    if (!pancreas) {
        d.status = HEALTH_DEAD;
        strcpy(d.message, "Not Initialized");
        d.load_percent = 0;
        return d;
    }

    /* 파싱 실패율 계산 */
    uint64_t total = pancreas->total_parsed + pancreas->parse_failures;
    int failure_rate = 0;

    if (total > 0) {
        failure_rate = (pancreas->parse_failures * 100) / total;
    }

    d.load_percent = failure_rate;

    /* 진단 */
    if (failure_rate >= 50) {
        d.status = HEALTH_CRITICAL;
        strcpy(d.message, "Parser Malfunction!");
    } else if (failure_rate >= 20) {
        d.status = HEALTH_WARNING;
        strcpy(d.message, "Parse Errors Detected");
    } else {
        d.status = HEALTH_HEALTHY;
        strcpy(d.message, "Digesting Normally");
    }

    return d;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Spine Diagnosis (척수 진단)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
diagnosis_t health_check_spine(const spine_t* spine) {
    diagnosis_t d;
    memset(&d, 0, sizeof(diagnosis_t));

    strcpy(d.organ_name, "Spine (Control Bus)");
    d.timestamp = get_timestamp_us();

    if (!spine) {
        d.status = HEALTH_DEAD;
        strcpy(d.message, "Not Initialized");
        d.load_percent = 0;
        return d;
    }

    /* 신호 부하율 계산 (지연시간 기준) */
    /* 정상: 0-10us, 주의: 10-50us, 위험: 50us+ */
    uint64_t latency = spine->avg_latency_us;

    if (latency < 10) {
        d.load_percent = (latency * 100) / 10;  /* 0-100% */
    } else if (latency < 50) {
        d.load_percent = 100;
    } else {
        d.load_percent = 100;
    }

    /* 진단 */
    if (latency >= 50) {
        d.status = HEALTH_CRITICAL;
        strcpy(d.message, "Nerve Damage (High Latency)");
    } else if (latency >= 10) {
        d.status = HEALTH_WARNING;
        strcpy(d.message, "Nerve Stress (Slow)");
    } else {
        d.status = HEALTH_HEALTHY;
        strcpy(d.message, "Fast Transmission");
    }

    return d;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Full Body Scan (전신 헬스체크)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
health_report_t* health_full_scan(health_monitor_t* monitor) {
    if (!monitor) return NULL;

    health_report_t* report = (health_report_t*)malloc(sizeof(health_report_t));
    if (!report) return NULL;

    /* 진단 배열 할당 (최대 3개 장기) */
    report->diagnoses = (diagnosis_t*)malloc(sizeof(diagnosis_t) * 3);
    if (!report->diagnoses) {
        free(report);
        return NULL;
    }

    report->count = 0;
    report->system_critical = false;
    report->timestamp = get_timestamp_us();

    /* 1. 위장 진단 */
    if (monitor->stomach) {
        report->diagnoses[report->count++] = health_check_stomach(monitor->stomach);
    }

    /* 2. 췌장 진단 */
    if (monitor->pancreas) {
        report->diagnoses[report->count++] = health_check_pancreas(monitor->pancreas);
    }

    /* 3. 척수 진단 */
    if (monitor->spine) {
        report->diagnoses[report->count++] = health_check_spine(monitor->spine);
    }

    /* 종합 상태 계산 */
    health_status_t worst_status = HEALTH_HEALTHY;

    for (int i = 0; i < report->count; i++) {
        health_status_t s = report->diagnoses[i].status;

        if (s < worst_status) {
            worst_status = s;
        }

        if (s == HEALTH_CRITICAL || s == HEALTH_DEAD) {
            report->system_critical = true;
            monitor->total_criticals++;
        } else if (s == HEALTH_WARNING) {
            monitor->total_warnings++;
        }
    }

    report->overall_status = worst_status;

    /* 통계 업데이트 */
    monitor->total_scans++;
    monitor->last_scan_time = report->timestamp;

    return report;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Print Report (리포트 출력)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void health_print_report(const health_report_t* report) {
    if (!report) return;

    printf("\n🏥 [CNS] 전신 헬스체크 (Full Body Scan)\n");
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("│ %-20s │ %-8s │ %-4s │ %-25s │\n",
           "Organ", "Status", "Load", "Message");
    printf("═══════════════════════════════════════════════════════════════════\n");

    for (int i = 0; i < report->count; i++) {
        diagnosis_t* d = &report->diagnoses[i];

        printf("│ %-20s │ %s %-7s │ %3d%% │ %-25s │\n",
               d->organ_name,
               health_status_icon(d->status),
               health_status_string(d->status),
               d->load_percent,
               d->message);
    }

    printf("═══════════════════════════════════════════════════════════════════\n");

    /* 종합 판단 */
    if (report->system_critical) {
        printf("🚨 [Brain] 비상 사태 선포! (System Critical)\n");
        printf("   👉 자율신경계에 '방어 모드' 가동 명령 하달\n");
    } else if (report->overall_status == HEALTH_WARNING) {
        printf("⚠️  [Brain] 주의 필요 (System Warning)\n");
        printf("   👉 모니터링 강화 및 예방 조치 권장\n");
    } else {
        printf("✨ [Brain] 시스템 상태 양호 (System Normal)\n");
    }

    printf("\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Report Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void health_report_destroy(health_report_t* report) {
    if (!report) return;
    if (report->diagnoses) {
        free(report->diagnoses);
    }
    free(report);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void health_monitor_stats(const health_monitor_t* monitor) {
    if (!monitor) return;

    printf("\n[Health Monitor Statistics]\n");
    printf("  Total Scans:     %lu\n", monitor->total_scans);
    printf("  Total Warnings:  %lu\n", monitor->total_warnings);
    printf("  Total Criticals: %lu\n", monitor->total_criticals);

    if (monitor->total_scans > 0) {
        float warning_rate = (float)monitor->total_warnings / monitor->total_scans * 100.0f;
        float critical_rate = (float)monitor->total_criticals / monitor->total_scans * 100.0f;

        printf("  Warning Rate:    %.1f%%\n", warning_rate);
        printf("  Critical Rate:   %.1f%%\n", critical_rate);
    }

    printf("\n");
}

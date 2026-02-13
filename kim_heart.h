/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_heart.h
 *
 * Heart Engine - System Clock & Scheduler
 *
 * 생물학적 역할:
 *   - 박동으로 산소와 영양분 운반 (데이터 유속 제어)
 *   - 혈압 조절 (시스템 부하 관리)
 *   - 자율 신경과 연동 (자동 완급 조절)
 *
 * 소프트웨어 역할:
 *   - System Clock (BPM 기반)
 *   - Flow Rate Controller (데이터 유속)
 *   - Adaptive Scheduler (부하 기반 자동 조절)
 *
 * Priority: ★핵심★ (1/5)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_HEART_H
#define KIM_HEART_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

/* Forward declarations */
struct stomach_t;
struct cortex_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Configuration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* BPM (Beats Per Minute) 범위 */
#define BPM_MIN             30      /* 서맥 (Bradycardia) */
#define BPM_IDLE            60      /* 아이들링 (휴식) */
#define BPM_NORMAL          90      /* 정상 (크루징) */
#define BPM_HIGH            120     /* 빈맥 (부스터) */
#define BPM_MAX             150     /* 최대 (위험) */

/* 부하 임계값 (Load Threshold) */
#define LOAD_LOW            20      /* 저부하 */
#define LOAD_NORMAL         60      /* 정상 부하 */
#define LOAD_HIGH           85      /* 고부하 */
#define LOAD_CRITICAL       95      /* 위험 부하 */

/* 박동 간격 (ms) = 60000 / BPM */
#define BEAT_INTERVAL(bpm)  (60000 / (bpm))

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Data Structures
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 심장 기어 (Heart Gear) */
typedef enum {
    GEAR_IDLE       = 1,    /* 아이들링 (유지 모드) */
    GEAR_CRUISE     = 2,    /* 크루징 (일반 모드) */
    GEAR_BOOST      = 3     /* 부스터 (고속 모드) */
} heart_gear_t;

/* 혈압 상태 (Blood Pressure Status) */
typedef enum {
    BP_HYPOTENSION  = 0,    /* 저혈압 (유속 부족) */
    BP_NORMAL       = 1,    /* 정상 */
    BP_PREHYPER     = 2,    /* 전 고혈압 (주의) */
    BP_HYPERTENSION = 3     /* 고혈압 (과부하) */
} blood_pressure_t;

/* 밸브 상태 (Valve State) */
typedef struct {
    int   open;             /* 1=열림, 0=닫힘 */
    int   flow_rate;        /* 유속 (0-100%) */
    char  target[32];       /* 대상 장기 */
} valve_t;

/* 심장 엔진 (Heart Engine) */
typedef struct {
    /* BPM Control */
    int                 current_bpm;        /* 현재 박동수 */
    int                 target_bpm;         /* 목표 박동수 */
    heart_gear_t        gear;               /* 현재 기어 */

    /* Blood Pressure (System Load) */
    blood_pressure_t    bp_status;          /* 혈압 상태 */
    int                 systolic;           /* 수축기 압력 (0-100) */
    int                 diastolic;          /* 이완기 압력 (0-100) */

    /* Valves (Flow Control) */
    valve_t             valve_stomach;      /* 위장 → 간 밸브 */
    valve_t             valve_brain;        /* 간 → 뇌 밸브 */
    valve_t             valve_kidney;       /* 뇌 → 콩팥 밸브 */

    /* Monitoring */
    struct stomach_t*   stomach;            /* 위장 상태 모니터링 */
    struct cortex_t*    cortex;             /* 뇌 상태 모니터링 (미구현) */

    /* Thread Control */
    pthread_t           beat_thread;        /* 박동 스레드 */
    pthread_mutex_t     lock;
    int                 running;            /* 1=작동 중 */

    /* Statistics */
    uint64_t            total_beats;        /* 총 박동 수 */
    uint64_t            gear_shifts;        /* 기어 변속 횟수 */
    uint64_t            bp_warnings;        /* 혈압 경고 횟수 */
} heart_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 심장 생성/삭제 */
heart_t* heart_create(void);
void     heart_destroy(heart_t* heart);

/* 엔진 제어 */
int  heart_start(heart_t* heart);           /* 박동 시작 */
int  heart_stop(heart_t* heart);            /* 박동 중지 */
void heart_set_bpm(heart_t* heart, int bpm);/* BPM 설정 */
void heart_set_gear(heart_t* heart, heart_gear_t gear);

/* 장기 연동 */
void heart_connect_stomach(heart_t* heart, struct stomach_t* stomach);
void heart_connect_cortex(heart_t* heart, struct cortex_t* cortex);

/* 자동 조절 (Adaptive Control) */
void heart_auto_adjust(heart_t* heart);     /* BPM 자동 조절 */
void heart_check_pressure(heart_t* heart);  /* 혈압 체크 */

/* 밸브 제어 */
void heart_open_valve(heart_t* heart, const char* target);
void heart_close_valve(heart_t* heart, const char* target);
void heart_set_flow_rate(heart_t* heart, const char* target, int rate);

/* 상태 확인 */
int  heart_get_bpm(const heart_t* heart);
heart_gear_t heart_get_gear(const heart_t* heart);
blood_pressure_t heart_get_bp_status(const heart_t* heart);
void heart_stats(const heart_t* heart);

/* 유틸리티 */
const char* gear_string(heart_gear_t gear);
const char* bp_status_string(blood_pressure_t bp);

#endif /* KIM_HEART_H */

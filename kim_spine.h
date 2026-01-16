/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_spine.h
 *
 * 척수(Spinal Cord) - High-Speed Control Bus
 *
 * 생물학적 역할:
 *   - 뇌와 신체 간의 신호 전달 (초고속)
 *   - 상행로 (Sensory): Body → Brain (입력/이벤트)
 *   - 하행로 (Motor): Brain → Body (명령/제어)
 *   - 척수 반사 (Reflex): 뇌 없이 즉시 처리
 *
 * 소프트웨어 역할:
 *   - IPC (Inter-Process Communication)
 *   - Interrupt Handler
 *   - Event Bus (Low-Latency)
 *   - Signal/Slot Pattern
 *
 * vs Blood Vessels:
 *   - Vessels: Data Transport (Heavy Payload, Queue)
 *   - Spine: Signal Transport (Light Control, Function Call)
 *
 * Priority: ★핵심★ (Nervous System)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_SPINE_H
#define KIM_SPINE_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Signal Types (신호 종류)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef enum {
    SIGNAL_NONE        = 0,     /* 없음 */

    /* Sensory Signals (감각 신호 - 상행) */
    SIGNAL_PAIN        = 1,     /* 통증 (에러) */
    SIGNAL_HEAT        = 2,     /* 열 감지 (과부하) */
    SIGNAL_PRESSURE    = 3,     /* 압력 (부하) */
    SIGNAL_TOUCH       = 4,     /* 접촉 (입력) */
    SIGNAL_VISION      = 5,     /* 시각 (데이터 도착) */
    SIGNAL_SOUND       = 6,     /* 청각 (알림) */

    /* Motor Commands (운동 명령 - 하행) */
    SIGNAL_MOVE        = 100,   /* 움직임 (실행) */
    SIGNAL_STOP        = 101,   /* 정지 (중단) */
    SIGNAL_SPEED_UP    = 102,   /* 가속 (성능 향상) */
    SIGNAL_SPEED_DOWN  = 103,   /* 감속 (성능 제한) */
    SIGNAL_SECRETE     = 104,   /* 분비 (데이터 생성) */
    SIGNAL_CONTRACT    = 105,   /* 수축 (메모리 해제) */
    SIGNAL_RELAX       = 106,   /* 이완 (대기) */

    /* Reflexes (반사 신호) */
    REFLEX_WITHDRAW    = 200,   /* 철회 반사 (긴급 차단) */
    REFLEX_COUGH       = 201,   /* 기침 반사 (에러 배출) */
    REFLEX_BLINK       = 202,   /* 눈 깜빡임 (인터럽트 ACK) */

    SIGNAL_UNKNOWN     = 999    /* 알 수 없음 */
} signal_type_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Signal Packet (신호 패킷)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    signal_type_t type;         /* 신호 타입 */
    int           source_id;    /* 발신자 (장기 ID) */
    int           target_id;    /* 수신자 (장기 ID) */
    void*         data;         /* 부가 데이터 (경량) */
    size_t        data_size;    /* 데이터 크기 */
    int64_t       timestamp;    /* 발신 시각 (us) */
    uint32_t      priority;     /* 우선순위 (0=최고) */
} signal_packet_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Reflex Handler (반사 핸들러)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef int (*reflex_handler_t)(signal_packet_t* signal);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Spine (척수)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    /* Ascending Path (상행로 - Sensory) */
    void (*sensory_callback)(signal_packet_t*);     /* 뇌로 보고 */

    /* Descending Path (하행로 - Motor) */
    void (*motor_callback)(signal_packet_t*);       /* 장기로 명령 */

    /* Reflex Arc (반사 신경) */
    reflex_handler_t reflex_handlers[256];          /* 반사 핸들러 배열 */
    int              reflex_count;                  /* 등록된 반사 수 */

    /* Statistics */
    uint64_t total_sensory;         /* 상행 신호 수 */
    uint64_t total_motor;           /* 하행 신호 수 */
    uint64_t total_reflexes;        /* 반사 처리 수 */
    uint64_t avg_latency_us;        /* 평균 지연시간 (us) */

    /* Thread Safety */
    pthread_mutex_t lock;
} spine_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 척수 생성/삭제 */
spine_t* spine_create(void);
void     spine_destroy(spine_t* spine);

/* 콜백 등록 */
void spine_set_sensory_callback(spine_t* spine, void (*callback)(signal_packet_t*));
void spine_set_motor_callback(spine_t* spine, void (*callback)(signal_packet_t*));

/* 반사 신경 등록 */
int spine_register_reflex(spine_t* spine, signal_type_t type, reflex_handler_t handler);

/* 신호 전송 */
int spine_send_sensory(spine_t* spine, signal_packet_t* signal);   /* 상행 (Body→Brain) */
int spine_send_motor(spine_t* spine, signal_packet_t* signal);     /* 하행 (Brain→Body) */

/* 반사 체크 */
int spine_check_reflex(spine_t* spine, signal_packet_t* signal);

/* 통계 */
void spine_stats(const spine_t* spine);

/* 신호 패킷 생성 헬퍼 */
signal_packet_t* signal_create(
    signal_type_t type,
    int source_id,
    int target_id,
    void* data,
    size_t data_size
);
void signal_destroy(signal_packet_t* signal);

#endif /* KIM_SPINE_H */

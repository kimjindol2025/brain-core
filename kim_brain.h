/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_brain.h
 *
 * Brain (뇌) - Master Orchestrator & Central Intelligence
 *
 * 생물학적 역할:
 *   - 모든 신경계 통합 및 조율
 *   - 기관 간 신호 전달 및 의사결정
 *   - 생명 주기 관리 (탄생→동작→수면→종료)
 *   - 통일된 지능형 인터페이스 제공
 *
 * 소프트웨어 역할:
 *   - 13개 기관 통합 관리 및 초기화
 *   - 데이터 파이프라인: Input → Process → Remember → Output
 *   - 24/7 이벤트 루프 (Heart-driven, Circadian-controlled)
 *   - 통합 API: brain_think(), brain_remember(), brain_recall(), brain_dream()
 *
 * 통합 기관 (13개):
 *   1. Spine - IPC 제어 버스
 *   2. Heart - 시스템 클록 및 흐름 제어
 *   3. Circadian - 24시간 리듬 및 자원 할당
 *   4. Watchdog - 자가 치유 및 모니터링
 *   5. Health - 시스템 건강 상태 점검
 *   6. Stomach - 입력 버퍼링
 *   7. Pancreas - 입력 파싱
 *   8. Cortex - 사고 및 의사결정
 *   9. Thalamus - 이벤트 라우팅
 *   10. Liver - 메모리 관리 및 GC
 *   11. Lungs - 비동기 I/O 관리
 *   12. Hippocampus - 장기 기억 저장소
 *   13. Math - 수학 연산 가속기
 *
 * Priority: ★1★ (Critical - 시스템 완성의 최종 단계)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_BRAIN_H
#define KIM_BRAIN_H

#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <time.h>

/* Include all organs */
#include "kim_spine.h"
#include "kim_heart.h"
#include "kim_cortex.h"             /* Define cortex_t before hippocampus uses it */
#include "kim_stomach.h"
#include "kim_pancreas.h"
#include "kim_liver.h"
#include "kim_lungs.h"
#include "kim_thalamus.h"
#include "kim_hippocampus.h"        /* Uses cortex_t from cortex.h */
#include "kim_circadian.h"
#include "kim_watchdog.h"
#include "kim_health.h"
#include "kim_math.h"

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Configuration
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define BRAIN_MAX_INPUT_SIZE    4096   /* 최대 입력 크기 (바이트) */
#define BRAIN_MAX_OUTPUT_SIZE   8192   /* 최대 출력 크기 (바이트) */
#define BRAIN_TICK_INTERVAL_MS  100    /* 기본 tick 간격 (100ms) */
#define BRAIN_NUM_ORGANS        13     /* 총 기관 수 */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Brain State Machine
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Brain 상태 (생명 주기) */
typedef enum {
    BRAIN_STATE_BIRTH = 0,      /* 👶 탄생: 기관 초기화 중 */
    BRAIN_STATE_AWAKE,          /* 👁️  각성: 활동 중 (DAY/DAWN/DUSK) */
    BRAIN_STATE_DREAMING,       /* 😴 꿈: 수면/정리 중 (NIGHT) */
    BRAIN_STATE_SHUTDOWN,       /* 🔴 종료: 정상 종료 진행 중 */
    BRAIN_STATE_DEAD            /* ⚫ 사망: 완전히 종료됨 */
} brain_state_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Brain Organ Collection
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    /* Core Systems (1-5): 핵심 시스템 */
    spine_t*              spine;           /* [1] 척추: IPC 제어 버스 */
    heart_t*              heart;           /* [2] 심장: 시스템 클록 */
    circadian_t*          circadian;       /* [3] 일주기: 24시간 리듬 */
    watchdog_t*           watchdog;        /* [4] 감시자: 자가 치유 */
    health_monitor_t*     health;          /* [5] 건강: 상태 점검 */

    /* Processing Organs (6-9): 처리 기관 */
    stomach_t*            stomach;         /* [6] 위: 입력 버퍼 */
    pancreas_t*           pancreas;        /* [7] 췌장: 파서 */
    cortex_t*             cortex;          /* [8] 대뇌피질: 사고 */
    thalamus_t*           thalamus;        /* [9] 시상: 이벤트 라우터 */

    /* Memory & I/O (10-12): 메모리 및 I/O */
    liver_t*              liver;           /* [10] 간: 메모리 관리 */
    lungs_t*              lungs;           /* [11] 폐: 비동기 I/O */
    hippocampus_t*        hippocampus;     /* [12] 해마: 장기 기억 */

    /* Utilities (13): 유틸리티 */
    math_unit_t*          math;            /* [13] 수학: 연산 가속기 */
} brain_organs_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Brain Main Structure
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    /* Organs Collection */
    brain_organs_t   organs;          /* 모든 13개 기관 */

    /* State Management */
    brain_state_t    state;           /* 현재 Brain 상태 */
    int              running;         /* 실행 중 플래그 (1=실행, 0=종료) */
    pthread_t        main_thread;     /* 메인 루프 스레드 ID */

    /* Thread Safety */
    pthread_mutex_t  lock;            /* 뮤텍스 (기관 접근 보호) */
    pthread_cond_t   cond;            /* 조건 변수 (신호 대기) */

    /* Statistics & Monitoring */
    uint64_t total_thoughts;          /* 총 사고(think) 횟수 */
    uint64_t total_memories;          /* 총 기억(remember) 횟수 */
    uint64_t total_recalls;           /* 총 회상(recall) 횟수 */
    uint64_t total_dreams;            /* 총 꿈(dream) 횟수 */
    uint64_t total_ticks;             /* 총 tick 수 (Heart beats) */
    uint64_t birth_time;              /* 탄생 시각 (마이크로초) */

    /* Performance Metrics */
    uint64_t avg_think_time_us;       /* 평균 think 처리 시간 */
    uint64_t avg_pipeline_latency_us; /* 평균 파이프라인 지연시간 */
    uint32_t thoughts_per_minute;     /* 분당 사고 횟수 */
} brain_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Core API - Lifecycle Management
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_create() - 뇌 생성 및 모든 기관 초기화
 *
 * 실행 순서:
 *   1. Spine 생성 (IPC 버스)
 *   2. Heart 생성 (시스템 클록)
 *   3. Circadian 생성 (24시간 리듬)
 *   4. 기타 기관 생성
 *   5. Spine에 모든 기관 등록
 *   6. 기관 간 연결 (Cortex ↔ Hippocampus 등)
 *
 * @return brain_t* - 생성된 뇌 포인터 (실패시 NULL)
 */
brain_t* brain_create(void);

/**
 * brain_destroy() - 뇌 종료 및 모든 자원 정리
 *
 * @param brain - brain_create()로 생성한 뇌 포인터
 */
void brain_destroy(brain_t* brain);

/**
 * brain_start() - 뇌 실행 (24/7 이벤트 루프 시작)
 *
 * 실행 내용:
 *   - Heart-driven 메인 루프 스레드 시작
 *   - Circadian 위상 기반 동작 모드 전환
 *   - 통계 수집 시작
 *
 * @param brain - 뇌 포인터
 * @return 0 (성공), -1 (실패)
 */
int brain_start(brain_t* brain);

/**
 * brain_stop() - 뇌 정지 (우아한 종료)
 *
 * @param brain - 뇌 포인터
 * @return 0 (성공), -1 (실패)
 */
int brain_stop(brain_t* brain);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * High-Level Intelligence API
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_think() - 입력을 처리하고 지능형 응답 생성
 *
 * 데이터 파이프라인:
 *   Input (string)
 *     ↓ [Stomach]    - 입력 버퍼에 저장
 *     ↓ [Pancreas]   - 파싱 및 토큰화
 *     ↓ [Cortex]     - 임베딩 및 패턴 인식
 *     ↓ [Hippocampus] - 과거 유사 기억 검색 (top-5)
 *     ↓ [Cortex]     - 기억 기반 최종 의사결정
 *     ↓ [Hippocampus] - 중요한 사고 저장
 *   Output (string)
 *
 * @param brain - 뇌 포인터
 * @param input - 입력 문자열 (최대 BRAIN_MAX_INPUT_SIZE 바이트)
 * @param output - 출력 버퍼 (BRAIN_MAX_OUTPUT_SIZE 이상)
 * @param output_size - 출력 버퍼 크기
 * @return 0 (성공), -1 (실패)
 */
int brain_think(brain_t* brain, const char* input,
                char* output, size_t output_size);

/**
 * brain_remember() - 중요한 내용을 장기 기억에 저장
 *
 * @param brain - 뇌 포인터
 * @param content - 기억할 내용
 * @param importance - 중요도 (0.0~1.0, 기본 임계값 0.7)
 * @return 0 (성공), -1 (실패), 1 (저장됨)
 */
int brain_remember(brain_t* brain, const char* content, float importance);

/**
 * brain_recall() - 유사한 기억 검색
 *
 * @param brain - 뇌 포인터
 * @param query - 검색 쿼리
 * @param top_k - 상위 K개 기억 반환
 * @return 기억 배열 (호출자가 free 필요), NULL (실패)
 */
char** brain_recall(brain_t* brain, const char* query, int top_k);

/**
 * brain_dream() - 수면/정리/최적화 사이클
 *
 * 실행 내용:
 *   - Hippocampus consolidation (메모리 통합)
 *   - Liver GC (가비지 컬렉션)
 *   - Cortex pattern optimization
 *   - 통계 리셋
 *
 * 일반적으로 Circadian EVENING/NIGHT phase에서 자동 호출됨
 *
 * @param brain - 뇌 포인터
 */
void brain_dream(brain_t* brain);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Monitoring & Status
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_stats() - 통합 뇌 통계 출력
 *
 * 출력 내용:
 *   - 현재 상태 (State)
 *   - 13개 기관 상태
 *   - 통계 (총 사고/기억/회상/꿈 횟수)
 *   - 성능 지표 (처리 시간, 처리량)
 *   - 가동 시간 (Uptime)
 *
 * @param brain - 뇌 포인터
 */
void brain_stats(const brain_t* brain);

/**
 * brain_get_state() - 현재 뇌 상태 조회
 *
 * @param brain - 뇌 포인터
 * @return brain_state_t - 현재 상태
 */
brain_state_t brain_get_state(const brain_t* brain);

/**
 * brain_get_uptime() - 가동 시간 조회 (초 단위)
 *
 * @param brain - 뇌 포인터
 * @return uint64_t - 가동 시간 (초)
 */
uint64_t brain_get_uptime(const brain_t* brain);

/**
 * brain_get_thought_count() - 총 사고 횟수 조회
 *
 * @param brain - 뇌 포인터
 * @return uint64_t - 총 사고 횟수
 */
uint64_t brain_get_thought_count(const brain_t* brain);

/**
 * brain_get_memory_count() - 장기 기억 개수 조회
 *
 * @param brain - 뇌 포인터
 * @return uint32_t - 저장된 기억 개수
 */
uint32_t brain_get_memory_count(const brain_t* brain);

/**
 * brain_is_healthy() - 전체 시스템 건강 상태 확인
 *
 * 확인 항목:
 *   - 모든 기관이 정상 상태인가?
 *   - Watchdog 경고는 없는가?
 *   - Health check 통과했는가?
 *
 * @param brain - 뇌 포인터
 * @return 1 (건강), 0 (문제 있음)
 */
int brain_is_healthy(const brain_t* brain);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Utility Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_state_string() - 뇌 상태를 문자열로 변환
 *
 * @param state - 뇌 상태
 * @return const char* - 상태 문자열
 */
const char* brain_state_string(brain_state_t state);

/**
 * brain_diagnose() - 뇌 진단 리포트
 *
 * 출력 내용:
 *   - 문제 있는 기관 목록
 *   - 경고 메시지
 *   - 추천 조치사항
 *
 * @param brain - 뇌 포인터
 */
void brain_diagnose(const brain_t* brain);

#endif /* KIM_BRAIN_H */

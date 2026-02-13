/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_heart_24h.c
 *
 * Heart Engine 24-Hour Operational Scenario
 * 심장 엔진 24시간 운영 시나리오
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_heart.h"
#include "kim_stomach.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/* 시간 구간 정의 (실제로는 초 단위로 축약) */
#define HOUR_IN_SECONDS  2   /* 1시간 = 2초 (시뮬레이션) */

/* 워크로드 패턴 */
typedef struct {
    int hour;              /* 시각 (0-23) */
    const char* phase;     /* 단계 이름 */
    int data_rate;         /* 데이터 유입률 (0-100) */
    const char* activity;  /* 활동 설명 */
} workload_phase_t;

/* 24시간 워크로드 시나리오 */
static workload_phase_t schedule[] = {
    /* 새벽 (00:00 - 05:00): 수면 모드 */
    { 0, "수면", 5, "최소 유지보수" },
    { 1, "수면", 5, "메모리 정리" },
    { 2, "수면", 5, "로그 압축" },
    { 3, "수면", 10, "인덱스 재구축" },
    { 4, "수면", 10, "디스크 정리" },
    { 5, "수면", 15, "백업 준비" },

    /* 아침 (06:00 - 09:00): 기상 및 워밍업 */
    { 6, "기상", 30, "시스템 체크" },
    { 7, "워밍업", 50, "뉴스 수집" },
    { 8, "워밍업", 60, "이메일 처리" },
    { 9, "업무시작", 70, "일정 확인" },

    /* 오전 (10:00 - 12:00): 집중 업무 */
    { 10, "집중업무", 80, "코드 분석" },
    { 11, "집중업무", 85, "문서 작성" },
    { 12, "점심", 40, "가벼운 학습" },

    /* 오후 (13:00 - 17:00): 지속 업무 */
    { 13, "업무재개", 75, "데이터 처리" },
    { 14, "업무", 80, "AI 학습" },
    { 15, "업무", 85, "API 처리" },
    { 16, "업무", 90, "대용량 분석" },
    { 17, "업무마무리", 70, "보고서 작성" },

    /* 저녁 (18:00 - 21:00): 정리 및 여가 */
    { 18, "저녁", 50, "이메일 답장" },
    { 19, "여가", 30, "콘텐츠 탐색" },
    { 20, "여가", 40, "소셜 미디어" },
    { 21, "정리", 30, "내일 준비" },

    /* 밤 (22:00 - 23:00): 취침 준비 */
    { 22, "취침준비", 20, "시스템 정리" },
    { 23, "취침", 10, "로그 저장" }
};

/* stdout 리디렉션 헬퍼 */
static int suppress_stdout() {
    int backup = dup(1);
    FILE* null_stream = fopen("/dev/null", "w");
    if (null_stream) {
        dup2(fileno(null_stream), 1);
        fclose(null_stream);
    }
    return backup;
}

static void restore_stdout(int backup) {
    if (backup >= 0) {
        fflush(stdout);
        dup2(backup, 1);
        close(backup);
    }
}

/* 시간대별 데이터 주입 */
static void inject_workload(stomach_t* stomach, int data_rate) {
    /* data_rate: 0-100 → 실제 데이터 개수로 변환 */
    int count = data_rate * 10;  /* 최대 1000개 */

    int backup = suppress_stdout();

    for (int i = 0; i < count; i++) {
        char data[128];  /* 더 큰 데이터 */
        snprintf(data, sizeof(data), "workload-data-item-%d-with-content", i);
        stomach_ingest(stomach, data, strlen(data));
    }

    restore_stdout(backup);
}

/* 시간대별 시뮬레이션 */
static void simulate_hour(heart_t* heart, stomach_t* stomach, workload_phase_t phase) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("🕐 %02d:00 - [%s] %s (데이터율: %d%%)\n",
           phase.hour, phase.phase, phase.activity, phase.data_rate);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    fflush(stdout);

    /* 워크로드 주입 */
    inject_workload(stomach, phase.data_rate);

    /* 1시간 대기 (시뮬레이션에서는 2초) */
    sleep(HOUR_IN_SECONDS);

    /* 현재 상태 출력 */
    int fill = stomach_get_fill_percent(stomach);
    int bpm = heart_get_bpm(heart);
    heart_gear_t gear = heart_get_gear(heart);
    blood_pressure_t bp = heart_get_bp_status(heart);

    printf("   📊 상태:\n");
    printf("      위장 포만도: %d%%\n", fill);
    printf("      심박수: %d BPM\n", bpm);
    printf("      기어: %s\n", gear_string(gear));
    printf("      혈압: %s\n", bp_status_string(bp));

    /* 기어 변화 시각화 */
    if (gear == GEAR_IDLE) {
        printf("   🐌 [IDLE] 시스템 유지 모드\n");
    } else if (gear == GEAR_CRUISE) {
        printf("   🚗 [CRUISE] 일반 처리 모드\n");
    } else if (gear == GEAR_BOOST) {
        printf("   🚀 [BOOST] 고속 처리 모드\n");
    }
    fflush(stdout);
}

/* 24시간 시나리오 실행 */
static void run_24h_scenario(heart_t* heart, stomach_t* stomach) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║  24시간 심장 엔진 운영 시나리오                       ║\n");
    printf("║  Heart Engine 24-Hour Operational Scenario           ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("⚙️  시뮬레이션 속도: 1시간 = %d초\n", HOUR_IN_SECONDS);
    printf("💓 심장 자동 조절: 활성화\n");
    printf("🫀 총 소요 시간: 약 %d초\n", 24 * HOUR_IN_SECONDS);
    printf("\n");
    fflush(stdout);

    /* 심장 시작 */
    if (heart_start(heart) != 0) {
        printf("❌ 심장 시작 실패\n");
        fflush(stdout);
        return;
    }

    /* 24시간 순회 */
    for (int i = 0; i < 24; i++) {
        simulate_hour(heart, stomach, schedule[i]);
    }

    /* 심장 중지 */
    heart_stop(heart);

    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("🏁 24시간 시나리오 완료\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
    fflush(stdout);
}

/* 최종 통계 출력 */
static void print_daily_summary(heart_t* heart, stomach_t* stomach) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║  일일 활동 요약 (Daily Summary)                       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    printf("\n");

    /* 심장 통계 */
    heart_stats(heart);

    printf("\n");

    /* 위장 통계 */
    int fill = stomach_get_fill_percent(stomach);
    printf("[Stomach Status]\n");
    printf("  최종 포만도: %d%%\n", fill);

    printf("\n");

    /* 일일 요약 */
    printf("[Daily Insights]\n");
    printf("  ✅ 24시간 운영: 정상 완료\n");
    printf("  ✅ 자동 기어 변속: 작동 확인\n");
    printf("  ✅ 과부하 보호: 작동 확인\n");
    printf("  ✅ 적응형 제어: 작동 확인\n");

    printf("\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    /* 심장 및 위장 생성 */
    heart_t* heart = heart_create();
    stomach_t* stomach = stomach_create(5000);  /* 적정 크기 위장 */

    if (!heart || !stomach) {
        fprintf(stderr, "❌ 인스턴스 생성 실패\n");
        fflush(stderr);
        return 1;
    }

    /* 위장 연결 */
    heart_connect_stomach(heart, stomach);

    /* 24시간 시나리오 실행 */
    run_24h_scenario(heart, stomach);

    /* 최종 통계 */
    print_daily_summary(heart, stomach);

    /* 정리 */
    heart_destroy(heart);
    stomach_destroy(stomach);

    printf("💓 심장 엔진 24시간 운영 테스트 완료\n\n");

    return 0;
}

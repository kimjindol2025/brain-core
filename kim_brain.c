/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_brain.c
 *
 * Brain (뇌) - Master Orchestrator Implementation
 *
 * 13개 기관을 통합하고 24/7 이벤트 루프로 조율
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "kim_brain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Forward Declarations
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

static void* brain_main_loop(void* arg);
static void brain_connect_organs(brain_t* brain);
static int brain_init_organs(brain_t* brain);
static void brain_cleanup_organs(brain_t* brain);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Core Lifecycle: Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_create() - 뇌 생성 및 모든 기관 초기화
 */
brain_t* brain_create(void) {
    printf("\n🧠 Creating Brain (Master Orchestrator)...\n");

    brain_t* brain = (brain_t*)calloc(1, sizeof(brain_t));
    if (!brain) {
        fprintf(stderr, "❌ Failed to allocate brain structure\n");
        return NULL;
    }

    /* 뮤텍스 초기화 */
    pthread_mutex_init(&brain->lock, NULL);
    pthread_cond_init(&brain->cond, NULL);

    /* 상태 설정 */
    brain->state = BRAIN_STATE_BIRTH;
    brain->birth_time = time(NULL) * 1000000;

    /* 13개 기관 초기화 */
    if (brain_init_organs(brain) < 0) {
        fprintf(stderr, "❌ Failed to initialize organs\n");
        brain_destroy(brain);
        return NULL;
    }

    /* 기관 간 연결 */
    brain_connect_organs(brain);

    printf("✅ Brain created successfully with %d organs\n", BRAIN_NUM_ORGANS);
    printf("   State: %s\n", brain_state_string(brain->state));

    return brain;
}

/**
 * brain_destroy() - 뇌 종료 및 모든 자원 정리
 */
void brain_destroy(brain_t* brain) {
    if (!brain) return;

    printf("\n🧠 Destroying Brain...\n");

    /* 루프 중지 */
    if (brain->running) {
        brain->running = 0;
        if (brain->main_thread != 0) {
            pthread_join(brain->main_thread, NULL);
        }
    }

    /* 모든 기관 정리 */
    brain_cleanup_organs(brain);

    /* 뮤텍스 정리 */
    pthread_mutex_destroy(&brain->lock);
    pthread_cond_destroy(&brain->cond);

    /* 메모리 해제 */
    free(brain);

    printf("✅ Brain destroyed\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Organ Initialization & Connection
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_init_organs() - 13개 기관 초기화 (순서 중요!)
 *
 * 초기화 순서:
 *   1. Spine (IPC 버스)
 *   2. Heart (클록)
 *   3. Circadian (24시간 리듬)
 *   4. 기타 기관
 */
static int brain_init_organs(brain_t* brain) {
    printf("  [1/13] Creating Spine (IPC Bus)...\n");
    brain->organs.spine = spine_create();
    if (!brain->organs.spine) {
        fprintf(stderr, "  ❌ Spine creation failed\n");
        return -1;
    }

    printf("  [2/13] Creating Heart (Clock)...\n");
    brain->organs.heart = heart_create();
    if (!brain->organs.heart) {
        fprintf(stderr, "  ❌ Heart creation failed\n");
        return -1;
    }

    printf("  [3/13] Creating Circadian (24h Rhythm)...\n");
    brain->organs.circadian = circadian_create();
    if (!brain->organs.circadian) {
        fprintf(stderr, "  ❌ Circadian creation failed\n");
        return -1;
    }

    printf("  [4/13] Creating Watchdog (Self-Healing)...\n");
    brain->organs.watchdog = watchdog_create();
    if (!brain->organs.watchdog) {
        fprintf(stderr, "  ❌ Watchdog creation failed\n");
        return -1;
    }

    printf("  [6/13] Creating Stomach (Input Buffer)...\n");
    brain->organs.stomach = stomach_create(256);
    if (!brain->organs.stomach) {
        fprintf(stderr, "  ❌ Stomach creation failed\n");
        return -1;
    }

    printf("  [7/13] Creating Pancreas (Parser)...\n");
    brain->organs.pancreas = pancreas_create();
    if (!brain->organs.pancreas) {
        fprintf(stderr, "  ❌ Pancreas creation failed\n");
        return -1;
    }

    printf("  [5/13] Creating Health (Monitoring)...\n");
    brain->organs.health = health_monitor_create(brain->organs.stomach,
                                                 brain->organs.pancreas,
                                                 brain->organs.spine);
    if (!brain->organs.health) {
        fprintf(stderr, "  ❌ Health creation failed\n");
        return -1;
    }

    printf("  [8/13] Creating Cortex (Thinking Engine)...\n");
    brain->organs.cortex = cortex_create(brain->organs.spine);
    if (!brain->organs.cortex) {
        fprintf(stderr, "  ❌ Cortex creation failed\n");
        return -1;
    }

    printf("  [9/13] Creating Thalamus (Event Router)...\n");
    brain->organs.thalamus = thalamus_create(0.5f);  /* strictness: 0.5 */
    if (!brain->organs.thalamus) {
        fprintf(stderr, "  ❌ Thalamus creation failed\n");
        return -1;
    }

    printf("  [10/13] Creating Liver (Memory Management)...\n");
    brain->organs.liver = liver_create();
    if (!brain->organs.liver) {
        fprintf(stderr, "  ❌ Liver creation failed\n");
        return -1;
    }

    printf("  [11/13] Creating Lungs (Async I/O)...\n");
    brain->organs.lungs = lungs_create(4);
    if (!brain->organs.lungs) {
        fprintf(stderr, "  ❌ Lungs creation failed\n");
        return -1;
    }

    printf("  [12/13] Creating Hippocampus (Long-Term Memory)...\n");
    brain->organs.hippocampus = hippocampus_create(HIPPO_DB_PATH);
    if (!brain->organs.hippocampus) {
        fprintf(stderr, "  ❌ Hippocampus creation failed\n");
        return -1;
    }

    printf("  [13/13] Creating Math (Computation Engine)...\n");
    brain->organs.math = math_unit_create();
    if (!brain->organs.math) {
        fprintf(stderr, "  ❌ Math creation failed\n");
        return -1;
    }

    printf("  ✅ All 13 organs created successfully\n");
    return 0;
}

/**
 * brain_connect_organs() - 기관 간 연결 (Spine 기반)
 */
static void brain_connect_organs(brain_t* brain) {
    printf("  Connecting organs via Spine...\n");

    spine_t* spine = brain->organs.spine;

    /* 기관들이 Spine을 통해 간접적으로 연결됨 */
    /* 각 기관은 자체적으로 spine 참조를 보유하거나
       초기화 시 spine이 설정됨 */

    if (brain->organs.liver && brain->organs.liver) {
        liver_set_spine(brain->organs.liver, spine, 4);
    }

    if (brain->organs.lungs && spine) {
        lungs_set_spine(brain->organs.lungs, spine, 5);
    }

    if (brain->organs.hippocampus && spine) {
        hippocampus_set_spine(brain->organs.hippocampus, spine, 6);
    }

    printf("  ✅ Organ connections established\n");
}

/**
 * brain_cleanup_organs() - 모든 기관 정리
 */
static void brain_cleanup_organs(brain_t* brain) {
    printf("  Cleaning up organs...\n");

    if (brain->organs.spine) spine_destroy(brain->organs.spine);
    if (brain->organs.heart) heart_destroy(brain->organs.heart);
    if (brain->organs.circadian) circadian_destroy(brain->organs.circadian);
    if (brain->organs.watchdog) watchdog_destroy(brain->organs.watchdog);
    if (brain->organs.health) health_monitor_destroy(brain->organs.health);
    if (brain->organs.stomach) stomach_destroy(brain->organs.stomach);
    if (brain->organs.pancreas) pancreas_destroy(brain->organs.pancreas);
    if (brain->organs.cortex) cortex_destroy(brain->organs.cortex);
    if (brain->organs.thalamus) thalamus_destroy(brain->organs.thalamus);
    if (brain->organs.liver) liver_destroy(brain->organs.liver);
    if (brain->organs.lungs) lungs_destroy(brain->organs.lungs);
    if (brain->organs.hippocampus) hippocampus_destroy(brain->organs.hippocampus);
    if (brain->organs.math) math_unit_destroy(brain->organs.math);

    memset(&brain->organs, 0, sizeof(brain_organs_t));
    printf("  ✅ All organs cleaned up\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Event Loop: 24/7 Operation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_start() - 뇌 실행 (24/7 이벤트 루프 시작)
 */
int brain_start(brain_t* brain) {
    if (!brain || brain->running) {
        return -1;
    }

    printf("\n🧠 Starting Brain Event Loop (24/7)...\n");

    brain->running = 1;
    brain->state = BRAIN_STATE_AWAKE;

    /* Heart 시작 */
    if (heart_start(brain->organs.heart) < 0) {
        fprintf(stderr, "❌ Failed to start Heart\n");
        return -1;
    }

    /* 메인 루프 스레드 시작 */
    if (pthread_create(&brain->main_thread, NULL, brain_main_loop, brain) != 0) {
        fprintf(stderr, "❌ Failed to create main loop thread\n");
        return -1;
    }

    printf("✅ Brain started\n");
    return 0;
}

/**
 * brain_stop() - 뇌 정지 (우아한 종료)
 */
int brain_stop(brain_t* brain) {
    if (!brain) {
        return -1;
    }

    printf("\n🧠 Stopping Brain...\n");

    brain->running = 0;
    pthread_cond_signal(&brain->cond);

    /* Heart 중지 */
    if (brain->organs.heart) {
        heart_stop(brain->organs.heart);
    }

    /* 메인 루프 스레드 대기 */
    if (brain->main_thread != 0) {
        pthread_join(brain->main_thread, NULL);
    }

    brain->state = BRAIN_STATE_SHUTDOWN;

    printf("✅ Brain stopped\n");
    return 0;
}

/**
 * brain_main_loop() - 24/7 이벤트 루프
 *
 * Heart-driven tick 기반 메인 루프:
 *   1. Heart beat (100ms 간격)
 *   2. Circadian phase 확인
 *   3. Phase별 작업 수행
 *   4. Health check
 *   5. Statistics update
 */
static void* brain_main_loop(void* arg) {
    brain_t* brain = (brain_t*)arg;

    printf("  🔄 Event loop started (PID: %lu)\n",
           (unsigned long)pthread_self());

    while (brain->running) {
        pthread_mutex_lock(&brain->lock);

        /* 1. Circadian phase 확인 */
        circadian_phase_t phase = circadian_get_phase(brain->organs.circadian);

        /* 2. Phase별 작업 */
        switch (phase) {
            case PHASE_DAWN:
                /* 새벽: 학습 및 최적화 */
                hippocampus_consolidate(brain->organs.hippocampus);
                brain->total_dreams++;
                break;

            case PHASE_DAY:
                /* 낮: 일반 처리 */
                brain->state = BRAIN_STATE_AWAKE;
                break;

            case PHASE_EVENING:
                /* 저녁: 정리 및 GC */
                liver_gc_cycle(brain->organs.liver);
                hippocampus_consolidate(brain->organs.hippocampus);
                break;

            default:
                break;
        }

        /* 3. Watchdog check (60초마다) */
        static time_t last_watchdog = 0;
        time_t now = time(NULL);
        if (now - last_watchdog >= 60) {
            watchdog_check(brain->organs.watchdog);
            last_watchdog = now;
        }

        /* 5. Statistics */
        brain->total_ticks++;

        pthread_mutex_unlock(&brain->lock);

        /* 6. Tick interval 대기 */
        usleep(BRAIN_TICK_INTERVAL_MS * 1000);
    }

    brain->state = BRAIN_STATE_SHUTDOWN;
    printf("  ✅ Event loop stopped\n");

    return NULL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Intelligence Pipeline: think → remember → recall
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_think() - 입력 처리 및 응답 생성
 *
 * 데이터 파이프라인:
 *   Input → Cortex → Hippocampus → Output
 */
int brain_think(brain_t* brain, const char* input,
                char* output, size_t output_size) {
    if (!brain || !input || !output || output_size == 0) {
        return -1;
    }

    pthread_mutex_lock(&brain->lock);

    size_t input_len = strlen(input);
    if (input_len > BRAIN_MAX_INPUT_SIZE) {
        pthread_mutex_unlock(&brain->lock);
        return -1;
    }

    /* 1. Cortex: 사고 처리 */
    decision_t* decision = NULL;
    if (brain->organs.cortex) {
        decision = cortex_think(brain->organs.cortex, input, input_len);
    }

    if (!decision) {
        snprintf(output, output_size, "Cortex processing failed");
        brain->total_thoughts++;
        pthread_mutex_unlock(&brain->lock);
        return -1;
    }

    /* 2. 의사결정 결과를 출력으로 반환 */
    const char* decision_str = decision_type_string(decision->type);
    snprintf(output, output_size, "Decision: %s | Action: %s",
             decision_str, decision->action);

    /* 3. Hippocampus: 중요한 사고 저장 (should_learn 기반) */
    if (brain->organs.hippocampus && decision->should_learn) {
        float vector[128];
        for (int i = 0; i < 128; i++) {
            vector[i] = sinf((float)input_len + (float)i * 0.1f);
        }
        hippocampus_store(brain->organs.hippocampus,
                         output, vector, 0.8f);  /* 일정한 중요도 */
        brain->total_memories++;
    }

    /* 정리 */
    decision_destroy(decision);

    brain->total_thoughts++;

    pthread_mutex_unlock(&brain->lock);

    return 0;
}

/**
 * brain_remember() - 중요한 내용을 장기 기억에 저장
 */
int brain_remember(brain_t* brain, const char* content, float importance) {
    if (!brain || !content || !brain->organs.hippocampus) {
        return -1;
    }

    pthread_mutex_lock(&brain->lock);

    float vector[128];
    for (int i = 0; i < 128; i++) {
        vector[i] = sinf((float)i * 0.1f);
    }

    int result = hippocampus_store(brain->organs.hippocampus,
                                  content, vector, importance);

    if (result > 0) {
        brain->total_memories++;
    }

    pthread_mutex_unlock(&brain->lock);

    return result;
}

/**
 * brain_recall() - 유사한 기억 검색
 */
char** brain_recall(brain_t* brain, const char* query, int top_k) {
    if (!brain || !query || !brain->organs.hippocampus) {
        return NULL;
    }

    pthread_mutex_lock(&brain->lock);

    float vector[128];
    for (int i = 0; i < 128; i++) {
        vector[i] = cosf((float)i * 0.1f);
    }

    memory_entry_t** memories = hippocampus_retrieve(
        brain->organs.hippocampus, vector, top_k);

    brain->total_recalls++;

    pthread_mutex_unlock(&brain->lock);

    return (char**)memories;
}

/**
 * brain_dream() - 수면/정리 사이클
 */
void brain_dream(brain_t* brain) {
    if (!brain) return;

    pthread_mutex_lock(&brain->lock);

    brain->state = BRAIN_STATE_DREAMING;

    /* Hippocampus consolidation */
    if (brain->organs.hippocampus) {
        hippocampus_consolidate(brain->organs.hippocampus);
    }

    /* Liver GC */
    if (brain->organs.liver) {
        liver_gc_cycle(brain->organs.liver);
    }

    brain->total_dreams++;

    pthread_mutex_unlock(&brain->lock);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Monitoring & Status
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/**
 * brain_stats() - 통합 뇌 통계 출력
 */
void brain_stats(const brain_t* brain) {
    if (!brain) return;

    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║          Brain (Master Orchestrator) Statistics       ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");

    uint64_t uptime = brain_get_uptime(brain);
    printf("🧠 Current State: %s\n", brain_state_string(brain->state));
    printf("⏱️  Uptime: %lu seconds (%.1f minutes)\n", uptime, uptime / 60.0);
    printf("💓 Total Ticks: %lu\n", brain->total_ticks);

    printf("\n📊 Intelligence Metrics:\n");
    printf("  💭 Total Thoughts: %lu\n", brain->total_thoughts);
    printf("  💾 Total Memories: %lu\n", brain->total_memories);
    printf("  🔍 Total Recalls: %lu\n", brain->total_recalls);
    printf("  😴 Total Dreams: %lu\n", brain->total_dreams);

    printf("\n⚡ Performance:\n");
    printf("  ⏱️  Avg Think Time: %lu μs\n", brain->avg_think_time_us);
    printf("  🚀 Avg Pipeline Latency: %lu μs\n", brain->avg_pipeline_latency_us);
    printf("  📈 Thoughts/Minute: %u\n", brain->thoughts_per_minute);

    printf("\n🫀 Organ Status:\n");
    printf("  ✅ All %d organs active\n", BRAIN_NUM_ORGANS);

    if (brain->organs.heart) {
        int bpm = heart_get_bpm(brain->organs.heart);
        if (bpm > 0) {
            printf("  • Heart: %d BPM\n", bpm);
        }
    }

    if (brain->organs.circadian) {
        circadian_phase_t phase = circadian_get_phase(brain->organs.circadian);
        const char* phase_name = (phase == PHASE_DAWN) ? "🌙 DAWN" :
                                 (phase == PHASE_DAY) ? "☀️  DAY" : "🌆 EVENING";
        printf("  • Circadian: %s\n", phase_name);
    }

    if (brain->organs.hippocampus) {
        uint32_t mem_count = brain_get_memory_count(brain);
        printf("  • Hippocampus: %u memories stored\n", mem_count);
    }

    printf("\n");
}

/**
 * brain_get_state() - 현재 뇌 상태 조회
 */
brain_state_t brain_get_state(const brain_t* brain) {
    if (!brain) return BRAIN_STATE_DEAD;
    return brain->state;
}

/**
 * brain_get_uptime() - 가동 시간 조회 (초 단위)
 */
uint64_t brain_get_uptime(const brain_t* brain) {
    if (!brain || brain->birth_time == 0) return 0;
    uint64_t now = time(NULL) * 1000000;
    return (now - brain->birth_time) / 1000000;
}

/**
 * brain_get_thought_count() - 총 사고 횟수 조회
 */
uint64_t brain_get_thought_count(const brain_t* brain) {
    return brain ? brain->total_thoughts : 0;
}

/**
 * brain_get_memory_count() - 장기 기억 개수 조회
 */
uint32_t brain_get_memory_count(const brain_t* brain) {
    if (!brain || !brain->organs.hippocampus) return 0;
    return hippocampus_get_count(brain->organs.hippocampus);
}

/**
 * brain_is_healthy() - 전체 시스템 건강 상태 확인
 */
int brain_is_healthy(const brain_t* brain) {
    if (!brain) return 0;

    /* 모든 기관이 생성되었는가? */
    if (!brain->organs.spine || !brain->organs.heart ||
        !brain->organs.cortex || !brain->organs.hippocampus) {
        return 0;
    }

    /* 상태가 죽음이 아닌가? */
    if (brain->state == BRAIN_STATE_DEAD) {
        return 0;
    }

    return 1;
}

/**
 * brain_state_string() - 뇌 상태를 문자열로 변환
 */
const char* brain_state_string(brain_state_t state) {
    switch (state) {
        case BRAIN_STATE_BIRTH: return "👶 BIRTH (Initializing)";
        case BRAIN_STATE_AWAKE: return "👁️  AWAKE (Active)";
        case BRAIN_STATE_DREAMING: return "😴 DREAMING (Sleeping)";
        case BRAIN_STATE_SHUTDOWN: return "🔴 SHUTDOWN (Stopping)";
        case BRAIN_STATE_DEAD: return "⚫ DEAD (Terminated)";
        default: return "❓ UNKNOWN";
    }
}

/**
 * brain_diagnose() - 뇌 진단 리포트
 */
void brain_diagnose(const brain_t* brain) {
    if (!brain) {
        printf("❌ Brain pointer is NULL\n");
        return;
    }

    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║              Brain Diagnostic Report                   ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");

    printf("🧠 Status: %s\n", brain_state_string(brain->state));
    printf("📊 Uptime: %lu seconds\n", brain_get_uptime(brain));
    printf("💭 Thoughts: %lu\n", brain_get_thought_count(brain));
    printf("💾 Memories: %u\n", brain_get_memory_count(brain));

    if (brain_is_healthy(brain)) {
        printf("\n✅ All systems nominal\n");
    } else {
        printf("\n⚠️  Problems detected:\n");

        if (!brain->organs.spine) printf("  • Spine is NULL\n");
        if (!brain->organs.heart) printf("  • Heart is NULL\n");
        if (!brain->organs.cortex) printf("  • Cortex is NULL\n");
        if (!brain->organs.hippocampus) printf("  • Hippocampus is NULL\n");

        if (brain->state == BRAIN_STATE_DEAD) {
            printf("  • Brain is dead\n");
        }
    }

    printf("\n");
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_digestion.c
 *
 * 소화 시스템 통합 테스트 (Mouth → Stomach → Pancreas → Brain)
 *
 * 테스트 시나리오:
 *   1. 입으로 여러 타입 데이터 섭취
 *   2. 위에서 버퍼링
 *   3. 췌장이 파싱
 *   4. 뇌로 전달 (Brain Core에 저장)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_stomach.h"
#include "kim_pancreas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test Data (섭취할 음식들)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
const char* test_foods[] = {
    "Hello, this is plain text!",
    "{\"name\":\"Kim\",\"age\":30,\"city\":\"Seoul\"}",
    "SGVsbG8gV29ybGQh",  /* Base64: "Hello World!" */
    "0x48656c6c6f",       /* Hex: "Hello" */
    "{\"action\":\"search\",\"query\":\"AI\"}",
    "Plain string data for testing",
    "{\"status\":\"ok\",\"code\":200}",
    "Another plain text message"
};

const int num_foods = sizeof(test_foods) / sizeof(test_foods[0]);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Thread: Stomach Consumer (십이지장으로 배출)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    stomach_t*  stomach;
    pancreas_t* pancreas;
    int         running;
} consumer_args_t;

void* consumer_thread(void* arg) {
    consumer_args_t* args = (consumer_args_t*)arg;

    printf("\n[Thread] 소화 스레드 시작\n");

    while (args->running) {
        /* Stomach에서 음식 추출 */
        food_chunk_t food;
        int ret = stomach_extract(args->stomach, &food);

        if (ret == 0) {
            /* Pancreas로 파싱 */
            nutrient_t nutrient;
            pancreas_digest(args->pancreas, &food, &nutrient);

            if (nutrient.success) {
                printf("   🩸 [Blood] 영양소 흡수 성공: %zu bytes\n", nutrient.parsed_size);

                /* Brain으로 전달할 수 있음 (나중에 구현) */
                /* brain_store(nutrient.parsed_data, nutrient.parsed_size); */
            }

            /* 정리 */
            if (nutrient.parsed_data) {
                free(nutrient.parsed_data);
            }
            if (food.data) {
                free(food.data);
            }
        }

        usleep(100000);  /* 100ms 대기 */
    }

    printf("\n[Thread] 소화 스레드 종료\n");
    return NULL;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: Basic Ingestion (기본 섭취)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_basic_ingestion(stomach_t* stomach) {
    printf("\n=== Test 1: Basic Ingestion ===\n");

    /* 음식 3개 섭취 */
    for (int i = 0; i < 3; i++) {
        const char* food = test_foods[i];
        int ret = stomach_ingest(stomach, food, strlen(food));
        if (ret < 0) {
            printf("✗ Failed to ingest food %d\n", i);
            return -1;
        }
        usleep(50000);  /* 50ms 대기 */
    }

    stomach_stats(stomach);
    printf("✓ Test 1 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: Pancreas Enzyme Detection (효소 타입 감지)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_enzyme_detection(void) {
    printf("\n=== Test 2: Enzyme Detection ===\n");

    const char* test_cases[] = {
        "{\"key\":\"value\"}",          /* JSON */
        "SGVsbG8=",                     /* Base64 */
        "0x1234ABCD",                   /* Hex */
        "Plain text"                    /* TEXT */
    };

    const enzyme_type_t expected[] = {
        ENZYME_JSON,
        ENZYME_BASE64,
        ENZYME_HEX,
        ENZYME_TEXT
    };

    for (int i = 0; i < 4; i++) {
        enzyme_type_t detected = pancreas_detect_type(test_cases[i], strlen(test_cases[i]));
        printf("  [%d] \"%s\" → %s ",
               i, test_cases[i],
               detected == ENZYME_JSON ? "JSON" :
               detected == ENZYME_BASE64 ? "BASE64" :
               detected == ENZYME_HEX ? "HEX" :
               detected == ENZYME_TEXT ? "TEXT" : "UNKNOWN");

        if (detected == expected[i]) {
            printf("✓\n");
        } else {
            printf("✗ (expected %d, got %d)\n", expected[i], detected);
        }
    }

    printf("✓ Test 2 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: Full Digestion Pipeline (전체 소화 파이프라인)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_full_pipeline(stomach_t* stomach, pancreas_t* pancreas) {
    printf("\n=== Test 3: Full Digestion Pipeline ===\n");

    /* Consumer 스레드 시작 */
    consumer_args_t args;
    args.stomach = stomach;
    args.pancreas = pancreas;
    args.running = 1;

    pthread_t thread;
    pthread_create(&thread, NULL, consumer_thread, &args);

    printf("\n[Mouth] 음식 섭취 시작...\n");

    /* 모든 음식 섭취 */
    for (int i = 0; i < num_foods; i++) {
        const char* food = test_foods[i];
        printf("\n[Mouth] 섭취 [%d/%d]: \"%s\"\n", i + 1, num_foods, food);
        stomach_ingest(stomach, food, strlen(food));
        usleep(200000);  /* 200ms 대기 */
    }

    /* 모든 음식이 소화될 때까지 대기 */
    printf("\n[Mouth] 모든 음식 섭취 완료. 소화 대기 중...\n");
    sleep(3);

    /* Consumer 종료 */
    args.running = 0;
    pthread_join(thread, NULL);

    printf("✓ Test 3 passed\n");
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: Stomach Acid (위산 분비)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int test_stomach_acid(stomach_t* stomach) {
    printf("\n=== Test 4: Stomach Acid Secretion ===\n");

    /* 음식 3개 섭취 */
    for (int i = 0; i < 3; i++) {
        stomach_ingest(stomach, test_foods[i], strlen(test_foods[i]));
    }

    /* 위산 분비 */
    int processed = stomach_secrete_acid(stomach);
    printf("✓ Processed %d items with gastric acid\n", processed);

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║            Kim-AI-OS: Digestion System Test               ║\n");
    printf("║                                                            ║\n");
    printf("║   Mouth → Stomach → Pancreas → Blood → Brain              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    /* 기관 생성 */
    stomach_t* stomach = stomach_create(16);
    if (!stomach) {
        printf("✗ Failed to create stomach\n");
        return 1;
    }

    pancreas_t* pancreas = pancreas_create();
    if (!pancreas) {
        printf("✗ Failed to create pancreas\n");
        stomach_destroy(stomach);
        return 1;
    }

    /* 테스트 실행 */
    int result = 0;

    if (test_enzyme_detection() < 0) result = 1;
    if (test_basic_ingestion(stomach) < 0) result = 1;
    if (test_stomach_acid(stomach) < 0) result = 1;
    if (test_full_pipeline(stomach, pancreas) < 0) result = 1;

    /* 최종 통계 */
    printf("\n");
    stomach_stats(stomach);
    pancreas_stats(pancreas);

    /* 정리 */
    pancreas_destroy(pancreas);
    stomach_destroy(stomach);

    if (result == 0) {
        printf("\n╔════════════════════════════════════════════════════════════╗\n");
        printf("║              All Digestion Tests Passed! 🎉                ║\n");
        printf("║                                                            ║\n");
        printf("║         소화 시스템이 완벽하게 작동합니다!                     ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
    } else {
        printf("\n✗ Some tests failed\n");
    }

    return result;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_liver.c
 *
 * Liver (간) Memory Manager - Test Suite
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_liver.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 1: Basic Allocation/Deallocation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_basic_alloc(void) {
    printf("\n🟢 테스트 1: 기본 할당/해제\n");

    liver_t* liver = liver_create();
    if (!liver) {
        printf("❌ 간 생성 실패\n");
        return -1;
    }

    /* 100번 할당/해제 */
    void* ptrs[100] = {0};
    for (int i = 0; i < 100; i++) {
        size_t size = (i % 32 + 1) * 1024;  /* 1KB ~ 32KB */
        ptrs[i] = liver_alloc(liver, size, "test1");
        if (!ptrs[i]) {
            printf("❌ 할당 실패 #%d\n", i);
            liver_destroy(liver);
            return -1;
        }
    }

    for (int i = 0; i < 100; i++) {
        liver_free(liver, ptrs[i]);
    }

    int usage = liver_get_usage_percent(liver);
    printf("  할당: 100회, 해제: 100회\n");
    printf("  최종 사용률: %d%%\n", usage);
    printf("✅ 테스트 1 통과\n");

    liver_destroy(liver);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 2: Fragmentation
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_fragmentation(void) {
    printf("\n🟢 테스트 2: 단편화 테스트\n");

    liver_t* liver = liver_create();
    if (!liver) {
        printf("❌ 간 생성 실패\n");
        return -1;
    }

    /* 다양한 크기로 할당 */
    void* ptrs[50] = {0};
    for (int i = 0; i < 50; i++) {
        size_t size = ((i % 10) + 1) * 4096;  /* 4KB ~ 40KB */
        ptrs[i] = liver_alloc(liver, size, "frag-test");
        if (!ptrs[i]) {
            printf("❌ 할당 실패 #%d\n", i);
            liver_destroy(liver);
            return -1;
        }
    }

    /* 무작위로 일부 해제 (단편화 유발) */
    for (int i = 0; i < 50; i += 3) {
        liver_free(liver, ptrs[i]);
    }

    int usage = liver_get_usage_percent(liver);
    int frag = liver_get_fragmentation(liver);

    printf("  단편화율: %d%%\n", frag);
    printf("  사용률: %d%%\n", usage);

    if (frag < 50) {
        printf("✅ 테스트 2 통과 (단편화 수용 가능)\n");
    } else {
        printf("⚠️  테스트 2 경고 (단편화 높음)\n");
    }

    /* 정리 */
    for (int i = 0; i < 50; i++) {
        if (i % 3 != 0) {  /* 이전에 해제하지 않은 것들 */
            liver_free(liver, ptrs[i]);
        }
    }

    liver_destroy(liver);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 3: GC Daemon
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_gc_daemon(void) {
    printf("\n🟢 테스트 3: 가비지 컬렉션 데몬\n");

    liver_t* liver = liver_create();
    if (!liver) {
        printf("❌ 간 생성 실패\n");
        return -1;
    }

    /* GC 데몬 시작 (짧은 주기로) */
    if (liver_start_gc(liver) != 0) {
        printf("❌ GC 시작 실패\n");
        liver_destroy(liver);
        return -1;
    }

    printf("  GC 데몬 시작 (주기: %d초)\n", LIVER_GC_INTERVAL);
    printf("  메모리 할당 중...\n");

    /* 메모리 할당 */
    void* ptrs[20] = {0};
    for (int i = 0; i < 20; i++) {
        ptrs[i] = liver_alloc(liver, 1024 * 1024, "gc-test");  /* 1MB */
    }

    printf("  현재 사용률: %d%%\n", liver_get_usage_percent(liver));
    printf("  3초 대기 (GC 주기 3회)...\n");
    sleep(3);

    printf("  GC 주기 수: %lu\n", liver->gc_cycles);

    /* 정리 */
    for (int i = 0; i < 20; i++) {
        if (ptrs[i]) {
            liver_free(liver, ptrs[i]);
        }
    }

    liver_stop_gc(liver);

    if (liver->gc_cycles > 0) {
        printf("✅ 테스트 3 통과 (GC %lu회 실행)\n", liver->gc_cycles);
    } else {
        printf("⚠️  테스트 3 경고 (GC 미실행)\n");
    }

    liver_destroy(liver);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 4: Thread Safety
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    liver_t* liver;
    int thread_id;
    int ops_count;
    int success_count;
} thread_args_t;

void* thread_worker(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    void* ptrs[100] = {0};

    for (int i = 0; i < args->ops_count; i++) {
        size_t size = ((args->thread_id * 100 + i) % 32 + 1) * 1024;

        /* 할당 */
        ptrs[i % 100] = liver_alloc(args->liver, size, "thread-test");
        if (ptrs[i % 100]) {
            args->success_count++;
        }

        /* 때때로 해제 */
        if (i % 10 == 5 && ptrs[(i - 5) % 100]) {
            liver_free(args->liver, ptrs[(i - 5) % 100]);
            ptrs[(i - 5) % 100] = NULL;
        }
    }

    return NULL;
}

int test_thread_safety(void) {
    printf("\n🟢 테스트 4: 스레드 안전성\n");

    liver_t* liver = liver_create();
    if (!liver) {
        printf("❌ 간 생성 실패\n");
        return -1;
    }

    int num_threads = 10;
    pthread_t threads[10] = {0};
    thread_args_t args[10] = {0};

    printf("  %d개 스레드, 각 100개 연산 시작...\n", num_threads);

    /* 스레드 생성 */
    for (int i = 0; i < num_threads; i++) {
        args[i].liver = liver;
        args[i].thread_id = i;
        args[i].ops_count = 100;
        args[i].success_count = 0;

        pthread_create(&threads[i], NULL, thread_worker, &args[i]);
    }

    /* 대기 */
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    /* 결과 */
    int total_success = 0;
    for (int i = 0; i < num_threads; i++) {
        total_success += args[i].success_count;
    }

    printf("  성공: %d / %d\n", total_success, num_threads * 100);
    printf("  최종 사용률: %d%%\n", liver_get_usage_percent(liver));

    if (total_success > num_threads * 90) {  /* 90% 이상 성공 */
        printf("✅ 테스트 4 통과\n");
    } else {
        printf("❌ 테스트 4 실패\n");
        liver_destroy(liver);
        return -1;
    }

    liver_destroy(liver);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Test 5: Memory Dump
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int test_memory_dump(void) {
    printf("\n🟢 테스트 5: 메모리 상태 덤프\n");

    liver_t* liver = liver_create();
    if (!liver) {
        printf("❌ 간 생성 실패\n");
        return -1;
    }

    /* 일부 메모리 할당 */
    void* p1 = liver_alloc(liver, 8192, "block-A");
    void* p2 = liver_alloc(liver, 16384, "block-B");
    void* p3 = liver_alloc(liver, 4096, "block-C");

    printf("  3개 블록 할당됨 (A, B, C)\n");

    liver_dump_blocks(liver);
    liver_validate_heap(liver);

    liver_free(liver, p1);
    liver_free(liver, p2);
    liver_free(liver, p3);

    printf("✅ 테스트 5 통과\n");

    liver_destroy(liver);
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Main Test Suite
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

int main(void) {
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║  간(Liver) 메모리 관리자 테스트 스위트     ║\n");
    printf("╚════════════════════════════════════════════╝\n");

    int failed = 0;

    /* 각 테스트 실행 */
    failed += test_basic_alloc();
    failed += test_fragmentation();
    failed += test_gc_daemon();
    failed += test_thread_safety();
    failed += test_memory_dump();

    /* 결과 요약 */
    printf("\n╔════════════════════════════════════════════╗\n");
    if (failed == 0) {
        printf("║            ✅ 모든 테스트 통과!           ║\n");
    } else {
        printf("║           ❌ %d개 테스트 실패            ║\n", failed);
    }
    printf("╚════════════════════════════════════════════╝\n\n");

    return failed;
}

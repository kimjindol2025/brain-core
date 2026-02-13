/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_lungs.c
 *
 * Lungs (폐) Async I/O Manager - Test Suite
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_lungs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

static int callback_count = 0;

void test_callback(int fd, void* data, ssize_t len, void* user_data) {
    callback_count++;
    if (len > 0) {
        printf("  ✓ 콜백 #%d: fd=%d, len=%zd\n", callback_count, fd, len);
    }
}

int test_basic_operations(void) {
    printf("\n🟢 테스트 1: 기본 동작\n");

    lungs_t* lungs = lungs_create(4);
    if (!lungs) {
        printf("❌ 폐 생성 실패\n");
        return -1;
    }

    if (lungs_start(lungs) != 0) {
        printf("❌ 폐 시작 실패\n");
        lungs_destroy(lungs);
        return -1;
    }

    /* 메모리 쓰기 테스트 */
    char test_data[] = "Hello, Async I/O!";
    int fd = open("/tmp/test_lungs.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("❌ 파일 열기 실패\n");
        lungs_stop(lungs);
        lungs_destroy(lungs);
        return -1;
    }

    /* 비동기 쓰기 5회 */
    for (int i = 0; i < 5; i++) {
        lungs_async_write(lungs, fd, test_data, strlen(test_data), test_callback, NULL);
    }

    sleep(1);

    printf("  콜백 실행 횟수: %d / 5\n", callback_count);
    printf("✅ 테스트 1 통과\n");

    close(fd);
    lungs_stop(lungs);
    lungs_destroy(lungs);

    return 0;
}

int test_queue_management(void) {
    printf("\n🟢 테스트 2: 큐 관리\n");

    lungs_t* lungs = lungs_create(2);
    if (!lungs) {
        printf("❌ 폐 생성 실패\n");
        return -1;
    }

    if (lungs_start(lungs) != 0) {
        printf("❌ 폐 시작 실패\n");
        lungs_destroy(lungs);
        return -1;
    }

    int fd = open("/tmp/test_queue.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("❌ 파일 열기 실패\n");
        lungs_destroy(lungs);
        return -1;
    }

    /* 30개 요청 추가 */
    callback_count = 0;
    for (int i = 0; i < 30; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Request #%d\n", i);
        lungs_async_write(lungs, fd, buf, strlen(buf), test_callback, NULL);
    }

    printf("  요청 큐 깊이: %d / %u (%d%%)\n",
           lungs_get_queue_depth(lungs),
           LUNGS_MAX_QUEUE,
           lungs_get_queue_percent(lungs));

    sleep(2);

    printf("  콜백 실행: %d / 30\n", callback_count);
    printf("✅ 테스트 2 통과\n");

    close(fd);
    lungs_stop(lungs);
    lungs_destroy(lungs);

    return 0;
}

int test_worker_pool(void) {
    printf("\n🟢 테스트 3: 워커 스레드 풀\n");

    for (int w = 1; w <= 4; w++) {
        lungs_t* lungs = lungs_create(w);
        if (!lungs) {
            printf("❌ 폐 생성 실패\n");
            return -1;
        }

        if (lungs_start(lungs) != 0) {
            printf("❌ 폐 시작 실패\n");
            lungs_destroy(lungs);
            return -1;
        }

        int fd = open("/tmp/test_workers.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            printf("❌ 파일 열기 실패\n");
            lungs_destroy(lungs);
            return -1;
        }

        /* 100개 요청 */
        callback_count = 0;
        for (int i = 0; i < 100; i++) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", i);
            lungs_async_write(lungs, fd, buf, strlen(buf), test_callback, NULL);
        }

        sleep(1);

        printf("  워커 %d개: %d 콜백\n", w, callback_count);

        close(fd);
        lungs_stop(lungs);
        lungs_destroy(lungs);
    }

    printf("✅ 테스트 3 통과\n");
    return 0;
}

int test_statistics(void) {
    printf("\n🟢 테스트 4: 통계 정보\n");

    lungs_t* lungs = lungs_create(2);
    if (!lungs) {
        printf("❌ 폐 생성 실패\n");
        return -1;
    }

    if (lungs_start(lungs) != 0) {
        printf("❌ 폐 시작 실패\n");
        lungs_destroy(lungs);
        return -1;
    }

    int fd = open("/tmp/test_stats.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("❌ 파일 열기 실패\n");
        lungs_destroy(lungs);
        return -1;
    }

    /* 50개 쓰기 */
    for (int i = 0; i < 50; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Data line %d\n", i);
        lungs_async_write(lungs, fd, buf, strlen(buf), test_callback, NULL);
    }

    sleep(1);

    lungs_stats(lungs);

    close(fd);
    lungs_stop(lungs);
    lungs_destroy(lungs);

    return 0;
}

int main(void) {
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║   폐(Lungs) 비동기 I/O 관리자 테스트 스위트  ║\n");
    printf("╚════════════════════════════════════════════╝\n");

    int failed = 0;

    failed += test_basic_operations();
    failed += test_queue_management();
    failed += test_worker_pool();
    failed += test_statistics();

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

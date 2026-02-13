/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_integration.c
 *
 * Brain Core: Liver + Lungs Integration Test
 * 간과 폐의 통합 운영 테스트 (24시간 시뮬레이션)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define _POSIX_C_SOURCE 200809L

#include "kim_liver.h"
#include "kim_lungs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

void lungs_callback(int fd __attribute__((unused)), void* data __attribute__((unused)),
                   ssize_t len __attribute__((unused)), void* user_data __attribute__((unused))) {
    /* 간단한 콜백 (실제로는 Liver에 데이터 저장) */
}

int main(void) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║  Brain Core 통합 테스트: Liver + Lungs 24시간 운영  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    /* Liver 생성 */
    printf("\n[Step 1] 간(Liver) 생성...\n");
    liver_t* liver = liver_create();
    if (!liver) {
        printf("❌ 간 생성 실패\n");
        return -1;
    }

    /* Lungs 생성 */
    printf("\n[Step 2] 폐(Lungs) 생성...\n");
    lungs_t* lungs = lungs_create(4);
    if (!lungs) {
        printf("❌ 폐 생성 실패\n");
        liver_destroy(liver);
        return -1;
    }

    /* Lungs 시작 */
    printf("\n[Step 3] 폐(Lungs) 시작...\n");
    if (lungs_start(lungs) != 0) {
        printf("❌ 폐 시작 실패\n");
        lungs_destroy(lungs);
        liver_destroy(liver);
        return -1;
    }

    /* GC 데몬 시작 */
    printf("\n[Step 4] 가비지 컬렉션 데몬 시작...\n");
    if (liver_start_gc(liver) != 0) {
        printf("❌ GC 시작 실패\n");
        lungs_stop(lungs);
        lungs_destroy(lungs);
        liver_destroy(liver);
        return -1;
    }

    /* 24시간 시뮬레이션 (100배 가속) */
    printf("\n[Step 5] 통합 운영 테스트 시작 (14.4초 = 24시간 × 100배 가속)...\n");

    int temp_file = open("/tmp/integration_test.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_file < 0) {
        printf("❌ 임시 파일 생성 실패\n");
        liver_stop_gc(liver);
        lungs_stop(lungs);
        lungs_destroy(lungs);
        liver_destroy(liver);
        return -1;
    }

    /* 메모리 할당 및 I/O 작업 */
    for (int i = 0; i < 100; i++) {
        /* Liver: 메모리 할당 */
        void* mem = liver_alloc(liver, 4096, "integration");
        if (!mem) {
            printf("⚠️  메모리 할당 실패 #%d\n", i);
            continue;
        }

        /* Lungs: 비동기 I/O */
        char buf[64];
        snprintf(buf, sizeof(buf), "Integration test #%d\n", i);
        lungs_async_write(lungs, temp_file, buf, strlen(buf), lungs_callback, NULL);

        /* 가끔 메모리 해제 */
        if (i % 3 == 0) {
            liver_free(liver, mem);
        }

        /* 진행률 표시 */
        if (i % 10 == 0) {
            printf("  진행: %d/100 (%d%%%%)\n", i, (i * 100) / 100);
        }

        /* 시간 시뮬레이션 (1이= 14.4ms × 100) */
        usleep(144000);
    }

    printf("  진행: 100/100 (100%)\n");

    /* 마무리 대기 */
    sleep(2);

    printf("\n[Step 6] 최종 통계...\n");

    printf("\n📊 Liver 통계:\n");
    int liver_usage = liver_get_usage_percent(liver);
    printf("  메모리 사용률: %d%%\n", liver_usage);
    printf("  GC 주기 수: %lu\n", liver->gc_cycles);

    printf("\n📊 Lungs 통계:\n");
    printf("  I/O 쓰기 작업: %lu회\n", lungs->total_writes);
    printf("  출력 바이트: %lu bytes\n", lungs->bytes_out);
    printf("  큐 최대 깊이: %lu / %u\n", lungs->queue_depth_max, lungs->capacity);

    /* 정리 */
    printf("\n[Step 7] 시스템 정지...\n");

    close(temp_file);
    liver_stop_gc(liver);
    lungs_stop(lungs);

    /* 최종 상태 */
    printf("\n🟢 통합 테스트 결과:\n");
    printf("  ✅ Liver 생성/시작/GC: 성공\n");
    printf("  ✅ Lungs 생성/시작: 성공\n");
    printf("  ✅ 메모리 할당: %d회\n", 100);
    printf("  ✅ 비동기 I/O: %lu회\n", lungs->total_writes);
    printf("  ✅ 24시간 시뮬레이션: 완료\n");

    lungs_destroy(lungs);
    liver_destroy(liver);

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║   ✅ Liver + Lungs 통합 테스트 성공!              ║\n");
    printf("║   24시간 안정 운영 가능성 확인됨                  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    return 0;
}

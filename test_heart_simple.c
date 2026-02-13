/* Simple Heart Test */
#define _POSIX_C_SOURCE 200809L

#include "kim_heart.h"
#include "kim_stomach.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("=== Simple Heart Test ===\n\n");

    /* 심장 생성 */
    heart_t* heart = heart_create();
    if (!heart) {
        printf("❌ 심장 생성 실패\n");
        return 1;
    }

    printf("✅ 심장 생성 성공 (BPM: %d)\n", heart_get_bpm(heart));

    /* 박동 시작 */
    if (heart_start(heart) != 0) {
        printf("❌ 박동 시작 실패\n");
        heart_destroy(heart);
        return 1;
    }

    printf("✅ 박동 시작 성공\n");
    printf("   2초 대기 중...\n");
    sleep(2);

    /* 기어 변속 */
    heart_set_gear(heart, GEAR_BOOST);
    printf("✅ BOOST 기어 변속\n");
    sleep(1);

    /* 박동 중지 */
    heart_stop(heart);
    printf("✅ 박동 중지\n");

    /* 통계 */
    heart_stats(heart);

    /* 삭제 */
    heart_destroy(heart);
    printf("\n✅ 모든 테스트 완료\n");

    return 0;
}

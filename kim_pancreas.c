/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_pancreas.c
 *
 * Pancreas (Parser Engine) Implementation
 *
 * Zero Dependency: stdlib + string.h만 사용 (JSON 파서 수동 구현)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "kim_pancreas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Create/Destroy
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
pancreas_t* pancreas_create(void) {
    pancreas_t* p = (pancreas_t*)malloc(sizeof(pancreas_t));
    if (!p) {
        fprintf(stderr, "[Pancreas] Error: malloc failed\n");
        return NULL;
    }

    memset(p, 0, sizeof(pancreas_t));

    printf("[Pancreas] 췌장 생성 완료\n");
    return p;
}

void pancreas_destroy(pancreas_t* p) {
    if (p) {
        free(p);
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Detect Data Type (데이터 타입 자동 감지)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
enzyme_type_t pancreas_detect_type(const char* data, size_t size) {
    if (!data || size == 0) return ENZYME_UNKNOWN;

    /* JSON: '{' 또는 '[' 시작 */
    if (data[0] == '{' || data[0] == '[') {
        return ENZYME_JSON;
    }

    /* Base64: A-Za-z0-9+/= 로만 구성 */
    int is_base64 = 1;
    for (size_t i = 0; i < size && i < 100; i++) {
        char c = data[i];
        if (!isalnum(c) && c != '+' && c != '/' && c != '=' && c != '\n' && c != '\r') {
            is_base64 = 0;
            break;
        }
    }
    if (is_base64 && size > 20) {
        return ENZYME_BASE64;
    }

    /* Hex: "0x" 시작 또는 0-9a-fA-F로만 구성 */
    if (size > 2 && data[0] == '0' && data[1] == 'x') {
        return ENZYME_HEX;
    }

    /* 기본: TEXT */
    return ENZYME_TEXT;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Get Timestamp (microseconds)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static int64_t get_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Enzyme: TEXT (단순 복사)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int enzyme_parse_text(const char* data, size_t size, void** out, size_t* out_size) {
    *out = malloc(size + 1);
    if (!*out) return -1;

    memcpy(*out, data, size);
    ((char*)*out)[size] = '\0';
    *out_size = size;

    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Enzyme: JSON (간단한 키-값 추출)
 *
 * 완전한 파서는 아니고, 최상위 레벨 키만 추출
 * 예: {"name":"Kim","age":30} → "name=Kim, age=30"
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int enzyme_parse_json(const char* data, size_t size, void** out, size_t* out_size) {
    /* 일단 원본 복사 후 간단한 정보 추출 */
    char* buffer = (char*)malloc(size * 2);  /* 여유 있게 */
    if (!buffer) return -1;

    size_t pos = 0;
    int in_string = 0;
    int key_count = 0;

    pos += snprintf(buffer + pos, size * 2 - pos, "[JSON parsed] Keys: ");

    /* 간단히 "key" 문자열 찾기 */
    for (size_t i = 0; i < size - 1; i++) {
        if (data[i] == '"') {
            /* 문자열 시작 */
            size_t start = i + 1;
            size_t end = start;
            while (end < size && data[end] != '"') end++;

            if (end < size && i > 0 && (data[i - 1] == '{' || data[i - 1] == ',' || data[i - 1] == ' ')) {
                /* 이게 키일 가능성 */
                size_t key_len = end - start;
                if (key_len > 0 && key_len < 100) {
                    if (key_count > 0) {
                        pos += snprintf(buffer + pos, size * 2 - pos, ", ");
                    }
                    pos += snprintf(buffer + pos, size * 2 - pos, "%.*s", (int)key_len, data + start);
                    key_count++;
                }
            }
            i = end;
        }
    }

    *out = buffer;
    *out_size = pos;
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Enzyme: Base64 Decode
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int enzyme_parse_base64(const char* data, size_t size, void** out, size_t* out_size) {
    /* 간단한 구현: 4바이트 → 3바이트 변환 */
    size_t output_len = (size / 4) * 3;
    char* buffer = (char*)malloc(output_len + 1);
    if (!buffer) return -1;

    size_t j = 0;
    for (size_t i = 0; i < size; i += 4) {
        if (i + 3 >= size) break;

        /* 각 문자를 6비트 값으로 변환 */
        int val[4];
        for (int k = 0; k < 4; k++) {
            const char* p = strchr(base64_table, data[i + k]);
            val[k] = p ? (int)(p - base64_table) : 0;
        }

        /* 24비트 → 3바이트 */
        buffer[j++] = (val[0] << 2) | (val[1] >> 4);
        buffer[j++] = (val[1] << 4) | (val[2] >> 2);
        buffer[j++] = (val[2] << 6) | val[3];
    }

    buffer[j] = '\0';
    *out = buffer;
    *out_size = j;
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Enzyme: Hex Decode
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int enzyme_parse_hex(const char* data, size_t size, void** out, size_t* out_size) {
    size_t start = 0;
    if (size > 2 && data[0] == '0' && data[1] == 'x') {
        start = 2;  /* "0x" 건너뛰기 */
    }

    size_t output_len = (size - start) / 2;
    char* buffer = (char*)malloc(output_len + 1);
    if (!buffer) return -1;

    size_t j = 0;
    for (size_t i = start; i < size - 1; i += 2) {
        char hex[3] = { data[i], data[i + 1], '\0' };
        buffer[j++] = (char)strtol(hex, NULL, 16);
    }

    buffer[j] = '\0';
    *out = buffer;
    *out_size = j;
    return 0;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Digest (소화 - 파싱 실행)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int pancreas_digest(
    pancreas_t* p,
    const food_chunk_t* food,
    nutrient_t* out_nutrient
) {
    if (!p || !food || !out_nutrient) return -1;

    /* 타입 감지 */
    enzyme_type_t type = pancreas_detect_type(food->data, food->size);
    printf("   🧪 [Pancreas] 효소 타입 감지: ");

    /* 효소별 파싱 */
    int result = -1;
    void* parsed_data = NULL;
    size_t parsed_size = 0;

    switch (type) {
        case ENZYME_TEXT:
            printf("TEXT\n");
            result = enzyme_parse_text(food->data, food->size, &parsed_data, &parsed_size);
            p->enzyme_text++;
            break;

        case ENZYME_JSON:
            printf("JSON\n");
            result = enzyme_parse_json(food->data, food->size, &parsed_data, &parsed_size);
            p->enzyme_json++;
            break;

        case ENZYME_BASE64:
            printf("BASE64\n");
            result = enzyme_parse_base64(food->data, food->size, &parsed_data, &parsed_size);
            p->enzyme_base64++;
            break;

        case ENZYME_HEX:
            printf("HEX\n");
            result = enzyme_parse_hex(food->data, food->size, &parsed_data, &parsed_size);
            break;

        default:
            printf("UNKNOWN\n");
            result = -1;
            break;
    }

    /* 결과 저장 */
    out_nutrient->type = type;
    out_nutrient->parsed_data = parsed_data;
    out_nutrient->parsed_size = parsed_size;
    out_nutrient->timestamp = get_timestamp_us();
    out_nutrient->success = (result == 0);

    p->total_parsed++;
    if (result != 0) {
        p->parse_failures++;
    }

    printf("   ✅ [Pancreas] 파싱 %s: %zu bytes → %zu bytes\n",
           result == 0 ? "성공" : "실패", food->size, parsed_size);

    return result;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Hormone Secretion (호르몬 분비 - 시스템 부하 조절)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void pancreas_secrete_insulin(pancreas_t* p, int traffic_load) {
    if (!p) return;

    if (traffic_load > 80) {
        p->insulin_level++;
        printf("   💉 [Pancreas] 인슐린 분비! (혈당 과다, 처리 속도 제한: 레벨 %d)\n",
               p->insulin_level);
    } else {
        if (p->insulin_level > 0) {
            p->insulin_level--;
        }
    }
}

void pancreas_secrete_glucagon(pancreas_t* p, int traffic_load) {
    if (!p) return;

    if (traffic_load < 20) {
        p->glucagon_level++;
        printf("   🚀 [Pancreas] 글루카곤 분비! (혈당 부족, 처리 속도 증가: 레벨 %d)\n",
               p->glucagon_level);
    } else {
        if (p->glucagon_level > 0) {
            p->glucagon_level--;
        }
    }
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Statistics
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void pancreas_stats(const pancreas_t* p) {
    if (!p) return;

    printf("\n[Pancreas Statistics]\n");
    printf("  Total Parsed:    %lu\n", p->total_parsed);
    printf("  Parse Failures:  %lu (%.1f%%)\n",
           p->parse_failures,
           p->total_parsed > 0 ? (float)p->parse_failures / p->total_parsed * 100.0f : 0.0f);
    printf("  Enzyme Usage:\n");
    printf("    TEXT:          %lu\n", p->enzyme_text);
    printf("    JSON:          %lu\n", p->enzyme_json);
    printf("    BASE64:        %lu\n", p->enzyme_base64);
    printf("    Protobuf:      %lu\n", p->enzyme_protobuf);
    printf("  Hormone Levels:\n");
    printf("    Insulin:       %d\n", p->insulin_level);
    printf("    Glucagon:      %d\n", p->glucagon_level);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * kim_pancreas.h
 *
 * 췌장(Pancreas) - Data Parser & Decoder Engine
 *
 * 생물학적 역할:
 *   - 십이지장에 소화 효소 분비
 *   - Amylase (탄수화물), Lipase (지방), Trypsin (단백질)
 *   - 인슐린/글루카곤으로 혈당 조절
 *
 * 소프트웨어 역할:
 *   - Raw Data → Structured Object 변환
 *   - JSON/XML/Protobuf 파싱
 *   - 데이터 타입 자동 감지
 *   - 시스템 부하 조절 (Flow Control)
 *
 * Priority: ★시급★ (2/3)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef KIM_PANCREAS_H
#define KIM_PANCREAS_H

#include <stdint.h>
#include <stddef.h>
#include "kim_stomach.h"

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Enzyme Types (효소 종류)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef enum {
    ENZYME_TEXT,        /* 일반 텍스트 (Plain String) */
    ENZYME_JSON,        /* JSON 파싱 */
    ENZYME_PROTOBUF,    /* Protobuf 디코딩 */
    ENZYME_BASE64,      /* Base64 디코딩 */
    ENZYME_HEX,         /* Hex → Binary */
    ENZYME_CSV,         /* CSV 파싱 */
    ENZYME_UNKNOWN      /* 알 수 없음 */
} enzyme_type_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Parsed Data (소화된 영양소)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    enzyme_type_t type;         /* 효소 타입 */
    void*         parsed_data;  /* 파싱된 데이터 (malloc) */
    size_t        parsed_size;  /* 파싱된 크기 */
    int64_t       timestamp;    /* 파싱 시각 */
    int           success;      /* 성공 여부 */
} nutrient_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Pancreas (췌장)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
typedef struct {
    /* 호르몬 (시스템 상태) */
    int insulin_level;      /* 인슐린 (처리 속도 제한) */
    int glucagon_level;     /* 글루카곤 (처리 속도 증가) */

    /* 통계 */
    uint64_t total_parsed;          /* 총 파싱 횟수 */
    uint64_t parse_failures;        /* 파싱 실패 */
    uint64_t enzyme_text;           /* TEXT 처리 */
    uint64_t enzyme_json;           /* JSON 처리 */
    uint64_t enzyme_protobuf;       /* Protobuf 처리 */
    uint64_t enzyme_base64;         /* Base64 처리 */
} pancreas_t;

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Functions
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* 췌장 생성/삭제 */
pancreas_t* pancreas_create(void);
void        pancreas_destroy(pancreas_t* pancreas);

/* 효소 분비 (데이터 타입 자동 감지) */
enzyme_type_t pancreas_detect_type(const char* data, size_t size);

/* 소화 효소 주입 (파싱 실행) */
int pancreas_digest(
    pancreas_t* pancreas,
    const food_chunk_t* food,
    nutrient_t* out_nutrient
);

/* 호르몬 분비 (시스템 부하 조절) */
void pancreas_secrete_insulin(pancreas_t* pancreas, int traffic_load);
void pancreas_secrete_glucagon(pancreas_t* pancreas, int traffic_load);

/* 통계 */
void pancreas_stats(const pancreas_t* pancreas);

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Enzyme Functions (각 효소별 파싱 함수)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* TEXT: 단순 복사 */
int enzyme_parse_text(const char* data, size_t size, void** out, size_t* out_size);

/* JSON: 간단한 파싱 (Zero Dependency - 수동 파싱) */
int enzyme_parse_json(const char* data, size_t size, void** out, size_t* out_size);

/* Base64: 디코딩 */
int enzyme_parse_base64(const char* data, size_t size, void** out, size_t* out_size);

/* Hex: 16진수 → 바이너리 */
int enzyme_parse_hex(const char* data, size_t size, void** out, size_t* out_size);

#endif /* KIM_PANCREAS_H */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * brain_format.h
 *
 * Brain 바이너리 파일 규격 정의
 *
 * 목적:
 *   - 벡터, 메타데이터를 효율적으로 저장
 *   - mmap 기반 Zero-Copy 접근
 *   - ID 기반 빠른 검색 (O(1))
 *
 * 파일 구조:
 *   [Header] [Index Section] [Data Section]
 *
 *   Header: 전체 메타데이터
 *   Index: ID → Offset 매핑
 *   Data: 실제 벡터 + 메타데이터
 *
 * Zero Dependency: 표준 C만 사용
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef BRAIN_FORMAT_H
#define BRAIN_FORMAT_H

#include <stdint.h>
#include <stddef.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 상수 정의
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#define BRAIN_MAGIC         0x4252414E  /* "BRAN" (리틀 엔디안) */
#define BRAIN_VERSION       1           /* 파일 포맷 버전 */
#define BRAIN_DEFAULT_DIM   128         /* 기본 벡터 차원 */
#define BRAIN_MAX_METADATA  256         /* 메타데이터 최대 크기 */

/* Index Hash Table 크기 (소수로 설정하여 충돌 최소화) */
#define BRAIN_INDEX_BUCKETS 10007       /* 약 1만개 버킷 */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 파일 헤더 (64 bytes)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    uint32_t magic;             /* 매직 넘버: 0x4252414E ("BRAN") */
    uint32_t version;           /* 파일 포맷 버전 */

    uint32_t vector_dim;        /* 벡터 차원 (128, 256, 512 등) */
    uint32_t vector_count;      /* 저장된 벡터 개수 */

    uint64_t index_offset;      /* Index Section 시작 오프셋 */
    uint64_t data_offset;       /* Data Section 시작 오프셋 */

    uint64_t file_size;         /* 전체 파일 크기 */

    uint32_t flags;             /* 플래그 (향후 확장용) */
    uint32_t reserved;          /* 예약 (64바이트 정렬) */

} __attribute__((packed)) brain_header_t;

/* 컴파일 타임 크기 검증 */
_Static_assert(sizeof(brain_header_t) == 48, "Header must be 48 bytes");

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Index Entry (16 bytes per entry)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    int64_t  vector_id;         /* 벡터 ID (음수 = 빈 슬롯) */
    uint64_t data_offset;       /* Data Section 내 오프셋 */
} brain_index_entry_t;

_Static_assert(sizeof(brain_index_entry_t) == 16, "Index entry must be 16 bytes");

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * Data Entry (가변 길이)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

typedef struct {
    int64_t  vector_id;         /* 벡터 ID (중복 저장 - 검증용) */
    uint32_t vector_dim;        /* 벡터 차원 */
    uint32_t metadata_len;      /* 메타데이터 길이 */

    int64_t  timestamp;         /* 생성/수정 시간 (Unix epoch) */
    uint32_t access_count;      /* 접근 횟수 */
    float    importance;        /* 중요도 (0.0 ~ 1.0) */

    /* 이후 가변 길이 데이터:
     *   float vector[vector_dim]
     *   char  metadata[metadata_len]
     */
} brain_data_entry_t;

_Static_assert(sizeof(brain_data_entry_t) == 32, "Data entry header must be 32 bytes");

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 헬퍼 매크로
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Data Entry 전체 크기 계산 */
#define BRAIN_DATA_ENTRY_SIZE(dim, meta_len) \
    (sizeof(brain_data_entry_t) + (dim) * sizeof(float) + (meta_len))

/* Data Entry에서 벡터 포인터 얻기 */
#define BRAIN_VECTOR_PTR(entry) \
    ((float*)((char*)(entry) + sizeof(brain_data_entry_t)))

/* Data Entry에서 메타데이터 포인터 얻기 */
#define BRAIN_METADATA_PTR(entry) \
    ((char*)(entry) + sizeof(brain_data_entry_t) + (entry)->vector_dim * sizeof(float))

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 파일 레이아웃 예시
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 *
 * Offset       Size        Section
 * ───────────────────────────────────────────────────────────────
 * 0x0000       64          Header
 * 0x0040       160,112     Index (10007 entries × 16 bytes)
 * 0x27160      ...         Data Section
 *
 * Index Section:
 *   Hash(ID) % 10007 → bucket
 *   Linear probing으로 충돌 해결
 *
 * Data Section:
 *   각 entry는 순차 배치
 *   Index에서 offset으로 접근
 *
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 함수 선언 (구현은 각 .c 파일에서)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* brain_format.c (유틸리티) */
int brain_validate_header(const brain_header_t* header);
void brain_init_header(brain_header_t* header, uint32_t dim);
void brain_print_header(const brain_header_t* header);

#endif /* BRAIN_FORMAT_H */

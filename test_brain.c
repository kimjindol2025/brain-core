/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * test_brain.c
 *
 * Brain Core 통합 테스트
 *
 * 테스트 시나리오:
 *   1. brain_format: 헤더 검증
 *   2. mmap_loader: 파일 생성 및 매핑
 *   3. index_manager: ID→오프셋 해시맵
 *   4. 통합: 헤더 + 인덱스 + 데이터 저장/조회
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#include "brain_format.h"
#include "mmap_loader.h"
#include "index_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TEST_FILE "test_brain.db"

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 테스트 1: 파일 구조 생성 및 초기화
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void test_file_creation() {
    printf("\n=== Test 1: File Creation ===\n");

    /* 파일 크기 계산 */
    size_t header_size = sizeof(brain_header_t);
    size_t index_size = sizeof(brain_index_entry_t) * BRAIN_INDEX_BUCKETS;
    size_t data_size = 1024 * 1024;  /* 1MB 데이터 영역 */
    size_t total_size = header_size + index_size + data_size;

    printf("File layout:\n");
    printf("  Header:  %zu bytes\n", header_size);
    printf("  Index:   %zu bytes (%d buckets)\n", index_size, BRAIN_INDEX_BUCKETS);
    printf("  Data:    %zu bytes\n", data_size);
    printf("  Total:   %zu bytes (%.2f MB)\n", total_size, total_size / 1024.0 / 1024.0);

    /* 파일 생성 */
    mmap_file_t* mf = mmap_file_create(TEST_FILE, total_size);
    if (!mf) {
        fprintf(stderr, "❌ File creation failed\n");
        return;
    }

    /* 헤더 초기화 */
    brain_header_t* header = (brain_header_t*)mf->addr;
    header->magic = BRAIN_MAGIC;
    header->version = BRAIN_VERSION;
    header->vector_dim = BRAIN_DEFAULT_DIM;
    header->vector_count = 0;
    header->index_offset = header_size;
    header->data_offset = header_size + index_size;
    header->file_size = total_size;
    header->flags = 0;

    printf("✓ Header initialized\n");

    /* 인덱스 초기화 */
    brain_index_entry_t* index = (brain_index_entry_t*)((char*)mf->addr + header->index_offset);
    index_init(index);

    printf("✓ Index initialized\n");

    /* 동기화 및 닫기 */
    mmap_file_sync(mf);
    mmap_file_close(mf);

    printf("✓ File created: %s\n", TEST_FILE);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 테스트 2: Index 해시맵 테스트
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void test_index_operations() {
    printf("\n=== Test 2: Index Operations ===\n");

    /* 파일 열기 */
    mmap_file_t* mf = mmap_file_open(TEST_FILE, 1);  /* writable */
    if (!mf) {
        fprintf(stderr, "❌ File open failed\n");
        return;
    }

    brain_header_t* header = (brain_header_t*)mf->addr;
    brain_index_entry_t* index = (brain_index_entry_t*)((char*)mf->addr + header->index_offset);

    /* Insert 테스트 */
    printf("\n[Insert Test]\n");
    int ids[] = {100, 200, 300, 999, 1234};
    for (int i = 0; i < 5; i++) {
        uint64_t offset = 1000 + i * 100;
        if (index_insert(index, ids[i], offset) == 0) {
            printf("  ✓ Inserted ID=%d → offset=%lu\n", ids[i], offset);
        }
    }

    /* Lookup 테스트 */
    printf("\n[Lookup Test]\n");
    for (int i = 0; i < 5; i++) {
        int64_t offset = index_lookup(index, ids[i]);
        if (offset >= 0) {
            printf("  ✓ Found ID=%d → offset=%ld\n", ids[i], offset);
        } else {
            printf("  ❌ Not found ID=%d\n", ids[i]);
        }
    }

    /* 존재하지 않는 ID */
    printf("\n[Negative Test]\n");
    int64_t offset = index_lookup(index, 9999);
    if (offset < 0) {
        printf("  ✓ ID=9999 not found (expected)\n");
    }

    /* Delete 테스트 */
    printf("\n[Delete Test]\n");
    if (index_delete(index, 200) == 0) {
        printf("  ✓ Deleted ID=200\n");
        offset = index_lookup(index, 200);
        if (offset < 0) {
            printf("  ✓ ID=200 lookup returns -1 (expected)\n");
        }
    }

    /* 통계 */
    printf("\n");
    index_stats(index);

    /* 덤프 */
    printf("\n");
    index_dump(index, 10);

    mmap_file_close(mf);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 테스트 3: 벡터 저장 및 조회 (간단 버전)
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
void test_vector_storage() {
    printf("\n=== Test 3: Vector Storage ===\n");

    mmap_file_t* mf = mmap_file_open(TEST_FILE, 1);
    if (!mf) return;

    brain_header_t* header = (brain_header_t*)mf->addr;
    brain_index_entry_t* index = (brain_index_entry_t*)((char*)mf->addr + header->index_offset);
    char* data_section = (char*)mf->addr + header->data_offset;

    /* 벡터 저장 */
    printf("\n[Store Vector]\n");

    int64_t vector_id = 10001;
    uint32_t dim = 128;
    const char* metadata = "hello";
    uint32_t meta_len = strlen(metadata) + 1;

    /* Data Entry 작성 */
    brain_data_entry_t* entry = (brain_data_entry_t*)data_section;
    entry->vector_id = vector_id;
    entry->vector_dim = dim;
    entry->metadata_len = meta_len;
    entry->timestamp = time(NULL);
    entry->access_count = 0;
    entry->importance = 0.8f;

    /* 벡터 데이터 (랜덤) */
    float* vec = BRAIN_VECTOR_PTR(entry);
    for (int i = 0; i < dim; i++) {
        vec[i] = (float)rand() / RAND_MAX;
    }

    /* 메타데이터 */
    char* meta = BRAIN_METADATA_PTR(entry);
    memcpy(meta, metadata, meta_len);

    /* Index에 등록 */
    uint64_t data_offset = (char*)entry - (char*)mf->addr;
    index_insert(index, vector_id, data_offset);

    header->vector_count++;

    printf("  ✓ Stored vector ID=%ld (dim=%d, meta=\"%s\")\n",
           vector_id, dim, metadata);

    /* 벡터 조회 */
    printf("\n[Retrieve Vector]\n");

    int64_t found_offset = index_lookup(index, vector_id);
    if (found_offset >= 0) {
        brain_data_entry_t* found = (brain_data_entry_t*)((char*)mf->addr + found_offset);
        float* found_vec = BRAIN_VECTOR_PTR(found);
        char* found_meta = BRAIN_METADATA_PTR(found);

        printf("  ✓ Retrieved vector ID=%ld\n", found->vector_id);
        printf("    - Dimension: %d\n", found->vector_dim);
        printf("    - Metadata:  \"%s\"\n", found_meta);
        printf("    - Timestamp: %ld\n", found->timestamp);
        printf("    - Importance: %.2f\n", found->importance);
        printf("    - Vector[0:3]: [%.4f, %.4f, %.4f, ...]\n",
               found_vec[0], found_vec[1], found_vec[2]);
    }

    mmap_file_close(mf);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 메인
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */
int main() {
    printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
    printf("┃  Brain Core Test Suite                          ┃\n");
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");

    /* 기존 파일 삭제 */
    unlink(TEST_FILE);

    /* 테스트 실행 */
    test_file_creation();
    test_index_operations();
    test_vector_storage();

    printf("\n┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
    printf("┃  All tests completed!                           ┃\n");
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");

    printf("\nTest file: %s (kept for inspection)\n", TEST_FILE);

    return 0;
}

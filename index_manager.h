/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * index_manager.h
 *
 * ID → Offset Hash Map Header
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

#include "brain_format.h"
#include <stdint.h>

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * 함수 선언
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* Index 테이블 초기화 */
void index_init(brain_index_entry_t* index_table);

/* ID와 오프셋 삽입 */
int index_insert(brain_index_entry_t* index_table, int64_t id, uint64_t offset);

/* ID로 오프셋 검색 */
int64_t index_lookup(const brain_index_entry_t* index_table, int64_t id);

/* ID 삭제 */
int index_delete(brain_index_entry_t* index_table, int64_t id);

/* 통계 출력 */
void index_stats(const brain_index_entry_t* index_table);

/* 내용 덤프 (디버깅) */
void index_dump(const brain_index_entry_t* index_table, int max_entries);

#endif /* INDEX_MANAGER_H */

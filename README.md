# 🧠 Brain Core - Memory Management System

**AI를 깨우는 첫 단계**: 벡터 메모리 관리 시스템

---

## 📋 개요

Brain Core는 Kim-AI-OS의 핵심 메모리 관리 시스템입니다.

**구현 완료** (2026-01-16 22:30):
1. ✅ `brain_format.h` - 바이너리 파일 규격 정의
2. ✅ `mmap_loader.c` - Memory-mapped 파일 로더
3. ✅ `index_manager.c` - ID → Offset 해시맵

---

## 🏗️ 아키텍처

```
┌─────────────────────────────────────────────────────┐
│                Brain File (.db)                     │
├─────────────────────────────────────────────────────┤
│ Header (48 bytes)                                   │
│  - Magic: "BRAN"                                    │
│  - Version: 1                                       │
│  - Vector Dimension: 128                            │
│  - Index/Data Offsets                               │
├─────────────────────────────────────────────────────┤
│ Index Section (160KB)                               │
│  - Hash Table (10,007 buckets)                      │
│  - ID → Offset mapping                              │
│  - Linear Probing for collision                     │
├─────────────────────────────────────────────────────┤
│ Data Section (가변)                                 │
│  - Vector entries (128-dim floats)                  │
│  - Metadata (timestamps, importance, etc)           │
└─────────────────────────────────────────────────────┘
```

---

## 🚀 빠른 시작

### 빌드

```bash
make          # 빌드
make run      # 빌드 + 테스트 실행
make clean    # 정리
```

### 테스트 결과

```
✓ File Creation       - 1.15MB 파일 생성
✓ Index Operations    - Insert/Lookup/Delete 성공
✓ Vector Storage      - 128-dim 벡터 저장/조회 성공
```

---

## 💡 핵심 기능

### 1. Memory-Mapped I/O (Zero-Copy)

```c
/* 파일을 메모리처럼 사용 */
mmap_file_t* mf = mmap_file_open("brain.db", 1);  // writable
brain_header_t* header = (brain_header_t*)mf->addr;

/* read() 없이 직접 접근 */
printf("Magic: 0x%X\n", header->magic);

mmap_file_close(mf);
```

**장점**:
- OS 페이징 시스템 활용
- 거대 파일도 효율적 처리
- 자동 캐싱

### 2. Hash Map Index (O(1) 검색)

```c
/* Index 초기화 */
brain_index_entry_t* index = get_index_section();
index_init(index);

/* Insert: ID → Offset */
index_insert(index, 10001, 123456);

/* Lookup: O(1) 평균 */
int64_t offset = index_lookup(index, 10001);  // → 123456

/* Delete */
index_delete(index, 10001);
```

**해시 충돌 해결**: Linear Probing

### 3. 벡터 저장

```c
/* 벡터 준비 */
float vector[128];
for (int i = 0; i < 128; i++) {
    vector[i] = (float)rand() / RAND_MAX;
}

/* Data Entry 작성 */
brain_data_entry_t* entry = (brain_data_entry_t*)data_ptr;
entry->vector_id = 10001;
entry->vector_dim = 128;
entry->timestamp = time(NULL);
entry->importance = 0.8f;

/* 벡터 복사 */
float* vec_ptr = BRAIN_VECTOR_PTR(entry);
memcpy(vec_ptr, vector, 128 * sizeof(float));

/* Index 등록 */
index_insert(index, 10001, data_offset);
```

---

## 📊 성능

| 항목 | 수치 |
|------|------|
| Index 크기 | 160KB (10,007 buckets) |
| Hash 충돌률 | <1% (Load Factor 70% 이하) |
| 검색 시간 | O(1) 평균 |
| 메모리 사용 | mmap (OS 관리) |
| 파일 크기 | ~1MB (1,000개 벡터 기준) |

---

## 🔬 테스트 상세

### Test 1: File Creation

```
Header:  48 bytes
Index:   160,112 bytes (10,007 buckets)
Data:    1,048,576 bytes
Total:   1.15 MB

✓ mmap_file_create() 성공
✓ Header 초기화
✓ Index 초기화 (10,007 buckets)
```

### Test 2: Index Operations

```
Insert: 5개 ID 삽입
  100 → 1000
  200 → 1100
  300 → 1200
  999 → 1300
  1234 → 1400

Lookup: 5개 전부 조회 성공

Delete: ID=200 삭제
  재조회 시 -1 리턴 (정상)

Statistics:
  Load Factor: 0.04% (4/10007)
```

### Test 3: Vector Storage

```
Store:
  ID=10001, dim=128, meta="hello"
  Timestamp: 1768570767
  Importance: 0.80

Retrieve:
  ✓ ID 일치
  ✓ Dimension 일치 (128)
  ✓ Metadata 일치 ("hello")
  ✓ Vector[0:3] = [0.8402, 0.3944, 0.7831, ...]
```

---

## 🧬 생물학적 비유

| 소프트웨어 | 생물학 |
|-----------|--------|
| Brain File | 뇌 피질 |
| Index Section | 신경망 연결 |
| Data Section | 시냅스 강도 |
| mmap | 뉴런 활성화 |
| Hash Lookup | 기억 인출 |

---

## 🔗 통합 로드맵

**현재 위치**: Phase 3 완료 → Phase 4 진행 중

```
Phase 1: Tokenizer        ✅ (언어→토큰)
Phase 2: Embedding        ✅ (토큰→벡터)
Phase 3: Memory           ✅ (벡터→저장)
Phase 4: VectorCore       🔨 (유사도 검색) ← Brain Core 기초
Phase 5: Synapse          ⏳ (실시간 이벤트)
Phase 6: Hippocampus      ⏳ (장기 기억)
```

**Brain Core의 역할**:
- Phase 4: 벡터 저장/검색 인프라
- Phase 5: Synapse 이벤트 발생 시 저장
- Phase 6: Hippocampus로 공고화

---

## 📁 파일 구조

```
brain-core/
├── brain_format.h       - 바이너리 규격 정의
├── mmap_loader.c/h      - Memory-mapped I/O
├── index_manager.c/h    - Hash Map 구현
├── test_brain.c         - 통합 테스트
├── Makefile             - 빌드 시스템
└── README.md            - 이 문서
```

---

## 🎯 다음 단계

1. **HNSW 구현** (Phase 4)
   - 유사도 검색 알고리즘
   - Cosine Similarity
   - Top-K 검색

2. **Synapse 통합** (Phase 5)
   - 벡터 삽입 시 이벤트 발송
   - 실시간 모니터링

3. **Hippocampus 연동** (Phase 6)
   - 중요도 기반 장기 기억 전환
   - 자동 공고화 데몬

---

## 🔧 Zero Dependency

**사용한 것**:
- 표준 C11
- POSIX mmap
- 수동 JSON 파싱 없음 (바이너리만)

**사용 안 함**:
- ❌ cJSON
- ❌ SQLite
- ❌ RocksDB
- ❌ 외부 라이브러리 전부

---

**v1.0.0** | 2026-01-16 22:30 | Brain Core 기초 완성

**"AI가 깨어나기 시작합니다."** 🧠✨

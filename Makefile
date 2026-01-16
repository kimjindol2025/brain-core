# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# Brain Core Makefile
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -O2 -g
LDFLAGS  = -lm

# 소스 파일
SRCS     = mmap_loader.c index_manager.c
OBJS     = $(SRCS:.c=.o)

# HNSW 소스 파일
HNSW_SRCS = hnsw.c
HNSW_OBJS = $(HNSW_SRCS:.c=.o)

# Digestion 소스 파일
DIGEST_SRCS = kim_stomach.c kim_pancreas.c
DIGEST_OBJS = $(DIGEST_SRCS:.c=.o)

# Spine 소스 파일
SPINE_SRCS = kim_spine.c
SPINE_OBJS = $(SPINE_SRCS:.c=.o)

# Health 소스 파일
HEALTH_SRCS = kim_health.c
HEALTH_OBJS = $(HEALTH_SRCS:.c=.o)

# Cortex 소스 파일
CORTEX_SRCS = kim_cortex.c
CORTEX_OBJS = $(CORTEX_SRCS:.c=.o)

# Circadian 소스 파일
CIRCADIAN_SRCS = kim_circadian.c
CIRCADIAN_OBJS = $(CIRCADIAN_SRCS:.c=.o)

# Watchdog 소스 파일
WATCHDOG_SRCS = kim_watchdog.c
WATCHDOG_OBJS = $(WATCHDOG_SRCS:.c=.o)

# 테스트 프로그램
TEST        = test_brain
TEST_SRC    = test_brain.c
TEST_HNSW   = test_hnsw
TEST_HNSW_SRC = test_hnsw.c
TEST_DIGEST = test_digestion
TEST_DIGEST_SRC = test_digestion.c
TEST_SPINE  = test_spine
TEST_SPINE_SRC = test_spine.c
TEST_HEALTH = test_health
TEST_HEALTH_SRC = test_health.c
TEST_CORTEX = test_cortex
TEST_CORTEX_SRC = test_cortex.c
TEST_CIRCADIAN = test_circadian
TEST_CIRCADIAN_SRC = test_circadian.c
TEST_WATCHDOG = test_watchdog
TEST_WATCHDOG_SRC = test_watchdog.c

# 기본 타겟
all: $(TEST) $(TEST_HNSW) $(TEST_DIGEST) $(TEST_SPINE) $(TEST_HEALTH) $(TEST_CORTEX) $(TEST_CIRCADIAN) $(TEST_WATCHDOG)

# 테스트 프로그램 빌드
$(TEST): $(OBJS) $(TEST_SRC)
	@echo "🔨 Building $(TEST)..."
	$(CC) $(CFLAGS) $(TEST_SRC) $(OBJS) -o $(TEST) $(LDFLAGS)
	@echo "✅ $(TEST) created"

$(TEST_HNSW): $(HNSW_OBJS) $(TEST_HNSW_SRC)
	@echo "🔨 Building $(TEST_HNSW)..."
	$(CC) $(CFLAGS) $(TEST_HNSW_SRC) $(HNSW_OBJS) -o $(TEST_HNSW) $(LDFLAGS)
	@echo "✅ $(TEST_HNSW) created"

$(TEST_DIGEST): $(DIGEST_OBJS) $(TEST_DIGEST_SRC)
	@echo "🔨 Building $(TEST_DIGEST)..."
	$(CC) $(CFLAGS) $(TEST_DIGEST_SRC) $(DIGEST_OBJS) -o $(TEST_DIGEST) $(LDFLAGS) -pthread
	@echo "✅ $(TEST_DIGEST) created"

$(TEST_SPINE): $(SPINE_OBJS) $(TEST_SPINE_SRC)
	@echo "🔨 Building $(TEST_SPINE)..."
	$(CC) $(CFLAGS) $(TEST_SPINE_SRC) $(SPINE_OBJS) -o $(TEST_SPINE) $(LDFLAGS) -pthread
	@echo "✅ $(TEST_SPINE) created"

$(TEST_HEALTH): $(DIGEST_OBJS) $(SPINE_OBJS) $(HEALTH_OBJS) $(TEST_HEALTH_SRC)
	@echo "🔨 Building $(TEST_HEALTH)..."
	$(CC) $(CFLAGS) $(TEST_HEALTH_SRC) $(DIGEST_OBJS) $(SPINE_OBJS) $(HEALTH_OBJS) -o $(TEST_HEALTH) $(LDFLAGS) -pthread
	@echo "✅ $(TEST_HEALTH) created"

$(TEST_CORTEX): $(SPINE_OBJS) $(CORTEX_OBJS) $(TEST_CORTEX_SRC)
	@echo "🔨 Building $(TEST_CORTEX)..."
	$(CC) $(CFLAGS) $(TEST_CORTEX_SRC) $(SPINE_OBJS) $(CORTEX_OBJS) -o $(TEST_CORTEX) $(LDFLAGS) -pthread
	@echo "✅ $(TEST_CORTEX) created"

$(TEST_CIRCADIAN): $(CIRCADIAN_OBJS) $(TEST_CIRCADIAN_SRC)
	@echo "🔨 Building $(TEST_CIRCADIAN)..."
	$(CC) $(CFLAGS) $(TEST_CIRCADIAN_SRC) $(CIRCADIAN_OBJS) -o $(TEST_CIRCADIAN) $(LDFLAGS)
	@echo "✅ $(TEST_CIRCADIAN) created"

$(TEST_WATCHDOG): $(WATCHDOG_OBJS) $(TEST_WATCHDOG_SRC)
	@echo "🔨 Building $(TEST_WATCHDOG)..."
	$(CC) $(CFLAGS) $(TEST_WATCHDOG_SRC) $(WATCHDOG_OBJS) -o $(TEST_WATCHDOG) $(LDFLAGS)
	@echo "✅ $(TEST_WATCHDOG) created"

# 오브젝트 파일 생성
%.o: %.c %.h brain_format.h
	@echo "🔨 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

hnsw.o: hnsw.c hnsw.h
	@echo "🔨 Compiling hnsw.c..."
	$(CC) $(CFLAGS) -c hnsw.c -o hnsw.o

kim_stomach.o: kim_stomach.c kim_stomach.h
	@echo "🔨 Compiling kim_stomach.c..."
	$(CC) $(CFLAGS) -c kim_stomach.c -o kim_stomach.o

kim_pancreas.o: kim_pancreas.c kim_pancreas.h kim_stomach.h
	@echo "🔨 Compiling kim_pancreas.c..."
	$(CC) $(CFLAGS) -c kim_pancreas.c -o kim_pancreas.o

kim_spine.o: kim_spine.c kim_spine.h
	@echo "🔨 Compiling kim_spine.c..."
	$(CC) $(CFLAGS) -c kim_spine.c -o kim_spine.o

kim_health.o: kim_health.c kim_health.h kim_stomach.h kim_pancreas.h kim_spine.h
	@echo "🔨 Compiling kim_health.c..."
	$(CC) $(CFLAGS) -c kim_health.c -o kim_health.o

kim_cortex.o: kim_cortex.c kim_cortex.h kim_spine.h
	@echo "🔨 Compiling kim_cortex.c..."
	$(CC) $(CFLAGS) -c kim_cortex.c -o kim_cortex.o

kim_circadian.o: kim_circadian.c kim_circadian.h
	@echo "🔨 Compiling kim_circadian.c..."
	$(CC) $(CFLAGS) -c kim_circadian.c -o kim_circadian.o

kim_watchdog.o: kim_watchdog.c kim_watchdog.h
	@echo "🔨 Compiling kim_watchdog.c..."
	$(CC) $(CFLAGS) -c kim_watchdog.c -o kim_watchdog.o

# 실행
run: $(TEST)
	@echo ""
	@echo "🚀 Running $(TEST)..."
	@echo ""
	./$(TEST)

run-hnsw: $(TEST_HNSW)
	@echo ""
	@echo "🚀 Running $(TEST_HNSW)..."
	@echo ""
	./$(TEST_HNSW)

run-digestion: $(TEST_DIGEST)
	@echo ""
	@echo "🚀 Running $(TEST_DIGEST)..."
	@echo ""
	./$(TEST_DIGEST)

run-spine: $(TEST_SPINE)
	@echo ""
	@echo "🚀 Running $(TEST_SPINE)..."
	@echo ""
	./$(TEST_SPINE)

run-health: $(TEST_HEALTH)
	@echo ""
	@echo "🚀 Running $(TEST_HEALTH)..."
	@echo ""
	./$(TEST_HEALTH)

run-cortex: $(TEST_CORTEX)
	@echo ""
	@echo "🚀 Running $(TEST_CORTEX)..."
	@echo ""
	./$(TEST_CORTEX)

run-circadian: $(TEST_CIRCADIAN)
	@echo ""
	@echo "🚀 Running $(TEST_CIRCADIAN)..."
	@echo ""
	./$(TEST_CIRCADIAN)

run-watchdog: $(TEST_WATCHDOG)
	@echo ""
	@echo "🚀 Running $(TEST_WATCHDOG)..."
	@echo ""
	./$(TEST_WATCHDOG)

# 청소
clean:
	@echo "🧹 Cleaning..."
	rm -f $(OBJS) $(HNSW_OBJS) $(DIGEST_OBJS) $(SPINE_OBJS) $(HEALTH_OBJS) $(CORTEX_OBJS) $(CIRCADIAN_OBJS) $(WATCHDOG_OBJS) $(TEST) $(TEST_HNSW) $(TEST_DIGEST) $(TEST_SPINE) $(TEST_HEALTH) $(TEST_CORTEX) $(TEST_CIRCADIAN) $(TEST_WATCHDOG) test_brain.db
	@echo "✅ Clean complete"

# 헬프
help:
	@echo "Brain Core Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make                - Build all test programs"
	@echo "  make run            - Build and run test_brain"
	@echo "  make run-hnsw       - Build and run test_hnsw"
	@echo "  make run-digestion  - Build and run test_digestion"
	@echo "  make run-spine      - Build and run test_spine"
	@echo "  make run-health     - Build and run test_health"
	@echo "  make run-cortex     - Build and run test_cortex"
	@echo "  make run-circadian  - Build and run test_circadian"
	@echo "  make run-watchdog   - Build and run test_watchdog"
	@echo "  make clean          - Remove build artifacts"
	@echo "  make help           - Show this message"
	@echo ""
	@echo "Files:"
	@echo "  brain_format.h     - Binary format spec"
	@echo "  mmap_loader.c/h    - Memory-mapped file loader"
	@echo "  index_manager.c/h  - ID→Offset hash map"
	@echo "  hnsw.c/h           - HNSW vector search"
	@echo "  kim_stomach.c/h    - Ring Buffer (Stomach)"
	@echo "  kim_pancreas.c/h   - Data Parser (Pancreas)"
	@echo "  kim_spine.c/h      - Control Bus (Spinal Cord)"
	@echo "  kim_health.c/h     - Health Monitor (CNS)"
	@echo "  kim_cortex.c/h     - Cerebral Cortex (Thinking)"
	@echo "  kim_circadian.c/h  - 24/7 Operation (Circadian)"
	@echo "  kim_watchdog.c/h   - Self-Healing (Watchdog)"
	@echo "  test_brain.c       - Brain Core test"
	@echo "  test_hnsw.c        - HNSW search test"
	@echo "  test_digestion.c   - Digestion system test"
	@echo "  test_spine.c       - Spinal Cord test"
	@echo "  test_health.c      - Health Monitor test"
	@echo "  test_cortex.c      - Cortex thinking test"
	@echo "  test_circadian.c   - Circadian 24/7 test"
	@echo "  test_watchdog.c    - Watchdog self-healing test"

.PHONY: all run run-hnsw run-digestion run-spine run-health run-cortex run-circadian run-watchdog clean help

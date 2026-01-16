# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# Brain Core Makefile
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -O2 -g
LDFLAGS  =

# 소스 파일
SRCS     = mmap_loader.c index_manager.c
OBJS     = $(SRCS:.c=.o)

# 테스트 프로그램
TEST     = test_brain
TEST_SRC = test_brain.c

# 기본 타겟
all: $(TEST)

# 테스트 프로그램 빌드
$(TEST): $(OBJS) $(TEST_SRC)
	@echo "🔨 Building $(TEST)..."
	$(CC) $(CFLAGS) $(TEST_SRC) $(OBJS) -o $(TEST) $(LDFLAGS)
	@echo "✅ $(TEST) created"

# 오브젝트 파일 생성
%.o: %.c %.h brain_format.h
	@echo "🔨 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# 실행
run: $(TEST)
	@echo ""
	@echo "🚀 Running $(TEST)..."
	@echo ""
	./$(TEST)

# 청소
clean:
	@echo "🧹 Cleaning..."
	rm -f $(OBJS) $(TEST) test_brain.db
	@echo "✅ Clean complete"

# 헬프
help:
	@echo "Brain Core Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build test program"
	@echo "  make run      - Build and run test"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this message"
	@echo ""
	@echo "Files:"
	@echo "  brain_format.h     - Binary format spec"
	@echo "  mmap_loader.c/h    - Memory-mapped file loader"
	@echo "  index_manager.c/h  - ID→Offset hash map"
	@echo "  test_brain.c       - Integration test"

.PHONY: all run clean help

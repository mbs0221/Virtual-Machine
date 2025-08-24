# Virtual-Machine Linux Makefile
# 编译器和标志
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = 

# 目录
SRCDIR = .
BUILDDIR = build
BINDIR = bin

# 子项目
SUBDIRS = Parser Asm CPU

# 默认目标
all: $(SUBDIRS)

# 创建必要的目录
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# 编译子项目
Parser: $(BUILDDIR) $(BINDIR)
	$(MAKE) -C Parser

Asm: $(BUILDDIR) $(BINDIR)
	$(MAKE) -C Asm

CPU: $(BUILDDIR) $(BINDIR)
	$(MAKE) -C CPU

# 清理
clean:
	@echo "清理构建文件..."
	rm -rf $(BUILDDIR) $(BINDIR)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

# 安装
install: all
	@echo "安装到系统目录..."
	sudo cp $(BINDIR)/* /usr/local/bin/

# 运行测试
test: all
	@echo "运行测试..."
	@echo "1. 运行Parser..."
	./$(BINDIR)/parser
	@echo "2. 运行Asm..."
	./$(BINDIR)/asm
	@echo "3. 运行CPU..."
	./$(BINDIR)/cpu

# 帮助
help:
	@echo "可用的目标:"
	@echo "  all     - 编译所有项目"
	@echo "  clean   - 清理构建文件"
	@echo "  install - 安装到系统"
	@echo "  test    - 运行测试"
	@echo "  help    - 显示此帮助"

.PHONY: all clean install test help $(SUBDIRS)

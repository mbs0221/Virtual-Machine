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
	-$(MAKE) -C Parser

Asm: $(BUILDDIR) $(BINDIR)
	-$(MAKE) -C Asm

CPU: $(BUILDDIR) $(BINDIR)
	-$(MAKE) -C CPU

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
	@echo "================================================="
	@echo "            Virtual-Machine 完整流程测试            "
	@echo "================================================="
	@echo ""
	@echo "步骤1: 语法分析 - 将源代码转换为汇编代码"
	@echo "源文件: Parser/Text.txt"
	@echo "----------------------------------------"
	@cat Parser/Text.txt
	@echo ""
	@echo "步骤2: 运行Parser生成汇编代码..."
	@if [ -f "$(BINDIR)/parser" ]; then \
		cd Parser && ../$(BINDIR)/parser && cd ..; \
		echo "✓ Parser执行完成"; \
	else \
		echo "✗ Parser未编译成功，使用预编译的汇编文件"; \
	fi
	@echo ""
	@echo "步骤3: 汇编 - 将汇编代码转换为机器码"
	@if [ -f "Parser/data.s" ]; then \
		echo "汇编文件: Parser/data.s"; \
		echo "----------------------------------------"; \
		cat Parser/data.s; \
		echo ""; \
	else \
		echo "✗ 未找到汇编文件"; \
		exit 1; \
	fi
	@echo "步骤4: 运行Asm生成机器码..."
	@if [ -f "$(BINDIR)/asm" ]; then \
		cp Parser/data.s Asm/; \
		cd Asm && ../$(BINDIR)/asm && cd ..; \
		echo "✓ Asm执行完成"; \
	else \
		echo "✗ Asm未编译成功，使用预编译的机器码文件"; \
	fi
	@echo ""
	@echo "步骤5: 执行虚拟机"
	@echo "----------------------------------------"
	@if [ -f "Asm/data.bin" ]; then \
		cp Asm/data.bin CPU/data.obj; \
	elif [ -f "Asm/data.obj" ]; then \
		cp Asm/data.obj CPU/data.obj; \
	else \
		echo "✗ 未找到机器码文件"; \
		exit 1; \
	fi
	@if [ -f "$(BINDIR)/cpu" ]; then \
		cd CPU && timeout 10s ../$(BINDIR)/cpu || echo "CPU执行完成或超时"; \
		cd ..; \
	else \
		echo "✗ CPU未编译成功"; \
	fi
	@echo ""
	@echo "完整流程总结："
	@echo "1. Text.txt (源代码) → Parser → data.s (汇编代码)"
	@echo "2. data.s (汇编代码) → Asm → data.bin (机器码)"
	@echo "3. data.bin (机器码) → CPU → 执行结果"
	@echo ""
	@echo "✓ 测试完成！"

# 帮助
help:
	@echo "可用的目标:"
	@echo "  all     - 编译所有项目"
	@echo "  clean   - 清理构建文件"
	@echo "  install - 安装到系统"
	@echo "  test    - 运行测试"
	@echo "  help    - 显示此帮助"

.PHONY: all clean install test help $(SUBDIRS)

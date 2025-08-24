# CPU Makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = 

# 源文件
SOURCES = main.cpp vm.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = cpu

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# 编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJECTS) $(TARGET)

# 安装
install: $(TARGET)
	cp $(TARGET) ../bin/

# 运行
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean install run

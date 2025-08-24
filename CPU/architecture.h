#ifndef __ARCHITECTURE_H_
#define __ARCHITECTURE_H_

#include <string>

// 抽象架构基类
class Architecture {
public:
    virtual ~Architecture() = default;
    
    // 基本接口
    virtual void reset() = 0;
    virtual void load_program(const char* filename) = 0;
    virtual void execute() = 0;
    virtual void dump_registers() = 0;
    virtual void dump_memory(uint32_t start, uint32_t end) = 0;
    
    // 架构信息
    virtual std::string get_name() const = 0;
    virtual std::string get_description() const = 0;
    virtual uint32_t get_word_size() const = 0;
    virtual uint32_t get_register_count() const = 0;
    virtual uint32_t get_memory_size() const = 0;
    
    // 状态查询
    virtual bool is_running() const = 0;
    virtual uint32_t get_pc() const = 0;
    virtual uint32_t get_instruction_count() const = 0;
};

// 架构工厂类
class ArchitectureFactory {
public:
    enum ArchitectureType {
        TOY,
        RV32
    };
    
    static Architecture* create_architecture(ArchitectureType type);
    static void destroy_architecture(Architecture* arch);
    
    static std::string get_architecture_name(ArchitectureType type);
    static std::string get_architecture_description(ArchitectureType type);
};

#endif

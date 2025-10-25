#include "toy.h"
#include <fstream>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include "Common/Logger.h"

ToyCPU::ToyCPU() {
    // 初始化日志系统
    bool init_result = Common::Logger::getInstance().initializeDefault("DEBUG", "Logs/cpu.log");
    LOG_INFO("CPU.Core", "ToyCPU constructor called, logger init: " + std::to_string(init_result));
    LOG_DEBUG("CPU.Core", "DEBUG logging is enabled");
    
    // 初始化MMIO设备数组
    for (int i = 0; i < 16; i++) {
        mmio_devices[i] = nullptr;
    }
    
    // 初始化中断向量表
    for (int i = 0; i < TOY_INTERRUPT_VECTORS; i++) {
        interrupt_vectors[i] = 0;
    }
    
    // 初始化异常向量表
    for (int i = 0; i < EXCEPTION_MAX; i++) {
        exception_vectors[i] = 0;
    }
    
    // 注册默认设备到Memory抽象
    memory.register_mmio_device(0, new ConsoleDevice());
    memory.register_mmio_device(1, new TimerDevice());
    memory.register_mmio_device(2, new DisplayDevice());
    memory.register_mmio_device(3, new StorageDevice());
    memory.register_mmio_device(4, new AudioDevice());
    
    // 同时保持向后兼容性
    mmio_devices[0] = memory.get_mmio_device(0);
    mmio_devices[1] = memory.get_mmio_device(1);
    mmio_devices[2] = memory.get_mmio_device(2);
    mmio_devices[3] = memory.get_mmio_device(3);
    mmio_devices[4] = memory.get_mmio_device(4);
    
    // 初始化NES优化器
    nes_optimizer = new NESOptimizer(this);
    
    LOG_INFO("CPU.MMIO", "MMIO devices registered: Console(0), Timer(1), Display(2), Storage(3), Audio(4)");
    
    // 使用顺序执行模式
    LOG_INFO("CPU", "使用顺序执行模式");
    LOG_INFO("CPU.Core", "Pipeline disabled, using sequential execution");
    
    reset();
}

void ToyCPU::reset() {
    LOG_INFO("CPU.Core", "CPU reset initiated");
    
    // 重置寄存器文件
    registers.reset();
    
    // 重置内存系统
    memory.reset();
    
    // 初始化特殊寄存器
    registers.set_stack_pointer(TOY_MEM_SIZE - 4);
    registers.set_instruction_pointer(0);
    
    // 设置指令指针到内存抽象
    memory.set_instruction_pointer(registers.get_instruction_pointer());
    
    // 重置ALU
    alu.reset();
    
    // 重置完成
    
    // 初始化中断状态
    interrupts_enabled = false;
    in_interrupt = false;
    saved_ip = saved_flags = 0;
    
    // 初始化异常和特权状态
    current_privilege_level = PRIVILEGE_KERNEL;  // 启动时在内核模式
    pending_exception = EXCEPTION_NONE;
    exception_enabled = true;
    saved_privilege_level = PRIVILEGE_KERNEL;
    
    // 初始化MMU和TLB
    tlb = new TLB();
    mmu = new MMU(&memory, tlb);
    
    // 初始化缓存管理器
    cache_manager = new CacheManager(&memory);
    
    // 初始化状态
    running = true;
    instruction_count = 0;
}

ToyCPU::~ToyCPU() {
    // 清理MMIO设备
    for (int i = 0; i < 16; i++) {
        if (mmio_devices[i]) {
            delete mmio_devices[i];
            mmio_devices[i] = nullptr;
        }
    }
    
    // 清理NES优化器
    if (nes_optimizer) {
        delete nes_optimizer;
        nes_optimizer = nullptr;
    }
    
    // 清理MMU和TLB
    if (mmu) {
        delete mmu;
        mmu = nullptr;
    }
    if (tlb) {
        delete tlb;
        tlb = nullptr;
    }
    
    // 清理缓存管理器
    if (cache_manager) {
        delete cache_manager;
        cache_manager = nullptr;
    }
}


void ToyCPU::load_program(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("CPU.Memory", "无法打开文件: " + std::string(filename));
        return;
    }
    
    // 读取程序头信息 (16字节文件头)
    WORD DS, CS;
    WORD reserved1, reserved2; // 保留字段
    file.read(reinterpret_cast<char*>(&DS), sizeof(WORD));
    file.read(reinterpret_cast<char*>(&CS), sizeof(WORD));
    file.read(reinterpret_cast<char*>(&dataSize), sizeof(WORD));
    file.read(reinterpret_cast<char*>(&codeSize), sizeof(WORD));
    file.read(reinterpret_cast<char*>(&reserved1), sizeof(WORD));
    file.read(reinterpret_cast<char*>(&reserved2), sizeof(WORD));
    
    LOG_INFO("CPU.Memory", "=== Loading Toy Binary File ===");
    LOG_INFO("CPU.Memory", "File header: DS=" + std::to_string(DS) + ", CS=" + std::to_string(CS) + ", dataSize=" + std::to_string(dataSize) + ", codeSize=" + std::to_string(codeSize));
    LOG_INFO("CPU.Memory", "Reserved fields: " + std::to_string(reserved1) + ", " + std::to_string(reserved2));
    
    // 计算总内存大小
    WORD totalSize = CS + codeSize;
    LOG_DEBUG("CPU.Memory", "Total memory size: " + std::to_string(totalSize));
    
    // 跳过文件头，读取数据段到内存
    file.seekg(16 + DS, std::ios::beg);  // 跳过16字节文件头
    for (WORD i = 0; i < dataSize; i++) {
        if (DS + i < TOY_MEM_SIZE) {  // 确保不超出内存边界
            BYTE data_byte;
            file.read(reinterpret_cast<char*>(&data_byte), sizeof(BYTE));
            memory.write_byte(DS + i, data_byte);
        }
    }
    
    // 读取代码段到内存
    file.seekg(16 + CS, std::ios::beg);  // 跳过16字节文件头
    for (WORD i = 0; i < codeSize; i++) {
        if (CS + i < TOY_MEM_SIZE) {  // 确保不超出内存边界
            BYTE code_byte;
            file.read(reinterpret_cast<char*>(&code_byte), sizeof(BYTE));
            memory.write_byte(CS + i, code_byte);
        }
    }
    
    file.close();
    
    // 设置段寄存器和指令指针
    registers.set_data_segment(DS);
    registers.set_code_segment(CS);
    registers.set_instruction_pointer(CS);  // 程序代码从CS地址开始（内存地址）
    memory.set_instruction_pointer(CS);
    
    LOG_INFO("CPU.Memory", "Toy程序已加载");
    LOG_INFO("CPU.Memory", "DS: " + std::to_string(DS) + ", CS: " + std::to_string(CS) + ", 数据段大小: " + std::to_string(dataSize) + ", 代码段大小: " + std::to_string(codeSize));
}

void ToyCPU::execute() {
    LOG_INFO("CPU.Core", "开始执行Toy程序...");
    
    LOG_INFO("CPU.Core", "使用顺序执行模式");
    execute_sequential_mode();
}

void ToyCPU::execute_sequential_mode() {
    LOG_INFO("CPU.Core", "开始顺序执行模式");
    
    while (registers.get_instruction_pointer() < TOY_MEM_SIZE - 4 && running) {
        // 检查异常（在指令执行前）
        if (exception_enabled && pending_exception != EXCEPTION_NONE) {
            handle_exception(pending_exception);
            pending_exception = EXCEPTION_NONE;
        }
        
        // 检查中断（在指令执行前）
        if (interrupts_enabled && !in_interrupt) {
            check_interrupts();
        }
        
        // 执行单条指令
        execute_instruction();
        instruction_count++;
        
        // 设备时钟更新（每个指令周期）
        update_devices();
        
        // 检查异常（在指令执行后）
        if (exception_enabled && pending_exception != EXCEPTION_NONE) {
            handle_exception(pending_exception);
            pending_exception = EXCEPTION_NONE;
        }
        
        // 检查中断（在指令执行后）
        if (interrupts_enabled && !in_interrupt) {
            check_interrupts();
        }
    }
    
    LOG_INFO("CPU.Core", "程序执行完成，共执行 " + std::to_string(instruction_count) + " 条指令");
}

void ToyCPU::update_devices() {
    // 定时器设备时钟更新
    TimerDevice* timer = get_timer_device();
    if (timer) {
        timer->tick();
    }
    
    // 检查控制台输入
    ConsoleDevice* console = get_console_device();
    if (console) {
        console->check_input();
    }
    
    // 其他设备可以在这里添加更新逻辑
    // 例如：显示设备刷新、音频设备处理等
}

void ToyCPU::execute_instruction() {
    // 读取指令操作码
    BYTE OP = ReadB();
    
    // 提取基础操作码（去掉寻址模式位）
    BYTE base_op = OP & 0x3F;  // 保留低6位，清除寻址模式位
    
    // 输出指令信息（使用基础操作码）
    print_instruction_info(base_op);
    
    // 检查停机指令
    if (base_op == HALT) {
        LOG_INFO("CPU.Core", "程序结束");
        running = false;
        return;
    }
    
    // 根据指令类型分发执行
    switch (base_op) {
        // 算术运算指令 (100-109)
        case ADD: case SUB: case MUL: case DIV: case MOD: case CMP: case NEG:
        case '+': case '-': case '*': case '/': case '%': case '<': case '>': case '=':
            execute_arithmetic();
            break;
            
        // 位运算指令 (110-119)
        case AND: case OR: case XOR: case NOT:
        case SHL: case SHR: case SAL: case SAR: case SRL: case SRR:
            execute_bitwise();
            break;
            
        // 数据传输指令 (120-129)
        case MOV: case IN: case OUT:
        case LOAD: case STORE: case LEA:
        case PUSH: case POP:
            execute_memory();
            break;
            
        // 跳转指令 (130-139)
        case JMP: case JNE: case JG: case JE: case JB: case JGE: case JBE:
        case CALL: case RET:
            execute_jump();
            break;
            
        // 数学运算指令 (140-149)
        case INC: case DEC: case ABS:
            execute_math();
            break;
            
        // 条件设置指令 (150-159)
        case SETZ: case SETNZ: case SETG: case SETL: case SETGE: case SETLE:
            execute_conditional();
            break;
            
        // 字符串操作指令 (160-169)
        case STRLEN: case STRCPY: case STRCMP: case STRCHR:
            execute_string();
            break;
            
            
        // 中断和系统调用指令 (170-179)
        case INT_INST: case IRET: case CLI_INST: case STI_INST:
        case SYSCALL: case HLT:
            if (base_op >= 170 && base_op <= 173) {
                execute_interrupt_instruction();
            } else {
                execute_privilege_instruction();
            }
            break;
            
        // 未知指令
        default:
            LOG_ERROR("CPU", "未知操作码: 0x" + std::to_string((int)OP) + " (base: 0x" + std::to_string((int)base_op) + ")");
            running = false;
            break;
    }
}

void ToyCPU::execute_arithmetic() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    
    // 读取三个寄存器操作数
    BYTE reg1 = ReadB();
    BYTE reg2 = ReadB();
    BYTE reg3 = ReadB();
    
    // 记录详细指令信息
    std::vector<WORD> operands = {reg1, reg2, reg3};
    print_detailed_instruction_info(OP & 0x3F, operands);
    
    // 设置ALU操作
    WORD operand_a = registers.read_word(reg1);
    WORD operand_b = registers.read_word(reg2);
    
    // 执行运算
    if (alu.execute(OP, operand_a, operand_b)) {
        // 将结果存储到目标寄存器
        registers.write_word(reg3, alu.get_result());
        LOG_DEBUG("CPU", "R" + std::to_string(reg3) + " = R" + std::to_string(reg1) + "(" + std::to_string(operand_a) + ") " + get_op_name(OP) + " R" + std::to_string(reg2) + "(" + std::to_string(operand_b) + ") = " + std::to_string(alu.get_result()));
    } else {
        LOG_ERROR("CPU", "ALU operation failed: " + alu.get_last_error());
    }
}

void ToyCPU::execute_shift() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    
    // 读取两个寄存器操作数：源寄存器和移位数量
    BYTE reg1 = ReadB();  // 源寄存器
    BYTE reg2 = ReadB();  // 移位数量寄存器
    BYTE reg3 = ReadB();  // 目标寄存器
    
    // 记录详细指令信息
    std::vector<WORD> operands = {reg1, reg2, reg3};
    print_detailed_instruction_info(OP & 0x3F, operands);
    
    WORD value = registers.read_word(reg1);
    BYTE shift_count = registers.read_word(reg2) & 0x0F;  // 限制移位数量在0-15之间
    
    WORD result = 0;
    const char* op_name = "";
    
    switch (OP) {
        case SHL:  // 逻辑左移
            result = alu.shift_left(value, shift_count);
            op_name = "SHL";
            break;
        case SHR:  // 逻辑右移
            result = alu.shift_right(value, shift_count);
            op_name = "SHR";
            break;
        case SAL:  // 算术左移（与SHL相同）
            result = alu.shift_left(value, shift_count);
            op_name = "SAL";
            break;
        case SAR:  // 算术右移（保持符号位）
            if (value & 0x8000) {  // 负数
                result = (value >> shift_count) | (0xFFFF << (16 - shift_count));
            } else {  // 正数
                result = value >> shift_count;
            }
            op_name = "SAR";
            break;
        case SRL:  // 循环左移
            result = alu.rotate_left(value, shift_count);
            op_name = "SRL";
            break;
        case SRR:  // 循环右移
            result = alu.rotate_right(value, shift_count);
            op_name = "SRR";
            break;
        default:
            LOG_ERROR("CPU", "Unknown shift instruction: 0x" + std::to_string(OP));
            return;
    }
    
    // 将结果存储到目标寄存器
    registers.write_word(reg3, result);
    
    LOG_DEBUG("CPU", "R" + std::to_string(reg3) + " = R" + std::to_string(reg1) + "(" + std::to_string(value) + ") " + op_name + " R" + std::to_string(reg2) + "(" + std::to_string(shift_count) + ") = " + std::to_string(result));
}

void ToyCPU::execute_jump() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    WORD addr = ReadW();
    
    // 记录详细指令信息
    std::vector<WORD> operands = {addr};
    print_detailed_instruction_info(OP & 0x3F, operands);
    
    bool should_jump = false;
    
    switch (OP) {
        case JE:
            should_jump = alu.get_equal_flag();
            break;
        case JNE:
            should_jump = !alu.get_equal_flag();
            break;
        case JG:
            should_jump = alu.get_greater_flag();
            break;
        case JB:
            should_jump = alu.get_negative_flag();
            break;
        case JGE:
            should_jump = alu.get_greater_flag() || alu.get_equal_flag();
            break;
        case JBE:
            should_jump = alu.get_negative_flag() || alu.get_equal_flag();
            break;
        case JMP:
            should_jump = true;
            break;
        default:
            LOG_ERROR("CPU", "Unimplemented branch instruction: 0x" + std::to_string(OP));
            return;
    }
    
    if (should_jump) {
        registers.set_instruction_pointer(addr);
        LOG_DEBUG("CPU", "Jump to address 0x" + std::to_string(addr));
    } else {
        LOG_DEBUG("CPU", "Condition not met, continue execution");
    }
}

void ToyCPU::execute_stack() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    
    switch (OP) {
        case PUSH:
            {
                BYTE reg = ReadB();
                // 记录详细指令信息
                std::vector<WORD> operands = {reg};
                print_detailed_instruction_info(OP & 0x3F, operands);
                WORD sp = registers.get_stack_pointer();
                LOG_DEBUG("CPU", "R" + std::to_string(reg) + "(" + std::to_string(registers.read_word(reg)) + ") pushed to stack [SP:" + std::to_string(sp) + "]");
                memory.write_byte(sp--, registers.read_word(reg));
                registers.set_stack_pointer(sp);
            }
            break;
        case POP:
            {
                BYTE reg = ReadB();
                // 记录详细指令信息
                std::vector<WORD> operands = {reg};
                print_detailed_instruction_info(OP & 0x3F, operands);
                WORD sp = registers.get_stack_pointer();
                LOG_DEBUG("CPU", "Popped from stack [SP:" + std::to_string(sp+1) + "] to R" + std::to_string(reg));
                registers.write_word(reg, memory.read_byte(++sp));
                registers.set_stack_pointer(sp);
            }
            break;
        default:
            LOG_ERROR("CPU", "Unknown stack instruction: 0x" + std::to_string(OP));
            break;
    }
}

void ToyCPU::execute_memory() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    
    // 读取寄存器和地址/立即数
    BYTE reg = ReadB();
    WORD operand = ReadW();
    
    // 记录详细指令信息
    std::vector<WORD> operands = {reg, operand};
    print_detailed_instruction_info(OP & 0x3F, operands);
    
    // 提取基础操作码（去掉寻址模式位）
    BYTE base_op = OP & 0x3F;  // 保留低6位，清除寻址模式位
    
    // 提取寻址模式
    BYTE addressing_mode = OP & 0xE0;  // 保留高3位作为寻址模式
    
    switch (base_op) {
        case LOAD:
            if (addressing_mode == MR_A) {
                // 立即数寻址 (MR_A = 0x00)
                registers.write_word(reg, operand);
                LOG_DEBUG("CPU", "LOAD: R" + std::to_string(reg) + " = immediate value(" + std::to_string(operand) + ") [" + get_addressing_mode_name(addressing_mode) + "]");
            } else {
                // 其他寻址模式（直接寻址或间接寻址）
                WORD address = resolve_address(addressing_mode, operand);
                registers.write_word(reg, ReadW(address));
                LOG_DEBUG("CPU", "LOAD: R" + std::to_string(reg) + " = memory[0x" + std::to_string(address) + "](" + std::to_string(ReadW(address)) + ") [" + get_addressing_mode_name(addressing_mode) + "]");
            }
            break;
        case STORE: {
            // STORE支持直接寻址和间接寻址
            WORD store_address = resolve_address(addressing_mode, operand);
            WriteW(store_address, registers.read_word(reg));
            LOG_DEBUG("CPU", "STORE: memory[0x" + std::to_string(store_address) + "] = R" + std::to_string(reg) + "(" + std::to_string(registers.read_word(reg)) + ") [" + get_addressing_mode_name(addressing_mode) + "]");
            break;
        }
        case LEA:
            if (addressing_mode == MR_A) {
                // 立即数寻址
                registers.write_word(reg, operand);
                LOG_DEBUG("CPU", "LEA: R" + std::to_string(reg) + " = immediate address 0x" + std::to_string(operand) + " [" + get_addressing_mode_name(addressing_mode) + "]");
            } else {
                // 其他寻址模式：计算有效地址但不访问内存
                WORD address = resolve_address(addressing_mode, operand);
                registers.write_word(reg, address);
                LOG_DEBUG("CPU", "LEA: R" + std::to_string(reg) + " = effective address 0x" + std::to_string(address) + " [" + get_addressing_mode_name(addressing_mode) + "]");
            }
            break;
        default:
            LOG_ERROR("CPU", "Unknown memory instruction: 0x" + std::to_string(OP) + " (base: 0x" + std::to_string(base_op) + ")");
            break;
    }
}

void ToyCPU::execute_mov() {
    // 读取两个寄存器
    BYTE reg1 = ReadB();
    BYTE reg2 = ReadB();
    
    // 记录详细指令信息
    std::vector<WORD> operands = {reg1, reg2};
    print_detailed_instruction_info(MOV, operands);
    
    // 将reg1的值移动到reg2
    registers.write_word(reg2, registers.read_word(reg1));
    LOG_DEBUG("CPU", "R" + std::to_string(reg2) + " = R" + std::to_string(reg1) + "(" + std::to_string(registers.read_word(reg1)) + ")");
}

void ToyCPU::execute_io() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    BYTE reg = ReadB();  // 寄存器
    BYTE port = ReadB(); // 端口号
    
    // 记录详细指令信息
    std::vector<WORD> operands = {reg, port};
    print_detailed_instruction_info(OP & 0x3F, operands);
    
    if (OP == IN) {
        // IN指令：从端口读取数据到寄存器
        if (port < 16 && mmio_devices[port]) {
            // 从MMIO设备读取数据
            WORD data = mmio_devices[port]->read(1);
            registers.write_word(reg, data);
            LOG_DEBUG("CPU", "IN: R" + std::to_string(reg) + " = port" + std::to_string(port) + "(" + std::to_string(data) + ")");
        } else {
            // 从I/O端口读取数据
            registers.write_word(reg, IN_PORT[port]);
            LOG_DEBUG("CPU", "IN: R" + std::to_string(reg) + " = I/O port" + std::to_string(port) + "(" + std::to_string(IN_PORT[port]) + ")");
        }
    } else if (OP == OUT) {
        // OUT指令：将寄存器数据写入端口
        WORD data = registers.read_word(reg);
        if (port < 16 && mmio_devices[port]) {
            // 写入MMIO设备
            mmio_devices[port]->write(1, data);
            LOG_DEBUG("CPU", "OUT: port" + std::to_string(port) + " = R" + std::to_string(reg) + "(" + std::to_string(data) + ")");
        } else {
            // 写入I/O端口
            OUT_PORT[port] = data;
            LOG_DEBUG("CPU", "OUT: I/O port" + std::to_string(port) + " = R" + std::to_string(reg) + "(" + std::to_string(data) + ")");
        }
    }
}

void ToyCPU::execute_function() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    
    switch (OP) {
        case CALL:
            {
                WORD addr = ReadW();
                // 记录详细指令信息
                std::vector<WORD> operands = {addr};
                print_detailed_instruction_info(OP & 0x3F, operands);
                // 将相对地址转换为绝对地址
                WORD abs_addr = registers.get_code_segment() + addr;
                LOG_DEBUG("CPU", "Function call to address 0x" + std::to_string(addr) + " (absolute address 0x" + std::to_string(abs_addr) + ")");
                // 将返回地址压栈
                WORD sp = registers.get_stack_pointer();
                WORD return_addr = registers.get_instruction_pointer() + 2;
                sp -= 2;
                memory.write_byte(sp, return_addr >> 8);
                memory.write_byte(sp + 1, return_addr & 0xFF);
                registers.set_stack_pointer(sp);
                LOG_DEBUG("CPU", "Return address 0x" + std::to_string(return_addr) + " pushed to stack [SP:" + std::to_string(sp) + "]");
                // 跳转到函数地址
                registers.set_instruction_pointer(abs_addr);
            }
            break;
        case RET:
            {
                // 记录详细指令信息（RET指令没有操作数）
                std::vector<WORD> operands = {};
                print_detailed_instruction_info(OP & 0x3F, operands);
                LOG_DEBUG("CPU", "Function return");
                // 从栈中弹出返回地址
                WORD sp = registers.get_stack_pointer();
                WORD return_addr = (memory.read_byte(sp) << 8) | memory.read_byte(sp + 1);
                sp += 2;
                registers.set_stack_pointer(sp);
                registers.set_instruction_pointer(return_addr);
                LOG_DEBUG("CPU", "Return address 0x" + std::to_string(return_addr) + " popped from stack [SP:" + std::to_string(sp-2) + "]");
            }
            break;
        default:
            LOG_ERROR("CPU", "Unknown function instruction: 0x" + std::to_string(OP));
            break;
    }
}

BYTE ToyCPU::ReadB() {
    WORD ip = registers.get_instruction_pointer();
    memory.set_instruction_pointer(ip);
    
    if (cache_manager && cache_manager->is_enabled()) {
        BYTE value;
        if (cache_manager->read_byte(ip, value)) {
            registers.set_instruction_pointer(ip + 1);
            return value;
        }
    }
    
    BYTE value = memory.read_next_byte();
    registers.set_instruction_pointer(memory.get_instruction_pointer());
    return value;
}

BYTE ToyCPU::ReadB(WORD addr) {
    if (cache_manager && cache_manager->is_enabled()) {
        BYTE value;
        if (cache_manager->read_byte(addr, value)) {
            return value;
        }
    }
    return memory.read_byte(addr);
}

WORD ToyCPU::ReadW() {
    WORD ip = registers.get_instruction_pointer();
    memory.set_instruction_pointer(ip);
    
    if (cache_manager && cache_manager->is_enabled()) {
        WORD value;
        if (cache_manager->read_word(ip, value)) {
            registers.set_instruction_pointer(ip + 2);
            return value;
        }
    }
    
    WORD value = memory.read_next_word();
    registers.set_instruction_pointer(memory.get_instruction_pointer());
    return value;
}

WORD ToyCPU::ReadW(WORD addr) {
    if (cache_manager && cache_manager->is_enabled()) {
        WORD value;
        if (cache_manager->read_word(addr, value)) {
            return value;
        }
    }
    return memory.read_word(addr);
}

void ToyCPU::WriteW(WORD addr, WORD data) {
    if (cache_manager && cache_manager->is_enabled()) {
        cache_manager->write_word(addr, data);
    } else {
        memory.write_word(addr, data);
    }
}

void ToyCPU::dump_registers() {
    LOG_INFO("CPU.Core", "=== Toy寄存器状态 ===");
    registers.print_register_dump();
}

void ToyCPU::dump_memory(uint32_t start, uint32_t end) {
    LOG_INFO("CPU.Memory", "=== Toy内存状态 (" + std::to_string(start) + "-" + std::to_string(end) + ") ===");
    std::string memory_dump = memory.dump_memory_hex(start, end);
    LOG_INFO("CPU.Memory", memory_dump);
}

const char* ToyCPU::get_op_name(BYTE op) {
    switch (op) {
        case ADD: return "+";
        case SUB: return "-";
        case MUL: return "*";
        case DIV: return "/";
        case MOD: return "%";
        case CMP: return "==";
        case NEG: return "NEG";
        case '+': return "+";
        case '-': return "-";
        case '*': return "*";
        case '/': return "/";
        case '%': return "%";
        case '<': return "<";
        case '>': return ">";
        case '=': return "==";
        default: return "UNK";
    }
}

const char* ToyCPU::get_instruction_name(BYTE op) {
    // 提取基础操作码（去掉寻址模式位）
    BYTE base_op = op & 0x3F;
    
    switch (base_op) {
        case HALT: return "HALT";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case MOD: return "MOD";
        case CMP: return "CMP";
        case NEG: return "NEG";
        case SHL: return "SHL";
        case SHR: return "SHR";
        case SAL: return "SAL";
        case SAR: return "SAR";
        case SRL: return "SRL";
        case SRR: return "SRR";
        case LOAD: return "LOAD";
        case STORE: return "STORE";
        case LEA: return "LEA";
        case PUSH: return "PUSH";
        case POP: return "POP";
        case CALL: return "CALL";
        case RET: return "RET";
        case INT_INST: return "INT";
        case IRET: return "IRET";
        case CLI_INST: return "CLI";
        case STI_INST: return "STI";
        case SYSCALL: return "SYSCALL";
        case HLT: return "HLT";
        case AND: return "AND";
        case OR: return "OR";
        case XOR: return "XOR";
        case NOT: return "NOT";
        case INC: return "INC";
        case DEC: return "DEC";
        case ABS: return "ABS";
        case SETZ: return "SETZ";
        case SETNZ: return "SETNZ";
        case SETG: return "SETG";
        case SETL: return "SETL";
        case SETGE: return "SETGE";
        case SETLE: return "SETLE";
        case STRLEN: return "STRLEN";
        case STRCPY: return "STRCPY";
        case STRCMP: return "STRCMP";
        case STRCHR: return "STRCHR";
        case JMP: return "JMP";
        case JE: return "JE";
        case JNE: return "JNE";
        case JG: return "JG";
        case JB: return "JB";
        case JGE: return "JGE";
        case JBE: return "JBE";
        case MOV: return "MOV";
        case IN: return "IN";
        case OUT: return "OUT";
        case '+': return "ADD";
        case '-': return "SUB";
        case '*': return "MUL";
        case '/': return "DIV";
        case '%': return "MOD";
        case '<': return "CMP";
        case '>': return "CMP";
        case '=': return "CMP";
        default: return "UNKNOWN";
    }
}

void ToyCPU::print_instruction_info(BYTE op, const char* details) {
    std::string inst_info = "[PC:" + std::to_string(registers.get_instruction_pointer()-1) + "] " + std::string(get_instruction_name(op)) + " (0x" + std::to_string(op) + ")";
    if (details && strlen(details) > 0) {
        inst_info += " " + std::string(details);
    }
    LOG_DEBUG("CPU.Core", inst_info);
}

void ToyCPU::print_detailed_instruction_info(BYTE op, const std::vector<WORD>& operands) {
    // 格式化指令信息，类似 [0006][0004][0000]load  $10 $01 $0048
    std::string inst_info = "[" + std::to_string(registers.get_instruction_pointer()-1) + "]";
    
    // 添加操作数信息
    for (size_t i = 0; i < operands.size(); i++) {
        inst_info += "[" + std::to_string(operands[i]) + "]";
    }
    
    // 添加指令名称
    inst_info += std::string(get_instruction_name(op));
    
    // 添加操作数详情
    for (size_t i = 0; i < operands.size(); i++) {
        inst_info += " $" + std::to_string(operands[i]);
    }
    
    LOG_DEBUG("CPU.Core", inst_info);
}

// MMIO和中断管理方法实现
void ToyCPU::register_mmio_device(int device_id, MMIODevice* device) {
    if (device_id >= 0 && device_id < 16) {
        mmio_devices[device_id] = device;
    }
}

void ToyCPU::set_interrupt_vector(BYTE vector, WORD handler_address) {
    if (vector < TOY_INTERRUPT_VECTORS) {
        interrupt_vectors[vector] = handler_address;
    }
}

ConsoleDevice* ToyCPU::get_console_device() const {
    return dynamic_cast<ConsoleDevice*>(mmio_devices[0]);
}

TimerDevice* ToyCPU::get_timer_device() const {
    return dynamic_cast<TimerDevice*>(mmio_devices[1]);
}

WORD ToyCPU::read_mmio(WORD address) {
    WORD offset = address - TOY_MMIO_BASE;
    int device_id = offset / 16;  // 每个设备占用16字节
    WORD device_offset = offset % 16;
    
    if (device_id >= 0 && device_id < 16 && mmio_devices[device_id]) {
        return mmio_devices[device_id]->read(device_offset);
    }
    
    return 0;
}

void ToyCPU::write_mmio(WORD address, WORD value) {
    WORD offset = address - TOY_MMIO_BASE;
    int device_id = offset / 16;  // 每个设备占用16字节
    WORD device_offset = offset % 16;
    
    LOG_DEBUG("CPU", "[调试] MMIO写入: 地址=0x" + std::to_string(address) + ", 值=" + std::to_string(value) + ", 设备ID=" + std::to_string(device_id) + ", 偏移=" + std::to_string(device_offset));
    
    if (device_id >= 0 && device_id < 16 && mmio_devices[device_id]) {
        mmio_devices[device_id]->write(device_offset, value);
    }
}

void ToyCPU::check_interrupts() {
    for (int i = 0; i < 16; i++) {
        if (mmio_devices[i] && mmio_devices[i]->has_interrupt()) {
            BYTE vector = mmio_devices[i]->get_interrupt_vector();
            handle_interrupt(vector);
            break;  // 一次只处理一个中断
        }
    }
}

void ToyCPU::handle_interrupt(BYTE vector) {
    if (vector >= TOY_INTERRUPT_VECTORS || interrupt_vectors[vector] == 0) {
        return;  // 无效的中断向量
    }
    
    // 保存当前状态
    saved_ip = registers.get_instruction_pointer();
    saved_flags = alu.get_flags();
    in_interrupt = true;
    
    // 跳转到中断处理程序
    registers.set_instruction_pointer(interrupt_vectors[vector]);
    
    LOG_INFO("CPU", "[中断] 向量 " + std::to_string(vector) + "，跳转到 " + std::to_string(interrupt_vectors[vector]));
}

void ToyCPU::execute_interrupt_instruction() {
    // 使用已经读取的指令
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);  // 回退到当前指令
    BYTE base_op = OP & 0x3F;
    
    switch (base_op) {
        case 170: { // INT_INST
            // 软件中断
            BYTE vector = ReadB();
            // 记录详细指令信息
            std::vector<WORD> operands = {vector};
            print_detailed_instruction_info(base_op, operands);
            if (interrupts_enabled) {
                handle_interrupt(vector);
            }
            break;
        }
        case 171: { // IRET
            // 记录详细指令信息（IRET指令没有操作数）
            std::vector<WORD> operands = {};
            print_detailed_instruction_info(base_op, operands);
            // 中断返回
            registers.set_instruction_pointer(saved_ip);
            alu.set_flags(saved_flags);
            in_interrupt = false;
            LOG_INFO("CPU", "[中断返回] 恢复到 " + std::to_string(saved_ip));
            break;
        }
        case 172: { // CLI_INST
            // 记录详细指令信息（CLI指令没有操作数）
            std::vector<WORD> operands = {};
            print_detailed_instruction_info(base_op, operands);
            // 清除中断标志
            interrupts_enabled = false;
            LOG_INFO("CPU", "[CLI] 中断已禁用");
            break;
        }
        case 173: { // STI_INST
            // 记录详细指令信息（STI指令没有操作数）
            std::vector<WORD> operands = {};
            print_detailed_instruction_info(base_op, operands);
            // 设置中断标志
            interrupts_enabled = true;
            LOG_INFO("CPU", "[STI] 中断已启用");
            break;
        }
        default:
            LOG_ERROR("CPU", "[错误] 未知的中断指令: " + std::to_string(base_op));
            break;
    }
}

// 寻址模式解析函数
WORD ToyCPU::resolve_address(BYTE addressing_mode, WORD operand) {
    switch (addressing_mode) {
        case MR_A:
            // 立即数寻址：直接返回操作数
            return operand;
        case MR_B:
            // 直接寻址：操作数就是地址
            return operand;
        case MR_INDIRECT:
            // 寄存器间接寻址：操作数是寄存器号，返回寄存器中的值作为地址
            if (operand < TOY_REG_COUNT) {
                WORD address = registers.read_word(operand);
                LOG_DEBUG("CPU", "Indirect addressing: R" + std::to_string(operand) + " contains address 0x" + std::to_string(address));
                return address;
            } else {
                LOG_ERROR("CPU", "Invalid register number for indirect addressing: " + std::to_string(operand));
                return 0;
            }
        case MR_PC_REL:
            // PC相对寻址：PC + 偏移量
            {
                WORD pc = registers.get_instruction_pointer();
                WORD address = pc + operand;
                LOG_DEBUG("CPU", "PC-relative addressing: PC(0x" + std::to_string(pc) + ") + offset(" + std::to_string(operand) + ") = 0x" + std::to_string(address));
                return address;
            }
        default:
            LOG_ERROR("CPU", "Unknown addressing mode: 0x" + std::to_string(addressing_mode));
            return operand;
    }
}

const char* ToyCPU::get_addressing_mode_name(BYTE mode) {
    switch (mode) {
        case MR_A:
            return "Immediate";
        case MR_B:
            return "Direct";
        case MR_INDIRECT:
            return "Indirect";
        case MR_PC_REL:
            return "PC-Relative";
        default:
            return "Unknown";
    }
}

// 简化的执行模式 - 专注于NES游戏优化

// NES游戏优化方法实现
void ToyCPU::enable_nes_mode(bool enable) {
    if (enable) {
        nes_optimizer->initialize_nes_mode();
        LOG_INFO("CPU.NES", "NES模式已启用");
    } else {
        LOG_INFO("CPU.NES", "NES模式已禁用");
    }
}

bool ToyCPU::is_nes_mode_enabled() const {
    return nes_optimizer != nullptr;
}

void ToyCPU::optimize_for_nes_game() {
    if (nes_optimizer) {
        nes_optimizer->optimize_for_nes_game();
        LOG_INFO("CPU.NES", "NES游戏优化完成");
    }
}

void ToyCPU::print_nes_performance_report() {
    if (nes_optimizer) {
        nes_optimizer->print_performance_report();
    }
}

// 异常和特权处理实现

void ToyCPU::set_exception_vector(ExceptionType type, WORD handler_address) {
    if (type < EXCEPTION_MAX) {
        exception_vectors[type] = handler_address;
        LOG_INFO("CPU.Exception", "设置异常向量 " + std::to_string(type) + " -> 0x" + std::to_string(handler_address));
    }
}

void ToyCPU::set_privilege_level(BYTE level) {
    if (level <= PRIVILEGE_MAX) {
        current_privilege_level = level;
        // 更新标志寄存器中的特权位
        WORD flags = alu.get_flags();
        if (level == PRIVILEGE_USER) {
            flags |= BIT_PRIV;
        } else {
            flags &= ~BIT_PRIV;
        }
        alu.set_flags(flags);
        LOG_INFO("CPU.Privilege", "特权级别设置为: " + std::to_string(level));
    }
}

void ToyCPU::raise_exception(ExceptionType type) {
    if (type < EXCEPTION_MAX) {
        pending_exception = type;
        LOG_INFO("CPU.Exception", "异常触发: " + std::to_string(type));
    }
}

void ToyCPU::handle_exception(ExceptionType type) {
    if (type >= EXCEPTION_MAX || exception_vectors[type] == 0) {
        LOG_ERROR("CPU.Exception", "无效的异常向量: " + std::to_string(type));
        return;
    }
    
    // 保存当前状态
    saved_ip = registers.get_instruction_pointer();
    saved_flags = alu.get_flags();
    saved_privilege_level = current_privilege_level;
    
    // 切换到内核模式
    switch_to_kernel_mode();
    
    // 跳转到异常处理程序
    registers.set_instruction_pointer(exception_vectors[type]);
    
    LOG_INFO("CPU.Exception", "异常处理: 类型=" + std::to_string(type) + ", 跳转到=0x" + std::to_string(exception_vectors[type]));
}

void ToyCPU::execute_privilege_instruction() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    BYTE base_op = OP & 0x3F;
    
    switch (base_op) {
        case 174: { // SYSCALL
            BYTE syscall_number = ReadB();
            std::vector<WORD> operands = {syscall_number};
            print_detailed_instruction_info(base_op, operands);
            
            if (current_privilege_level == PRIVILEGE_USER) {
                // 用户模式下的系统调用
                execute_syscall(syscall_number);
            } else {
                LOG_ERROR("CPU.Privilege", "内核模式下不允许系统调用");
                raise_exception(EXCEPTION_PRIVILEGE_VIOLATION);
            }
            break;
        }
        case 175: { // HLT
            std::vector<WORD> operands = {};
            print_detailed_instruction_info(base_op, operands);
            
            if (current_privilege_level == PRIVILEGE_KERNEL) {
                LOG_INFO("CPU.Privilege", "系统停机");
                running = false;
            } else {
                LOG_ERROR("CPU.Privilege", "用户模式下不允许停机指令");
                raise_exception(EXCEPTION_PRIVILEGE_VIOLATION);
            }
            break;
        }
        default:
            LOG_ERROR("CPU.Privilege", "未知的特权指令: 0x" + std::to_string(base_op));
            raise_exception(EXCEPTION_INVALID_OPCODE);
            break;
    }
}

void ToyCPU::execute_syscall(BYTE syscall_number) {
    LOG_INFO("CPU.Syscall", "系统调用: " + std::to_string(syscall_number));
    
    switch (syscall_number) {
        case 0: // exit
            LOG_INFO("CPU.Syscall", "程序退出");
            running = false;
            break;
        case 1: // write
            {
                // 从寄存器获取参数: $0=文件描述符, $1=缓冲区地址, $2=长度
                WORD fd = registers.read_word(0);
                WORD buffer = registers.read_word(1);
                WORD length = registers.read_word(2);
                
                LOG_INFO("CPU.Syscall", "write(fd=" + std::to_string(fd) + ", buffer=0x" + std::to_string(buffer) + ", length=" + std::to_string(length) + ")");
                
                // 简单的控制台输出实现
                for (WORD i = 0; i < length; i++) {
                    BYTE ch = memory.read_byte(buffer + i);
                    if (fd == 1) { // stdout
                        printf("%c", ch);
                    }
                }
                fflush(stdout);
                break;
            }
        case 2: // read
            {
                // 从寄存器获取参数: $0=文件描述符, $1=缓冲区地址, $2=长度
                WORD fd = registers.read_word(0);
                WORD buffer = registers.read_word(1);
                WORD length = registers.read_word(2);
                
                LOG_INFO("CPU.Syscall", "read(fd=" + std::to_string(fd) + ", buffer=0x" + std::to_string(buffer) + ", length=" + std::to_string(length) + ")");
                
                // 简单的控制台输入实现
                if (fd == 0) { // stdin
                    char ch = getchar();
                    memory.write_byte(buffer, ch);
                    registers.write_word(0, 1); // 返回读取的字节数
                } else {
                    registers.write_word(0, 0); // 返回0表示没有读取到数据
                }
                break;
            }
        default:
            LOG_ERROR("CPU.Syscall", "未知的系统调用: " + std::to_string(syscall_number));
            raise_exception(EXCEPTION_INVALID_OPCODE);
            break;
    }
}

bool ToyCPU::check_privilege(BYTE required_level) const {
    return current_privilege_level <= required_level;
}

void ToyCPU::switch_to_kernel_mode() {
    current_privilege_level = PRIVILEGE_KERNEL;
    WORD flags = alu.get_flags();
    flags &= ~BIT_PRIV;
    alu.set_flags(flags);
    LOG_INFO("CPU.Privilege", "切换到内核模式");
}

void ToyCPU::switch_to_user_mode() {
    current_privilege_level = PRIVILEGE_USER;
    WORD flags = alu.get_flags();
    flags |= BIT_PRIV;
    alu.set_flags(flags);
    LOG_INFO("CPU.Privilege", "切换到用户模式");
}

// MMU和虚拟内存实现

WORD ToyCPU::translate_address(WORD virtual_addr, bool is_write, bool is_execute) {
    if (!mmu) {
        return virtual_addr;  // MMU未初始化时直接返回虚拟地址
    }
    
    return mmu->translate_address(virtual_addr, is_write, is_execute);
}

void ToyCPU::handle_page_fault(WORD virtual_addr, BYTE fault_flags) {
    if (mmu) {
        mmu->handle_page_fault(virtual_addr, fault_flags);
        raise_exception(EXCEPTION_PAGE_FAULT);
    }
}

void ToyCPU::invalidate_tlb() {
    if (tlb) {
        tlb->invalidate_all();
    }
}

void ToyCPU::invalidate_tlb_entry(WORD virtual_addr) {
    if (tlb) {
        tlb->invalidate_entry(virtual_addr);
    }
}

// 新指令执行实现

void ToyCPU::execute_bitwise() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    
    if (OP == NOT) {
        // 一元位运算：NOT
        BYTE reg = ReadB();
        std::vector<WORD> operands = {reg};
        print_detailed_instruction_info(OP, operands);
        
        WORD value = registers.read_word(reg);
        WORD result = ~value;
        registers.write_word(reg, result);
        
        LOG_DEBUG("CPU", "NOT: R" + std::to_string(reg) + " = ~" + std::to_string(value) + " = " + std::to_string(result));
    } else {
        // 二元位运算：AND, OR, XOR
        BYTE reg1 = ReadB();
        BYTE reg2 = ReadB();
        BYTE reg3 = ReadB();
        
        std::vector<WORD> operands = {reg1, reg2, reg3};
        print_detailed_instruction_info(OP, operands);
        
        WORD operand_a = registers.read_word(reg1);
        WORD operand_b = registers.read_word(reg2);
        
        if (alu.execute(OP, operand_a, operand_b)) {
            registers.write_word(reg3, alu.get_result());
            LOG_DEBUG("CPU", "R" + std::to_string(reg3) + " = R" + std::to_string(reg1) + "(" + std::to_string(operand_a) + ") " + get_op_name(OP) + " R" + std::to_string(reg2) + "(" + std::to_string(operand_b) + ") = " + std::to_string(alu.get_result()));
        }
    }
}

void ToyCPU::execute_math() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    BYTE reg = ReadB();
    
    std::vector<WORD> operands = {reg};
    print_detailed_instruction_info(OP, operands);
    
    WORD value = registers.read_word(reg);
    WORD result = 0;
    
    switch (OP) {
        case INC:
            result = value + 1;
            break;
        case DEC:
            result = value - 1;
            break;
        case ABS:
            result = (static_cast<int16_t>(value) < 0) ? -value : value;
            break;
    }
    
    registers.write_word(reg, result);
    LOG_DEBUG("CPU", std::string(get_instruction_name(OP)) + ": R" + std::to_string(reg) + " = " + std::to_string(value) + " -> " + std::to_string(result));
}

void ToyCPU::execute_conditional() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    BYTE reg = ReadB();
    
    std::vector<WORD> operands = {reg};
    print_detailed_instruction_info(OP, operands);
    
    WORD result = 0;
    
    switch (OP) {
        case SETZ:
            result = alu.get_zero_flag() ? 1 : 0;
            break;
        case SETNZ:
            result = alu.get_zero_flag() ? 0 : 1;
            break;
        case SETG:
            result = alu.get_greater_flag() ? 1 : 0;
            break;
        case SETL:
            result = alu.get_negative_flag() ? 1 : 0;
            break;
        case SETGE:
            result = (alu.get_greater_flag() || alu.get_equal_flag()) ? 1 : 0;
            break;
        case SETLE:
            result = (alu.get_negative_flag() || alu.get_equal_flag()) ? 1 : 0;
            break;
    }
    
    registers.write_word(reg, result);
    LOG_DEBUG("CPU", std::string(get_instruction_name(OP)) + ": R" + std::to_string(reg) + " = " + std::to_string(result));
}

void ToyCPU::execute_string() {
    BYTE OP = memory.read_byte(registers.get_instruction_pointer()-1);
    
    switch (OP) {
        case STRLEN: {
            BYTE reg1 = ReadB();  // 字符串地址寄存器
            BYTE reg2 = ReadB();  // 结果寄存器
            
            std::vector<WORD> operands = {reg1, reg2};
            print_detailed_instruction_info(OP, operands);
            
            WORD str_addr = registers.read_word(reg1);
            WORD length = 0;
            
            // 计算字符串长度（以null结尾）
            while (memory.read_byte(str_addr + length) != 0 && length < 1000) {
                length++;
            }
            
            registers.write_word(reg2, length);
            LOG_DEBUG("CPU", "STRLEN: 地址0x" + std::to_string(str_addr) + " 长度=" + std::to_string(length));
            break;
        }
        case STRCPY: {
            BYTE reg1 = ReadB();  // 源地址寄存器
            BYTE reg2 = ReadB();  // 目标地址寄存器
            
            std::vector<WORD> operands = {reg1, reg2};
            print_detailed_instruction_info(OP, operands);
            
            WORD src_addr = registers.read_word(reg1);
            WORD dst_addr = registers.read_word(reg2);
            WORD pos = 0;
            
            // 复制字符串
            do {
                BYTE ch = memory.read_byte(src_addr + pos);
                memory.write_byte(dst_addr + pos, ch);
                pos++;
            } while (memory.read_byte(src_addr + pos - 1) != 0 && pos < 1000);
            
            LOG_DEBUG("CPU", "STRCPY: 从0x" + std::to_string(src_addr) + " 到 0x" + std::to_string(dst_addr));
            break;
        }
        case STRCMP: {
            BYTE reg1 = ReadB();  // 字符串1地址寄存器
            BYTE reg2 = ReadB();  // 字符串2地址寄存器
            BYTE reg3 = ReadB();  // 结果寄存器
            
            std::vector<WORD> operands = {reg1, reg2, reg3};
            print_detailed_instruction_info(OP, operands);
            
            WORD str1_addr = registers.read_word(reg1);
            WORD str2_addr = registers.read_word(reg2);
            WORD pos = 0;
            WORD result = 0;
            
            // 比较字符串
            do {
                BYTE ch1 = memory.read_byte(str1_addr + pos);
                BYTE ch2 = memory.read_byte(str2_addr + pos);
                
                if (ch1 < ch2) {
                    result = -1;
                    break;
                } else if (ch1 > ch2) {
                    result = 1;
                    break;
                } else if (ch1 == 0) {
                    result = 0;
                    break;
                }
                pos++;
            } while (pos < 1000);
            
            registers.write_word(reg3, result);
            LOG_DEBUG("CPU", "STRCMP: 结果=" + std::to_string(result));
            break;
        }
        case STRCHR: {
            BYTE reg1 = ReadB();  // 字符串地址寄存器
            BYTE reg2 = ReadB();  // 字符寄存器
            BYTE reg3 = ReadB();  // 结果寄存器
            
            std::vector<WORD> operands = {reg1, reg2, reg3};
            print_detailed_instruction_info(OP, operands);
            
            WORD str_addr = registers.read_word(reg1);
            BYTE target_char = registers.read_word(reg2) & 0xFF;
            WORD pos = 0;
            WORD result = -1;  // 未找到
            
            // 查找字符
            while (pos < 1000) {
                BYTE ch = memory.read_byte(str_addr + pos);
                if (ch == 0) break;  // 字符串结束
                if (ch == target_char) {
                    result = pos;
                    break;
                }
                pos++;
            }
            
            registers.write_word(reg3, result);
            LOG_DEBUG("CPU", "STRCHR: 字符'" + std::string(1, target_char) + "' 位置=" + std::to_string(result));
            break;
        }
    }
}

#include "mmio.h"
#include "Common/Logger.h"
#include <cstdio>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// MMIO 设备实现

// ConsoleDevice 实现
ConsoleDevice::ConsoleDevice() : interrupt_pending(false), interrupt_vector(1), input_buffer(0), input_ready(false) {}

WORD ConsoleDevice::read(WORD offset) {
    switch (offset) {
        case 0: // 状态寄存器
            return (interrupt_pending ? 1 : 0) | (input_ready ? 2 : 0);
        case 1: // 数据寄存器
            if (input_ready) {
                input_ready = false;
                return input_buffer;
            }
            return 0;
        default:
            return 0;
    }
}

void ConsoleDevice::write(WORD offset, WORD value) {
    switch (offset) {
        case 0: // 状态寄存器
            if (value & 2) { // 清除中断
                interrupt_pending = false;
            }
            break;
        case 1: // 数据寄存器
            // 输出字符到控制台
            char output_char = (char)(value & 0xFF);
            printf("%c", output_char);
            fflush(stdout);
            // 只记录真正输出的字符，使用CPU.MMIO模块
            LOG_INFO("CPU.MMIO", "Console output: '" + std::string(1, output_char) + "'");
            break;
    }
}

bool ConsoleDevice::has_interrupt() const {
    return interrupt_pending;
}

BYTE ConsoleDevice::get_interrupt_vector() const {
    return interrupt_vector;
}

void ConsoleDevice::check_input() {
    // 检查是否有输入可用（非阻塞）
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    
    if (result > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
        char ch;
        if (::read(STDIN_FILENO, &ch, 1) > 0) {
            input_buffer = (WORD)ch;
            input_ready = true;
            interrupt_pending = true;
        }
    }
}

// TimerDevice 实现
TimerDevice::TimerDevice() : counter(0), reload_value(1000), interrupt_pending(false), interrupt_vector(2) {}

WORD TimerDevice::read(WORD offset) {
    switch (offset) {
        case 0: // 计数器值
            return counter;
        case 1: // 重载值
            return reload_value;
        case 2: // 状态寄存器
            return interrupt_pending ? 1 : 0;
        default:
            return 0;
    }
}

void TimerDevice::write(WORD offset, WORD value) {
    switch (offset) {
        case 0: // 计数器值
            counter = value;
            break;
        case 1: // 重载值
            reload_value = value;
            break;
        case 2: // 控制寄存器
            if (value & 1) { // 启动定时器
                counter = reload_value;
            }
            if (value & 2) { // 清除中断
                interrupt_pending = false;
            }
            break;
    }
}

bool TimerDevice::has_interrupt() const {
    return interrupt_pending;
}

BYTE TimerDevice::get_interrupt_vector() const {
    return interrupt_vector;
}

void TimerDevice::tick() {
    if (counter > 0) {
        counter--;
        if (counter == 0) {
            interrupt_pending = true;
            counter = reload_value; // 自动重载
        }
    }
}

// DisplayDevice 实现
DisplayDevice::DisplayDevice() : cursor_x(0), cursor_y(0), color_fg(7), color_bg(0), 
                                 interrupt_pending(false), interrupt_vector(3), screen_dirty(false) {
    clear_screen();
    // 初始化上次渲染的缓冲区
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            last_screen_buffer[y][x] = ' ';
        }
    }
}

WORD DisplayDevice::read(WORD offset) {
    switch (offset) {
        case 0: // 状态寄存器
            return interrupt_pending ? 1 : 0;
        case 1: // 光标X位置
            return cursor_x;
        case 2: // 光标Y位置
            return cursor_y;
        case 3: // 前景色
            return color_fg;
        case 4: // 背景色
            return color_bg;
        case 5: // 屏幕宽度
            return SCREEN_WIDTH;
        case 6: // 屏幕高度
            return SCREEN_HEIGHT;
        default:
            return 0;
    }
}

void DisplayDevice::write(WORD offset, WORD value) {
    switch (offset) {
        case 0: // 控制寄存器
            if (value & 1) { // 清除屏幕
                clear_screen();
            }
            if (value & 2) { // 清除中断
                interrupt_pending = false;
            }
            if (value & 4) { // 渲染屏幕
                render();
            }
            break;
        case 1: // 光标X位置
            cursor_x = value % SCREEN_WIDTH;
            break;
        case 2: // 光标Y位置
            cursor_y = value % SCREEN_HEIGHT;
            break;
        case 3: // 前景色
            color_fg = value & 0x0F;
            break;
        case 4: // 背景色
            color_bg = value & 0x0F;
            break;
        case 5: // 字符输出
            put_char((char)(value & 0xFF));
            break;
    }
}

bool DisplayDevice::has_interrupt() const {
    return interrupt_pending;
}

BYTE DisplayDevice::get_interrupt_vector() const {
    return interrupt_vector;
}

void DisplayDevice::clear_screen() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            screen_buffer[y][x] = ' ';
        }
    }
    cursor_x = cursor_y = 0;
    screen_dirty = true;  // 标记屏幕内容已改变
}

void DisplayDevice::set_cursor(WORD x, WORD y) {
    cursor_x = x % SCREEN_WIDTH;
    cursor_y = y % SCREEN_HEIGHT;
}

void DisplayDevice::put_char(char ch) {
    if (ch == '\n') {
        cursor_x = 0;
        cursor_y = (cursor_y + 1) % SCREEN_HEIGHT;
    } else if (ch == '\r') {
        cursor_x = 0;
    } else if (ch == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
        if (cursor_x >= SCREEN_WIDTH) {
            cursor_x = 0;
            cursor_y = (cursor_y + 1) % SCREEN_HEIGHT;
        }
    } else if (ch >= 32 && ch <= 126) {
        screen_buffer[cursor_y][cursor_x] = ch;
        screen_dirty = true;  // 标记屏幕内容已改变
        cursor_x++;
        if (cursor_x >= SCREEN_WIDTH) {
            cursor_x = 0;
            cursor_y = (cursor_y + 1) % SCREEN_HEIGHT;
        }
    }
}

void DisplayDevice::put_string(const char* str) {
    while (*str) {
        put_char(*str++);
    }
}

void DisplayDevice::render() {
    // 只在屏幕内容改变时才更新
    if (!screen_dirty) {
        return;
    }
    
    // 移动光标到左上角，不清屏
    printf("\033[H");
    
    // 渲染屏幕内容
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            printf("%c", screen_buffer[y][x]);
        }
        printf("\n");
    }
    
    // 确保光标回到起始位置
    printf("\033[H");
    fflush(stdout);
    
    // 更新上次渲染的缓冲区并清除脏标记
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            last_screen_buffer[y][x] = screen_buffer[y][x];
        }
    }
    screen_dirty = false;
    
    LOG_DEBUG("CPU.MMIO", "Display rendered to screen");
}

// StorageDevice 实现
StorageDevice::StorageDevice() : current_sector(0), sector_count(MAX_SECTORS), 
                                 interrupt_pending(false), interrupt_vector(4) {
    // 初始化磁盘数据为0
    for (int i = 0; i < MAX_SECTORS; i++) {
        for (int j = 0; j < SECTOR_SIZE; j++) {
            disk_data[i][j] = 0;
        }
    }
}

WORD StorageDevice::read(WORD offset) {
    switch (offset) {
        case 0: // 状态寄存器
            return interrupt_pending ? 1 : 0;
        case 1: // 当前扇区号
            return current_sector;
        case 2: // 扇区总数
            return sector_count;
        case 3: // 扇区大小
            return SECTOR_SIZE;
        default:
            if (offset >= 4 && offset < 4 + SECTOR_SIZE) {
                // 读取当前扇区数据
                return disk_data[current_sector][offset - 4];
            }
            return 0;
    }
}

void StorageDevice::write(WORD offset, WORD value) {
    switch (offset) {
        case 0: // 控制寄存器
            if (value & 1) { // 清除中断
                interrupt_pending = false;
            }
            break;
        case 1: // 设置当前扇区
            current_sector = value % sector_count;
            break;
        default:
            if (offset >= 4 && offset < 4 + SECTOR_SIZE) {
                // 写入当前扇区数据
                disk_data[current_sector][offset - 4] = value & 0xFF;
            }
            break;
    }
}

bool StorageDevice::has_interrupt() const {
    return interrupt_pending;
}

BYTE StorageDevice::get_interrupt_vector() const {
    return interrupt_vector;
}

void StorageDevice::read_sector(WORD sector, BYTE* buffer) {
    if (sector < sector_count) {
        for (int i = 0; i < SECTOR_SIZE; i++) {
            buffer[i] = disk_data[sector][i];
        }
    }
}

void StorageDevice::write_sector(WORD sector, const BYTE* buffer) {
    if (sector < sector_count) {
        for (int i = 0; i < SECTOR_SIZE; i++) {
            disk_data[sector][i] = buffer[i];
        }
    }
}

// AudioDevice 实现
AudioDevice::AudioDevice() : frequency(440), volume(50), duration(1000), 
                            playing(false), interrupt_pending(false), interrupt_vector(5) {}

WORD AudioDevice::read(WORD offset) {
    switch (offset) {
        case 0: // 状态寄存器
            return (playing ? 1 : 0) | (interrupt_pending ? 2 : 0);
        case 1: // 频率
            return frequency;
        case 2: // 音量
            return volume;
        case 3: // 持续时间
            return duration;
        default:
            return 0;
    }
}

void AudioDevice::write(WORD offset, WORD value) {
    switch (offset) {
        case 0: // 控制寄存器
            if (value & 1) { // 开始播放
                play_tone(frequency, volume, duration);
            }
            if (value & 2) { // 停止播放
                stop();
            }
            if (value & 4) { // 清除中断
                interrupt_pending = false;
            }
            break;
        case 1: // 设置频率
            frequency = value;
            break;
        case 2: // 设置音量
            volume = value % 101; // 0-100
            break;
        case 3: // 设置持续时间
            duration = value;
            break;
    }
}

bool AudioDevice::has_interrupt() const {
    return interrupt_pending;
}

BYTE AudioDevice::get_interrupt_vector() const {
    return interrupt_vector;
}

void AudioDevice::play_tone(WORD freq, WORD vol, WORD dur) {
    frequency = freq;
    volume = vol;
    duration = dur;
    playing = true;
    
    // 简单的音频输出（使用系统beep）
    printf("\a"); // 系统响铃
    fflush(stdout);
    LOG_DEBUG("CPU.MMIO", "Audio beep played");
    
    // 模拟播放完成后的中断
    interrupt_pending = true;
}

void AudioDevice::stop() {
    playing = false;
    interrupt_pending = false;
}

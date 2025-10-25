#ifndef MMIO_H
#define MMIO_H

#include <cstdint>

// 前向声明
typedef uint16_t WORD;
typedef uint8_t BYTE;

// MMIO 常量定义
#define TOY_MMIO_BASE 0xF000
#define TOY_MMIO_SIZE 0x1000
#define TOY_INTERRUPT_VECTORS 16

// 中断标志位
#define BIT_INT 0x0100
#define BIT_IF 0x0080

// MMIO 设备基类
class MMIODevice {
public:
    virtual ~MMIODevice() = default;
    virtual WORD read(WORD offset) = 0;
    virtual void write(WORD offset, WORD value) = 0;
    virtual bool has_interrupt() const = 0;
    virtual BYTE get_interrupt_vector() const = 0;
};

// 控制台设备
class ConsoleDevice : public MMIODevice {
private:
    bool interrupt_pending;
    BYTE interrupt_vector;
    WORD input_buffer;
    bool input_ready;
    
public:
    ConsoleDevice();
    WORD read(WORD offset) override;
    void write(WORD offset, WORD value) override;
    bool has_interrupt() const override;
    BYTE get_interrupt_vector() const override;
    void check_input();
};

// 定时器设备
class TimerDevice : public MMIODevice {
private:
    WORD counter;
    WORD reload_value;
    bool interrupt_pending;
    BYTE interrupt_vector;
    
public:
    TimerDevice();
    WORD read(WORD offset) override;
    void write(WORD offset, WORD value) override;
    bool has_interrupt() const override;
    BYTE get_interrupt_vector() const override;
    void tick();
};

// 显示设备
class DisplayDevice : public MMIODevice {
private:
    static const int SCREEN_WIDTH = 80;
    static const int SCREEN_HEIGHT = 25;
    char screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
    char last_screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];  // 上次渲染的内容
    WORD cursor_x, cursor_y;
    WORD color_fg, color_bg;
    bool interrupt_pending;
    BYTE interrupt_vector;
    bool screen_dirty;  // 屏幕内容是否已改变
    
public:
    DisplayDevice();
    WORD read(WORD offset) override;
    void write(WORD offset, WORD value) override;
    bool has_interrupt() const override;
    BYTE get_interrupt_vector() const override;
    void clear_screen();
    void set_cursor(WORD x, WORD y);
    void put_char(char ch);
    void put_string(const char* str);
    void render();
};

// 存储设备
class StorageDevice : public MMIODevice {
private:
    static const int SECTOR_SIZE = 512;
    static const int MAX_SECTORS = 1024;
    BYTE disk_data[MAX_SECTORS][SECTOR_SIZE];
    WORD current_sector;
    WORD sector_count;
    bool interrupt_pending;
    BYTE interrupt_vector;
    
public:
    StorageDevice();
    WORD read(WORD offset) override;
    void write(WORD offset, WORD value) override;
    bool has_interrupt() const override;
    BYTE get_interrupt_vector() const override;
    void read_sector(WORD sector, BYTE* buffer);
    void write_sector(WORD sector, const BYTE* buffer);
};

// 音频设备
class AudioDevice : public MMIODevice {
private:
    WORD frequency;
    WORD volume;
    WORD duration;
    bool playing;
    bool interrupt_pending;
    BYTE interrupt_vector;
    
public:
    AudioDevice();
    WORD read(WORD offset) override;
    void write(WORD offset, WORD value) override;
    bool has_interrupt() const override;
    BYTE get_interrupt_vector() const override;
    void play_tone(WORD freq, WORD vol, WORD dur);
    void stop();
};

#endif // MMIO_H
